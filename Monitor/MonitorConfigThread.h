#ifndef MONITORCONFIGTHREAD_H
#define MONITORCONFIGTHREAD_H

#include <QThread>
#include "../include/CfgServerLoader.h"
#include "../include/SocketIO.h"

class MonitorConfigThread : public QThread
{
	Q_OBJECT

public:
	MonitorConfigThread(QString ip1, int port1, QString ip2, int port2, QString instanceStrId, int instanceNo);
	virtual ~MonitorConfigThread();

private:
	void run() override;

public:
	void reconnect(QString ip1, int port1, QString ip2, int port2, QString instanceStrId, int instanceNo);

	// Data section
	//
private:
	CfgLoader* m_cfgLoader = nullptr;
	Tcp::Thread* m_cfgLoaderThread = nullptr;
};

#endif // MONITORCONFIGTHREAD_H
