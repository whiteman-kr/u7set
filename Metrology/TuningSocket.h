#ifndef TUNINGSOCKET_H
#define TUNINGSOCKET_H

// This class is designed to receive signals from TuningSrv
//
// Algorithm:
//
// onConnection()
//              |
//              TDS_GET_TUNING_SOURCES_INFO
//              |
//              TDS_GET_TUNING_SOURCES_STATES   <-----------------------------------------------|
//              |                                                                               |
//              TDS_TUNING_SIGNALS_READ (Hashes get from theTuningSignalBase)                   |
//              |                                                                               |
//              TDS_TUNING_SIGNALS_WRITE (Hashes and Values get from theTuningSignalBase)       |
//              |_______________________________________________________________________________|

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"

#include "../Proto/network.pb.h"

// ==============================================================================================

const int           TUNING_SOCKET_TIMEOUT_STATE     = 100;  // 100 ms

// ==============================================================================================

const int           TUNING_SOCKET_MAX_READ_SIGNAL   = 100;
const int           TUNING_SOCKET_MAX_WRITE_CMD     = 100;

// ==============================================================================================

class TuningSocket : public Tcp::Client
{
     Q_OBJECT

public:
                    TuningSocket(const HostAddressPort& serverAddressPort);
                    TuningSocket(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);
    virtual         ~TuningSocket();

    virtual void    onClientThreadStarted() override;
    virtual void    onClientThreadFinished() override;

public:

    virtual void    onConnection() override;
    virtual void    onDisconnection() override;

    virtual void    processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;     // for processing functions: Request - Reply

private:

    // protobuf messages
    //

    Network::GetTuningSourcesInfo           m_getTuningSourcesInfo;         // TDS_GET_TUNING_SOURCES_INFO
    Network::GetTuningSourcesStatesReply    m_tuningDataSourcesStatesReply;

    Network::GetTuningSourcesStates         m_getTuningSourcesStates;       // TDS_GET_TUNING_SOURCES_STATES
    Network::GetDataSourcesInfoReply        m_tuningDataSourcesInfoReply;

    Network::TuningSignalsRead              m_readTuningSignals;            // TDS_TUNING_SIGNALS_READ
    Network::TuningSignalsReadReply         m_readTuningSignalsReply;

    Network::TuningSignalsWrite             m_writeTuningSignals;           // TDS_TUNING_SIGNALS_WRITE
    Network::TuningSignalsWriteReply        m_writeTuningSignalsReply;

    // functions: Request - Reply
    //

    void            requestTuningSourcesInfo();                                                 // TDS_GET_TUNING_SOURCES_INFO
    void            replyTuningSourcesInfo(const char* replyData, quint32 replyDataSize);

    void            requestTuningSourcesState();                                                // TDS_GET_TUNING_SOURCES_INFO
    void            replyTuningSourcesState(const char* replyData, quint32 replyDataSize);

    void            requestReadTuningSignals();                                                 // TDS_TUNING_SIGNALS_READ
    void            replyReadTuningSignals(const char* replyData, quint32 replyDataSize);

    void            requestWriteTuningSignals();                                                // TDS_TUNING_SIGNALS_WRITE
    void            replyWriteTuningSignals(const char* replyData, quint32 replyDataSize);

    int             m_readTuningSignalsIndex = 0;

private slots:

public slots:

	void			configurationLoaded();

signals:

    void            socketConnected();
    void            socketDisconnected();

    void            sourcesLoaded();
};

// ==============================================================================================

#endif // TUNINGSOCKET_H
