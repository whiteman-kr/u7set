#ifndef SIGNALSOCKET_H
#define SIGNALSOCKET_H

// This class is designed to receive signals from AppDataSrv
//
// Algorithm:
//
// onConnection()
//              |
//              ADS_GET_APP_SIGNAL_LIST_START
//              |
//              ADS_GET_APP_SIGNAL_LIST_NEXT
//              |
//              ADS_GET_APP_SIGNAL_PARAM
//              |                   |
//              ADS_GET_UNITS       Start timer (m_updateSignalStateTimer -- updateSignalState())
//                                  |
//                                  ADS_GET_APP_SIGNAL_STATE (Hashes get from theSignalBase)


#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"
#include "../lib/AppSignalState.h"

#include "../Proto/network.pb.h"

class SignalSocket : public Tcp::Client
{
     Q_OBJECT

public:
    SignalSocket(const HostAddressPort& serverAddressPort);
    SignalSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
    virtual ~SignalSocket();

    virtual void    onClientThreadStarted() override;
    virtual void    onClientThreadFinished() override;

public:

    virtual void    onConnection() override;
    virtual void    onDisconnection() override;

    virtual void    processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;     // for processing functions: Request - Reply

private:

    // protobuf messages
    //

    Network::GetSignalListStartReply    m_getSignalListStartReply;          // ADS_GET_APP_SIGNAL_LIST_START
    QVector<Hash>                       m_signalHashList;

    Network::GetSignalListNextRequest   m_getSignalListNextRequest;         // ADS_GET_APP_SIGNAL_LIST_NEXT
    Network::GetSignalListNextReply     m_getSignalListNextReply;

    Network::GetAppSignalParamRequest   m_getSignalParamRequest;            // ADS_GET_APP_SIGNAL_PARAM
    Network::GetAppSignalParamReply     m_getSignalParamReply;
    int                                 m_paramIndex = 0;

    Network::GetUnitsRequest            m_getUnitsRequest;                  // ADS_GET_UNITS
    Network::GetUnitsReply              m_getUnitsReply;

    Network::GetAppSignalStateRequest   m_getSignalStateRequest;            // ADS_GET_APP_SIGNAL_STATE
    Network::GetAppSignalStateReply     m_getSignalStateReply;

    // functions: Request - Reply
    //

    void            requestSignalListStart();                                               // ADS_GET_APP_SIGNAL_LIST_START
    void            replySignalListStart(const char* replyData, quint32 replyDataSize);

    void            requestSignalListNext(int partIndex);                                   // ADS_GET_APP_SIGNAL_LIST_NEXT
    void            replySignalListNext(const char* replyData, quint32 replyDataSize);

    void            requestSignalParam(int startIndex);                                     // ADS_GET_APP_SIGNAL_PARAM
    void            replySignalParam(const char* replyData, quint32 replyDataSize);

    void            requestUnits();                                                         // ADS_GET_UNITS
    void            replyUnits(const char* replyData, quint32 replyDataSize);

    void            requestSignalState();                                                   // ADS_GET_APP_SIGNAL_STATE
    void            replySignalState(const char* replyData, quint32 replyDataSize);

    QTimer*         m_updateSignalStateTimer = nullptr;
    void            startSignalStateTimer();
    void            stopSignalStateTimer();

private slots:

    void            updateSignalState();

signals:

    void            socketConnected();
    void            socketDisconnected();

    void            signalsLoaded();
    void            unitsLoaded();
};

#endif // SIGNALSOCKET_H
