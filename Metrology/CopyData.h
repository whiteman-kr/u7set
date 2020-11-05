#ifndef COPYDATADIALOG_H
#define COPYDATADIALOG_H

#include <QTableView>

// ==============================================================================================

class CopyData : public QObject
{
	Q_OBJECT

public:

	CopyData(QTableView* pView, bool copyHiddenColumn);
	virtual ~CopyData();

private:

	QTableView*		m_pView = nullptr;
	bool			m_copyHiddenColumn = false;

	bool			m_copyCancel = true;

	bool			copyToMemory();

public:

	void			exec();

signals:

public slots:

	void			copyCancel();
	void			copyComplited();

};

// ==============================================================================================

#endif // COPYDATADIALOG_H
