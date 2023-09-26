#include "AppSignalSetProvider.h"

AppSignalSetProvider* AppSignalSetProvider::m_instance = nullptr;
QThread* AppSignalSetProvider::m_thread = nullptr;

AppSignalSetProvider::AppSignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	QObject(parentWidget),
	m_db(dbController),
	m_parentWidget(parentWidget),
	m_signalsLoadTimer(this)
{
	Q_ASSERT(m_instance == nullptr);
	m_instance = this;

	m_thread = QThread::currentThread();

	TEST_PTR_RETURN(dbController);

	connect(this, &AppSignalSetProvider::signalsUpdated,
			&m_propertyManager, &AppSignalPropertyManager::slot_detectNewProperties);

	connect(this, &AppSignalSetProvider::detectNewProperties,
			&m_propertyManager, &AppSignalPropertyManager::slot_detectNewProperties);

	connect(&m_signalsLoadTimer, &QTimer::timeout, this, &AppSignalSetProvider::onSignalsLoadTimer);
}

AppSignalSetProvider::~AppSignalSetProvider()
{
}

AppSignalSetProvider* AppSignalSetProvider::getInstance()
{
	Q_ASSERT(m_instance != nullptr);
	return m_instance;
}

DbController* AppSignalSetProvider::dbController()
{
	return m_db;
}

void AppSignalSetProvider::projectOpened()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	m_signalSet.clear();

	m_currentUserID = m_db->currentUser().userId();
	m_currentUserIsAdmin = m_db->currentUser().isAdminstrator();

	loadUsers();
	reloadPropertiesBehaviour();

	startSignalsLoading();
}

void AppSignalSetProvider::projectClosed()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	terminateSignalsLoading();

	m_currentUserID = -1;
	m_currentUserIsAdmin = false;

	m_signalSet.clear();
	m_users.clear();
	m_propertyManager.clear();

	emit signalsCountChanged();
}

bool AppSignalSetProvider::projectProperty_uppercaseAppSignalID() const
{
	bool uppercaseAppSignalID = false;

	bool result = m_db->getProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, &uppercaseAppSignalID, m_parentWidget);

	ASSERT_RETURN_IF_FALSE(result);

	return uppercaseAppSignalID;
}

int AppSignalSetProvider::currentUserID() const
{
	return m_currentUserID;
}

bool AppSignalSetProvider::currentUserIsAdmin() const
{
	return m_currentUserIsAdmin;
}

const AppSignalSet& AppSignalSetProvider::signalSet() const
{
	return m_signalSet;
}

AppSignalPropertyManager& AppSignalSetProvider::signalPropertyManager()
{
	return m_propertyManager;
}

void AppSignalSetProvider::reloadAllSignals()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	startSignalsLoading();
}

void AppSignalSetProvider::reloadSignals(const std::vector<int>& signalIds)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	if (signalIds.size() == 0)
	{
		return;
	}

	std::vector<AppSignal> updatedSignals;

	m_db->getLatestSignalsWithoutProgress(signalIds, &updatedSignals, nullptr);

	std::vector<int> signalsIndexes;

	for (const AppSignal& updatedSignal: updatedSignals)
	{
		auto [s, index] = m_signalSet.updateSignal(updatedSignal);

		if (index != BAD_INDEX)
		{
			signalsIndexes.push_back(index);
		}
	}

	emit signalsUpdated(signalsIndexes);
}

void AppSignalSetProvider::enforceAllSignalsLoading()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	if (m_signalsLoading == false)
	{
		return;
	}

	m_signalsLoadTimer.stop();

	std::vector<int> signalIds;

	signalIds.reserve(m_signalSet.size());

	for (const AppSignal* s : m_signalSet)
	{
		if (s->isLoaded() == false)
		{
			signalIds.push_back(s->ID());
		}
	}

	reloadSignals(signalIds);

	m_signalsLoading = false;
	m_signalsLoadTimer.stop();
}

const AppSignal* AppSignalSetProvider::loadSignal(int signalId, bool updateViews)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal loadedSignal;

	m_db->getLatestSignal(signalId, &loadedSignal, nullptr);

	auto [updatedSignal, index] = m_signalSet.updateSignal(loadedSignal);

	TEST_PTR_RETURN_NULLPTR(updatedSignal);

	if (updateViews == true && index != BAD_INDEX)
	{
		emit signalsUpdated({index});
	}

	return updatedSignal;
}

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}

QString AppSignalSetProvider::getUserName(int userId)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	auto it = m_users.find(userId);

	if (it != m_users.end())
	{
		return it->second;
	}

	loadUsers();		// try to reload users

	it = m_users.find(userId);

	if (it != m_users.end())
	{
		return it->second;
	}

	Q_ASSERT(false);

	return QString("Unknown user ID=%1").arg(userId);
}

AppSignal* AppSignalSetProvider::getSignal(const QString& appSignalID)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.getSignal(appSignalID);
}

AppSignal* AppSignalSetProvider::getSignalByID(int signalID)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.getSignal(signalID);
}

AppSignal* AppSignalSetProvider::getSignalByIndex(int index)
{
	return m_signalSet.at(index);
}

const AppSignal* AppSignalSetProvider::getSignalByIndex(int index) const
{
	return m_signalSet.at(index);
}

bool AppSignalSetProvider::getChannelSignalsID(const AppSignal& signal, std::vector<int>* channelSignalIDs) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.getChannelSignalsID(signal, channelSignalIDs);
}

bool AppSignalSetProvider::getChannelSignalsID(int signalID, int groupID, std::vector<int>* channelSignalIDs) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.getChannelSignalsID(signalID, groupID, channelSignalIDs);
}

int AppSignalSetProvider::signalIndex(int signalID) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.signalIndex(signalID);
}

int AppSignalSetProvider::signalID(int index) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	const AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_VALUE(s, -1);

	return s->ID();
}

AppSignal* AppSignalSetProvider::getLoadedSignal(AppSignal* s, bool updateViews)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	TEST_PTR_RETURN_NULLPTR(s);

	if (s->isLoaded() == false)
	{
		loadSignal(s->ID(), updateViews);
	}

	return s;
}

AppSignal* AppSignalSetProvider::getLoadedSignalByID(int signalID, bool updateViews)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal* s = m_signalSet.getSignal(signalID);

	return getLoadedSignal(s, updateViews);
}

AppSignal* AppSignalSetProvider::getLoadedSignalByIndex(int index, bool updateViews)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal* s = m_signalSet.at(index);

	return getLoadedSignal(s, updateViews);
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(int index)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal signal(*getLoadedSignalByIndex(index, false));

	signal.cacheSpecPropValues();

	AppSignalParam param;

	param.load(signal);

	return param;
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(const QString& appSignalId)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignalParam param;

	AppSignal* signal = getSignal(appSignalId);
	if (signal == nullptr)
	{
		assert(false);
		return param;
	}

	if (signal->isLoaded())
	{
		signal->cacheSpecPropValues();
		param.load(*signal);
		return param;
	}

	return getAppSignalParam(m_signalSet.signalIndex(signal->ID()));
}

QVector<int> AppSignalSetProvider::getSameChannelSignals(int index)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	QVector<int> sameChannelSignalRows;

	const AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_VALUE(s, sameChannelSignalRows);

	if (s->signalGroupID() != 0)
	{
		std::vector<int> sameChannelSignalIDs;

		m_signalSet.getChannelSignalsID(*s, &sameChannelSignalIDs);

		for(int id : sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(m_signalSet.signalIndex(id));
		}
	}
	else
	{
		sameChannelSignalRows.append(index);
	}

	return sameChannelSignalRows;
}

bool AppSignalSetProvider::isEditableSignal(int index) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return isEditableSignal(m_signalSet.at(index));
}

bool AppSignalSetProvider::isEditableSignal(const AppSignal* signal) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	TEST_PTR_RETURN_FALSE(signal);

	if (!signal->checkedOut() ||
		(signal->userID() == m_currentUserID || m_currentUserIsAdmin))
	{
		return true;
	}

	return false;
}

bool AppSignalSetProvider::isCheckinableSignalForMe(int index) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return isCheckinableSignalForMe(m_signalSet.at(index));
}

bool AppSignalSetProvider::isCheckinableSignalForMe(const AppSignal* signal) const
{
	Q_ASSERT(m_thread == QThread::currentThread());

	TEST_PTR_RETURN_FALSE(signal);

	if (signal->checkedOut() &&
		(signal->userID() == m_currentUserID || m_currentUserIsAdmin))
	{
		return true;
	}

	return false;
}

void AppSignalSetProvider::loadUsers()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<DbUser> users;

	m_db->getUserList(&users, nullptr);

	m_users.clear();

	for (auto const& user : users)
	{
		m_users.emplace(user.userId(), user.username());
	}
}

void AppSignalSetProvider::reloadPropertiesBehaviour()
{
	TEST_PTR_RETURN(m_db);

	int etcFileId = m_db->systemFileId(DbDir::EtcDir);

	DbFileInfo propBehaviourFileInfo;

	m_db->getFileInfo(etcFileId, QString(Db::File::SignalPropertyBehaviorFileName),
								&propBehaviourFileInfo, nullptr);

	if (propBehaviourFileInfo.isNull() == true)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("File \"%1\" is not found!").
										arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	std::shared_ptr<DbFile> file;

	bool result = m_db->getLatestVersion(propBehaviourFileInfo, &file, nullptr);

	if (result == false)
	{
		QMessageBox::critical(m_parentWidget, "Error", QString("Could not load file \"%1\"").
										arg(Db::File::SignalPropertyBehaviorFileName));
		return;
	}

	m_propertyManager.clear();

	QString errMsg;

	m_propertyManager.updatePropertiesBehaviour(file->data(), &errMsg);

	if (errMsg.isEmpty() == false)
	{
		QMessageBox::critical(m_parentWidget, "Error", errMsg);
	}
}

void AppSignalSetProvider::startSignalsLoading()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	if (m_signalsLoading == true)
	{
		terminateSignalsLoading();
	}

	loadIdAppSignalId();

	m_signalsLoadTimer.setInterval(100);
	m_signalsLoadTimer.start();

	m_signalsLoading = true;

	emit signalsCountChanged();
}

void AppSignalSetProvider::terminateSignalsLoading()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	m_signalsLoadTimer.stop();
	m_signalsLoading = false;
}

void AppSignalSetProvider::loadIdAppSignalId()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<ID_AppSignalID> ids;

	m_db->getSignalsIDAppSignalID(&ids, false, nullptr);

	m_signalSet.clear();
	m_signalSet.reserve(ids.size());

	for(const ID_AppSignalID& id : ids)
	{
		m_signalSet.append(id);
	}
}

void AppSignalSetProvider::appendSignalsAndUpdateViews(const std::vector<AppSignal>& newSignals)
{
	if (newSignals.size() == 0)
	{
		return;
	}

	std::vector<int> newIndexes;

	for(const AppSignal& newSignal : newSignals)
	{
		auto [ns, index] = m_signalSet.append(newSignal);

		newIndexes.push_back(index);
	}

	emit signalsUpdated(newIndexes);		// to check new signals properties
	emit signalsCountChanged();
}

void AppSignalSetProvider::onSignalsLoadTimer()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	if (m_signalsLoading == false)
	{
		Q_ASSERT(false);
		m_signalsLoadTimer.stop();
		return;
	}

	static const int MAX_SIGNALS_COUNT = 500;

	std::vector<int> signalIds;

	signalIds.reserve(MAX_SIGNALS_COUNT);

	int low = -1;
	int high = 0;

	if (m_middleVisibleSignalIndex >= 0)
	{
		low = m_middleVisibleSignalIndex - 1;
		high = m_middleVisibleSignalIndex;
	}
	else
	{
		high = 0;
	}

	int signalCount = m_signalSet.count();

	AppSignal* s = nullptr;

	while ((low >= 0 || high < signalCount) &&
		   signalIds.size() <= (MAX_SIGNALS_COUNT - 2))
	{
		while(low >= 0)
		{
			s = m_signalSet.at(low);

			low--;

			if (s != nullptr && s->isLoaded() == false)
			{
				signalIds.push_back(s->ID());
				break;
			}
		}

		while(high < signalCount)
		{
			s = m_signalSet.at(high);

			high++;

			if (s != nullptr && s->isLoaded() == false)
			{
				signalIds.push_back(s->ID());
				break;
			}
		}
	}

	if (signalIds.size() > 0)
	{
		reloadSignals(signalIds);
	}

	if (signalIds.size() == 0 ||
		(low < 0 && high >= signalCount))
	{
		m_signalsLoadTimer.stop();
		m_signalsLoading = false;
	}
}

QString AppSignalSetProvider::errorMessage(const ObjectState& state)
{
	// Converts ObjectState.errCode to human readable message
	//
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT:
			return QString("Signal %1 is not checked out").arg(state.id);

		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER:
			return QString("Signal %1 is checked out by user %2").arg(state.id).arg(getUserName(state.userId));

		case ERR_SIGNAL_DELETED:
			return QString("Signal %1 was deleted already").arg(state.id);

		case ERR_SIGNAL_NOT_FOUND:
			return QString("Signal %1 not found").arg(state.id);

		case ERR_SIGNAL_EXISTS:
				return QString();				// error message is displayed by PGSql driver

		default:
			return QString("Unknown error code %1").arg(state.errCode);
	}
}

bool AppSignalSetProvider::showError(const ObjectState& state)
{
	// Throws error signal with human readable message for single ObjectState
	//
	if (state.errCode != ERR_SIGNAL_OK)
	{
		QString message = errorMessage(state);

		if (!message.isEmpty())
		{
			emit error(message);
		}

		return false;
	}

	return true;
}

bool AppSignalSetProvider::showErrors(const std::vector<ObjectState>& states)
{
	// Throws single error signal with human readable message for set of ObjectState
	//
	if (states.empty())
	{
		return true;
	}

	QString message;

	for(const ObjectState& state : states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			if (message.isEmpty() == false)
			{
				message.append(QStringLiteral("\n"));
			}

			message += errorMessage(state);
		}
	}

	if (message.isEmpty() == false)
	{
		emit error(message);
		return false;
	}

	return true;
}

bool AppSignalSetProvider::addSignals(E::SignalType signalType, std::vector<AppSignal>* newSignals, QWidget* parentWidget)
{
	TEST_PTR_RETURN_FALSE(newSignals);

	bool result = m_db->addSignals(signalType, newSignals, parentWidget);

	if (result == true)
	{
		appendSignalsAndUpdateViews(*newSignals);
	}

	return result;
}

bool AppSignalSetProvider::autoAddSignals(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignals,
					std::vector<AppSignal>* newSignals, QWidget* parentWidget)
{
	TEST_PTR_RETURN_FALSE(newSignals);

	bool result = m_db->autoAddSignals(deviceSignals, newSignals, parentWidget);

	if (result == true)
	{
		appendSignalsAndUpdateViews(*newSignals);
	}

	return result;
}

bool AppSignalSetProvider::setSignalWorkcopy(AppSignal* signal, ObjectState* objectState, QWidget* parentWidget)
{
	bool result = m_db->setSignalWorkcopy(signal, objectState, parentWidget);

	if (result == true)
	{
		auto [s, index] = m_signalSet.updateSignal(*signal);

		emit signalsUpdated({index});
	}

	return result;
}

bool AppSignalSetProvider::checkoutSignalByIndex(int index, QString* message)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal* s = m_signalSet.at(index);

	return checkoutSignal(s, message);
}

bool AppSignalSetProvider::checkoutSignal(const AppSignal* s, QString* message)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	TEST_PTR_RETURN_FALSE(s);

	if (s->checkedOut() == true)
	{
		if (s->userID() == m_currentUserID ||
			m_currentUserIsAdmin)
		{
			return true;
		}
		else
		{
			*message = QString(tr("Signal %1 is already checked out by user %2")).
								arg(s->appSignalID()).arg(getUserName(s->userID()));
			return false;
		}
	}

	std::vector<int> signalsIDs;

	getChannelSignalsID(*s, &signalsIDs);

	std::vector<ObjectState> objectStates;

	bool res = m_db->checkoutSignals(signalsIDs, &objectStates, nullptr);

	RETURN_IF_FALSE(res);

	if (objectStates.empty())
	{
		return false;
	}

	if (message == nullptr)
	{
		showErrors(objectStates);
	}
	else
	{
		foreach (const ObjectState& objectState, objectStates)
		{
			if (objectState.errCode != ERR_SIGNAL_OK)
			{
				*message += errorMessage(objectState) + "\n";
			}
		}
	}

	for(const ObjectState& objectState : objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER &&
			objectState.userId != m_currentUserID &&
			m_currentUserIsAdmin == false)
		{
			return false;
		}
	}

	reloadSignals(signalsIDs);

	return true;
}

bool AppSignalSetProvider::checkinSignals(const std::vector<int>& signalIDs,
										  QString comment)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<ObjectState> states;

	bool result = m_db->checkinSignals(signalIDs, comment, &states, m_parentWidget);

	showErrors(states);

	bool reloadAll = false;

	for(const ObjectState& state : states)
	{
		if (state.deleted == true)
		{
			reloadAll = true;
			break;
		}
	}

	if (reloadAll)
	{
		reloadAllSignals();
	}
	else
	{
		reloadSignals(signalIDs);
	}

	return result;
}

bool AppSignalSetProvider::undoSignalsChanges(const std::vector<int>& signalIDs, QWidget* parentWidget)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<int> ids;

	for(int id : signalIDs)
	{
		const AppSignal* s = m_signalSet.getSignal(id);

		TEST_PTR_CONTINUE(s);

		if (s->checkedOut() == false)
		{
			continue;
		}

		std::vector<int> channelIDs;

		m_signalSet.getChannelSignalsID(id, &channelIDs);

		ids.insert(ids.end(), channelIDs.begin(), channelIDs.end());
	}

	if (ids.empty() == true)
	{
		return true;
	}

	if (parentWidget == nullptr)
	{
		parentWidget = m_parentWidget;
	}

	std::vector<ObjectState> states;

	bool result = m_db->undoSignalsChanges(ids, &states, parentWidget);

	RETURN_IF_FALSE(result);

	result &= showErrors(states);

	reloadSignals(ids);

	return result;
}

bool AppSignalSetProvider::undoSignal(int id)
{
	return undoSignalsChanges(std::vector<int>{id});
}

bool AppSignalSetProvider::undoSignal(const AppSignal& s)
{
	return undoSignal(s.ID());
}

void AppSignalSetProvider::deleteSignal(int signalID)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	ObjectState state;

	m_db->deleteSignal(signalID, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

bool AppSignalSetProvider::updateSignalsSpecProps(const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate,
													QString* errMsg)
{
	TEST_PTR_RETURN_FALSE(errMsg)

	QStringList equipmentIDs;

	for(const Hardware::DeviceAppSignal* deviceSignal: deviceSignalsToUpdate)
	{
		TEST_PTR_CONTINUE(deviceSignal)
		equipmentIDs.append(deviceSignal->equipmentId());
	}

	std::map<QString, std::set<int>> signalIDsMap;

	bool result = m_db->getMultipleSignalsIDsWithEquipmentID(equipmentIDs, &signalIDsMap, nullptr);

	if (result == false)
	{
		return false;
	}

	std::vector<int> checkoutSignalIDs;
	std::vector<AppSignal> newSignalWorkcopies;

	for(const Hardware::DeviceAppSignal* deviceSignal: deviceSignalsToUpdate)
	{
		TEST_PTR_CONTINUE(deviceSignal)

		QString deviceSignalSpecPropStruct = deviceSignal->signalSpecPropsStruct();

		if (	deviceSignalSpecPropStruct.contains(AppSignalPropNames::MISPRINT_lowEngineeringUnitsCaption) ||
				deviceSignalSpecPropStruct.contains(AppSignalPropNames::MISPRINT_highEngineeringUnitsCaption))
		{
			*errMsg = QString(tr("Misprinted signal specific properties HighEngEneeringUnits/LowEngEneeringUnits has detected in device signal %1. \n\n"
								"Update module preset first. \n\nUpdating from preset is aborted!")).
								arg(deviceSignal->equipmentId());
			return false;
		}

		auto mapIt = signalIDsMap.find(deviceSignal->equipmentId());

		if (mapIt == signalIDsMap.end())
		{
			continue;
		}

		const std::set<int>& signalIDs = mapIt->second;

		if (signalIDs.size() == 0)
		{
			continue;
		}

		for(int signalID : signalIDs)
		{
			bool signalChanged = false;

			AppSignal s;

			result = m_db->getLatestSignal(signalID, &s, nullptr);

			if (result == false)
			{
				*errMsg = QString(tr("Cannot getLatestSignal with id = %1, update from preset is aborted.")).arg(signalID);
				return false;
			}

			if (s.specPropStruct() != deviceSignalSpecPropStruct)
			{
				signalChanged = true;
			}

			AppSignalSpecPropValues specPropValues;

			result = specPropValues.parseValuesFromArray(s.protoSpecPropValues());

			if (result == false)
			{
				*errMsg = QString(tr("Signal %1 specific properties values parsing error, \nupdate from preset is aborted.")).arg(s.appSignalID());
				return false;
			}

			result = specPropValues.updateFromSpecPropStruct(deviceSignalSpecPropStruct);

			if (result == false)
			{
				*errMsg = QString(tr("Signal %1 specific properties values updating error, \nupdate from preset is aborted.")).arg(s.appSignalID());
				return false;
			}

			QByteArray newValues;

			result = specPropValues.serializeValuesToArray(&newValues);

			if (newValues != s.protoSpecPropValues())		// compare proto-data arrays
			{
				signalChanged = true;
			}

			if (signalChanged == false)
			{
				continue;
			}

			// signal should be updated
			//
			s.setSpecPropStruct(deviceSignalSpecPropStruct);
			s.setProtoSpecPropValues(newValues);

			checkoutSignalIDs.push_back(signalID);
			newSignalWorkcopies.emplace_back(s);
		}
	}

	if (checkoutSignalIDs.size() == 0)
	{
		return true;
	}

	std::vector<ObjectState> objStates;

	result = m_db->checkoutSignals(checkoutSignalIDs, &objStates, nullptr);

	if (result == false)
	{
		*errMsg = QString(tr("App signals check out error, update is not possible!"));
		return false;
	}

	if (objStates.size() != checkoutSignalIDs.size())
	{
		*errMsg = QString(tr("Not all necessery app signals was checked out, update is not possible!"));
		return false;
	}

	bool allSignalsCheckedOut = true;

	for(const ObjectState& objState : objStates)
	{
		if (objState.checkedOut == false || objState.errCode != ERR_SIGNAL_OK)
		{
			allSignalsCheckedOut = false;
			break;
		}
	}

	if (allSignalsCheckedOut == false)
	{
		*errMsg = QString(tr("Cannot check out one or more app signals, update from preset is not posible."));
		return false;
	}

	result = m_db->setSignalsWorkcopies(newSignalWorkcopies, nullptr);

	if (result == false)
	{
		*errMsg = QString(tr("Error setting signals new workcopies, update from preset is aborted."));
		return false;
	}

	reloadSignals(checkoutSignalIDs);

	return result;
}

bool AppSignalSetProvider::createNewSignals(const AppSignal& signalTemplate,
											int channelsCount,
											int signalsCount,
											std::vector<int>* addedSignalIDs)
{
	TEST_PTR_RETURN_FALSE(addedSignalIDs);

	std::vector<AppSignal> resultSignalVector;

	resultSignalVector.reserve(signalsCount * channelsCount);

	bool uppercase = projectProperty_uppercaseAppSignalID();

	for (int s = 0; s < signalsCount; s++)
	{
		std::vector<AppSignal> newSignalsVector;

		for (int ch = 0; ch < channelsCount; ch++)
		{
			AppSignal& newSignal = newSignalsVector.emplace_back(signalTemplate);

			QString suffix;

			if (signalsCount > 1)
			{
				suffix = QString("_SIG%1").arg(s, 3, 10, QChar('0'));
			}

			if (channelsCount > 1)
			{
				suffix += "_" + QString(QChar('A' + ch));
			}

			QString appSignalID = newSignal.appSignalID() + suffix;
			QString customAppSignalID = newSignal.customAppSignalID() + suffix;

			if (uppercase)
			{
				appSignalID = appSignalID.toUpper();
				customAppSignalID = customAppSignalID.toUpper();
			}

			newSignal.setAppSignalID(appSignalID);
			newSignal.setCustomAppSignalID(customAppSignalID);
		}

		if (m_db->addSignals(signalTemplate.signalType(), &newSignalsVector, m_parentWidget) == true)
		{
			for (const AppSignal& s : newSignalsVector)
			{
				resultSignalVector.emplace_back(s);
			}
		}
		else
		{
			Q_ASSERT(false);
			return false;
		}

		for(const AppSignal& newSignal : newSignalsVector)
		{
			m_signalSet.append(newSignal);
			addedSignalIDs->push_back(newSignal.ID());
		}
	}

	emit signalsCountChanged();

	return true;
}

void AppSignalSetProvider::deleteSignals(const std::vector<int>& signalIDs)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}

	reloadAllSignals();
}

void AppSignalSetProvider::saveSignal(AppSignal& signal)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	ObjectState state;

	signal.trimTextFields();

	m_db->setSignalWorkcopy(&signal, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	m_signalSet.updateSignal(signal);
}

void AppSignalSetProvider::saveSignals(const std::vector<AppSignal*>& signalVector)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<ObjectState> states;

	for (AppSignal* s : signalVector)
	{
		ObjectState state;

		s->trimTextFields();

		m_db->setSignalWorkcopy(s, &state, nullptr);

		states.emplace_back(state);

		m_signalSet.updateSignal(*s);
	}

	showErrors(states);
}

std::vector<int> AppSignalSetProvider::cloneSignals(const std::vector<int>& signalIDsToClone)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<int> resultSignalIDs;

	std::set<int> clonedSignalIDs;

	for (const int signalID : signalIDsToClone)
	{
		if (clonedSignalIDs.contains(signalID))
		{
			continue;
		}

		const AppSignal* signalToClone = m_signalSet.getSignal(signalID);

		TEST_PTR_CONTINUE(signalToClone);

		const AppSignal signal(*signalToClone);

		E::SignalType type = signal.signalType();
		std::vector<int> groupSignalIDs;

		m_signalSet.getChannelSignalsID(signal, &groupSignalIDs);

		clonedSignalIDs.insert(groupSignalIDs.begin(), groupSignalIDs.end());

		QString cloneSuffix = "_CLONE";
		int suffixNumerator = 1;
		bool hasConflict = false;

		do
		{
			hasConflict = false;

			for(int groupSignalID : groupSignalIDs)
			{
				const AppSignal* groupSignal = m_signalSet.getSignal(groupSignalID);

				TEST_PTR_CONTINUE(groupSignal);

				QString cloneAppSignalID = groupSignal->appSignalID() + cloneSuffix;

				if (m_signalSet.contains(cloneAppSignalID))
				{
					hasConflict = true;
					break;
				}
			}

			if (hasConflict)
			{
				suffixNumerator++;
				cloneSuffix = QString("_CLONE%1").arg(suffixNumerator);
			}
		}
		while (hasConflict && suffixNumerator < 1000);

		if (suffixNumerator >= 1000)
		{
			assert(false);
			return std::vector<int>();
		}

		std::vector<AppSignal> signalsToCreate(groupSignalIDs.size());

		int i = 0;

		for(int groupSignalID : groupSignalIDs)
		{
			const AppSignal* groupSignal = m_signalSet.getSignal(groupSignalID);

			TEST_PTR_CONTINUE(groupSignal);

			AppSignal& signalToCreate = signalsToCreate[i];

			signalToCreate = *groupSignal;

			signalToCreate.trimTextFields();

			signalToCreate.setAppSignalID(groupSignal->appSignalID() + cloneSuffix);
			signalToCreate.setCustomAppSignalID(groupSignal->customAppSignalID() + cloneSuffix);

			i++;
		}

		m_db->addSignals(type, &signalsToCreate, nullptr);

		for (const AppSignal& s : signalsToCreate)
		{
			resultSignalIDs.push_back(s.ID());
		}
	}

	reloadAllSignals();

	return resultSignalIDs;
}

