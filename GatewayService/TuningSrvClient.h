#pragma once

#include "../OnlineLib/Tcp.h"

struct TgsSession;

class TuningSrvClient : public Tcp::Client
{
public:
	TuningSrvClient(std::shared_ptr<TgsSession> session,
					const SoftwareInfo& softwareInfo,
					const HostAddressPort& serverAddressPort1,
					const HostAddressPort& serverAddressPort2,
					const QString& clientDescription,
					const QString& serverEquipmentID);

	virtual void onConnection() override;
	virtual void onDisconnection() override;

private:
	virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	void onGetNextFilePart(const char* replyData, quint32 replyDataSize);

	void restartReceiveFile();
	void requestNextFilePart();

private:
	quint64 m_filePartNo = 0;
	quint64 m_filePartsCount = 0;
	std::atomic<bool> m_fileReady {false};
	std::vector<char> m_tuningSourcesFileData;
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
};
