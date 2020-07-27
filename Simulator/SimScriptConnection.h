#pragma once
#include "SimConnections.h"

namespace Sim
{
	class Simulator;


	class ScriptConnection : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(QString connectionID READ connectionId)
		Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
		Q_PROPERTY(bool timeout READ timeout)

	public:
		ScriptConnection() = default;
		ScriptConnection(const ScriptConnection& src);
		explicit ScriptConnection(std::shared_ptr<Connection> connection);

		ScriptConnection& operator=(const ScriptConnection& src);

	public:
		bool isNull() const;

		QString connectionId() const;

		bool enabled() const;
		void setEnabled(bool value);

		bool timeout() const;

	private:
		std::shared_ptr<Connection> m_connection;
	};

}

