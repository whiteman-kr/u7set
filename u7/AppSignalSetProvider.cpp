#include "AppSignalSetProvider.h"

AppSignalSetProvider* AppSignalSetProvider::m_instance = nullptr;

AppSignalSetProvider::AppSignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	QObject(parentWidget),
	m_dbController(dbController),
	m_propertyManager(dbController, parentWidget)
{
	assert(m_instance == nullptr);
	m_instance = this;

	connect(this, &AppSignalSetProvider::signalPropertiesChanged, &m_propertyManager, &AppSignalPropertyManager::detectNewProperties);
}

AppSignalSetProvider::~AppSignalSetProvider()
{
	if (m_lazyLoadSignalsTimer != nullptr)
	{
		m_lazyLoadSignalsTimer->stop();
		delete m_lazyLoadSignalsTimer;
	}
}

AppSignalSetProvider* AppSignalSetProvider::getInstance()
{
	assert(m_instance != nullptr);
	return m_instance;
}

const AppSignalSet& AppSignalSetProvider::signalSet() const
{
	return m_signalSet;
}

AppSignalPropertyManager& AppSignalSetProvider::signalPropertyManager()
{
	return m_propertyManager;
}

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
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

AppSignal* AppSignalSetProvider::getLoadedSignal(int index)
{
	AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_NULLPTR(s);

	if (s->isLoaded() == false)
	{
		int oldIndex = m_middleVisibleSignalIndex;
		m_middleVisibleSignalIndex = index;

		loadNextSignalsPortion();	// force loading this signal

		m_middleVisibleSignalIndex = oldIndex;
	}

	return s;
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

void AppSignalSetProvider::loadUsers()
{
	std::vector<DbUser> list;
	m_dbController->getUserList(&list, nullptr);

	m_usernameMap.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}
}

bool AppSignalSetProvider::isEditableSignal(const AppSignal* signal) const
{
	TEST_PTR_RETURN_FALSE(signal);

	if (!signal->checkedOut() ||
		(signal->userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}

	return false;
}

bool AppSignalSetProvider::isCheckinableSignalForMe(const AppSignal* signal) const
{
	TEST_PTR_RETURN_FALSE(signal);

	if (signal->checkedOut() &&
		(signal->userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}

	return false;
}

QString AppSignalSetProvider::getUserName(int userId) const
{
	auto it = m_usernameMap.find(userId);

	if (it != m_usernameMap.end())
	{
		return it->second;
	}

	return QString();
}

bool AppSignalSetProvider::checkoutSignal(int index, QString* message)
{
	AppSignal* s = m_signalSet.at(index);

	TEST_PTR_RETURN_FALSE(s);

	if (s->checkedOut() == true)
	{
		if (s->userID() == m_dbController->currentUser().userId() ||
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

	m_dbController->checkoutSignals(signalsIDs, &objectStates, nullptr);

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

AppSignal* AppSignalSetProvider::privateGetLoadedSignal(AppSignal* signal)
{
	TEST_PTR_RETURN_NULLPTR(signal);

	if (signal->isLoaded() == false)
	{
		loadSignal(signal->ID());
	}

	return signal;
}

// Converts ObjectState to human readable message
//
QString AppSignalSetProvider::errorMessage(const ObjectState& state)
{
	switch(state.errCode)
	{
		case ERR_SIGNAL_IS_NOT_CHECKED_OUT: return tr("Signal %1 is not checked out").arg(state.id);
		case ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER: return tr("Signal %1 is checked out by \"%2\"").arg(state.id).arg(m_usernameMap[state.userId]);
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
	m_partialLoading = true;

	if (m_lazyLoadSignalsTimer == nullptr)
	{
		m_lazyLoadSignalsTimer = new QTimer(this);
		connect(m_lazyLoadSignalsTimer, &QTimer::timeout, this, &AppSignalSetProvider::loadNextSignalsPortion);
	}

	m_lazyLoadSignalsTimer->start(100);
}

void AppSignalSetProvider::stopLoadingSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();
		m_partialLoading = false;
	}
}

void AppSignalSetProvider::finishLoadingSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();

		QVector<int> signalIds;

		for (const AppSignal* s : m_signalSet)
		{
			if (s->isLoaded() == false)
			{
				signalIds.push_back(s->ID());
			}
		}

		if (signalIds.size() > 0)
		{
			QVector<AppSignal> signalsToLoad;
			signalsToLoad.reserve(signalIds.size());

			dbController()->getLatestSignals(signalIds, &signalsToLoad, nullptr);

			for (const AppSignal& loadedSignal: signalsToLoad)
			{
				AppSignal* s = m_signalSet.replaceOrAppendIfNotExists(loadedSignal);

				emit signalUpdated(signalIndex(s->ID()));
				emit signalPropertiesChanged(*s);
			}
		}
	}

	m_partialLoading = false;
}

void AppSignalSetProvider::loadNextSignalsPortion()
{
	if (m_partialLoading == false)
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
			AppSignal* s = m_signalSet.replaceOrAppendIfNotExists(loadedSignal);

			emit signalUpdated(signalIndex(s->ID()));
			emit signalPropertiesChanged(*s);
		}
	}
	else
	{
		m_partialLoading = false;
	}
}

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
	AppSignal* s = m_signalSet.replaceOrAppendIfNotExists(signal);
	Q_UNUSED(s);
}

void AppSignalSetProvider::deleteSignals(const SignalIDsSet& signalIDs)
{
	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}

	loadSignals();
}

void AppSignalSetProvider::loadSignalSet(QVector<int> keys)
{
	for (int i = 0; i < keys.count(); i++)
	{
		loadSignal(keys[i]);
	}
}

const AppSignal* AppSignalSetProvider::loadSignal(int signalId)
{
	AppSignal* signalToLoad = m_signalSet.getSignal(signalId);

	TEST_PTR_RETURN_NULLPTR(signalToLoad)

	QString oldAppSignalID = signalToLoad->appSignalID();

	dbController()->getLatestSignal(signalId, signalToLoad, nullptr);

	m_signalSet.updateMaps(oldAppSignalID, signalToLoad);

	int index = m_signalSet.signalIndex(signalId);

	emit signalUpdated(index);
	emit signalPropertiesChanged(*getLoadedSignal(index));

	return signalToLoad;
}

void AppSignalSetProvider::loadSignals()
{
	if (m_partialLoading == true)
	{
		m_lazyLoadSignalsTimer->stop();
		m_partialLoading = false;
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
}

void AppSignalSetProvider::saveSignal(AppSignal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, nullptr);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	loadSignal(signal.ID());
}

void AppSignalSetProvider::saveSignals(QVector<AppSignal*> signalVector)
{
	QVector<ObjectState> states;
	for (int i = 0; i < signalVector.count(); i++)
	{
		ObjectState state;
		trimSignalTextFields(*signalVector[i]);

		dbController()->setSignalWorkcopy(signalVector[i], &state, nullptr);
		states.append(state);

		loadSignal(signalVector[i]->ID());
	}
	showErrors(states);
}

QVector<int> AppSignalSetProvider::cloneSignals(const SignalIDsSet& signalIDsToClone)
{
	QVector<int> resultSignalIDs;

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
			return QVector<int>();
		}

		QVector<AppSignal> signalsToCreate(groupSignalIDs.size());

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

		qsizetype prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + signalsToCreate.count());

		for (int i = 0; i < signalsToCreate.count(); i++)
		{
			resultSignalIDs[prevSize + i] = signalsToCreate[i].ID();
		}
	}

	loadSignals();

	return resultSignalIDs;
}

void AppSignalSetProvider::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		m_propertyManager.clear();
		m_signalSet.clear();
		emit signalCountChanged();
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
