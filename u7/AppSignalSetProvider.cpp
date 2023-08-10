#include "AppSignalSetProvider.h"

AppSignalSetProvider* AppSignalSetProvider::m_instance = nullptr;

AppSignalSetProvider::AppSignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	QObject(parentWidget),
	m_db(dbController),
	m_propertyManager(dbController, parentWidget),
	m_signalsLoadTimer(this)
{
	Q_ASSERT(m_instance == nullptr);
	m_instance = this;

	TEST_PTR_RETURN(dbController);

	connect(this, &AppSignalSetProvider::signalsPropertiesChanged,
			&m_propertyManager, &AppSignalPropertyManager::detectSignalsNewProperties);

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

void AppSignalSetProvider::projectOpened(DbController* dbController)
{
	TEST_PTR_RETURN(dbController);

	m_signalSet.clear();

	m_currentUserID = dbController->currentUser().userId();
	m_currentUserIsAdmin = dbController->currentUser().isAdminstrator();

	loadUsers();

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	startSignalsLoading();
}

void AppSignalSetProvider::projectClosed()
{
	terminateSignalsLoading();

	m_currentUserID = -1;
	m_currentUserIsAdmin = false;

	m_signalSet.clear();
	m_users.clear();
	m_propertyManager.clear();

	emit signalCountChanged();
}

const AppSignalSet& AppSignalSetProvider::signalSet() const
{
	return m_signalSet;
}

AppSignalPropertyManager& AppSignalSetProvider::signalPropertyManager()
{
	return m_propertyManager;
}

void AppSignalSetProvider::reloadSignals()
{
	startSignalsLoading();
}

void AppSignalSetProvider::loadSignals(const std::vector<int>& signalIds, bool withoutProgress)
{
	if (signalIds.size() == 0)
	{
		return;
	}

	std::vector<AppSignal> signalsToLoad;

	std::vector<const AppSignal*> updatedSignals;
	std::vector<int> updatedindexes;

	if (withoutProgress == true)
	{
		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, nullptr);
	}
	else
	{
		dbController()->getLatestSignals(signalIds, &signalsToLoad, nullptr);
	}
	int signalIndex = 0;

	for (const AppSignal& loadedSignal: signalsToLoad)
	{
		const AppSignal* s = m_signalSet.updateSignal(loadedSignal, &signalIndex);

		if (s !=  nullptr)
		{
			updatedSignals.push_back(s);
			updatedindexes.push_back(signalIndex);
		}
	}

	emit signalsUpdated(updatedindexes);
	emit signalsPropertiesChanged(updatedSignals);
}

void AppSignalSetProvider::reloadSignals(const std::vector<int>& signalIds)
{
	loadSignals(signalIds, true);
}

void AppSignalSetProvider::enforceAllSignalsLoading()
{
	if (m_signalsLoading == false)
	{
		return;
	}

	m_signalsLoadTimer.stop();

	std::vector<int> signalIds;

	for (const AppSignal* s : m_signalSet)
	{
		if (s->isLoaded() == false)
		{
			signalIds.push_back(s->ID());
		}
	}

	loadSignals(signalIds, false);

	m_signalsLoading = false;
	m_signalsLoadTimer.stop();
}

const AppSignal* AppSignalSetProvider::loadSignal(int signalId)
{
	AppSignal loadedSignal;

	dbController()->getLatestSignal(signalId, &loadedSignal, nullptr);

	const AppSignal* updatedSignal = m_signalSet.updateSignal(loadedSignal);

	TEST_PTR_RETURN_NULLPTR(updatedSignal);

	int index = m_signalSet.signalIndex(signalId);

	emit signalsUpdated(std::vector{index});
	emit signalsPropertiesChanged(std::vector{updatedSignal});

	return updatedSignal;
}

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}

QString AppSignalSetProvider::getUserName(int userId)
{
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

	return QString("UnknownUser");
}

AppSignal* AppSignalSetProvider::getSignal(const QString& appSignalID)
{
	return m_signalSet.getSignal(appSignalID);
}

AppSignal* AppSignalSetProvider::getSignal(int signalID)
{
	return m_signalSet.getSignal(signalID);
}

bool AppSignalSetProvider::getChannelSignalsID(const AppSignal& signal, SignalIDsSet* channelSignalIDs) const
{
	return m_signalSet.getChannelSignalsID(signal, channelSignalIDs);
}

bool AppSignalSetProvider::getChannelSignalsID(int signalID, int groupID, SignalIDsSet* channelSignalIDs) const
{
	return m_signalSet.getChannelSignalsID(signalID, groupID, channelSignalIDs);
}

int AppSignalSetProvider::signalIndex(int signalID) const
{
	return m_signalSet.signalIndex(signalID);
}

int AppSignalSetProvider::signalID(int index) const
{
	const AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_VALUE(s, -1);

	return s->ID();
}

AppSignal* AppSignalSetProvider::getLoadedSignal(AppSignal* s)
{
	TEST_PTR_RETURN_NULLPTR(s);

	if (s->isLoaded() == false)
	{
		loadSignal(s->ID());
	}

	return s;
}

AppSignal* AppSignalSetProvider::getLoadedSignalByID(int signalID)
{
	AppSignal* s = m_signalSet.getSignal(signalID);

	return getLoadedSignal(s);
}

AppSignal* AppSignalSetProvider::getLoadedSignal(int index)
{
	AppSignal* s = m_signalSet.at(index);

	return getLoadedSignal(s);
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(int index)
{
	AppSignal signal(*getLoadedSignal(index));

	signal.cacheSpecPropValues();

	AppSignalParam param;

	param.load(signal);

	return param;
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(const QString& appSignalId)
{
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
	QVector<int> sameChannelSignalRows;

	const AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_VALUE(s, sameChannelSignalRows);

	if (s->signalGroupID() != 0)
	{
		SignalIDsSet sameChannelSignalIDs;

		m_signalSet.getChannelSignalsID(*s, &sameChannelSignalIDs);

		foreach (const int id, sameChannelSignalIDs)
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
	return isEditableSignal(m_signalSet.at(index));
}

bool AppSignalSetProvider::isEditableSignal(const AppSignal* signal) const
{
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
	return isCheckinableSignalForMe(m_signalSet.at(index));
}

bool AppSignalSetProvider::isCheckinableSignalForMe(const AppSignal* signal) const
{
	TEST_PTR_RETURN_FALSE(signal);

	if (signal->checkedOut() &&
		(signal->userID() == m_db->currentUser().userId() || m_db->currentUser().isAdminstrator()))
	{
		return true;
	}

	return false;
}

bool AppSignalSetProvider::checkoutSignal(int index, QString* message)
{
	AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_FALSE(s);

	if (s->checkedOut() == true)
	{
		if (s->userID() == m_db->currentUser().userId() ||
			dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	SignalIDsSet signalsIDs;

	getChannelSignalsID(*s, &signalsIDs);

	QVector<ObjectState> objectStates;

	m_db->checkoutSignals(signalsIDs, &objectStates, nullptr);

	if (objectStates.count() == 0)
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

	int currentUserID = dbController()->currentUser().userId();
	bool userIsAdmin = dbController()->currentUser().isAdminstrator();

	for (const ObjectState& objectState : objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER &&
			objectState.userId != currentUserID &&
			!userIsAdmin)
		{
			return false;
		}
	}

	for (int id : signalsIDs)
	{
		loadSignal(id);
	}

	return true;
}

void AppSignalSetProvider::loadUsers()
{
	std::vector<DbUser> users;

	m_db->getUserList(&users, nullptr);

	m_users.clear();

	for (auto const& user : users)
	{
		m_users.emplace(user.userId(), user.username());
	}
}

void AppSignalSetProvider::startSignalsLoading()
{
	if (m_signalsLoading == true)
	{
		terminateSignalsLoading();
	}

	loadIdAppSignalId();

	m_signalsLoadTimer.setInterval(100);
	m_signalsLoadTimer.start();

	emit signalCountChanged();
}

void AppSignalSetProvider::terminateSignalsLoading()
{
	m_signalsLoadTimer.stop();
	m_signalsLoading = false;
}

void AppSignalSetProvider::loadIdAppSignalId()
{
	std::vector<ID_AppSignalID> ids;

	m_db->getSignalsIDAppSignalID(&ids, false, nullptr);

	m_signalSet.clear();
	m_signalSet.reserve(ids.size());

	for(const ID_AppSignalID& id : ids)
	{
		m_signalSet.append(id);
	}
}

void AppSignalSetProvider::onSignalsLoadTimer()
{
	if (m_signalsLoading == false)
	{
		Q_ASSERT(false);
		m_signalsLoadTimer.stop();
		return;
	}

	static const int MAX_SIGNALS_COUNT = 1000;

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
		loadSignals(signalIds, true);
	}

	if (signalIds.size() == 0 ||
		(low < 0 && high >= signalCount))
	{
		m_signalsLoadTimer.stop();
		m_signalsLoading = false;
	}
}

// Converts ObjectState to human readable message
//
QString AppSignalSetProvider::errorMessage(const ObjectState& state)
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_users[state.userId]);
		case ERR_SIGNAL_DELETED: return tr("Signal %1 was deleted already").arg(state.id);
		case ERR_SIGNAL_NOT_FOUND: return tr("Signal %1 not found").arg(state.id);
		case ERR_SIGNAL_EXISTS: return "";				// error message is displayed by PGSql driver
		default:
			return tr("Unknown error %1").arg(state.errCode);
	}
}

// Throws error signal with human readable message for single ObjectState
//
void AppSignalSetProvider::showError(const ObjectState& state)
{
	if (state.errCode != ERR_SIGNAL_OK)
	{
		QString message = errorMessage(state);
		if (!message.isEmpty())
		{
			emit error(message);
		}
	}
}

// Throws single error signal with human readable message for set of ObjectState
//
void AppSignalSetProvider::showErrors(const QVector<ObjectState>& states)
{
	QString message;

	for(const ObjectState& state : states)
	{
		if (state.errCode != ERR_SIGNAL_OK)
		{
			if (message.isEmpty() == false)
			{
				message += "\n";
			}

			message += errorMessage(state);
		}
	}

	if (message.isEmpty() == false)
	{
		emit error(message);
	}
}

void AppSignalSetProvider::trimSignalTextFields(AppSignal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}

/*
void AppSignalSetProvider::initLazyLoadSignals()
{
	loadUsers();

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	QVector<ID_AppSignalID> signalIds;
	dbController()->getSignalsIDAppSignalID(&signalIds, nullptr);

	for (const ID_AppSignalID& id : signalIds)
	{
		m_signalSet.append(id);
	}

	emit signalCountChanged();
	m_signalsLoading = true;

	if (m_signalsLoadTimer == nullptr)
	{
		m_signalsLoadTimer = new QTimer(this);
		connect(m_signalsLoadTimer, &QTimer::timeout, this, &AppSignalSetProvider::loadNextSignalsPortion);
	}

	m_signalsLoadTimer->start(100);
}*/

/*
void AppSignalSetProvider::loadNextSignalsPortion()
{
	if (m_signalsLoading == false)
	{
		return;
	}

	QVector<int> signalIds;
	signalIds.reserve(250);

	int low = m_middleVisibleSignalIndex - 1;
	int high = m_middleVisibleSignalIndex;

	if (m_middleVisibleSignalIndex == -1)
	{
		high = 0;
	}

	int signalCount = m_signalSet.count();

	AppSignal* s = nullptr;

	while ((low >= 0 || high < signalCount) && signalIds.count() <= 248)
	{
		while (low >= 0)
		{
			s = m_signalSet.at(low);

			if (s != nullptr && s->isLoaded() == false)
			{
				signalIds.push_back(s->ID());
				break;
			}

			low--;
		}

		while (high < signalCount)
		{
			s = m_signalSet.at(high);

			if (s != nullptr && s->isLoaded() == false)
			{
				signalIds.push_back(s->ID());
				break;
			}

			high++;
		}
	}

	if (signalIds.count() > 0)
	{
		QVector<AppSignal> signalsToLoad;
		signalsToLoad.reserve(signalIds.count());

		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, nullptr);

		for (const AppSignal& loadedSignal : signalsToLoad)
		{
			AppSignal* s = m_signalSet.updateSignal(loadedSignal);

			emit signalUpdated(signalIndex(s->ID()));
			emit signalPropertiesChanged(*s);
		}
	}
	else
	{
		m_signalsLoading = false;
	}
}
*/
bool AppSignalSetProvider::undoSignal(int id)
{
	const AppSignal* s = m_signalSet.getSignal(id);

	TEST_PTR_RETURN_FALSE(s);

	if (!s->checkedOut())
	{
		return false;
	}

	SignalIDsSet signalsIDs;

	m_signalSet.getChannelSignalsID(*s, &signalsIDs);

	QVector<ObjectState> states;

	for (int signalId : signalsIDs)
	{
		ObjectState state;

		dbController()->undoSignalChanges(signalId, &state, nullptr);

		if (state.errCode != ERR_SIGNAL_OK)
		{
			states << state;
		}
	}

	if (!states.isEmpty())
	{
		showErrors(states);
	}

	for (int signalId : signalsIDs)
	{
		loadSignal(signalId);
	}

	return true;
}

void AppSignalSetProvider::deleteSignal(int signalID)
{
	ObjectState state;

	dbController()->deleteSignal(signalID, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

void AppSignalSetProvider::addSignal(AppSignal& signal)
{
	Q_ASSERT(false); // REMOVED m_signalSet.replaceOrAppendIfNotExists(signal); CHECK THIS!

	m_signalSet.append(new AppSignal(signal));
}

void AppSignalSetProvider::deleteSignals(const SignalIDsSet& signalIDs)
{
	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}

	reloadSignals();
}

/*
void AppSignalSetProvider::loadSignals()
{
	if (m_signalsLoading == true)
	{
		m_signalsLoadTimer.>stop();
		m_signalsLoading = false;
	}

	m_propertyManager.init();
	m_propertyManager.reloadPropertyBehaviour();

	loadUsers();

	AppSignalSet signalSetForReplacement;

	if (!dbController()->getSignals(&signalSetForReplacement, false, nullptr))
	{
		emit error(tr("Could not load signals"));
	}

	for (const AppSignal* s : signalSetForReplacement)
	{
		m_propertyManager.detectNewProperties(*s);
	}

	m_signalSet.swap(signalSetForReplacement);

	emit signalCountChanged();
} */

void AppSignalSetProvider::saveSignal(AppSignal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	m_signalSet.updateSignal(signal);
}

void AppSignalSetProvider::saveSignals(const std::vector<AppSignal*>& signalVector)
{
	QVector<ObjectState> states;

	for (AppSignal* s : signalVector)
	{
		ObjectState state;

		trimSignalTextFields(*s);

		dbController()->setSignalWorkcopy(s, &state, nullptr);

		states.append(state);

		m_signalSet.updateSignal(*s);
	}

	showErrors(states);
}

std::vector<int> AppSignalSetProvider::cloneSignals(const SignalIDsSet& signalIDsToClone)
{
	std::vector<int> resultSignalIDs;

	SignalIDsSet clonedSignalIDs;

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
		SignalIDsSet groupSignalIDs;

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
			trimSignalTextFields(signalToCreate);

			signalToCreate.setAppSignalID(groupSignal->appSignalID() + cloneSuffix);
			signalToCreate.setCustomAppSignalID(groupSignal->customAppSignalID() + cloneSuffix);

			i++;
		}

		dbController()->addSignal(type, &signalsToCreate, nullptr);

		for (const AppSignal& s : signalsToCreate)
		{
			resultSignalIDs.push_back(s.ID());
		}
	}

	reloadSignals();

	return resultSignalIDs;
}

/*
void AppSignalSetProvider::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		m_propertyManager.clear();
		m_signalSet.clear();
		emit signalCountChanged();
	}
}*/

