#ifndef EXPORTDATADIALOG_H
#define EXPORTDATADIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QPushButton>
#include <QMessageBox>

// ==============================================================================================

#define EXPORT_WINDOW_TITLE QT_TRANSLATE_NOOP("ExportData.h", "Export data")

// ==============================================================================================

class ExportData : public QObject
{
	Q_OBJECT

public:

	ExportData(QTableView* pView, const QString& fileName);
	virtual ~ExportData();

private:

	QTableView*		m_pView = nullptr;
	QString			m_fileName;

	QDialog*		m_pProgressDialog = nullptr;
	QProgressBar*	m_progress = nullptr;
	QPushButton*	m_cancelButton = nullptr;

	bool			m_exportCancel = true;

	void			createProgressDialog(QTableView *pView);
	static void		startExportThread(ExportData* pThis, const QString& fileName);
	bool			saveFile(QString fileName);

public:

	void			exec();

signals:

	void			setValue(int);
	void			setRange(int, int);

	void			exportThreadFinish();

public slots:

	void			exportCancel() { m_exportCancel = true; }
	void			exportComplited() { QMessageBox::information(m_pProgressDialog, EXPORT_WINDOW_TITLE, tr("Export is complited!")); }
};

// ==============================================================================================

#endif // EXPORTDATADIALOG_H
