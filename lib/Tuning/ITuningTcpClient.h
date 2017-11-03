#ifndef ITUNINGTCPCLIENT_H
#define ITUNINGTCPCLIENT_H

class

class ITuningTcpClient
{
public:
	virtual void writeTuningSignal(const TuningWriteCommand& data) = 0;
	void writeTuningSignal(const std::vector<TuningWriteCommand>& data);

private:
	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	virtual void onReplyTimeout() override;

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

protected:
	void resetToGetTuningSources();
	void resetToGetTuningSourcesState();
	void resetToProcessTuningSignals();

	void requestTuningSourcesInfo();
	void processTuningSourcesInfo(const QByteArray& data);

	void requestTuningSourcesState();
	void processTuningSourcesState(const QByteArray& data);

	void requestReadTuningSignals();
	void processReadTuningSignals(const QByteArray& data);

	void requestWriteTuningSignals();
	void processWriteTuningSignals(const QByteArray& data);

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

public slots:
	void slot_signalsUpdated();
	void slot_serversArrived(HostAddressPort address1, HostAddressPort address2);

signals:
	void tuningSourcesArrived();

private:
	QString networkErrorStr(NetworkError error);

	// Properties
	//
public:
	QString instanceId() const;
	void setInstanceId(const QString& instanceId);

	int requestInterval() const;
	void setRequestInterval(int requestInterval);

	// Data
	//
private:
	QString m_instanceId;
	int m_requestInterval = 10;

	TuningSignalManager* m_signals;

	// Tuning sources
	//
	mutable QMutex m_tuningSourcesMutex;				// For access to m_tuningSources
	std::map<quint64, TuningSource> m_tuningSources;	// Key is ::Proto::DataSourceInfo::id

	// Processing
	//
	mutable QMutex m_writeQueueMutex;					// For access to m_writeQueue
	std::queue<TuningWriteCommand> m_writeQueue;

	int m_readTuningSignalIndex = 0;
	int m_readTuningSignalCount = 0;

	std::vector<Hash> m_signalHashes;

private:
	// Cached protobug messages
	//
	::Network::GetTuningSourcesStates m_getTuningSourcesStates;
	::Network::GetDataSourcesInfoReply m_tuningDataSourcesInfoReply;

	::Network::GetTuningSourcesInfo m_getTuningSourcesInfo;
	::Network::GetTuningSourcesStatesReply m_tuningDataSourcesStatesReply;

	::Network::TuningSignalsRead m_readTuningSignals;
	::Network::TuningSignalsReadReply m_readTuningSignalsReply;

	::Network::TuningSignalsWrite m_writeTuningSignals;
	::Network::TuningSignalsWriteReply m_writeTuningSignalsReply;
};


#endif // ITUNINGTCPCLIENT_H
