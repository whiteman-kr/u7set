#ifndef SOURCEWORKER_H
#define SOURCEWORKER_H

#include "../../lib/DataProtocols.h"

// ==============================================================================================

class SourceWorker : public QObject
{
	Q_OBJECT

public:

	explicit SourceWorker(QObject* pSource, const HostAddressPort& appDataSrvIP);
	virtual ~SourceWorker();

private:

	QObject*			m_pSource = nullptr;
	HostAddressPort		m_appDataSrvIP;				// this is AppDataReceivingIP of AppDataSrv

	Rup::SimFrame		m_simFrame;

	int					m_numerator = 0;
	int					m_sentFrames = 0;

	bool				m_finishThread = false;
	bool				m_threadIsFinished = false;

public:

	int					sentFrames() { return m_sentFrames; }

	bool				isRunnig() { return !m_finishThread; }
	void				wait() { while(m_threadIsFinished == false); }
	void				finish() { m_finishThread = true; }

signals:

	void				finished();

public slots:

	void				process();
};

// ==============================================================================================

#endif // SOURCEWORKER_H
