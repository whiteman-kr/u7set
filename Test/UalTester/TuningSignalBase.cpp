#include "TuningSignalBase.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSource class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource()
{
}

TuningSource::TuningSource(const Network::DataSourceInfo& info) :
	m_sourceID (info.id()),
	m_equipmentID (QString::fromStdString(info.lmequipmentid())),
	m_caption (QString::fromStdString(info.lmcaption())),
	m_serverIP (QString::fromStdString(info.lmip())),
	m_serverPort (info.lmport()),
	m_channel (QString::fromStdString(info.lmsubsystemchannel())),
	m_subSystem (QString::fromStdString(info.lmsubsystem())),
	m_lmNumber (info.lmnumber())
{

}

TuningSource::~TuningSource()
{
}

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSourceBase class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSourceBase::TuningSourceBase(QObject *parent) :
	QObject(parent)
{
}

TuningSourceBase::~TuningSourceBase()
{
}

void TuningSourceBase::clear()
{
	m_sourceMutex.lock();

		m_tuningSourceEquipmentID.clear();

		m_sourceList.clear();
		m_sourceIdMap.clear();

	m_sourceMutex.unlock();
}

int TuningSourceBase::count() const
{
	int count = 0;

	m_sourceMutex.lock();

		count = m_sourceList.count();

	m_sourceMutex.unlock();

	return count;
}

int TuningSourceBase::append(const TuningSource& source)
{
	int index = -1;

	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(source.sourceID()) == false)
		{
			m_sourceList.append(source);

			index = m_sourceList.count() - 1;

			m_sourceIdMap[source.sourceID() ] = index;
		}

	m_sourceMutex.unlock();

	return index;
}

TuningSource TuningSourceBase::source(int index) const
{
	TuningSource source;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			source = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return source;
}

TuningSourceState TuningSourceBase::state(quint64 sourceID)
{
	TuningSourceState state;

	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(sourceID) == true)
		{
			int index = m_sourceIdMap[sourceID];

			if (index >= 0 && index < m_sourceList.count())
			{
				state = m_sourceList[index].state();
			}
		}

	m_sourceMutex.unlock();

	return state;
}

void TuningSourceBase::setState(quint64 sourceID, const Network::TuningSourceState& state)
{
	m_sourceMutex.lock();

		if (m_sourceIdMap.contains(sourceID) == true)
		{
			int index = m_sourceIdMap[sourceID];

			if (index >= 0 && index < m_sourceList.count())
			{
				TuningSourceState& sourceState = m_sourceList[index].state();

				sourceState.setIsReply(state.isreply());
				sourceState.setRequestCount(state.requestcount());
				sourceState.setReplyCount(state.replycount());
				sourceState.setCommandQueueSize(state.commandqueuesize());
			}
		}

	m_sourceMutex.unlock();
}

void TuningSourceBase::sortByID()
{
	std::sort(m_sourceList.begin(), m_sourceList.end(),
			[](const TuningSource& ts1, const TuningSource& ts2) -> bool
			{
				return ts1.equipmentID() < ts2.equipmentID();
			});
}

// -------------------------------------------------------------------------------------------------------------------
//
// TuningSignalBase class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningSignalBase::TuningSignalBase(QObject *parent) :
	QObject(parent)
{
}

TuningSignalBase::~TuningSignalBase()
{
}

void TuningSignalBase::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();
		m_signalHashMap.clear();

	m_signalMutex.unlock();
}

int TuningSignalBase::count() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

int TuningSignalBase::append(TestSignal* pSignal)
{
	if (pSignal == nullptr)
	{
		return -1;
	}

	Signal& param = pSignal->param();

	if (param.appSignalID().isEmpty() == true || param.hash() == 0)
	{
		return -1;
	}

	if (param.enableTuning() == false)
	{
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(param.hash()) == false)
		{
			m_signalList.append(pSignal);
			index = m_signalList.count() - 1;

			m_signalHashMap[param.hash()] = index;
		}

	m_signalMutex.unlock();

	return index;
}

TestSignal* TuningSignalBase::signal(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return signal(calcHash(appSignalID));
}

TestSignal* TuningSignalBase::signal(const Hash& hash) const
{
	if (hash == 0)
	{
		assert(hash != 0);
		return nullptr;
	}

	TestSignal* pSignal = nullptr;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				pSignal = m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return pSignal;
}

TestSignal* TuningSignalBase::signal(int index) const
{
	TestSignal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

AppSignalState TuningSignalBase::state(const Hash& hash)
{
	if (hash == 0)
	{
		assert(hash != 0);
		return AppSignalState();
	}

	AppSignalState state;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				TestSignal* pSignal = m_signalList[index];
				if (pSignal != nullptr)
				{
					state = pSignal->state();
				}
			}
		}

	m_signalMutex.unlock();

	return state;
}

void TuningSignalBase::setState(const Network::TuningSignalState& state)
{
	if (state.signalhash() == 0)
	{
		assert(state.signalhash() != 0);
		return;
	}

	m_signalMutex.lock();

		if (m_signalHashMap.contains(state.signalhash()) == true)
		{
			int index = m_signalHashMap[state.signalhash()];

			if (index >= 0 && index < m_signalList.count())
			{
				TestSignal* pSignal = m_signalList[index];

				if (pSignal != nullptr)
				{
					//pSignal->state().setValid(state.valid());
					pSignal->state().m_flags.valid = state.valid();

					//pSignal->state().setValue(TuningSignalState(state).toDouble());
					pSignal->state().m_value = TuningSignalState(state).toDouble();

				}
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
//
// TuningBase class implementation
//
// -------------------------------------------------------------------------------------------------------------------

TuningBase::TuningBase(QObject *parent) :
	QObject(parent)
{
}

TuningBase::~TuningBase()
{
}

void TuningBase::clear()
{
	m_sourceBase.clear();
	m_signalsBase.clear();

	m_cmdFowWriteMutex.lock();

		m_cmdFowWriteList.clear();

	m_cmdFowWriteMutex.unlock();
}

int TuningBase::cmdFowWriteCount() const
{
	int count = 0;

	m_cmdFowWriteMutex.lock();

		count = m_cmdFowWriteList.count();

	m_cmdFowWriteMutex.unlock();

	return count;
}

void TuningBase::appendCmdFowWrite(const TuningWriteCmd& cmd)
{
	if (cmd.signalHash() == 0)
	{
		assert(cmd.signalHash() != 0);
		return;
	}

	m_cmdFowWriteMutex.lock();

		m_cmdFowWriteList.append(cmd);

	m_cmdFowWriteMutex.unlock();
}

void TuningBase::appendCmdFowWrite(const Hash& signalHash, TuningValueType type, QVariant value)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	m_cmdFowWriteMutex.lock();

		TuningWriteCmd cmd;

		cmd.setSignalHash(signalHash);
		cmd.setType(type);
		cmd.setValue(value);

		m_cmdFowWriteList.append(cmd);

	m_cmdFowWriteMutex.unlock();
}

TuningWriteCmd TuningBase::cmdFowWrite()
{
	TuningWriteCmd cmd;

	m_cmdFowWriteMutex.lock();

		if (m_cmdFowWriteList.isEmpty() == false)
		{
			cmd = m_cmdFowWriteList[0];

			m_cmdFowWriteList.remove(0);
		}

	m_cmdFowWriteMutex.unlock();

	return cmd;
}
