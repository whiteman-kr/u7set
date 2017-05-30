#include "BaseServiceStateWidget.h"
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QToolBar>
#include <QMessageBox>
#include "../lib/Types.h"


BaseServiceStateWidget::BaseServiceStateWidget(quint32 ip, int portIndex, QWidget *parent) :
	QMainWindow(parent),
	m_ip(ip),
	m_portIndex(portIndex)
{
	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	resize(640, 480);

	m_serviceInfo.set_type(portIndex);
	m_serviceInfo.set_servicestate(TO_INT(ServiceState::Undefined));

	QToolBar* toolBar = addToolBar("Service actions");
	m_startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	m_stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	m_restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	statusBar()->addWidget(m_connectionStateStatus = new QLabel(this));
	statusBar()->addWidget(m_uptimeStatus = new QLabel(this));
	statusBar()->addWidget(m_runningStatus = new QLabel(this));

	m_socketThread = new UdpSocketThread();

	m_baseClientSocket = new UdpClientSocket(QHostAddress(ip), serviceInfo[portIndex].port);
	connect(m_baseClientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceStateWidget::serviceNotFound);
	connect(m_baseClientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceStateWidget::serviceAckReceived);

	m_socketThread->addWorker(m_baseClientSocket);
	m_socketThread->start();

	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &BaseServiceStateWidget::askServiceState);
	m_timer->start(500);

	updateServiceState();
}

BaseServiceStateWidget::~BaseServiceStateWidget()
{
	m_timer->stop();

	if (m_socketThread)
	{
		m_socketThread->quitAndWait();
		delete m_socketThread;
	}
}

void BaseServiceStateWidget::updateServiceState()
{
	QString serviceName = "Unknown Service";
	QString serviceShortName = "???";
	for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
	{
		if (static_cast<ServiceType>(m_serviceInfo.type()) == serviceInfo[i].serviceType)
		{
			serviceName = serviceInfo[i].name;
			serviceShortName = serviceInfo[i].shortName;
			break;
		}
	}

	assert(serviceShortName != "???");

	ServiceState serviceState = static_cast<ServiceState>(m_serviceInfo.servicestate());

	switch (serviceState)
	{
		case Stopped:
		case Starts:
		case Work:
		case Stops:
			{
				setWindowTitle(serviceName +
							   QString(" v%1.%2.%3 - %4:%5")
							   .arg(m_serviceInfo.majorversion())
							   .arg(m_serviceInfo.minorversion())
							   .arg(m_serviceInfo.commitno())
							   .arg(QHostAddress(m_ip).toString())
							   .arg(serviceInfo[m_portIndex].port));

				m_connectionStateStatus->setText("Connected to " + serviceShortName + QString(" - %1").arg(m_udpAckQuantity));

				quint32 time = m_serviceInfo.uptime();

				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;

				m_uptimeStatus->setText(tr("Uptime ") + QString("(%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
				m_uptimeStatus->setHidden(false);

				m_runningStatus->setHidden(false);
			}

			break;

		case Undefined:
		case Unavailable:
			setWindowTitle(serviceName + " - No connection");

			m_connectionStateStatus->setText("No connection with " + serviceShortName);

			m_uptimeStatus->setHidden(true);
			m_runningStatus->setHidden(true);

			break;

		default:
			assert(false);
	}

	QString runningStateStr;
	switch (serviceState)
	{
		case ServiceState::Work:
			{
				quint32 time = m_serviceInfo.serviceuptime();

				int s = time % 60; time /= 60;
				int m = time % 60; time /= 60;
				int h = time % 24; time /= 24;

				runningStateStr = tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
			}
			break;

		case ServiceState::Stopped:
			runningStateStr = tr("Stopped");
			break;

		case ServiceState::Unavailable:
			runningStateStr = tr("Unavailable");
			break;

		case ServiceState::Undefined:
			runningStateStr = tr("Undefined");
			break;

		case ServiceState::Starts:
			runningStateStr = tr("Starts");
			break;

		case ServiceState::Stops:
			runningStateStr = tr("Stops");
			break;

		default:
			assert(false);
			runningStateStr = tr("Unknown state");
			break;
	}

	m_runningStatus->setText(runningStateStr);

	switch(serviceState)
	{
		case ServiceState::Work:
			m_runningStatus->setStyleSheet("background-color: rgb(127, 255, 127);");
			break;
		case ServiceState::Starts:
		case ServiceState::Stops:
		case ServiceState::Stopped:
			m_runningStatus->setStyleSheet("background-color: rgb(255, 255, 127);");
			break;
		case ServiceState::Unavailable:
			m_runningStatus->setStyleSheet("background-color: lightGray;");
			break;

		default:
			m_runningStatus->setStyleSheet("background-color: red;");
	}

	switch (serviceState)
	{
		case ServiceState::Work:
			m_startServiceButton->setEnabled(false);
			m_stopServiceButton->setEnabled(true);
			m_restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Stopped:
			m_startServiceButton->setEnabled(true);
			m_stopServiceButton->setEnabled(false);
			m_restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Unavailable:
		case ServiceState::Undefined:
		case ServiceState::Starts:
		case ServiceState::Stops:
			m_startServiceButton->setEnabled(false);
			m_stopServiceButton->setEnabled(false);
			m_restartServiceButton->setEnabled(false);
			break;
		default:
			assert(false);
			break;
	}

	emit connectionStatisticChanged();
}

void BaseServiceStateWidget::askServiceState()
{
	if (!m_baseClientSocket->isWaitingForAck())
	{
		m_baseClientSocket->sendRequest(RQID_SERVICE_GET_INFO);
	}
}

void BaseServiceStateWidget::startService()
{
	sendCommand(RQID_SERVICE_START);
}

void BaseServiceStateWidget::stopService()
{
	sendCommand(RQID_SERVICE_STOP);
}

void BaseServiceStateWidget::restartService()
{
	sendCommand(RQID_SERVICE_RESTART);
}

void BaseServiceStateWidget::serviceAckReceived(const UdpRequest udpRequest)
{
	m_udpAckQuantity++;

	switch (udpRequest.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			Network::ServiceInfo newServiceState;

			bool result = newServiceState.ParseFromArray(udpRequest.data(), udpRequest.dataSize());

			assert(result == true);

			ServiceState oldState = static_cast<ServiceState>(m_serviceInfo.servicestate());
			ServiceState newState = static_cast<ServiceState>(newServiceState.servicestate());

			if (newState != ServiceState::Work && oldState == ServiceState::Work)
			{
				emit invalidateData();
			}

			if (newState == ServiceState::Work &&
					(oldState != ServiceState::Work || newServiceState.serviceuptime() < m_serviceInfo.serviceuptime()))
			{
				emit needToReloadData();
			}

			m_serviceInfo = newServiceState;

			updateServiceState();
		}
		break;
		case RQID_SERVICE_START:
		case RQID_SERVICE_STOP:
		case RQID_SERVICE_RESTART:
			break;
		default:
			qDebug() << "Unknown packet ID";
	}
}

void BaseServiceStateWidget::serviceNotFound()
{
	m_udpAckQuantity = 0;

	if (m_serviceInfo.servicestate() != TO_INT(ServiceState::Unavailable))
	{
		m_serviceInfo.set_servicestate(TO_INT(ServiceState::Unavailable));
		updateServiceState();
	}
}

int BaseServiceStateWidget::addTab(QWidget* page, const QString& label)
{
	return m_tabWidget->addTab(page, label);
}

void BaseServiceStateWidget::sendCommand(int command)
{
	ServiceState state = static_cast<ServiceState>(m_serviceInfo.servicestate());

	if (!(state == ServiceState::Work && (command == RQID_SERVICE_STOP || command == RQID_SERVICE_RESTART)) &&
		!(state == ServiceState::Stopped && (command == RQID_SERVICE_START || command == RQID_SERVICE_RESTART)))
	{
		return;
	}
	if (m_baseClientSocket->isWaitingForAck())
	{
		QMessageBox::critical(this, tr("Command send error"), tr("Socket is waiting for ack, repeat your command later."));
		return;
	}
	m_baseClientSocket->sendRequest(command);
}
