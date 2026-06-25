#pragma once

#include <mutex>
#include <vector>
#include <deque>

#include "../OnlineLib/Tcp.h"

class TuningGatewaySession;

class TuningSrvClient : public Tcp::Client
{
	Q_OBJECT

private:
	enum class RequestType
	{
		Nothing,
		SourceStates,
		Read,
		Write,
		Apply,
		ChangeConttrolledSource
	};

	struct Request
	{
		RequestType requestType = RequestType::Nothing;
		quint64 rwRequestID = 0;
		std::string user;
		bool apply = false;
		std::vector<Hash> hashes;
		std::vector<double> values;
		std::string moduleEquipmentID;
		bool activateControl = false;

		void clear()
		{
			requestType = RequestType::Nothing;
			rwRequestID = 0;
			user.clear();
			apply = false;
			hashes.clear();
			values.clear();
			moduleEquipmentID.clear();
			activateControl = false;
		}

		bool isNull()
		{
			return (requestType == RequestType::Nothing);
		}
	};

public:
	TuningSrvClient(TuningGatewaySession& session,
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

	void getTuningSourcesState();

	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes);

	void tuningSignalsWrite(quint64 requestID,
						   const std::string& user,
						   bool apply,
						   std::vector<Hash>& hashes,
						   std::vector<double>& values);

	void tuningSignalsApply();
	void tuningChangeControlledSource(const std::string& moduleEquipmentID, bool activateControl);

private:
	void onTimer();

	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void onGetNextFilePart(const char* replyData, quint32 replyDataSize);
	void onGetTuningSourcesStates(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsRead(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsWrite(const char* replyData, quint32 replyDataSize);
	void onTuningSignalsApply(const char* replyData, quint32 replyDataSize);
	void onChangeControlledSource(const char* replyData, quint32 replyDataSize);

	void sendNextRequest();
	bool sendGetSourceStatesRequest(const Request& req);
	bool sendReadSignalsRequest(const Request& req);
	bool sendWriteSignalsRequest(const Request& req);
	bool sendApplySignalsRequest(const Request& req);
	bool sendChangeControlledSourceRequest(const Request& req);

	void setActiveRequest(const Request& req);

	void restartReceiveFile();
	void requestNextFilePart();
	void clearReceiveFileVars();

signals:
	void signal_sendNextRequest();

private:
	TuningGatewaySession& m_session;
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

	std::mutex m_requestQueueMutex;
	std::deque<Request> m_requestQueue;
	Request m_activeRequest;
};

class TuningSrvClientThread : public SimpleThread
{
public:
	TuningSrvClientThread(TuningGatewaySession& session,
					const SoftwareInfo& softwareInfo,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					const QString& clientDescription,
					const QString& serverEquipmentID,
					const AppSignals& appSignals);

	bool getTuningSourcesFileMetrics(quint64& fileSize, quint64& maxPartSize, quint64& partCount);
	bool getTuningSourcesFilePart(quint64 partNo, std::vector<char>& fileData, quint64& partSize);

	void getTuningSourcesState();

	void tuningSignalsRead(quint64 requestID,
						   std::vector<Hash>& hashes);

	void tuningSignalsWrite(quint64 requestID,
							const std::string& user,
							bool apply,
							std::vector<Hash>& hashes,
							std::vector<double>& values);

	void tuningSignalsApply();
	void tuningChangeControlledSource(const std::string& moduleEquipmentID, bool activateControl);

private:
	TuningSrvClient* m_client = nullptr;
};
