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
