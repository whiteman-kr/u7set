#ifndef FINDDATADIALOG_H
#define FINDDATADIALOG_H

#include <QTableView>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

// ==============================================================================================

class FindData : public QObject
{
	Q_OBJECT

public:

	FindData(QTableView* pView);
	virtual ~FindData();

private:

	QTableView*		m_pView = nullptr;

	QDialog*		m_pFindDialog = nullptr;
	QLineEdit*		m_pFindTextEdit = nullptr;
	QPushButton*	m_findNextButton = nullptr;

	static QString	m_findText;

	void			createInterface(QTableView *pView);

	int				find(int start);
	void			enableFindNextButton(int start);

public:

	int				exec();

signals:

public slots:

	void			findTextChanged();
	void			findNext();
};

// ==============================================================================================

#endif // FINDDATADIALOG_H
