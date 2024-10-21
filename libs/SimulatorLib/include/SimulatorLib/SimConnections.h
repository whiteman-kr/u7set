#pragma once
#include <QObject>
#include <memory>
#include <vector>

#include <HardwareLib/ConnectionsInfo.h>

namespace Sim
{
	class ConnectionImpl;
	class ConnectionsImpl;

	//
	// Connection
	//
	class Connection
	{
	private:
		friend class Connections;
		explicit Connection(std::shared_ptr<ConnectionImpl> impl);

	public:
		~Connection();

	public:
		[[nodiscard]] bool isNull() const;

		operator bool() const;
		bool operator==(std::nullptr_t) const;

		QString connectionId() const;

		QString typeStr() const;
		Hardware::Connection::Type type() const;
		const ::ConnectionInfo& connectionInfo() const;

		std::vector<::ConnectionPortInfo> ports() const;

		bool enabled() const;
		void setEnabled(bool value);

		bool timeout() const;

	private:
		std::shared_ptr<ConnectionImpl> m_impl;
	};

	//
	// Connections
	//
	class Connections : public QObject
	{
		Q_OBJECT

	private:
		friend class SimulatorPrivate;
		explicit Connections(ConnectionsImpl& impl, QObject* parent);

	public:
		Connection connection(QString connectionId) const;
		std::vector<Connection> connections() const;
		std::vector<Connection> lmConnections(const QString& lmEquipmentId) const;

		void enableConnection(QString connectionId, bool enable);
		void disableConnection(QString connectionId, bool disable);

	signals:
		void connectionStateChanged(QString connectionId, bool state);

	private:
		ConnectionsImpl& m_impl;
	};

} // namespace Sim
