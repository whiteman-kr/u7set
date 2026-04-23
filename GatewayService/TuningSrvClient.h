#pragma once

#include <mutex>
#include <vector>
#include <deque>

#include "../OnlineLib/Tcp.h"

struct TgsSession;

class TuningSrvClient : public Tcp::Client
{
	Q_OBJECT

private:
	struct ReadWriteSignalsRequest
	{
		bool readRequest = true;			// true - read request
											// false - write request
		quint64 rwRequestID = 0;
		std::string user;
		bool apply = false;
		std::vector<Hash> hashes;
		std::vector<double> values;

		void clear()
		{
			readRequest = true;
			rwRequestID = 0;
			user.clear();
			apply = false;
			hashes.clear();
			values.clear();
		}

		bool isNull()
		{
			return (rwRequestID == 0);
		}
	};

public:
	TuningSrvClient(std::shared_ptr<TgsSession>& session,
					const SoftwareInfo& softwareInfo,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					const QString& clientDescription,
					const QString& serverEquipmentID,
					const AppSignals& appSignals);

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	bool getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount);
	bool getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize);
	bool getTuningSourceStatesReply(std::vector<char>& reply);

	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes);

	void tuningSignalsWrite(quint64 requestID,
						   const std::string& user,
						   bool apply,
						   std::vector<Hash>& hashes,
						   std::vector<double>& values);
private:
	void onTimer();

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void onGetNextFilePart(const char* replyData, quint32 replyDataSize);
	void onGetTuningSourcesStates(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsRead(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsWrite(const char* replyData, quint32 replyDataSize);

	void sendNextRequest();
	void sendGetSourceStatesRequest();
	bool sendReadSignalsRequest(const ReadWriteSignalsRequest& req);
	bool sendWriteSignalsRequest(const ReadWriteSignalsRequest& req);

	void setActiveRwRequest(const ReadWriteSignalsRequest& req);
	void clearActiveRwRequest();

	void restartReceiveFile();
	void requestNextFilePart();
	void clearReceiveFileVars();

signals:
	void signal_sendNextRequest();

private:
	std::shared_ptr<TgsSession> m_session;
	const AppSignals& m_appSignals;

	static constexpr int TIMER_PERIOD = 10;
	static constexpr int REQUEST_SOURCE_STATES_PERIOD = 300;

	QTimer* m_timer = nullptr;
	int m_timerCtr = 0;

	quint64 m_filePartNo = 0;
	quint64 m_filePartsCount = 0;
	std::atomic<bool> m_fileReady {false};
	std::vector<char> m_tuningSourcesFileData;

	//

	std::mutex m_sourceStatesMutex;
	std::vector<char> m_sourceStatesReply;

	//

	std::mutex m_rwQueueMutex;
	std::deque<ReadWriteSignalsRequest> m_rwRequestQueue;
	ReadWriteSignalsRequest m_activeRwRequest;
};

class TuningSrvClientThread : public SimpleThread
{
public:
	TuningSrvClientThread(std::shared_ptr<TgsSession> session,
					const SoftwareInfo& softwareInfo,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					const QString& clientDescription,
					const QString& serverEquipmentID,
					const AppSignals& appSignals);

	bool getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount);
	bool getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize);
	bool getTuningSourceStatesReply(std::vector<char>& reply);

	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes);

	void tuningSignalsWrite(quint64 requestID,
							const std::string& user,
							bool apply,
							std::vector<Hash>& hashes,
							std::vector<double>& values);

private:
	TuningSrvClient* m_client = nullptr;
};
