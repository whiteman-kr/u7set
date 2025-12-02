#include <CommonLib/ConstStrings.h>
#include <QUuid>

#include "GrpcSessionGuard.h"
#include "../UtilsLib/WUtils.h"

GrpcSessionGuard::GrpcSessionGuard(	const SoftwareInfo& severSwInfo,
									bool allowAllClients,
									const std::vector<ClientInfo>& clients,
									bool checkHostName) :
	m_serverSwInfo(severSwInfo),
	m_allowAllClients(allowAllClients),
	m_clients(clients),
	m_checkHostName(checkHostName)
{
}

GrpcSessionGuard::~GrpcSessionGuard()
{
	stop();
}

void GrpcSessionGuard::start()
{
	if (m_sessionGuardThread.joinable())
	{
		return;
	}

	m_sessionGuardThread = std::jthread
	{
		[this](std::stop_token stopToken)
		{
			sessionGuardLoop(stopToken);
		}
	};
}

void GrpcSessionGuard::stop() noexcept
{
	if (m_sessionGuardThread.joinable())
	{
		m_sessionGuardThread.request_stop();
		m_sessionGuardThread.join();
	}
}

grpc::Status GrpcSessionGuard::handshake(const Grpc::HandshakeRequest* request,
								Grpc::HandshakeReply* reply)
{
	if (request == nullptr ||
		reply == nullptr)
	{
		return grpc::Status(grpc::StatusCode::CANCELLED, "Internal error");
	}

	m_serverSwInfo.serializeTo(reply->mutable_serversoftwareinfo());

	std::string errMsg;

	if (isValidClient(request, errMsg) == false)
	{
		reply->set_authtoken("");
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, errMsg);
	}

	const QUuid guid = QUuid::createUuid();
	const std::string authToken = guid.toString(QUuid::WithoutBraces).toStdString();

	const TimePoint expiresAt =	std::chrono::steady_clock::now() + std::chrono::seconds(m_sessionTimeout);

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		m_sessionExpirations[authToken] = expiresAt;

		SoftwareInfo clientInfo;

		clientInfo.serializeFrom(request->clientsoftwareinfo());

		m_clientsInfo[authToken] = std::move(clientInfo);
	}

	reply->set_authtoken(authToken);

	return grpc::Status::OK;
}

bool GrpcSessionGuard::extractAndValidateAuthToken(grpc::ServerContext* context, std::string* authToken)
{
	if (context == nullptr)
	{
		return false;
	}

	if (authToken != nullptr)
	{
		authToken->clear();
	}

	const std::string token = extractAuthTokenFromMetadata(context);

	bool res = validateAuthToken(token);

	if (res && authToken != nullptr)
	{
		*authToken = token;
	}

	return res;
}

std::string GrpcSessionGuard::extractAuthTokenFromMetadata(grpc::ServerContext* context) const
{
	if (context == nullptr)
	{
		return {};
	}

	const auto& md = context->client_metadata();

	auto it = md.find(Grpc::SESSION_AUTH_TOKEN);

	if (it == md.end())
	{
		return {};
	}

	return std::string(it->second.data(), it->second.size());
}

void GrpcSessionGuard::setSessionTimeout(int seconds)
{
	m_sessionTimeout = seconds;
}

SoftwareInfo GrpcSessionGuard::getSoftwareInfo(const std::string& authToken)
{
	SoftwareInfo swInfo;

	std::lock_guard<std::mutex> lock(m_sessionsMutex);

	swInfo = getValueOrDefault(m_clientsInfo, authToken, SoftwareInfo{});

	return swInfo;
}

QString GrpcSessionGuard::getSoftwareEquipmentID(const std::string& authToken)
{
	QString equipmentID;

	std::lock_guard<std::mutex> lock(m_sessionsMutex);

	auto it = m_clientsInfo.find(authToken);

	if (it != m_clientsInfo.end())
	{
		equipmentID = it->second.equipmentID();
	}

	return equipmentID;
}

bool GrpcSessionGuard::validateAuthToken(const std::string& authToken)
{
	if (authToken.empty())
	{
		return false;
	}

	const auto now = std::chrono::steady_clock::now();

	std::lock_guard<std::mutex> lock(m_sessionsMutex);

	auto it = m_sessionExpirations.find(authToken);

	if (it == m_sessionExpirations.end())
	{
		return false;
	}

	if (it->second <= now)
	{
		m_clientsInfo.erase(authToken);
		m_sessionExpirations.erase(it);
		return false;
	}

	const auto newExpiresAt = now + std::chrono::seconds(m_sessionTimeout);

	it->second = newExpiresAt;

	return true;
}

bool GrpcSessionGuard::isValidClient(const Grpc::HandshakeRequest* request, std::string& errMsg) const
{
	if (m_allowAllClients.load(std::memory_order_relaxed))
	{
		return true;
	}

	const QString clientEquipmentID = QString::fromStdString(request->clientsoftwareinfo().equipmentid());

	for(const ClientInfo& ci : m_clients)
	{
		if (ci.equipmentID == clientEquipmentID)
		{
			if (m_checkHostName == true)
			{
				if (QString::fromStdString(request->clientsoftwareinfo().hostname()) == ci.hostname)
				{
					return true;
				}

				errMsg = Grpc::WRONG_HOST_NAME;
				return false;
			}

			return true;
		}
	}

	errMsg = Grpc::WRONG_CLIENT_EQUIPMENT_ID;

	return false;
}

void GrpcSessionGuard::sessionGuardLoop(std::stop_token stopToken) noexcept
{
	const auto period = std::chrono::seconds(SESSION_CHECK_PERIOD_SEC);

	while (!stopToken.stop_requested())
	{
		std::this_thread::sleep_for(period);

		const auto now = std::chrono::steady_clock::now();

		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		for (auto it = m_sessionExpirations.begin(); it != m_sessionExpirations.end(); )
		{
			if (it->second <= now)
			{
				const std::string authToken = it->first;
				m_clientsInfo.erase(authToken);
				it = m_sessionExpirations.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}
