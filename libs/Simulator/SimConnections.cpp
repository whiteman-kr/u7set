#include "./include/Simulator/SimConnections.h"
#include "SimConnectionsImpl.h"

namespace Sim
{
	//
	// Sim::Connection
	//
	Connection::Connection(std::shared_ptr<ConnectionImpl> impl) :
		m_impl{impl}
	{
	}

	Connection::~Connection() = default;

	bool Connection::isNull() const
	{
		return m_impl == nullptr;
	}

	Connection::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool Connection::operator==(nullptr_t) const
	{
		return m_impl == nullptr;
	}

	QString Connection::connectionId() const
	{
		QString result = m_impl ? m_impl->connectionId() : QString{};
		return result;
	}

	QString Connection::typeStr() const
	{
		QString result = m_impl ? m_impl->typeStr() : QString{};
		return result;
	}

	Hardware::Connection::Type Connection::type() const
	{
		auto result = m_impl ? m_impl->type() : Hardware::Connection::Type{};
		return result;
	}

	const ::ConnectionInfo& Connection::connectionInfo() const
	{
		if (m_impl == nullptr)
		{
			Q_ASSERT(m_impl);
			static const ::ConnectionInfo empty;
			return empty;
		}

		return m_impl->connectionInfo();
	}

	std::vector<::ConnectionPortInfo> Connection::ports() const
	{
		std::vector<::ConnectionPortInfo> result;

		if (m_impl != nullptr)
		{
			auto ports = m_impl->ports();
			for (const auto& p : ports)
			{
				result.push_back(p.portInfo());
			}
		}

		return result;
	}

	bool Connection::enabled() const
	{
		auto result = m_impl ? m_impl->enabled() : false;
		return result;
	}

	void Connection::setEnabled(bool value)
	{
		if (m_impl != nullptr)
		{
			m_impl->setEnabled(value);
		}
	}

	bool Connection::timeout() const
	{
		auto result = m_impl ? m_impl->timeout() : false;
		return result;
	}


	//
	// Sim::Connections
	//
	Connections::Connections(ConnectionsImpl& impl, QObject* parent) :
		QObject{parent},
		m_impl{impl}
	{
		connect(&m_impl, &ConnectionsImpl::connectionStateChanged, this, &Connections::connectionStateChanged);
	}

	Connection Connections::connection(QString connectionId) const
	{
		return Connection{m_impl.connection(connectionId)};
	}

	std::vector<Connection> Connections::connections() const
	{
		auto cs = m_impl.connections();

		std::vector<Connection> result;
		result.reserve(cs.size());

		for (auto c : cs)
		{
			result.push_back(Connection{c});
		}

		return result;
	}

	std::vector<Connection> Connections::lmConnections(const QString& lmEquipmentId) const
	{
		auto cs = m_impl.lmConnections(lmEquipmentId);

		std::vector<Connection> result;
		result.reserve(cs.size());

		for (auto c : cs)
		{
			result.push_back(Connection{c});
		}

		return result;
	}

	void Connections::enableConnection(QString connectionId, bool enable)
	{
		return m_impl.enableConnection(connectionId, enable);
	}

	void Connections::disableConnection(QString connectionId, bool disable)
	{
		return m_impl.disableConnection(connectionId, disable);
	}
} // namespace Sim
