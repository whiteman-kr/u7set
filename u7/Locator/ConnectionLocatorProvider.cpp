#include "ConnectionLocatorProvider.h"
#include "../../Builder/ConnectionStorage.h"

namespace Locator
{
	ConnectionLocatorThread::ConnectionLocatorThread(DbController* dbc) :
		m_mainDbc{dbc}
	{
		Q_ASSERT(dbc);

		m_dbc.disableProgress();

		connect(dbc, &DbController::projectOpened, this, &ConnectionLocatorThread::openConnection, Qt::QueuedConnection);
		connect(dbc, &DbController::projectClosed, this, &ConnectionLocatorThread::closeConnection, Qt::QueuedConnection);

		return;
	}

	void ConnectionLocatorThread::updateText(const QString& text)
	{
		m_textMutex.lock();
		m_text = text;
		m_textMutex.unlock();
	}

	void ConnectionLocatorThread::openConnection()
	{
		m_dbc.setHost(m_mainDbc->host());
		m_dbc.setPort(m_mainDbc->port());
		m_dbc.setServerUsername(m_mainDbc->serverUsername());
		m_dbc.setServerPassword(m_mainDbc->serverPassword());

		bool opened = m_dbc.openProject(m_mainDbc->currentProject().projectName(),
										m_mainDbc->currentUser().username(),
										m_mainDbc->currentUser().password(),
										nullptr);
		if (opened == false)
		{
			qDebug() << "ConnectionLocatorThread::openConnection(): Error opening project " <<
						m_mainDbc->currentProject().projectName() <<
						"\n\tError: " << m_mainDbc->lastError();
		}

		return;
	}

	void ConnectionLocatorThread::closeConnection()
	{
		if (m_dbc.isProjectOpened() == true)
		{
			m_dbc.closeProject(nullptr);
		}

		return;
	}

	void ConnectionLocatorThread::newTask(const QString& text)
	{
		if (text.isEmpty() == true)
		{
			emit resultReady(text, {});
			return;
		}

		{
			QMutexLocker locker{&m_textMutex};
			if (m_text != text)
			{
				// This is an outdated message
				//
				return;
			}
		}

		// Get connections form the db
		//
		Builder::ConnectionStorage connections{&m_dbc};

		QString errorMessage;
		bool ok = connections.load(&errorMessage);

		if (ok == false)
		{
			qDebug() << "ConnectionLocatorThread: Error loading connections, " << errorMessage;
			emit resultReady(text, {});
			return;
		}

		std::vector<LocatedItem> result;
		result.reserve(8);
		for (const std::shared_ptr<Hardware::Connection>& c : connections)
		{
			if (c->connectionID().contains(text, Qt::CaseInsensitive) == true)
			{
				result.emplace_back(c->connectionID(),
									QString{"%1 connection"}.arg(c->typeStr()),
									QVariant{c->connectionID()});

				if (c->port1EquipmentID().isEmpty() == false)
				{
					result.emplace_back(c->port1EquipmentID(),
										QString{"Port1 in connection %1"}.arg(c->connectionID()),
										QVariant{c->connectionID()});
				}

				if (c->port2EquipmentID().isEmpty() == false)
				{
					result.emplace_back(c->port2EquipmentID(),
										QString{"Port2 in connection %1"}.arg(c->connectionID()),
										QVariant{c->connectionID()});
				}
			}

			if (c->port1EquipmentID().compare(text, Qt::CaseInsensitive) == 0)
			{
				result.emplace_back(c->port1EquipmentID(),
									QString{"Port1 in connection %1"}.arg(c->connectionID()),
									QVariant{c->connectionID()});
			}

			if (c->port2EquipmentID().compare(text, Qt::CaseInsensitive) == 0)
			{
				result.emplace_back(c->port2EquipmentID(),
									QString{"Port2 in connection %1"}.arg(c->connectionID()),
									QVariant{c->connectionID()});
			}
		}

		emit resultReady(text, std::move(result));
		return;
	}

	ConnectionLocatorProvider::ConnectionLocatorProvider(DbController* dbc) :
		m_thread{dbc}
	{
		m_thread.moveToThread(&m_thread);
		m_thread.start();

		connect(this, &ConnectionLocatorProvider::newTask, &m_thread, &ConnectionLocatorThread::newTask, Qt::QueuedConnection);
		connect(&m_thread, &ConnectionLocatorThread::resultReady, this, &ConnectionLocatorProvider::taskResultReady, Qt::QueuedConnection);
		return;
	}

	ConnectionLocatorProvider::~ConnectionLocatorProvider()
	{
		m_thread.exit();
		m_thread.wait(10000);
		return;
	}

	QString ConnectionLocatorProvider::name() const
	{
		return QStringLiteral("Connection");
	}

	void ConnectionLocatorProvider::locateFor(const QString& text)
	{
		if (text.size() < 1)
		{
			emit resultReady(text, this, {});
			return;
			return;
		}

		m_text = text;
		m_thread.updateText(text);

		emit newTask(text);
		return;
	}

	void ConnectionLocatorProvider::stopSearching()
	{
		return;
	}

	void ConnectionLocatorProvider::taskResultReady(QString text, const std::vector<LocatedItem>& result)
	{
		if (text != m_text)
		{
			// This is some old search
			//
			return;
		}

		emit resultReady(text, this, result);
		return;
	}
}

