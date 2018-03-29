#ifndef SOURCEWORKER_H
#define SOURCEWORKER_H

#include <QThread>

#include "../../lib/DataProtocols.h"

// ==============================================================================================

const int PS_SEND_FRAME_TIMEOUT = 5; // 5 ms

// ==============================================================================================

const int PS_FRAME_VERSION = 1;

// ==============================================================================================

const int PS_PORT = 65432;

// ==============================================================================================

#pragma pack(push, 1)

struct PsFrame
{
	Rup::Frame	rupFrame;

	quint16		version;
	quint32		destIP;
};

const int PsFrameSize = sizeof(PsFrame);

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

	PsFrame				m_psFrame;

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
