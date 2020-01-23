#ifndef SOURCEWORKER_H
#define SOURCEWORKER_H

#include <QThread>

#include "../../lib/DataProtocols.h"

// ==============================================================================================

class SourceWorker : public QObject
{
	Q_OBJECT

public:

	explicit SourceWorker(QObject* pSource = nullptr);
	virtual ~SourceWorker();

private:

	QObject*			m_pSource = nullptr;
	Rup::SimFrame		m_simFrame;

	int					m_numerator = 0;
	int					m_sentFrames = 0;

	bool				m_finishThread = false;


public:

	bool				m_finished = false;

	bool				isRunnig() { return !m_finishThread; }
	int					sentFrames() { return m_sentFrames; }

signals:

	void				finished();

public slots:

	void				process();
	void				finish() { m_finishThread = true; }
	void				wait() { while(m_finished == false); }

};

// ==============================================================================================

#endif // SOURCEWORKER_H
