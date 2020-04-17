#ifndef TUNINGSOCKET_H
#define TUNINGSOCKET_H

#include <assert.h>

// This class is designed to receive signals from TuningSrv
//
// Algorithm:
//
// onConnection()
//				|
//				TDS_GET_TUNING_SOURCES_INFO
//				|
//				TDS_GET_TUNING_SOURCES_STATES	<-----------------------------------------------|
//				|																				|
//				TDS_TUNING_SIGNALS_WRITE														|
//				|																				|
//				TDS_DATA_SOURCE_WRITE															|
//				|																				|
//				TDS_PACKETSOURCE_EXIT															|
//				|_______________________________________________________________________________|

#include "../lib/Tcp.h"
#include "../lib/SocketIO.h"
#include "../lib/Hash.h"

#include "../Proto/network.pb.h"

#include "TuningSourceBase.h"

// ==============================================================================================

const int			TUNING_SOCKET_TIMEOUT_STATE = 100;			// 100 ms

const int			TUNING_SOCKET_WAIT_RAPLY_TIMEOUT = 500;		// 500 ms

// ==============================================================================================

const int			TUNING_SOCKET_MAX_READ_SIGNAL = 100;
const int			TUNING_SOCKET_MAX_WRITE_CMD	 = 100;

// ==============================================================================================

class ChangeSignalState
{
public:

	ChangeSignalState() { clear(); }
	ChangeSignalState(const Hash &signalHash, TuningValueType type, double value) : m_signalHash (signalHash), m_type (type), m_value (value) {}
	virtual ~ChangeSignalState() {}

private:

	Hash m_signalHash = UNDEFINED_HASH;
	TuningValueType m_type = TuningValueType::Discrete;
	QVariant m_value;

public:

	void clear()
	{
		m_signalHash = UNDEFINED_HASH;
		m_type = TuningValueType::Discrete;
		m_value.clear();
	}

	bool isEmpty()
	{
		if (m_signalHash == UNDEFINED_HASH)
		{
			return true;
		}

		return false;
	}

	Hash signalHash() const { return m_signalHash; }
	void setSignalHash(Hash hash) { m_signalHash = hash; }

	TuningValueType type() const { return m_type; }
	int typeInt() const { return TO_INT(m_type); }
	void setType(TuningValueType valueType) { m_type = valueType; }

	QVariant value() const { return m_value; }
	void setValue(QVariant value) { m_value = value; }
};

// ==============================================================================================

class ChangeSourceState
{
public:

	ChangeSourceState() { clear(); }
	ChangeSourceState(const QString &sourceID, bool state) : m_sourceID (sourceID), m_state(state) {}
	virtual ~ChangeSourceState() {}

private:

	QString m_sourceID;
	bool m_state = false;

public:

	void clear()
	{
		m_sourceID.clear();
		m_state = false;
	}

	bool isEmpty()
	{
		if (m_sourceID.isEmpty() == true)
		{
			return true;
		}

		return false;
	}

	QString sourceID() const { return m_sourceID; }
	void setSourceID(const QString &sourceID) { m_sourceID = sourceID; }

	bool state() const { return m_state; }
	void setState(bool state) { m_state = state; }
};

// ==============================================================================================

class TuningSocket : public Tcp::Client
{
	Q_OBJECT

public:

	TuningSocket(const SoftwareInfo& softwareInfo, const HostAddressPort& serverAddressPort, TuningSourceBase* pTuningSourceBase);
	virtual ~TuningSocket() override;

private:

	TuningSourceBase* m_pTuningSourceBase = nullptr;

	int m_readTuningSignalsIndex = 0;

	// protobuf messages
	//
	Network::GetTuningSourcesInfo m_getTuningSourcesInfo;							// TDS_GET_TUNING_SOURCES_INFO
	Network::GetTuningSourcesStatesReply m_tuningDataSourcesStatesReply;

	Network::GetTuningSourcesStates m_getTuningSourcesStates;						// TDS_GET_TUNING_SOURCES_STATES
	Network::GetDataSourcesInfoReply m_tuningDataSourcesInfoReply;

	Network::TuningSignalsWrite m_writeTuningSignals;								// TDS_TUNING_SIGNALS_WRITE
	Network::TuningSignalsWriteReply m_writeTuningSignalsReply;

	Network::DataSourceWrite m_dataSourceWrite;										// TDS_DATA_SOURCE_WRITE
	Network::DataSourceWriteReply m_dataSourceWriteReply;

	Network::PacketSourceExit m_packetSourceExit;									// TDS_PACKETSOURCE_EXIT
	Network::PacketSourceExitReply m_packetSourceExitReply;

	// functions: Request - Reply
	//
	void requestTuningSourcesInfo();												// TDS_GET_TUNING_SOURCES_INFO
	void replyTuningSourcesInfo(const char* replyData, quint32 replyDataSize);

	void requestTuningSourcesState();												// TDS_GET_TUNING_SOURCES_INFO
	void replyTuningSourcesState(const char* replyData, quint32 replyDataSize);

	void requestWriteStateTuningSignals();											// TDS_TUNING_SIGNALS_WRITE
	void replyWriteStateTuningSignals(const char* replyData, quint32 replyDataSize);

	void requestWriteStateDataSource();												// TDS_DATA_SOURCE_WRITE
	void replyWriteStateDataSource(const char* replyData, quint32 replyDataSize);

	void requestPacketSourceExit();													// TDS_PACKETSOURCE_EXIT
	void replyPacketSourceExit(const char* replyData, quint32 replyDataSize);

	// commands
	//
		// Commands to change signal state
		//
	mutable QMutex m_signalStateMutex;
	QVector<ChangeSignalState> m_cmdSignalStateList;
	int cmdSignalStateCount() const;
	ChangeSignalState cmdSignalState();

		// Commands to change source state
		//
	mutable QMutex m_sourceStateMutex;
	QVector<ChangeSourceState> m_cmdSourceStateList;
	int cmdSourceStateCount() const;
	ChangeSourceState cmdSourceState();

		// Commands to exit PacketSourceConsole application
		//
	bool m_cmdExitPacketSource = false;

public:

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;	 // for processing functions: Request - Reply

	// Commands
	//
	void writeCmd(const ChangeSignalState& cmd);		// Commands for change signal state
	void writeCmd(const ChangeSourceState& cmd);		// Commands for change source state
	void writeCmd(bool exitPacketSource);				// Commands for exit PacketSourceConsole application

public slots:

signals:

	void socketConnected();
	void socketDisconnected();
};

// ==============================================================================================

#endif // TUNINGSOCKET_H
