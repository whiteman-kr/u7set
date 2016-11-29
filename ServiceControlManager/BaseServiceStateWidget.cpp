#include "BaseServiceStateWidget.h"
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QToolBar>
#include <QMessageBox>
#include "../lib/Types.h"


BaseServiceStateWidget::BaseServiceStateWidget(quint32 ip, int portIndex, QWidget *parent) : QMainWindow(parent)
{
	QHostAddress host = QHostAddress(ip);
	setWindowTitle(QString(serviceInfo[portIndex].name) + " - " + host.toString());

	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	resize(640, 480);

	serviceState.set_servicestate(TO_INT(ServiceState::Undefined));

	QToolBar* toolBar = addToolBar("Service actions");
	startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	statusBar()->addWidget(m_whoIsLabel = new QLabel(this));
	statusBar()->addWidget(m_uptimeLabel = new QLabel(this));
	statusBar()->addWidget(m_runningLabel = new QLabel(this));
	statusBar()->addWidget(m_clientRequestAddressLabel = new QLabel(this));

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
	QString serviceType;
	switch (static_cast<ServiceType>(serviceState.type()))
	{
		case ServiceType::BaseService:
			serviceType = "Base Service";
		break;
		case ServiceType::ConfigurationService:
			serviceType = "Configuration Service";
		break;
		case ServiceType::AppDataService:
			serviceType = "Application Data Service";
		break;
		case ServiceType::TuningService:
			serviceType = "Tuning Service";
		break;
		case ServiceType::ArchivingService:
			serviceType = "Archiving Service";
		break;
		case ServiceType::DiagDataService:
			serviceType = "Diagnostic Data Service";
		break;
		break;
		default:
			assert(false);
		break;
	}
	m_whoIsLabel->setText(serviceType + QString(" v%1.%2.%3(0x%4)")
						  .arg(serviceState.majorversion())
						  .arg(serviceState.minorversion())
						  .arg(serviceState.buildno())
						  .arg(serviceState.crc(), 0, 16, QChar('0')));

	ServiceState srvState = static_cast<ServiceState>(serviceState.servicestate());

	if (srvState != ServiceState::Undefined &&
		srvState != ServiceState::Unavailable)
	{
		quint32 time = serviceState.uptime();
		int s = time % 60; time /= 60;
		int m = time % 60; time /= 60;
		int h = time % 24; time /= 24;
		m_uptimeLabel->setText(tr("Uptime ") + QString("(%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
	}
	else
	{
		m_uptimeLabel->setText(tr("Uptime ") + "???");
	}

	QString str;
	switch (srvState)
	{
		case ServiceState::Work:
		{
			quint32 time = serviceState.serviceuptime();
			int s = time % 60; time /= 60;
			int m = time % 60; time /= 60;
			int h = time % 24; time /= 24;
			str = tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
		} break;
		case ServiceState::Stopped:
			str = tr("Stopped");
			break;
		case ServiceState::Unavailable:
			str = tr("Unavailable");
			break;
		case ServiceState::Undefined:
			str = tr("Undefined");
			break;
		case ServiceState::Starts:
			str = tr("Starts");
			break;
		case ServiceState::Stops:
			str = tr("Stops");
			break;
		default:
			assert(false);
			str = tr("Unknown state");
			break;
	}
	m_runningLabel->setText(str);

	if (srvState != ServiceState::Undefined &&
		srvState != ServiceState::Unavailable)
	{
		m_clientRequestAddressLabel->setText(tr("Listening clients on %1:%2")
										.arg(QHostAddress(serviceState.clientrequestip()).toString())
										.arg(serviceState.clientrequestport()));
	}
	else
	{
		m_clientRequestAddressLabel->setText(tr("Listening clients on ???:???"));
	}

	switch(srvState)
	{
		case ServiceState::Work:
			m_runningLabel->setStyleSheet("background-color: rgb(127, 255, 127);");
			break;
		case ServiceState::Starts:
		case ServiceState::Stops:
		case ServiceState::Stopped:
			m_runningLabel->setStyleSheet("background-color: rgb(255, 255, 127);");
			break;
		case ServiceState::Unavailable:
			m_runningLabel->setStyleSheet("background-color: lightGray;");
			break;

		default:
			m_runningLabel->setStyleSheet("background-color: red;");
	}

	switch (srvState)
	{
		case ServiceState::Work:
			startServiceButton->setEnabled(false);
			stopServiceButton->setEnabled(true);
			restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Stopped:
			startServiceButton->setEnabled(true);
			stopServiceButton->setEnabled(false);
			restartServiceButton->setEnabled(true);
			break;
		case ServiceState::Unavailable:
		case ServiceState::Undefined:
		case ServiceState::Starts:
		case ServiceState::Stops:
			startServiceButton->setEnabled(false);
			stopServiceButton->setEnabled(false);
			restartServiceButton->setEnabled(false);
			break;
		default:
			assert(false);
			break;
	}
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
	switch (udpRequest.ID())
	{
		case RQID_SERVICE_GET_INFO:
		{
			Network::ServiceInfo newServiceState;

			bool result = newServiceState.ParseFromArray(udpRequest.data(), udpRequest.dataSize());

			assert(result == true);

			ServiceState oldState = static_cast<ServiceState>(serviceState.servicestate());
			ServiceState newState = static_cast<ServiceState>(newServiceState.servicestate());

			if (newState != ServiceState::Work && oldState == ServiceState::Work)
			{
				emit invalidateData();
			}

			if (newState == ServiceState::Work &&
					(oldState != ServiceState::Work || newServiceState.serviceuptime() < serviceState.serviceuptime()))
			{
				emit needToReloadData();
			}

			serviceState = newServiceState;

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
	if (serviceState.servicestate() != TO_INT(ServiceState::Unavailable))
	{
		serviceState.set_servicestate(TO_INT(ServiceState::Unavailable));
		updateServiceState();
	}
}

int BaseServiceStateWidget::addTab(QWidget* page, const QString& label)
{
	return m_tabWidget->addTab(page, label);
}

void BaseServiceStateWidget::sendCommand(int command)
{
	ServiceState state = static_cast<ServiceState>(serviceState.servicestate());

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
