#ifndef SOURCEWORKER_H
#define SOURCEWORKER_H

#include <QThread>

#include "../../lib/DataProtocols.h"

// ==============================================================================================

#pragma pack(push, 1)

namespace PS
{
	const int SUPPORT_VERSION = 5; // last version of Rup::VERSION

	const int FrameVersion = 1;

	struct Frame
	{
		Rup::Frame	rupFrame;

		quint16		version = FrameVersion;
		quint32		lmIP = 0;
	};

	const int UDP_PORT = 65432;

	const int SendFrameTimeout = 5; // 5 ms
}

#pragma pack(pop)

// ==============================================================================================

class SourceWorker : public QObject
{
	Q_OBJECT

public:

	explicit SourceWorker(QObject* pSource = 0);
	virtual ~SourceWorker();

private:

	QObject*			m_pSource = nullptr;

	bool				m_finishThread = false;

	PS::Frame			m_psFrame;

	int					m_numerator = 0;

public:


signals:

	void				finished();

private slots:


public slots:

	void				process();
	void				finish() { m_finishThread = true; }

};

// ==============================================================================================

#endif // SOURCEWORKER_H
