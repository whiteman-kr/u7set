#include "SignalSocket.h"

#include <assert.h>

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------

SignalSocket::SignalSocket(const HostAddressPort& serverAddressPort) :
    Tcp::Client(serverAddressPort)
{
}


// -------------------------------------------------------------------------------------------------------------------

SignalSocket::SignalSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
    Tcp::Client(serverAddressPort1, serverAddressPort2)
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalSocket::~SignalSocket()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onClientThreadStarted()
{
    qDebug() << "SignalSocket::onSignalSocketThreadStarted()";

    return;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onClientThreadFinished()
{
    qDebug() << "SignalSocket::onSignalSocketThreadFinished()";
}


// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onConnection()
{
    qDebug() << "SignalSocket::onConnection()";

    emit socketConnected();

    requestSignalListStart();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::onDisconnection()
{
    qDebug() << "SignalSocket::onDisconnection";

    stopSignalStateTimer();

    emit socketDisconnected();
}


// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
    switch(requestID)
    {
        case ADS_GET_APP_SIGNAL_LIST_START:
            replySignalListStart(replyData, replyDataSize);
            break;

        case ADS_GET_APP_SIGNAL_LIST_NEXT:
            replySignalListNext(replyData, replyDataSize);
            break;

        case ADS_GET_APP_SIGNAL_PARAM:
            replySignalParam(replyData, replyDataSize);
            break;

        case ADS_GET_UNITS:
            replyUnits(replyData, replyDataSize);
            break;

        case ADS_GET_APP_SIGNAL_STATE:
            replySignalState(replyData, replyDataSize);
            break;

        default:
            assert(false);
    }

    return;
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_APP_SIGNAL_LIST_START

void SignalSocket::requestSignalListStart()
{
    m_signalHashList.clear();

    sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replySignalListStart(const char* replyData, quint32 replyDataSize)
{
    bool result = m_getSignalListStartReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

    if (result == false)
    {
        assert(false);

        requestSignalListStart();

        return;
    }

    if (m_getSignalListStartReply.error() != 0)
    {
        qDebug() << "SignalSocket::replyAppSignalListStart - error: " << m_getSignalListStartReply.error();
        assert(m_getSignalListStartReply.error() != 0);

        requestSignalListStart();

        return;
    }

    qDebug() << "SignalSocket::replyAppSignalListStart";
    qDebug() << "TotalItemCount:    "   << m_getSignalListStartReply.totalitemcount();
    qDebug() << "PartCount:         "   << m_getSignalListStartReply.partcount();
    qDebug() << "ItemsPerPart:      "   << m_getSignalListStartReply.itemsperpart();

    if (m_getSignalListStartReply.totalitemcount() == 0 || m_getSignalListStartReply.partcount() == 0)
    {
        // we have empty data
        //
        assert(m_getSignalListStartReply.totalitemcount() == 0);
        assert(m_getSignalListStartReply.partcount() == 0);

        // but need to risk, request params
        //
        requestSignalParam(0);
    }

    requestSignalListNext(0);
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_APP_SIGNAL_LIST_NEXT

void SignalSocket::requestSignalListNext(int partIndex)
{
    assert(isClearToSendRequest());

    if (partIndex >= m_getSignalListStartReply.partcount())
    {
        // all parts were requested, then begin to require params
        //
        if (m_signalHashList.size() != m_getSignalListStartReply.totalitemcount())
        {
            assert(m_signalHashList.size() != m_getSignalListStartReply.totalitemcount());
        }

        requestSignalParam(0);

        return;
    }

    m_getSignalListNextRequest.Clear();

    m_getSignalListNextRequest.set_part(partIndex);

    sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, m_getSignalListNextRequest);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replySignalListNext(const char* replyData, quint32 replyDataSize)
{
    bool result = m_getSignalListNextReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

    if (result == false)
    {
        assert(false);

        requestSignalListStart();

        return;
    }

    if (m_getSignalListStartReply.error() != 0)
    {
        qDebug() << "SignalSocket::replySignalListNext - error: " << m_getSignalListStartReply.error();
        assert(m_getSignalListStartReply.error() != 0);

        requestSignalListStart();

        return;
    }

    if (m_getSignalListNextReply.part() != m_getSignalListNextRequest.part())
    {
        // different parts in Reply and Request
        //
        assert(m_getSignalListNextReply.part() == m_getSignalListNextRequest.part());

        requestSignalListStart();

        return;
    }

    qDebug() << "SignalSocket::replySignalListNext";
    qDebug() << "PartIndex:         "   << m_getSignalListNextReply.part();
    qDebug() << "Items in the Part: "   << m_getSignalListNextReply.appsignalids_size();

    int stringCount = m_getSignalListNextReply.appsignalids_size();

    for(int i = 0; i < stringCount; i++)
    {
        Hash hash = calcHash(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));

        m_signalHashList.append(hash);
    }

    requestSignalListNext(m_getSignalListNextReply.part() + 1);
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_APP_SIGNAL_PARAM

void SignalSocket::requestSignalParam(int startIndex)
{
    assert(isClearToSendRequest());

    m_paramIndex = startIndex;

    if (startIndex == 0)
    {
        theSignalBase.clear();
    }

    if (startIndex >= m_signalHashList.size())
    {
        qDebug() << "SignalSocket::requestSignalParam - Signals were loaded: " << theSignalBase.size();

        requestUnits();

        emit signalsLoaded();

        startSignalStateTimer();

        return;
    }

    m_getSignalParamRequest.mutable_signalhashes()->Clear();
    m_getSignalParamRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_PARAM_MAX);

    for (int i = startIndex; i < startIndex + ADS_GET_APP_SIGNAL_PARAM_MAX && i < m_signalHashList.size(); i++)
    {
        m_getSignalParamRequest.add_signalhashes(m_signalHashList[i]);
    }

    sendRequest(ADS_GET_APP_SIGNAL_PARAM, m_getSignalParamRequest);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replySignalParam(const char* replyData, quint32 replyDataSize)
{
    bool result = m_getSignalParamReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

    if (result == false)
    {
        assert(false);

        requestSignalListStart();

        return;
    }

    if (m_getSignalParamReply.error() != 0)
    {
        qDebug() << "SignalSocket::replySignalParam - error: " << m_getSignalParamReply.error();
        assert(m_getSignalParamReply.error() != 0);

        requestSignalListStart();

        return;
    }

    for (int i = 0; i < m_getSignalParamReply.appsignalparams_size(); i++)
    {
        Signal param;
        param.serializeFromProtoAppSignal(&m_getSignalParamReply.appsignalparams(i));

        if (param.appSignalID().isEmpty() == true || param.hash() == 0)
        {
            assert(false);
            continue;
        }

        theSignalBase.appendSignal( param ) ;
    }

    requestSignalParam(m_paramIndex + ADS_GET_APP_SIGNAL_PARAM_MAX);
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_UNITS

void SignalSocket::requestUnits()
{
    assert(isClearToSendRequest());

    sendRequest(ADS_GET_UNITS, m_getUnitsRequest);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replyUnits(const char* replyData, quint32 replyDataSize)
{
    bool result = m_getUnitsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

    if (result == false)
    {
        assert(false);

        return;
    }

    if (m_getUnitsReply.error() != 0)
    {
        qDebug() << "SignalSocket::replyUnits - error: " << m_getUnitsReply.error();
        assert(m_getUnitsReply.error() != 0);

        return;
    }

    for (int i = 0; i < m_getUnitsReply.units_size(); i++)
    {
        theSignalBase.appendUnit(m_getUnitsReply.units(i).id(), QString::fromStdString(m_getUnitsReply.units(i).unit()));
    }

    qDebug() << "SignalSocket::replyUnits - Units were loaded: " << m_getUnitsReply.units_size();

    emit unitsLoaded();
}

// -------------------------------------------------------------------------------------------------------------------
// ADS_GET_APP_SIGNAL_STATE

void SignalSocket::requestSignalState()
{
    if (isClearToSendRequest() == false)
    {
        assert(false);
        return;
    }

    int hashCount = theSignalBase.hashForRequestStateCount();

    m_getSignalStateRequest.mutable_signalhashes()->Clear();
    m_getSignalStateRequest.mutable_signalhashes()->Reserve( hashCount );

    for (int i = 0; i < hashCount && i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
    {
        Hash hash = theSignalBase.hashForRequestState(i);

        if (hash == 0)
        {
            assert(false);
            continue;
        }

        m_getSignalStateRequest.add_signalhashes( hash );
    }

    sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::replySignalState(const char* replyData, quint32 replyDataSize)
{
    bool result = m_getSignalStateReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

    if (result == false)
    {
        assert(false);
        return;
    }

    if (m_getSignalStateReply.error() != 0)
    {
        qDebug() << "SignalSocket::replySignalState - error: " << m_getSignalStateReply.error();
        assert(m_getSignalStateReply.error() != 0);

        return;
    }

    for (int i = 0; i < m_getSignalStateReply.appsignalstates_size(); i++)
    {
        Hash hash = m_getSignalStateReply.appsignalstates(i).hash();

        if (hash == 0)
        {
            assert(hash != 0);
            continue;
        }

        AppSignalState state;
        state.getProtoAppSignalState(&m_getSignalStateReply.appsignalstates(i));

        theSignalBase.setSignalState(hash, state);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::startSignalStateTimer()
{
    if (m_updateSignalStateTimer == nullptr)
    {
        m_updateSignalStateTimer = new QTimer(this);
        connect(m_updateSignalStateTimer, &QTimer::timeout, this, &SignalSocket::updateSignalState);
    }

    m_updateSignalStateTimer->start(50); //   50 ms
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::stopSignalStateTimer()
{
    if (m_updateSignalStateTimer != nullptr)
    {
        m_updateSignalStateTimer->stop();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalSocket::updateSignalState()
{
    requestSignalState();
}

// -------------------------------------------------------------------------------------------------------------------
