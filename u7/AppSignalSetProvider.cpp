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
	Q_ASSERT(m_db != nullptr);
	return m_db;
}

const AppSignalSet& AppSignalSetProvider::signalSet() const
{
	return m_signalSet;
}

int AppSignalSetProvider::signalCount() const
{
	return m_signalSet.count();
}

AppSignalPropertyManager& AppSignalSetProvider::signalPropertyManager()
{
	return m_propertyManager;
}

void AppSignalSetProvider::onProjectOpened()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	m_signalSet.clear();

	m_currentUserID = m_db->currentUser().userId();
	m_currentUserIsAdmin = m_db->currentUser().isAdminstrator();

	loadUsers();
	reloadPropertiesBehaviour();

	startSignalsLoading();
}

void AppSignalSetProvider::onProjectClosed()
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

QString AppSignalSetProvider::signalCheckedOutByUser(const AppSignal& s)
{
	if (s.checkedOut() == true)
	{
		return getUserName(s.userID());
	}

	return QString();
}

void AppSignalSetProvider::reloadAllSignals()
{
	Q_ASSERT(m_thread == QThread::currentThread());

	startSignalsLoading();
}

void AppSignalSetProvider::reloadSignals(const std::vector<int>& signalIds, bool updateViews)
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

	if (updateViews)
	{
		emitSignalsUpdated(signalsIndexes);
	}
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

	reloadSignals(signalIds, true);

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
		emitSignalsUpdated({index});
	}

	return updatedSignal;
}

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}

AppSignal* AppSignalSetProvider::getSignal(const QString& appSignalID)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	return m_signalSet.getSignal(appSignalID);
}

const AppSignal* AppSignalSetProvider::getSignal(const QString& appSignalID) const
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

bool AppSignalSetProvider::signalExists(const QString& appSignalID) const
{
	return (getSignal(appSignalID) != nullptr);
}

const std::vector<AppSignal*>& AppSignalSetProvider::signalsVector() const
{
	return m_signalSet.signalsVector();
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

	TEST_PTR_RETURN_VALUE(s, AppSignalSet::BAD_ID);

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

void AppSignalSetProvider::getSameChannelSignalsIndexes(int signalIndex, std::vector<int>* sameChannelIndexes)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	TEST_PTR_RETURN(sameChannelIndexes);

	sameChannelIndexes->clear();

	const AppSignal* s = m_signalSet.at(signalIndex);

	TEST_PTR_RETURN(s);

	std::vector<int> sameChannelIDs;

	m_signalSet.getChannelSignalsID(*s, &sameChannelIDs);

	for(int id : sameChannelIDs)
	{
		int index = m_signalSet.signalIndex(id);

		if (index != BAD_INDEX)
		{
			sameChannelIndexes->push_back(index);
		}
		else
		{
			Q_ASSERT(false);
		}
	}
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

bool AppSignalSetProvider::isCheckinableSignalForMe(const ObjectState& objState) const
{
	return objState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER &&
				(objState.userId == m_currentUserID || m_currentUserIsAdmin == true);
}

bool AppSignalSetProvider::createNewSignals(const AppSignal& templateSignal,
											int channelsCount,
											int signalsCount,
											std::vector<int>* addedSignalIDs)
{
	TEST_PTR_RETURN_FALSE(addedSignalIDs);

	addedSignalIDs->clear();

	bool uppercase = projectProperty_uppercaseAppSignalID();

	channelsCount = std::clamp(channelsCount, MIN_CHANNEL_COUNT, MAX_CHANNEL_COUNT);

	int newSignalIndex = -1;

	for (int s = 0; s < signalsCount; s++)
	{
		std::vector<AppSignal> newSignalsVector;

		for (int ch = CHANNEL_1; ch < channelsCount; ch++)
		{
			AppSignal& newSignal = newSignalsVector.emplace_back(templateSignal);

			QString suffix;

			if (signalsCount > 1)
			{
				suffix = QString("_%1").arg(s, 3, 10, QChar('0'));
			}

			if (channelsCount > 1)
			{
				suffix += QString("_%1").arg(E::valueToString<E::Channel>(ch));
			}

			QString appSignalID = newSignal.appSignalID() + suffix;
			QString customAppSignalID = newSignal.customAppSignalID() + suffix;

			if (uppercase)
			{
				appSignalID = appSignalID.toUpper();
				//customAppSignalID = customAppSignalID.toUpper();
			}

			newSignal.setAppSignalID(appSignalID);
			newSignal.setCustomAppSignalID(customAppSignalID);
			newSignal.setCaption("Signal " + customAppSignalID);
		}

		if (m_db->addSignals(templateSignal.signalType(), &newSignalsVector, m_parentWidget) == true)
		{
			for (const AppSignal& newSignal : newSignalsVector)
			{
				auto [sg, index] = m_signalSet.append(newSignal);

				addedSignalIDs->push_back(newSignal.ID());
				newSignalIndex = index;
			}
		}
		else
		{
			Q_ASSERT(false);
			return false;
		}
	}

	// all signals of same type respectively have identical properties,
	// so property checking of one signal is enough
	//
	emit detectNewProperties(std::vector<int>{newSignalIndex});

	emit signalsCountChanged();

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

std::vector<int> AppSignalSetProvider::cloneSignals(const std::vector<int>& signalIDsToClone)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::vector<int> newSignalIDs;
	std::vector<int> newSignalIndexes;
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
			auto [sc, index] = m_signalSet.append(s);

			TEST_PTR_CONTINUE(sc);

			newSignalIDs.push_back(sc->ID());
			newSignalIndexes.push_back(index);
		}
	}

	emit detectNewProperties(newSignalIndexes);
	emit signalsCountChanged();

	return newSignalIDs;
}

bool AppSignalSetProvider::saveSignal(AppSignal* signal, QWidget* parentWidget)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	Q_UNUSED(parentWidget);

	TEST_PTR_RETURN_FALSE(signal);

	signal->trimTextFields();
	signal->uppercaseAppSignalID(projectProperty_uppercaseAppSignalID());

	ObjectState state;

	bool result = m_db->setSignalWorkcopy(signal, &state, nullptr);

	result &= showError(state);

	if (result == true)
	{
		auto [s, index] = m_signalSet.updateSignal(*signal);

		emitSignalsUpdated({index});
	}

	return result;
}

bool AppSignalSetProvider::saveSignals(const std::vector<AppSignal*>& signalsVector, QWidget* parentWidget)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	bool result = true;

	std::vector<ObjectState> states;
	std::vector<int> updatedIndexes;

	for (AppSignal* s : signalsVector)
	{
		ObjectState state;

		s->trimTextFields();

		bool res = m_db->setSignalWorkcopy(s, &state, parentWidget);

		result &= res;

		states.emplace_back(state);

		if (res == true && state.errCode == ERR_SIGNAL_OK)
		{
			auto [su, index] = m_signalSet.updateSignal(*s);

			updatedIndexes.push_back(index);
		}
	}

	if (updatedIndexes.empty() == false)
	{
		emitSignalsUpdated(updatedIndexes);
	}

	result &= showErrors(states);

	return result;
}

bool AppSignalSetProvider::checkoutSignalByIndex(int index, QString* message)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	AppSignal* s = m_signalSet.at(index);

	return checkoutSignal(s, message);
}

bool AppSignalSetProvider::checkoutSignal(const AppSignal* s, QString* errMsg,
										  std::vector<int>* checkedOutIDs)
{
	TEST_PTR_RETURN_FALSE(s);

	return checkoutSignals(std::vector<int>{s->ID()}, errMsg, checkedOutIDs);
}

bool AppSignalSetProvider::checkoutSignals(const std::vector<AppSignal*>& appSignals, QString* errMsg,
										   std::vector<int>* checkedOutIDs)
{
	std::vector<int> appSignalIDs;

	appSignalIDs.reserve(appSignals.size());

	for(const AppSignal* s : appSignals)
	{
		TEST_PTR_CONTINUE(s);

		appSignalIDs.push_back(s->ID());
	}

	return checkoutSignals(appSignalIDs, errMsg, checkedOutIDs);
}

bool AppSignalSetProvider::checkoutSignals(const std::vector<int>& appSignalIDs, QString* errMsg,
										   std::vector<int>* checkedOutIDs)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	if (checkedOutIDs != nullptr)
	{
		checkedOutIDs->clear();
	}

	std::set<int> uniqueIDs;
	std::vector<int> temp;

	for(int id : appSignalIDs)
	{
		m_signalSet.getChannelSignalsID(id, &temp);
		uniqueIDs.insert(temp.begin(), temp.end());
	}

	reloadSignals(std::vector<int>(uniqueIDs.begin(), uniqueIDs.end()), false);

	std::vector<int> signalsIDsToCheckout;

	// exclude signals already checked out by current user
	//
	for(int id : uniqueIDs)
	{
		const AppSignal* rs = getSignalByID(id);

		TEST_PTR_CONTINUE(rs);

		if (rs->checkedOut() == true &&
			(rs->userID() == m_currentUserID || m_currentUserIsAdmin))
		{
			continue;
		}

		signalsIDsToCheckout.push_back(id);
	}

	std::vector<ObjectState> objectStates;

	bool res = m_db->checkoutSignals(signalsIDsToCheckout, &objectStates, nullptr);

	RETURN_IF_FALSE(res);

	if (checkedOutIDs != nullptr)
	{
		checkedOutIDs->clear();
		checkedOutIDs->reserve(objectStates.size());
	}

	for(const ObjectState& os : objectStates)
	{
		if (os.errCode == ERR_SIGNAL_OK)
		{
			if (checkedOutIDs != nullptr)
			{
				checkedOutIDs->push_back(os.id);
			}
		}
		else
		{
			if (isCheckinableSignalForMe(os) == false)
			{
				if (errMsg != nullptr)
				{
					*errMsg += errorMessage(os) + "\n";
					res = false;
				}
			}
		}
	}

	if (errMsg == nullptr)
	{
		res = showErrors(objectStates);
	}

	reloadSignals(signalsIDsToCheckout, true);

	return res;
}

bool AppSignalSetProvider::checkinSignals(const std::vector<int>& signalIDs,
										  QString comment)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::set<int> uniqueSignalIDs;

	for(int id : signalIDs)
	{
		std::vector<int> chSignalsIDs;

		m_signalSet.getChannelSignalsID(id, &chSignalsIDs);

		uniqueSignalIDs.insert(chSignalsIDs.begin(), chSignalsIDs.end());
	}

	std::vector<int> channelSignalsIDs(uniqueSignalIDs.begin(), uniqueSignalIDs.end());

	std::vector<ObjectState> states;

	bool result = m_db->checkinSignals(channelSignalsIDs, comment, &states, m_parentWidget);

	RETURN_IF_FALSE(result);

	updateSignalSet(states);

	return result;
}

bool AppSignalSetProvider::undoSignalsChanges(const std::vector<int>& signalIDs, QWidget* parentWidget)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::set<int> uniqueIDs;

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

		uniqueIDs.insert(channelIDs.begin(), channelIDs.end());
	}

	if (uniqueIDs.empty() == true)
	{
		return true;
	}

	if (parentWidget == nullptr)
	{
		parentWidget = m_parentWidget;
	}

	std::vector<ObjectState> states;
	std::vector<int> ids(uniqueIDs.begin(), uniqueIDs.end());

	bool result = m_db->undoSignalsChanges(ids, &states, parentWidget);

	RETURN_IF_FALSE(result);

	updateSignalSet(states);

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

	std::set<int> checkoutSignalIDsSet;
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

			newSignalWorkcopies.emplace_back(s);

			checkoutSignalIDsSet.insert(signalID);
		}
	}

	if (checkoutSignalIDsSet.size() == 0)
	{
		return true;
	}

	std::vector<int> checkoutSignalIDs(checkoutSignalIDsSet.begin(), checkoutSignalIDsSet.end());

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

	reloadSignals(checkoutSignalIDs, true);

	return result;
}

void AppSignalSetProvider::deleteSignals(const std::vector<int>& signalIDs)
{
	Q_ASSERT(m_thread == QThread::currentThread());

	std::set<int> signalsToDeleteIDs;

	std::vector<int> channelSignalsIDs;

	for(int signalID : signalIDs)
	{
		channelSignalsIDs.clear();
		m_signalSet.getChannelSignalsID(signalID, &channelSignalsIDs);

		std::copy(channelSignalsIDs.begin(), channelSignalsIDs.end(),
				  std::inserter(signalsToDeleteIDs, signalsToDeleteIDs.end()));
	}

	reloadSignals(std::vector<int>(signalsToDeleteIDs.begin(), signalsToDeleteIDs.end()), false);

	//

	QString warning = "Signal(s):\n\n";
	int wrnCount = 0;

	for (const int signalID : signalsToDeleteIDs)
	{
		AppSignal* s = getSignalByID(signalID);

		TEST_PTR_CONTINUE(s);

		E::VcsItemAction ia = s->instanceAction();

		if (ia == E::VcsItemAction::Added && s->changesetID() == 0)
		{
			if (wrnCount < 5)
			{
				warning += s->appSignalID() + "\n";
			}

			wrnCount++;
		}
	}

	if (wrnCount > 0)
	{
		warning += "\n";

		if (wrnCount > 5)
		{
			warning += QString("and %1 more signal(s) ").arg(wrnCount - 5);
		}

		warning += "are just added  and not previously checked in.\n\n";
		warning += "So these signals will completely delete without possibility to restore.\n\n"
					"A you sure to completely delete these signals?";

		QMessageBox::StandardButton res = QMessageBox::warning(m_parentWidget, "Warning", warning, QMessageBox::Yes | QMessageBox::No);

		if (res == QMessageBox::No)
		{
			return;
		}
	}

	//

	std::vector<ObjectState> states;

	states.reserve(signalsToDeleteIDs.size());

	for (const int signalID : signalsToDeleteIDs)
	{
		ObjectState state;
		m_db->deleteSignal(signalID, &state, nullptr);

		states.emplace_back(state);
	}

	updateSignalSet(states);
}

bool AppSignalSetProvider::getProjectProperties(DbProjectProperties* projectProps) const
{
	TEST_PTR_RETURN_FALSE(projectProps);

	return m_db->getProjectProperties(projectProps, m_parentWidget);
}

bool AppSignalSetProvider::isSafetyProject() const
{
	DbProjectProperties projectProps;

	getProjectProperties(&projectProps);

	return projectProps.safetyProject();
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
	m_signalSet.reserve(TO_INT(ids.size()));

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

	emit detectNewProperties(newIndexes);
	emit signalsCountChanged();
}

void AppSignalSetProvider::updateSignalSet(const std::vector<ObjectState>& states)
{
	std::vector<int> updatedIDs;
	std::vector<int> removedIDs;

	for(const ObjectState& state : states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			showError(state);
			continue;
		}

		if (state.deleted == true)
		{
			if (state.checkedOut == true)
			{
				// deletion of previously checked in signal
				// signal is ONLY checked out and marked as deleted
				// but NOT physically deleted
				//
				updatedIDs.push_back(state.id);
			}
			else
			{
				// deletion of just added (not yet checked in) signal
				// or check in of signal already marked as deleted
				// physically removed signal and NOT checkedOut
				//
				removedIDs.push_back(state.id);
			}
		}
		else
		{
			updatedIDs.push_back(state.id);
		}
	}

	if (updatedIDs.empty() == false)
	{
		bool updateViews = removedIDs.empty();		// if removedIDs NOT empty views will update on signalsCountChanged() below

		reloadSignals(updatedIDs, updateViews);
	}

	if (removedIDs.empty() == false)
	{
		m_signalSet.removeSignals(removedIDs);
		emit signalsCountChanged();
	}
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
		reloadSignals(signalIds, true);
	}

	if (signalIds.size() == 0 ||
		(low < 0 && high >= signalCount))
	{
		m_signalsLoadTimer.stop();
		m_signalsLoading = false;
	}
}

void AppSignalSetProvider::emitSignalsUpdated(const std::vector<int>& indexes)
{
	emit signalsUpdated(indexes);
}

QString AppSignalSetProvider::errorMessage(const ObjectState& state)
{
	// Converts ObjectState.errCode to human readable message
	//
	QString signalID = QString::number(state.id);

	AppSignal* s = getSignalByID(state.id);

	if (s != nullptr)
	{
		signalID = s->appSignalID();
	}

	switch(state.errCode)
	{
	case ERR_SIGNAL_OK:
		return QString();

	case ERR_SIGNAL_IS_NOT_CHECKED_OUT:
		return QString("Signal %1 is not checked out").arg(signalID);

	case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER:
		return QString("Signal %1 is checked out by user %2").arg(signalID).arg(getUserName(state.userId));

	case ERR_SIGNAL_DELETED:
		return QString("Signal %1 was deleted already").arg(signalID);

	case ERR_SIGNAL_NOT_FOUND:
		return QString("Signal %1 not found").arg(signalID);

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
