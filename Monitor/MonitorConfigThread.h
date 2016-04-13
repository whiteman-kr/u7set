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

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	int m_appInstanceNo = -1;

	CfgLoader* m_cfgLoader = nullptr;
	CfgLoaderThread* m_cfgLoaderThread = nullptr;
};

#endif // MONITORCONFIGTHREAD_H
