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
		std::vector<Hash> hashes;
		std::vector<double> values;

		std::mutex* condVarMutex = nullptr;
		std::condition_variable* condVar = nullptr;
		std::vector<char>* replyData = nullptr;

		void clear()
		{
			readRequest = true;
			rwRequestID = 0;
			hashes.clear();
			values.clear();
			condVarMutex = nullptr;
			condVar = nullptr;
			replyData = nullptr;
		}

		bool isNull()
		{
			return rwRequestID == 0 ||
				   condVarMutex == nullptr ||
				   condVar == nullptr ||
				   replyData == nullptr;
		}
	};

public:
	TuningSrvClient(std::shared_ptr<TgsSession> session,
					const SoftwareInfo& softwareInfo,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					const QString& clientDescription,
					const QString& serverEquipmentID);

	virtual void onClientThreadStarted() override;
	virtual void onClientThreadFinished() override;

	virtual void onConnection() override;
	virtual void onDisconnection() override;

	bool getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount);
	bool getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize);
	bool getTuningSourceStatesReply(std::vector<char>& reply);

	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes,
						   std::mutex* condVarMutex,
						   std::condition_variable* condVar,
						   std::vector<char>* replyData);

private:
	void onTimer();

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void onGetNextFilePart(const char* replyData, quint32 replyDataSize);
	void onGetTuningSourcesStates(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsRead(const char* replyData, quint32 replyDataSize);

	void sendNextRequest();
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
					const QString& serverEquipmentID);

	bool getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount);
	bool getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize);
	bool getTuningSourceStatesReply(std::vector<char>& reply);
	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes,
						   std::mutex* condVarMutex,
						   std::condition_variable* condVar,
						   std::vector<char>* replyData);

private:
	TuningSrvClient* m_client = nullptr;
};
