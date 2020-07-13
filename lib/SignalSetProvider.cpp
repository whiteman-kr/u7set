#include "SignalSetProvider.h"
#include "DbController.h"

SignalSetProvider::SignalSetProvider(DbController* dbController, QWidget* parentWidget) :
	m_dbController(dbController),
	m_parentWidget(parentWidget)

{

}


Signal* SignalSetProvider::getSignalByStrID(const QString signalStrID)
{
	if (m_signalSet.ID2IndexMapIsEmpty())
	{
		m_signalSet.buildID2IndexMap();
	}
	return m_signalSet.getSignal(signalStrID);
}

QVector<int> SignalSetProvider::getSameChannelSignals(int index)
{
	QVector<int> sameChannelSignalRows;
	if (m_signalSet[index].signalGroupID() != 0)
	{
		QVector<int> sameChannelSignalIDs = m_signalSet.getChannelSignalsID(m_signalSet[index].signalGroupID());
		foreach (const int id, sameChannelSignalIDs)
		{
			sameChannelSignalRows.append(m_signalSet.keyIndex(id));
		}
	}
	else
	{
		sameChannelSignalRows.append(index);
	}
	return sameChannelSignalRows;
}

void SignalSetProvider::loadUsers()
{
	std::vector<DbUser> list;
	m_dbController->getUserList(&list, m_parentWidget);

	m_usernameMap.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		m_usernameMap[list[i].userId()] = list[i].username();
	}
}

bool SignalSetProvider::isEditableSignal(const Signal& signal) const
{
	if (signal.checkedOut() && (signal.userID() != m_dbController->currentUser().userId() && !m_dbController->currentUser().isAdminstrator()))
	{
		return false;
	}
	return true;
}

QString SignalSetProvider::getUserStr(int userId) const
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

bool SignalSetProvider::checkoutSignal(int index)
{
	Signal& s = m_signalSet[index];
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
	m_dbController->checkoutSignals(&signalsIDs, &objectStates, m_parentWidget);
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
	emit hasCheckedOutSignals(true);
	return true;
}

// Converts ObjectState to human readable message
//
QString SignalSetProvider::errorMessage(const ObjectState& state)
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
void SignalSetProvider::showError(const ObjectState& state)
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
void SignalSetProvider::showErrors(const QVector<ObjectState>& states)
{
	QString message;

	foreach (const ObjectState& state, states)
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


void SignalSetProvider::initLazyLoadSignals()
{
	m_partialLoading = true;

	loadUsers();
	emit usersLoaded();

	QVector<ID_AppSignalID> signalIds;
	dbController()->getSignalsIDAppSignalID(&signalIds, m_parentWidget);

	for (const ID_AppSignalID& id : signalIds)
	{
		m_signalSet.replaceOrAppendIfNotExists(id.ID, Signal(id));
	}

	emit signalCountChanged();
}

void SignalSetProvider::finishLoadSignals()
{
	if (m_partialLoading == true)
	{
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
			QVector<Signal> signalsToLoad;
			signalsToLoad.reserve(signalIds.count());

			dbController()->getLatestSignals(signalIds, &signalsToLoad, m_parentWidget);

			for (const Signal& loadedSignal: signalsToLoad)
			{
				m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

				emit signalUpdated(loadedSignal);
			}
		}
	}

	m_partialLoading = false;
}

void SignalSetProvider::loadNextSignalsPortion(int middlePosition)
{
	QVector<int> signalIds;
	signalIds.reserve(250);
	int low = middlePosition - 1;
	int high = middlePosition;

	if (middlePosition == -1)
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
		QVector<Signal> signalsToLoad;
		signalsToLoad.reserve(signalIds.count());

		dbController()->getLatestSignalsWithoutProgress(signalIds, &signalsToLoad, m_parentWidget);

		for (const Signal& loadedSignal: signalsToLoad)
		{
			m_signalSet.replaceOrAppendIfNotExists(loadedSignal.ID(), loadedSignal);

			signalUpdated(loadedSignal);
		}
	}
	else
	{
		m_partialLoading = false;
	}
}

bool SignalSetProvider::checkoutSignal(int index, QString& message)
{
	Signal& s = m_signalSet[index];
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
	dbController()->checkoutSignals(&signalsIDs, &objectStates, m_parentWidget);
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


bool SignalSetProvider::undoSignal(int id)
{
	const Signal& s = m_signalSet[m_signalSet.keyIndex(id)];
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
		dbController()->undoSignalChanges(signalId, &state, m_parentWidget);
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

void SignalSetProvider::deleteSignal(int signalID)
{
	ObjectState state;
	dbController()->deleteSignal(signalID, &state, m_parentWidget);
	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}
}

void SignalSetProvider::deleteSignals(const QSet<int>& signalIDs)
{
	for (const int signalID : signalIDs)
	{
		deleteSignal(signalID);
	}
	loadSignals();
}

void SignalSetProvider::loadSignalSet(QVector<int> keys)
{
	for (int i = 0; i < keys.count(); i++)
	{
		loadSignal(keys[i]);
	}
}

void SignalSetProvider::loadSignal(int signalId)
{
	int index = keyIndex(signalId);
	if (index == -1)
	{
		return;
	}
	dbController()->getLatestSignal(signalId, &m_signalSet[index], m_parentWidget);

	signalUpdated(index);
	signalUpdated(signal(index));
}

void SignalSetProvider::loadSignals()
{
	clearSignals();

	loadUsers();

	SignalSet temporarySignalSet;
	if (!dbController()->getSignals(&temporarySignalSet, false, m_parentWidget))
	{
		emit error(tr("Could not load signals"));
	}
}

void SignalSetProvider::saveSignal(Signal& signal)
{
	ObjectState state;
	trimSignalTextFields(signal);

	dbController()->setSignalWorkcopy(&signal, &state, m_parentWidget);

	if (state.errCode != ERR_SIGNAL_OK)
	{
		showError(state);
	}

	loadSignal(signal.ID());
}

QVector<int> SignalSetProvider::cloneSignals(const QSet<int>& signalIDs)
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

		const Signal&& signal = m_signalSet.value(signalID);
		E::SignalType type = signal.signalType();
		QVector<int> groupSignalIDs;

		if (signal.signalGroupID() == 0)
		{
			groupSignalIDs.append(signal.ID());
		}
		else
		{
			groupSignalIDs = m_signalSet.getChannelSignalsID(signal);
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

		QVector<Signal> groupSignals(groupSignalIDs.count());
		for (int i = 0; i < groupSignalIDs.count(); i++)
		{
			const Signal&& groupSignal = m_signalSet.value(groupSignalIDs[i]);
			groupSignals[i] = groupSignal;
			trimSignalTextFields(groupSignals[i]);

			groupSignals[i].setAppSignalID(groupSignal.appSignalID() + suffix);
			groupSignals[i].setCustomAppSignalID(groupSignal.customAppSignalID() + suffix);
		}

		dbController()->addSignal(type, &groupSignals, m_parentWidget);

		int prevSize = resultSignalIDs.size();
		resultSignalIDs.resize(prevSize + groupSignals.count());

		for (int i = 0; i < groupSignals.count(); i++)
		{
			resultSignalIDs[prevSize + i] = groupSignals[i].ID();
		}
	}
	loadSignals();
	return resultSignalIDs;
}

void SignalSetProvider::clearSignals()
{
	if (m_signalSet.count() != 0)
	{
		m_signalSet.clear();
		emit signalCountChanged();
	}
}

void SignalSetProvider::trimSignalTextFields(Signal& signal)
{
	signal.setAppSignalID(signal.appSignalID().trimmed());
	signal.setCustomAppSignalID(signal.customAppSignalID().trimmed());
	signal.setEquipmentID(signal.equipmentID().trimmed());
	signal.setBusTypeID(signal.busTypeID().trimmed());
	signal.setCaption(signal.caption().trimmed());
	signal.setUnit(signal.unit().trimmed());
}
