#ifndef CONFIGURATIONSERVICEWIDGET_H
#define CONFIGURATIONSERVICEWIDGET_H

class QLabel;

#include "BaseServiceStateWidget.h"

class ConfigurationServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);

public slots:
	void invalidateData();
	void parseData(UdpRequest udpRequest);

private:
	UdpClientSocket* m_clientSocket;
	UdpSocketThread m_clientSocketThread;
	QLabel* m_info;
	//ConfigurationServiceInfo configurationServiceInfo;
};

#endif // CONFIGURATIONSERVICEWIDGET_H
