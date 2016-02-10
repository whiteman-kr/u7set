#include "BaseServiceStateWidget.h"
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QToolBar>
#include <QMessageBox>

BaseServiceStateWidget::BaseServiceStateWidget(quint32 ip, int portIndex, QWidget *parent) : QMainWindow(parent)
{
	QHostAddress host = QHostAddress(ip);
	setWindowTitle(QString(serviceInfo[portIndex].name) + " - " + host.toString());

	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	resize(640, 480);

	serviceState.serviceState = ServiceState::Undefined;

	QToolBar* toolBar = addToolBar("Service actions");
	startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	statusBar()->addWidget(m_whoIsLabel = new QLabel(this));
	statusBar()->addWidget(m_uptimeLabel = new QLabel(this));
	statusBar()->addWidget(m_runningLabel = new QLabel(this));

	m_baseClientSocket = new UdpClientSocket(QHostAddress(ip), serviceInfo[portIndex].port);
	connect(m_baseClientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceStateWidget::serviceNotFound);
	connect(m_baseClientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceStateWidget::serviceAckReceived);

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &BaseServiceStateWidget::askServiceState);
	timer->start(500);

	updateServiceState();
}

BaseServiceStateWidget::~BaseServiceStateWidget()
{
	delete m_baseClientSocket;
}

void BaseServiceStateWidget::updateServiceState()
{
	m_whoIsLabel->setText("Base Service" /*serviceTypeStr[SERVICE_DATA_ACQUISITION]*/ + QString(" v%1.%2.%3(0x%4)")
						  .arg(serviceState.majorVersion)
						  .arg(serviceState.minorVersion)
						  .arg(serviceState.buildNo)
						  .arg(serviceState.crc, 0, 16, QChar('0')));

	if (serviceState.serviceState != ServiceState::Undefined && serviceState.serviceState != ServiceState::Unavailable)
	{
		quint32 time = serviceState.uptime;
		int s = time % 60; time /= 60;
		int m = time % 60; time /= 60;
		int h = time % 24; time /= 24;
		m_uptimeLabel->setText(tr("Uptime") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
	}
	else
	{
		m_uptimeLabel->setText(tr("Uptime") + "???");
	}

	QString str;
	switch (serviceState.serviceState)
	{
		case ServiceState::Work:
		{
			quint32 time = serviceState.serviceUptime;
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

	switch(serviceState.serviceState)
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

	switch (serviceState.serviceState)
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
			serviceState = *(ServiceInformation*)udpRequest.data();
			updateServiceState();
		}
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
	if (serviceState.serviceState != ServiceState::Unavailable)
	{
		serviceState.serviceState = ServiceState::Unavailable;
		updateServiceState();
	}
}

int BaseServiceStateWidget::addTab(QWidget* page, const QString& label)
{
	return m_tabWidget->addTab(page, label);
}

void BaseServiceStateWidget::sendCommand(int command)
{
	int state = serviceState.serviceState;
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
