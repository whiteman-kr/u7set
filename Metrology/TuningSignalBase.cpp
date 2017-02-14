#include "TuningSignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource()
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource::TuningSource(const Network::DataSourceInfo& info) :
    m_sourceID (info.id() ),
    m_equipmentID (info.equipmentid().c_str() ),
    m_caption (info.caption().c_str() ),
    m_serverIP (info.ip().c_str() ),
    m_serverPort (info.port() ),
    m_channel (info.channel() ),
    m_subSystem (info.subsystem().c_str() ),
    m_lmNumber (info.lmnumber() )
{

}

// -------------------------------------------------------------------------------------------------------------------

TuningSource::~TuningSource()
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSignal::TuningSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal::TuningSignal(const TuningSignal& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal::TuningSignal(const SignalParam& param)
{
    if (param.isValid() == false)
    {
        assert(false);
        return;
    }

    m_hash = param.hash();

    m_appSignalID = param.appSignalID();
    m_customAppSignalID = param.customAppSignalID();
    m_equipmentID = param.position().equipmentID();
    m_caption = param.caption();

    m_signalType = param.signalType();

    m_defaultValue = param.tuningDefaultValue();

    m_state.setLowLimit( param.inputPhysicalLowLimit() );
    m_state.setHighLimit( param.inputPhysicalHighLimit() );

    m_precision = param.inputPhysicalPrecision();
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal::~TuningSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignal::valueStr() const
{
    if (m_hash == 0)
    {
        return QString();
    }

    if (m_state.valid() == false)
    {
        return QString("No valid");
    }

    QString stateStr, formatStr;


    if (m_signalType == E::SignalType::Analog)
    {
        formatStr.sprintf( ("%%.%df"), m_precision );

        stateStr.sprintf( formatStr.toAscii(),  m_state.value());
    }

    if (m_signalType == E::SignalType::Discrete)
    {
        stateStr = m_state.value() == 0 ? QString("No") : QString("Yes");
    }

    return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignal::defaultValueStr() const
{
    if (m_hash == 0)
    {
        return QString();
    }

    QString stateStr, formatStr;

    if (m_signalType == E::SignalType::Analog)
    {
        formatStr.sprintf( ("%%.%df"), m_precision );

        stateStr.sprintf( formatStr.toAscii(), m_defaultValue );
    }

    if (m_signalType == E::SignalType::Discrete)
    {
        stateStr = m_defaultValue == 0 ? QString("No") : QString("Yes");
    }

    return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------

QString TuningSignal::rangeStr() const
{
    if (m_hash == 0)
    {
        return QString();
    }

    if (m_signalType == E::SignalType::Discrete)
    {
        return QString();
    }

    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_precision );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_state.lowLimit(), m_state.highLimit());

    return range;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningWriteCmd::TuningWriteCmd()
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningWriteCmd::TuningWriteCmd(const Hash& signalHash, float value) :
    m_signalHash (signalHash ),
    m_value (value)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningWriteCmd::~TuningWriteCmd()
{
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TuningSignalBase theTuningSignalBase;

// -------------------------------------------------------------------------------------------------------------------

TuningSignalBase::TuningSignalBase(QObject *parent) :
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
    m_sourceMutex.lock();

        m_sourceList.clear();
        m_sourceIdMap.clear();

    m_sourceMutex.unlock();

    m_signalMutex.lock();

        m_signalList.clear();
        m_signalHashMap.clear();

    m_signalMutex.unlock();

    m_cmdFowWriteMutex.lock();

        m_cmdFowWriteList.clear();

    m_cmdFowWriteMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::sourceCount() const
{
    int count = 0;

    m_sourceMutex.lock();

        count = m_sourceList.size();

    m_sourceMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::appendSource(const TuningSource& source)
{
    int index = -1;

    m_sourceMutex.lock();

        if ( m_sourceIdMap.contains(source.sourceID()) == false )
        {
            m_sourceList.append(source);

            index = m_sourceList.size() - 1;

            m_sourceIdMap[source.sourceID() ] = index;
        }

    m_sourceMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSource TuningSignalBase::source(int index) const
{
    TuningSource source;

    m_sourceMutex.lock();

        if (index >= 0 && index < m_sourceList.size())
        {
            source = m_sourceList[index];
        }

    m_sourceMutex.unlock();

    return source;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSourceState TuningSignalBase::sourceState(quint64 sourceID)
{
    TuningSourceState state;

    m_sourceMutex.lock();

        if (m_sourceIdMap.contains(sourceID) == true)
        {
            int index = m_sourceIdMap[sourceID];

            if (index >= 0 && index < m_sourceList.size())
            {
                state = m_sourceList[index].state();
            }
        }

    m_sourceMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::setSourceState(quint64 sourceID, const Network::TuningSourceState& state)
{
    m_sourceMutex.lock();

        if (m_sourceIdMap.contains(sourceID) == true)
        {
            int index = m_sourceIdMap[sourceID];

            if (index >= 0 && index < m_sourceList.size())
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

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::clearSourceList()
{
    m_sourceMutex.lock();

        m_sourceList.clear();
        m_sourceIdMap.clear();

    m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::createSignalList()
{
    int count = theSignalBase.signalCount();
    for(int i = 0; i < count; i++)
    {
        SignalParam param = theSignalBase.signalParam(i);
        if (param.isValid() == false)
        {
            continue;
        }

        if (param.enableTuning() == false)
        {
            continue;
        }

        appendSignal( TuningSignal( param ) );
    }
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::appendSignal(const TuningSignal& signal)
{
    int index = -1;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(signal.hash()) == false)
        {
            m_signalList.append(signal);
            index = m_signalList.size() - 1;

            m_signalHashMap[signal.hash()] = index;
        }

    m_signalMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal TuningSignalBase::signalForRead(const Hash& hash) const
{
    if (hash == 0)
    {
        assert(hash != 0);
        return TuningSignal();
    }

    TuningSignal signal;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
            int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                signal = m_signalList[index];
            }
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignal TuningSignalBase::signalForRead(int index) const
{
    TuningSignal signal;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

TuningSignalState TuningSignalBase::signalState(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return TuningSignalState();
    }

    TuningSignalState state;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
           int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                state = m_signalList[index].state();
            }
        }

    m_signalMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::setSignalState(const Hash& hash, const Network::TuningSignalState& state)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return;
    }

    m_signalMutex.lock();

        if (m_signalHashMap.contains(state.signalhash()) == true)
        {
            int index = m_signalHashMap[state.signalhash()];

            if (index >= 0 && index < m_signalList.size())
            {
                TuningSignalState& signalState = m_signalList[index].state();

                signalState.setValid( state.valid() );
                signalState.setValue( state.value() );
                signalState.setLimits( state.readlowbound(), state.readhighbound() );
            }
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::singalsSetNovalid()
{
    m_signalMutex.lock();

        int count = m_signalList.count();

        for(int i = 0; i < count; i++ )
        {
            m_signalList[i].state().setValid(false);
        }

    m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::clearSignalLlst()
{
    m_signalMutex.lock();

       m_signalList.clear();
       m_signalHashMap.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int TuningSignalBase::cmdFowWriteCount() const
{
    int count = 0;

    m_cmdFowWriteMutex.lock();

        count = m_cmdFowWriteList.size();

    m_cmdFowWriteMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::appendCmdFowWrite(const TuningWriteCmd& cmd)
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


// -------------------------------------------------------------------------------------------------------------------

void TuningSignalBase::appendCmdFowWrite(const Hash& signalHash, float value)
{
    if (signalHash == 0)
    {
        assert(signalHash != 0);
        return;
    }

    m_cmdFowWriteMutex.lock();

        TuningWriteCmd cmd;

        cmd.setSignalHash(signalHash);
        cmd.setValue(value);

        m_cmdFowWriteList.append(cmd);

    m_cmdFowWriteMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

TuningWriteCmd TuningSignalBase::cmdFowWrite(int index)
{
    TuningWriteCmd cmd;

    m_cmdFowWriteMutex.lock();

        if (index >= 0 && index < m_cmdFowWriteList.size())
        {
            cmd = m_cmdFowWriteList[index];

            m_cmdFowWriteList.remove(index);
        }

    m_cmdFowWriteMutex.unlock();

    return cmd;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------



