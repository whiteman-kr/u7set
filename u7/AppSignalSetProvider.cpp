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

void AppSignalSetProvider::setMiddleVisibleSignalIndex(int signalIndex)
{
	m_middleVisibleSignalIndex = signalIndex;
}


AppSignal* AppSignalSetProvider::getSignalByStrID(const QString& signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
}

const AppSignal& AppSignalSetProvider::getLoadedSignal(int index)
{
	if (m_signalSet[index].isLoaded() == false)
	{
		int oldIndex = m_middleVisibleSignalIndex;
		m_middleVisibleSignalIndex = index;

		loadNextSignalsPortion();	// force loading this signal

		m_middleVisibleSignalIndex = oldIndex;

	}
	return m_signalSet[index];
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(int index)
{
	AppSignal signal = getLoadedSignal(index);
	signal.cacheSpecPropValues();

	AppSignalParam param;
	param.load(signal);

	return param;
}

AppSignalParam AppSignalSetProvider::getAppSignalParam(const QString& appSignalId)
{
	AppSignalParam param;

	AppSignal* signal = getSignalByStrID(appSignalId);
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

	return getAppSignalParam(static_cast<int>(m_signalSet.keyIndex(signal->ID())));
}

QVector<int> AppSignalSetProvider::getSameChannelSignals(int index)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(static_cast<int>(m_signalSet.keyIndex(id)));
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

bool AppSignalSetProvider::isEditableSignal(const AppSignal& signal) const
{
	if (!signal.checkedOut() || (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

bool AppSignalSetProvider::isCheckinableSignalForMe(const AppSignal& signal) const
{
	if (signal.checkedOut() && (signal.userID() == m_dbController->currentUser().userId() || m_dbController->currentUser().isAdminstrator()))
	{
		return true;
	}
	return false;
}

QString AppSignalSetProvider::getUserStr(int userId) const
{
	if (m_usernameMap.contains(userId))
	{
		return m_usernameMap[userId];
	}
	else
	{
		return "";
	}
}

bool AppSignalSetProvider::checkoutSignal(int index)
{
	AppSignal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == m_dbController->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	m_dbController->checkoutSignals(&signalsIDs, &objectStates, nullptr);
	if (objectStates.count() == 0)
	{
		return false;
	}
	showErrors(objectStates);
	for (const ObjectState& objectState : objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
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
		m_signalSet.replaceOrAppendIfNotExists(id.ID, AppSignal(id));
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
		for (int i = 0; i < m_signalSet.count(); i++)
		{
			if (m_signalSet[i].isLoaded() == false)
			{
				signalIds.push_back(m_signalSet.key(i));
			}
		}

		if (signalIds.count() > 0)
		{
			QVector<AppSignal> signalsToLoad;
			signalsToLoad.reserve(signalIds.count());

			dbController()->getLatestSignals(signalIds, &signalsToLoad, nullptr);

			for (const AppSignal& loadedSignal: signalsToLoad)
			{
				m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

				emit signalUpdated(keyIndex(loadedSignal.ID()));
				emit signalPropertiesChanged(loadedSignal);
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

	while ((low >= 0 || high < signalCount()) && signalIds.count() <= 248)
	{
		while (low >= 0 && m_signalSet[low].isLoaded() == true)
		{
			low--;
		}

		if (low >= 0)
		{
			signalIds.push_back(m_signalSet.key(low));
			low--;
		}

		while (high < signalCount() && m_signalSet[high].isLoaded() == true)
		{
			high++;
		}

		if (high < signalCount())
		{
			signalIds.push_back(m_signalSet.key(high));
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
			m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);
		}

		for (const AppSignal& loadedSignal : signalsToLoad)
		{
			emit signalUpdated(keyIndex(loadedSignal.ID()));
			emit signalPropertiesChanged(loadedSignal);
		}
	}
	else
	{
		m_partialLoading = false;
	}
}

bool AppSignalSetProvider::checkoutSignal(int index, QString& message)
{
	AppSignal& s = m_signalSet[index];
	if (s.checkedOut())
	{
		if (s.userID() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	QVector<int> signalsIDs;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
	}
	else
	{
		signalsIDs << m_signalSet.key(index);
	}
	QVector<ObjectState> objectStates;
	dbController()->checkoutSignals(&signalsIDs, &objectStates, nullptr);
	if (objectStates.count() == 0)
	{
		return false;
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode != ERR_SIGNAL_OK)
		{
			message += errorMessage(objectState) + "\n";
		}
	}
	foreach (const ObjectState& objectState, objectStates)
	{
		if (objectState.errCode == ERR_SIGNAL_CHECKED_OUT_BY_ANOTHER_USER
				&& objectState.userId != dbController()->currentUser().userId() && !dbController()->currentUser().isAdminstrator())
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


bool AppSignalSetProvider::undoSignal(int id)
{
	const AppSignal& s = m_signalSet[m_signalSet.keyIndex(id)];
	if (!s.checkedOut())
	{
		return false;
	}

	QVector<int> signalsIDs;
	if (s.signalGroupID() != 0)
	{
		signalsIDs = m_signalSet.getChannelSignalsID(s.signalGroupID());
	}
	else
	{
		signalsIDs << id;
	}
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
	m_signalSet.replaceOrAppendIfNotExists(signal.ID(), signal);
}

void AppSignalSetProvider::deleteSignals(const QSet<int>& signalIDs)
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
	int index = keyIndex(signalId);

	if (index == -1)
	{
		return nullptr;
	}

	dbController()->getLatestSignal(signalId, &m_signalSet[index], nullptr);
	m_signalSet.updateID2IndexInMap(m_signalSet[index].appSignalID(), index);

	emit signalUpdated(index);
	emit signalPropertiesChanged(getLoadedSignal(index));

	return &m_signalSet[index];
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

	for (int i = 0; i < signalSetForReplacement.count(); i++)
	{
		m_propertyManager.detectNewProperties(signalSetForReplacement[i]);
	}

	m_signalSet.clear();

	m_signalSet = std::move(signalSetForReplacement);
	signalSetForReplacement.forget();	// Destructor will delete all Signal pointers which should be kept for m_signalSet

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

QVector<int> AppSignalSetProvider::cloneSignals(const QSet<int>& signalIDs)
{
	QVector<int> resultSignalIDs;
	m_signalSet.buildID2IndexMap();

	QSet<int> clonedSignalIDs;
	QList<int> signalIDsList = signalIDs.values();
	std::sort(signalIDsList.begin(), signalIDsList.end());
	for (const int signalID : signalIDsList)
	{
		if (clonedSignalIDs.contains(signalID))
		{
			continue;
		}

		const AppSignal&& signal = m_signalSet.value(signalID);
		E::SignalType type = signal.signalType();
		QVector<int> groupSignalIDs;

		if (signal.signalGroupID() == 0)
		{
			groupSignalIDs.append(signal.ID());
		}
		else
		{
			groupSignalIDs = m_signalSet.getChannelSignalsID(signal);

			if (groupSignalIDs.size() == 0)
			{
				Q_ASSERT(false);
				continue;
			}
		}
		std::sort(groupSignalIDs.begin(), groupSignalIDs.end());

		for (int groupSignalID : groupSignalIDs)
		{
			clonedSignalIDs.insert(groupSignalID);
		}

		QString suffix = "_CLONE";
		int suffixNumerator = 1;
		bool hasConflict;
		do
		{
			hasConflict = false;
			for (int groupSignalID : groupSignalIDs)
			{
				if (m_signalSet.contains(m_signalSet.value(groupSignalID).appSignalID() + suffix))
				{
					hasConflict = true;
					break;
				}
			}
			if (hasConflict)
			{
				suffixNumerator++;
				suffix = QString("_CLONE%1").arg(suffixNumerator);
			}
		}
		while (hasConflict && suffixNumerator < 1000);

		if (suffixNumerator >= 1000)
		{
			assert(false);
			return QVector<int>();
		}

		QVector<AppSignal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const AppSignal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;
			trimSignalTextFields(groupSignals[i]);

			groupSignals[i].setAppSignalID(groupSignal.appSignalID() + suffix);
			groupSignals[i].setCustomAppSignalID(groupSignal.customAppSignalID() + suffix);
		}

		dbController()->addSignal(type, &groupSignals, nullptr);

		qsizetype prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + groupSignals.count());

		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs[prevSize + i] = groupSignals[i].ID();
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
