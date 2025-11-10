#include <CommonLib/ConstStrings.h>
#include <QUuid>

#include "GrpcSessionGuard.h"

GrpcSessionGuard::GrpcSessionGuard(const SoftwareInfo& severSwInfo,
									const std::vector<ClientInfo>& clients,
									bool checkHostName) :
	m_serverSwInfo(severSwInfo),
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

bool GrpcSessionGuard::handshake(const Grpc::HandshakeRequest* request,
								Grpc::HandshakeReply* reply)
{
	if (request == nullptr ||
		reply == nullptr)
	{
		return false;
	}

	m_serverSwInfo.serializeTo(reply->mutable_serversoftwareinfo());

	if (isValidClient(request) == false)
	{
		reply->set_authtoken("");
		return true;
	}

	const QUuid guid = QUuid::createUuid();
	const std::string authToken = guid.toString(QUuid::WithoutBraces).toStdString();

	const TimePoint expiresAt =	std::chrono::steady_clock::now() + std::chrono::seconds(SESSION_TIMEOUT_SEC);

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);

		m_sessionExpirations[authToken] = expiresAt;

		SoftwareInfo clientInfo;

		clientInfo.serializeFrom(request->clientsoftwareinfo());

		m_clientsInfo[authToken] = std::move(clientInfo);
	}

	reply->set_authtoken(authToken);

	return true;
}

bool GrpcSessionGuard::extractAndValidateAuthToken(grpc::ServerContext* context)
{
	if (context == nullptr)
	{
		return false;
	}

	const std::string authToken = extractAuthTokenFromMetadata(context);

	return validateAuthToken(authToken);
}

void GrpcSessionGuard::setAllowAllClients(bool allowAll)
{
	m_allowAllClients.store(allowAll, std::memory_order_relaxed);
}

std::string GrpcSessionGuard::extractAuthTokenFromMetadata(grpc::ServerContext* context) const
{
	if (context == nullptr)
	{
		return {};
	}

	const auto& md = context->client_metadata();

	static const std::string authTokenKey = Grpc::SESSION_AUTH_TOKEN.toStdString();

	auto it = md.find(authTokenKey);

	if (it == md.end())
	{
		return {};
	}

	return std::string(it->second.data(), it->second.size());
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

	const auto newExpiresAt = now + std::chrono::seconds(SESSION_TIMEOUT_SEC);

	it->second = newExpiresAt;

	return true;
}

bool GrpcSessionGuard::isValidClient(const Grpc::HandshakeRequest* request) const
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

				return false;
			}

			return true;
		}
	}

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
