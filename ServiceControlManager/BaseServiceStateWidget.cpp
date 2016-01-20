#include "BaseServiceStateWidget.h"
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QToolBar>
#include <QMessageBox>

BaseServiceStateWidget::BaseServiceStateWidget(quint32 ip, int portIndex, QWidget *parent) : QMainWindow(parent)
{
	QHostAddress host = QHostAddress(ip);
	setWindowTitle(QString(serviceTypesInfo[portIndex].name) + " - " + host.toString());

	resize(640, 480);

	serviceState.mainFunctionState = SS_MF_UNDEFINED;

	QToolBar* toolBar = addToolBar("Service actions");
	startServiceButton = toolBar->addAction("Start", this, SLOT(startService()));
	stopServiceButton = toolBar->addAction("Stop", this, SLOT(stopService()));
	restartServiceButton = toolBar->addAction("Restart", this, SLOT(restartService()));

	statusBar()->addWidget(m_whoIsLabel = new QLabel(this));
	statusBar()->addWidget(m_uptimeLabel = new QLabel(this));
	statusBar()->addWidget(m_runningLabel = new QLabel(this));

	m_clientSocket = new UdpClientSocket(QHostAddress(ip), serviceTypesInfo[portIndex].port);
	connect(m_clientSocket, &UdpClientSocket::ackTimeout, this, &BaseServiceStateWidget::serviceNotFound);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &BaseServiceStateWidget::serviceAckReceived);

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &BaseServiceStateWidget::askServiceState);
	timer->start(500);

	updateServiceState();
}

BaseServiceStateWidget::~BaseServiceStateWidget()
{

}

void BaseServiceStateWidget::updateServiceState()
{
	m_whoIsLabel->setText(serviceTypeStr[SERVICE_DATA_ACQUISITION] + QString(" v%1.%2.%3(0x%4)")
						  .arg(serviceState.majorVersion)
						  .arg(serviceState.minorVersion)
						  .arg(serviceState.buildNo)
						  .arg(serviceState.crc, 0, 16, QChar('0')));

	if (serviceState.mainFunctionState != SS_MF_UNDEFINED && serviceState.mainFunctionState != SS_MF_UNAVAILABLE)
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
	switch (serviceState.mainFunctionState)
	{
		case SS_MF_WORK:
		{
			quint32 time = serviceState.mainFunctionUptime;
			int s = time % 60; time /= 60;
			int m = time % 60; time /= 60;
			int h = time % 24; time /= 24;
			str = tr("Running") + QString(" (%1d %2:%3:%4)").arg(time).arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
		} break;
		case SS_MF_STOPPED:
			str = tr("Stopped");
			break;
		case SS_MF_UNAVAILABLE:
			str = tr("Unavailable");
			break;
		case SS_MF_UNDEFINED:
			str = tr("Undefined");
			break;
		case SS_MF_STARTS:
			str = tr("Starts");
			break;
		case SS_MF_STOPS:
			str = tr("Stops");
			break;
		default:
			str = tr("Unknown state");
			break;
	}
	m_runningLabel->setText(str);
	switch(serviceState.mainFunctionState)
	{
		case SS_MF_WORK:
			m_runningLabel->setStyleSheet("background-color: rgb(127, 255, 127);");
			break;
		case SS_MF_STARTS:
		case SS_MF_STOPS:
		case SS_MF_STOPPED:
			m_runningLabel->setStyleSheet("background-color: rgb(255, 255, 127);");
			break;
		case SS_MF_UNAVAILABLE:
			m_runningLabel->setStyleSheet("background-color: lightGray;");
			break;
		default:
			m_runningLabel->setStyleSheet("background-color: red;");
	}

	switch (serviceState.mainFunctionState)
	{
		case SS_MF_WORK:
			startServiceButton->setEnabled(false);
			stopServiceButton->setEnabled(true);
			restartServiceButton->setEnabled(true);
			break;
		case SS_MF_STOPPED:
			startServiceButton->setEnabled(true);
			stopServiceButton->setEnabled(false);
			restartServiceButton->setEnabled(true);
			break;
		case SS_MF_UNAVAILABLE:
		case SS_MF_UNDEFINED:
		case SS_MF_STARTS:
		case SS_MF_STOPS:
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
	m_clientSocket->sendShortRequest(RQID_GET_SERVICE_INFO);
}

void BaseServiceStateWidget::startService()
{
	sendCommand(RQID_SERVICE_MF_START);
}

void BaseServiceStateWidget::stopService()
{
	sendCommand(RQID_SERVICE_MF_STOP);
}

void BaseServiceStateWidget::restartService()
{
	sendCommand(RQID_SERVICE_MF_RESTART);
}

void BaseServiceStateWidget::serviceAckReceived(const UdpRequest udpRequest)
{
	switch (udpRequest.ID())
	{
		case RQID_GET_SERVICE_INFO:
		{
			serviceState = *(ServiceInformation*)udpRequest.data();
			updateServiceState();
		}
		case RQID_SERVICE_MF_START:
		case RQID_SERVICE_MF_STOP:
		case RQID_SERVICE_MF_RESTART:
			break;
		default:
			qDebug() << "Unknown packet ID";
	}
}

void BaseServiceStateWidget::serviceNotFound()
{
	if (serviceState.mainFunctionState != SS_MF_UNAVAILABLE)
	{
		serviceState.mainFunctionState = SS_MF_UNAVAILABLE;
		updateServiceState();
	}
}

void BaseServiceStateWidget::sendCommand(int command)
{
	int state = serviceState.mainFunctionState;
	if (!(state == SS_MF_WORK && (command == RQID_SERVICE_MF_STOP || command == RQID_SERVICE_MF_RESTART)) &&
		!(state == SS_MF_STOPPED && (command == RQID_SERVICE_MF_START || command == RQID_SERVICE_MF_RESTART)))
	{
		return;
	}
	if (m_clientSocket->isWaitingForAck())
	{
		QMessageBox::critical(this, tr("Command send error"), tr("Socket is waiting for ack, repeat your command later."));
		return;
	}
	m_clientSocket->sendShortRequest(command);
}
