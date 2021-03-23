#include "TuningSignalBase.h"
#include "../Proto/network.pb.h"
#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource()
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource(const Network::DataSourceInfo& info) :
	m_sourceID (info.id()),
	m_equipmentID (QString::fromStdString(info.lmequipmentid())),
	m_caption (QString::fromStdString(info.lmcaption())),
	m_serverIP (QString::fromStdString(info.lmip())),
	m_serverPort (info.lmport()),
	m_channel (QString::fromStdString(info.lmsubsystemchannel())),
    m_subSystemID (QString::fromStdString(info.lmsubsystemid())),
	m_lmNumber (info.lmnumber())
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSourceBase::TuningSourceBase(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceBase::~TuningSourceBase()
{
}

 // -------------------------------------------------------------------------------------------------------------------

void TuningSourceBase::clear()
{
	QMutexLocker l(&m_sourceMutex);

	m_tuningSourceEquipmentID.clear();

	m_sourceList.clear();
	m_sourceIdMap.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceBase::count() const
{
	QMutexLocker l(&m_sourceMutex);

	return TO_INT(m_sourceList.size());
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSourceBase::append(const TuningSource& source)
{
	QMutexLocker l(&m_sourceMutex);

	if (m_sourceIdMap.contains(source.sourceID()) == true)
	{
		return -1;
	}

	m_sourceList.push_back(source);
	int index = TO_INT(m_sourceList.size() - 1);

	m_sourceIdMap[source.sourceID()] = index;

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource TuningSourceBase::source(int index) const
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return TuningSource();
	}

	return m_sourceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceState TuningSourceBase::state(quint64 sourceID)
{
	QMutexLocker l(&m_sourceMutex);

	if (m_sourceIdMap.contains(sourceID) == false)
	{
		return TuningSourceState();
	}

	int index = m_sourceIdMap[sourceID];

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return TuningSourceState();
	}

	return m_sourceList[static_cast<quint64>(index)].state();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceBase::setState(quint64 sourceID, const Network::TuningSourceState& state)
{
	QMutexLocker l(&m_sourceMutex);

	if (m_sourceIdMap.contains(sourceID) == false)
	{
		return;
	}

	int index = m_sourceIdMap[sourceID];

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return;
	}

	TuningSourceState& sourceState = m_sourceList[static_cast<quint64>(index)].state();

	sourceState.setIsReply(state.isreply());
	sourceState.setRequestCount(static_cast<quint64>(state.requestcount()));
	sourceState.setReplyCount(static_cast<quint64>(state.replycount()));
	sourceState.setCommandQueueSize(state.commandqueuesize());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSourceBase::sortByID()
{
	std::sort(m_sourceList.begin(), m_sourceList.end(),
			[](const TuningSource& ts1, const TuningSource& ts2) -> bool
			{
				return ts1.equipmentID() < ts2.equipmentID();
			});
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSignalBase::TuningSignalBase(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalBase::~TuningSignalBase()
{
}

 // -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::clear()
{
	QMutexLocker l(&m_signalMutex);

	m_signalList.clear();
	m_signalHashMap.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::count() const
{
	QMutexLocker l(&m_signalMutex);

	return TO_INT(m_signalList.size());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::createSignalList()
{
	int count = theSignalBase.signalCount();
	for(int i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = theSignalBase.signalPtr(i);
		if (pSignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pSignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.enableTuning() == false)
		{
			continue;
		}

		append(pSignal);
	}

	emit signalsCreated();
}


// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::append(Metrology::Signal* pSignal)
{
	if (pSignal == nullptr)
	{
		return -1;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false || param.enableTuning() == false)
	{
		return -1;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(param.hash()) == true)
	{
		return -1;
	}

	m_signalList.push_back(pSignal);
	int index = TO_INT(m_signalList.size() - 1);

	m_signalHashMap[param.hash()] = index;

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* TuningSignalBase::signal(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return nullptr;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(hash) == false)
	{
		return nullptr;
	}
	int index = m_signalHashMap[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	return m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* TuningSignalBase::signal(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	return m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState TuningSignalBase::state(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return Metrology::SignalState();
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(hash) == false)
	{
		return Metrology::SignalState();
	}

	int index = m_signalHashMap[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::SignalState();
	}

	Metrology::Signal* pSignal = m_signalList[static_cast<quint64>(index)];
	if (pSignal == nullptr)
	{
		return Metrology::SignalState();
	}

	return pSignal->state();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::setState(const Network::TuningSignalState& state)
{
	if (state.signalhash() == UNDEFINED_HASH)
	{
		assert(state.signalhash() != UNDEFINED_HASH);
		return;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashMap.contains(state.signalhash()) == false)
	{
		return;
	}

	int index = m_signalHashMap[state.signalhash()];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	Metrology::Signal* pSignal = m_signalList[static_cast<quint64>(index)];
	if (pSignal == nullptr)
	{
		return;
	}

	pSignal->state().setValid(state.valid());

	pSignal->state().setValue(TuningSignalState(state).toDouble());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::setNovalid()
{
	QMutexLocker l(&m_signalMutex);

	quint64 count = m_signalList.size();

	for(quint64 i = 0; i < count; i++)
	{
		Metrology::Signal* pSignal = m_signalList[i];
		if (pSignal == nullptr)
		{
			continue;
		}

		pSignal->state().setValid(false);
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningBase::TuningBase(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningBase::~TuningBase()
{
}

void TuningBase::clear()
{
	m_sourceBase.clear();
	m_signalsBase.clear();

	QMutexLocker l(&m_cmdFowWriteMutex);

	m_cmdFowWriteList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningBase::cmdFowWriteCount() const
{
	QMutexLocker l(&m_cmdFowWriteMutex);

	return TO_INT(m_cmdFowWriteList.size());
}

// -------------------------------------------------------------------------------------------------------------------

void TuningBase::appendCmdFowWrite(const TuningWriteCmd& cmd)
{
	if (cmd.signalHash() == UNDEFINED_HASH)
	{
		assert(cmd.signalHash() != UNDEFINED_HASH);
		return;
	}

	QMutexLocker l(&m_cmdFowWriteMutex);

	m_cmdFowWriteList.push_back(cmd);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningBase::appendCmdFowWrite(const Hash& signalHash, TuningValueType type, QVariant value)
{
	if (signalHash == UNDEFINED_HASH)
	{
		assert(signalHash != UNDEFINED_HASH);
		return;
	}

	QMutexLocker l(&m_cmdFowWriteMutex);

	TuningWriteCmd cmd;

	cmd.setSignalHash(signalHash);
	cmd.setType(type);
	cmd.setValue(value);

	m_cmdFowWriteList.push_back(cmd);
}

// -------------------------------------------------------------------------------------------------------------------

TuningWriteCmd TuningBase::cmdFowWrite()
{
	QMutexLocker l(&m_cmdFowWriteMutex);

	if (m_cmdFowWriteList.size() == 0)
	{
		return TuningWriteCmd();
	}

	auto firstCmd = m_cmdFowWriteList.begin();

	TuningWriteCmd cmd = *firstCmd;

	m_cmdFowWriteList.erase(firstCmd);

	return cmd;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------



