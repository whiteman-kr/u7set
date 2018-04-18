#ifndef SOURCEWORKER_H
#define SOURCEWORKER_H

#include <QThread>

#include "../../lib/DataProtocols.h"

// ==============================================================================================

class SourceWorker : public QObject
{
	Q_OBJECT

public:

	explicit SourceWorker(QObject* pSource = 0);
	virtual ~SourceWorker();

private:

	QObject*			m_pSource = nullptr;
	Rup::SimFrame		m_simFrame;

	int					m_numerator = 0;
	int					m_sentFrames = 0;

	bool				m_finishThread = false;

public:

	bool				isRunnig() { return !m_finishThread; }
	int					sentFrames() { return m_sentFrames; }

signals:

	void				finished();

public slots:

	void				process();
	void				finish() { m_finishThread = true; }

};

// ==============================================================================================

#endif // SOURCEWORKER_H
