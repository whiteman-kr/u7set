#ifndef CONFIGURATIONSERVICEWIDGET_H
#define CONFIGURATIONSERVICEWIDGET_H

class QStandardItemModel;
class TcpConfigServiceClient;

#include "BaseServiceStateWidget.h"

class ConfigurationServiceWidget : public BaseServiceStateWidget
{
	Q_OBJECT
public:
	ConfigurationServiceWidget(quint32 ip, int portIndex, QWidget *parent = 0);
	~ConfigurationServiceWidget();

public slots:
	void updateStateInfo();
	void updateBuildInfo();

private:
	void createTcpConnection(quint32 ip, quint16 port);
	void dropTcpConnection();

	QStandardItemModel* m_stateTabModel = nullptr;
	QStandardItemModel* m_buildTabModel = nullptr;
	TcpConfigServiceClient* m_tcpClientSocket = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;
};

#endif // CONFIGURATIONSERVICEWIDGET_H
