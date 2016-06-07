#include "ConfigurationServiceWidget.h"
#include <../lib/SocketIO.h>
#include <QLabel>

ConfigurationServiceWidget::ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent) :
	BaseServiceStateWidget(ip, portIndex, parent)
{
	m_clientSocket = new UdpClientSocket(QHostAddress(ip), PORT_CONFIGURATION_SERVICE_INFO);

	connect(m_clientSocket, &UdpClientSocket::ackTimeout, this, &ConfigurationServiceWidget::invalidateData);
	connect(m_clientSocket, &UdpClientSocket::ackReceived, this, &ConfigurationServiceWidget::parseData);

	m_clientSocketThread.addWorker(m_clientSocket);

	m_clientSocketThread.start();

	m_info = new QLabel(this);
	addTab(m_info, "Settings");

	invalidateData();
}

void ConfigurationServiceWidget::invalidateData()
{
	m_info->setText("Unknown service info");
	m_clientSocket->sendRequest(RQID_GET_CONFIGURATION_SERVICE_INFO);
}

void ConfigurationServiceWidget::parseData(UdpRequest udpRequest)
{
	switch (udpRequest.ID())
	{
		case RQID_GET_CONFIGURATION_SERVICE_INFO:
		{
			ConfigurationServiceInfo info;
			udpRequest.readStruct(&info);
			Builder::BuildInfo&& b = info.buildInfo();
			m_info->setText(QString("Project: %1 %2\n"
									"Date: %3"
									"User: %4"
									"Workstation: %5")
							.arg(b.project).arg(b.typeStr())
							.arg(b.dateStr())
							.arg(b.user)
							.arg(b.workstation));
			break;
		}
		default:
			quint32 value = udpRequest.ID();
			qDebug() << value;
			assert(false);
	}
}
