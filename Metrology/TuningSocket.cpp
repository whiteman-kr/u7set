#include "TuningSocket.h"

#include <assert.h>

#include "TuningSignalBase.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const HostAddressPort& serverAddressPort) :
    Tcp::Client(serverAddressPort)
{
}


// -------------------------------------------------------------------------------------------------------------------

TuningSocket::TuningSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
    Tcp::Client(serverAddressPort1, serverAddressPort2)
{
}

// -------------------------------------------------------------------------------------------------------------------

TuningSocket::~TuningSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onClientThreadStarted()
{
    qDebug() << "TuningSocket::onClientThreadStarted()";

    return;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onClientThreadFinished()
{
    qDebug() << "TuningSocket::onClientThreadFinished()";
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onConnection()
{
    qDebug() << "TuningSocket::onConnection()";

    emit socketConnected();

    requestTuningSourcesInfo();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::onDisconnection()
{
    qDebug() << "TuningSocket::onDisconnection";

    emit socketDisconnected();
}


// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
    if (replyData == nullptr)
    {
        assert(replyData);
        return;
    }

    switch(requestID)
    {
        case TDS_GET_TUNING_SOURCES_INFO:
            replyTuningSourcesInfo(replyData, replyDataSize);
            break;

        case TDS_GET_TUNING_SOURCES_STATES:
            replyTuningSourcesState(replyData, replyDataSize);
            break;

        case TDS_TUNING_SIGNALS_READ:
            replyReadTuningSignals(replyData, replyDataSize);
            break;

        case TDS_TUNING_SIGNALS_WRITE:
            replyWriteTuningSignals(replyData, replyDataSize);
            break;

        default:
            assert(false);
    }

    return;
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_GET_TUNING_SOURCES_INFO

void TuningSocket::requestTuningSourcesInfo()
{
    assert(isClearToSendRequest());

    theTuningSignalBase.clearSourceList();

    m_getTuningSourcesInfo.set_clientequipmentid( theOptions.tuningSocket().equipmentID().toUtf8() );

    sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyTuningSourcesInfo(const char* replyData, quint32 replyDataSize)
{
    if (replyData == nullptr)
    {
        assert(replyData);
        requestTuningSourcesInfo();
        return;
    }

    bool result = m_tuningDataSourcesInfoReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
    if (result == false)
    {
        qDebug() << "TuningSocket::replyTuningSourcesInfo - error: ParseFromArray";
        assert(result);
        requestTuningSourcesInfo();
        return;
    }

    if (m_tuningDataSourcesInfoReply.error() != 0)
    {
        qDebug() << "TuningSocket::replyTuningSourcesInfo - error: " << m_tuningDataSourcesInfoReply.error();
        assert(m_tuningDataSourcesInfoReply.error() != 0);
        requestTuningSourcesInfo();
        return;
    }

    int sourceCount = m_tuningDataSourcesInfoReply.datasourceinfo_size();

    qDebug() << "TuningSocket::replyTuningSourcesInfo - Tuning sources count:  " << sourceCount;

    for (int i = 0; i < sourceCount; i++)
    {
        const ::Network::DataSourceInfo& dsi = m_tuningDataSourcesInfoReply.datasourceinfo(i);
        theTuningSignalBase.appendSource( TuningSource(dsi) );

        qDebug() << "TuningSocket::replyTuningSourcesInfo - :  " << i << ". SubSystem:" << dsi.subsystem().c_str() << ", EquipmentID:" << dsi.equipmentid().c_str() << ", IP:" << dsi.ip().c_str();
    }

    emit sourcesLoaded();

    requestTuningSourcesState();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_GET_TUNING_SOURCES_STATES

void TuningSocket::requestTuningSourcesState()
{
    assert(isClearToSendRequest());

    QThread::msleep(TUNING_SOCKET_TIMEOUT_STATE);

    m_getTuningSourcesStates.set_clientequipmentid( theOptions.tuningSocket().equipmentID().toUtf8() );

    sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyTuningSourcesState(const char* replyData, quint32 replyDataSize)
{
    if (replyData == nullptr)
    {
        assert(replyData);
        requestTuningSourcesState();
        return;
    }

    bool result = m_tuningDataSourcesStatesReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
    if (result == false)
    {
        qDebug() << "TuningSocket::replyTuningSourcesState - error: ParseFromArray";
        assert(result);
        requestTuningSourcesState();
        return;
    }

    if (m_tuningDataSourcesStatesReply.error() != 0)
    {
        qDebug() << "TuningSocket::replyTuningSourcesState - error: " << m_tuningDataSourcesStatesReply.error();
        assert(m_tuningDataSourcesStatesReply.error() != 0);
        requestTuningSourcesState();
        return;
    }

    int sourceCount = m_tuningDataSourcesStatesReply.tuningsourcesstate_size();

    for (int i = 0; i < sourceCount; i++)
    {
        const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningsourcesstate(i);
        theTuningSignalBase.setSourceState( tss.sourceid(), tss );
    }

    requestReadTuningSignals();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_TUNING_SIGNALS_READ

void TuningSocket::requestReadTuningSignals()
{
    assert(isClearToSendRequest());

    int signalForReadCount = theTuningSignalBase.signalCount();
    if (signalForReadCount == 0)
    {
        requestTuningSourcesState();
        return;
    }

    m_readTuningSignals.set_clientequipmentid( theOptions.tuningSocket().equipmentID().toUtf8() );

    m_readTuningSignals.mutable_signalhash()->Reserve(signalForReadCount);

    m_readTuningSignals.mutable_signalhash()->Clear();

    int startIndex = m_readTuningSignalsIndex;

    for (int i = 0; i < TUNING_SOCKET_MAX_READ_SIGNAL; i++)
    {
        if (m_readTuningSignalsIndex >= signalForReadCount)
        {
            m_readTuningSignalsIndex = 0;
            break;
        }

        TuningSignal signal = theTuningSignalBase.signalForRead(i + startIndex);
        if (signal.hash() == 0)
        {
            continue;
        }

        m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(signal.hash());

        m_readTuningSignalsIndex ++;
    }

    sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyReadTuningSignals(const char* replyData, quint32 replyDataSize)
{
    if (replyData == nullptr)
    {
        assert(replyData);
        requestTuningSourcesState();
        return;
    }

    bool result = m_readTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
    if (result == false)
    {
        qDebug() << "TuningSocket::replyReadTuningSignals - error: ParseFromArray";
        assert(result);
        requestTuningSourcesState();
        return;
    }

    if (m_readTuningSignalsReply.error() != 0)
    {
        qDebug() << "TuningSocket::replyReadTuningSignals - error: " << m_readTuningSignalsReply.error();
        assert(m_readTuningSignalsReply.error() != 0);
        requestTuningSourcesState();
        return;
    }

    int readReplyCount = m_readTuningSignalsReply.tuningsignalstate_size();

    for (int i = 0; i < readReplyCount; i++)
    {
        const ::Network::TuningSignalState& tss = m_readTuningSignalsReply.tuningsignalstate(i);

        Hash hash = tss.signalhash();
        if (hash == 0)
        {
            assert(hash != 0);
            continue;
        }

        theTuningSignalBase.setSignalState(hash, tss);
    }

    requestWriteTuningSignals();
}

// -------------------------------------------------------------------------------------------------------------------
// TDS_TUNING_SIGNALS_WRITE

void TuningSocket::requestWriteTuningSignals()
{
    assert(isClearToSendRequest());

    int cmdCount = theTuningSignalBase.cmdFowWriteCount();
    if (cmdCount == 0)
    {
        requestTuningSourcesState();
        return;
    }

    m_writeTuningSignals.set_clientequipmentid( theOptions.tuningSocket().equipmentID().toUtf8() );
    m_writeTuningSignals.set_autoapply(true);

    m_writeTuningSignals.mutable_tuningsignalwrite()->Reserve(cmdCount);

    m_writeTuningSignals.mutable_tuningsignalwrite()->Clear();

    for (int i = 0; i < cmdCount && i < TUNING_SOCKET_MAX_WRITE_CMD; i++)
    {
        TuningWriteCmd cmd = theTuningSignalBase.cmdFowWrite(i);
        if (cmd.signalHash() == 0)
        {
            assert(cmd.signalHash() != 0);
            continue;
        }

        Network::TuningSignalWrite* wrCmd = new Network::TuningSignalWrite();
        if (wrCmd == nullptr)
        {
            continue;
        }

        wrCmd->set_value(cmd.value() );
        wrCmd->set_signalhash(cmd.signalHash());

        m_writeTuningSignals.mutable_tuningsignalwrite()->AddAllocated(  wrCmd );
    }

    sendRequest(TDS_TUNING_SIGNALS_WRITE, m_writeTuningSignals);
}

// -------------------------------------------------------------------------------------------------------------------

void TuningSocket::replyWriteTuningSignals(const char* replyData, quint32 replyDataSize)
{
    if (replyData == nullptr)
    {
        assert(replyData);
        requestTuningSourcesState();
        return;
    }

    bool result = m_writeTuningSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);
    if (result == false)
    {
        qDebug() << "TuningSocket::replyWriteTuningSignals - error: ParseFromArray";
        assert(result);
        requestTuningSourcesState();
        return;
    }

    if (m_writeTuningSignalsReply.error() != 0)
    {
        qDebug() << "TuningSocket::replyWriteTuningSignals - error: " << m_writeTuningSignalsReply.error();
        requestTuningSourcesState();
        return;
    }

    requestTuningSourcesState();
}

// -------------------------------------------------------------------------------------------------------------------

