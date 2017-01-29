#ifndef MEASURESAVEDIALOG_H
#define MEASURESAVEDIALOG_H

#include <QMessageBox>
#include <QDialog>
#include <QProgressBar>
#include <QPushButton>

#include "MeasureView.h"
#include "ReportView.h"

// ==============================================================================================

#define EXPORT_WINDOW_TITLE QT_TRANSLATE_NOOP("ExportMeasure.h", "Export data")

// ==============================================================================================

class ExportData : public QObject
{
    Q_OBJECT

public:

                    ExportData(QTableView* pView, const QString& fileName);
                    ~ExportData();

    void            exec();

private:

    QTableView*     m_pView = nullptr;
    QString         m_fileName;

    QDialog*        m_pProgressDialog = nullptr;
    QProgressBar*   m_progress = nullptr;
    QPushButton*    m_cancelButton = nullptr;

    bool            m_exportCancel = true;

    static void     startExportThread(ExportData* pThis, const QString& fileName);
    bool            saveFile(QString fileName);

signals:

    void            setValue(int);
    void            setRange(int, int);

    void            exportThreadFinish();

public slots:

    void            exportCancel() { m_exportCancel = true; }
    void            exportComplited() { QMessageBox::information(m_pProgressDialog, EXPORT_WINDOW_TITLE, tr("Export is complited!")); }
};

// ==============================================================================================

#endif // MEASURESAVEDIALOG_H
