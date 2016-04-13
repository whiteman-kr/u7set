#ifndef MONITORCONFIGTHREAD_H
#define MONITORCONFIGTHREAD_H

#include <QThread>
#include "../include/CfgServerLoader.h"
#include "../include/SocketIO.h"

class MonitorConfigController : public QObject
{
	Q_OBJECT

public:
	MonitorConfigController() = delete;

	MonitorConfigController(HostAddressPort address1, HostAddressPort address2);
	virtual ~MonitorConfigController();

private:
	//void run() override;

public:
	//void reconnect(QString ip1, int port1, QString ip2, int port2, QString instanceStrId, int instanceNo);

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	// Data section
	//
private:
	CfgLoader* m_cfgLoader = nullptr;
	CfgLoaderThread* m_cfgLoaderThread = nullptr;
};

#endif // MONITORCONFIGTHREAD_H
