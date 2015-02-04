#ifndef MEASURESAVEDIALOG_H
#define MEASURESAVEDIALOG_H

#include <QMessageBox>
#include <QDialog>
#include <QProgressBar>
#include <QPushButton>

#include "MeasureView.h"
#include "ReportView.h"

// ==============================================================================================

#define EXPORT_WINDOW_TITLE QT_TRANSLATE_NOOP("ExportMeasure.h", "Export measurements")

// ==============================================================================================

class ExportMeasure : public QObject
{
    Q_OBJECT

public:

    explicit        ExportMeasure(MeasureView* pMeasureView = nullptr);
                    ~ExportMeasure();

    void            exec();

private:

    MeasureView*    m_pMeasureView = nullptr;

    QDialog*        m_pDialog = nullptr;
    QProgressBar*   m_progress = nullptr;
    QPushButton*    m_cancelButton = nullptr;

    bool            m_exportCancel = true;

    static void     startExportThread(ExportMeasure* pThis, QString fileName);
    bool            saveFile(QString fileName);

signals:

    void            setValue(int);
    void            setRange(int, int);

    void            exportThreadFinish();

public slots:

    void            exportCancel() { m_exportCancel = true; }
    void            exportComplited() { QMessageBox::information(m_pDialog, EXPORT_WINDOW_TITLE, tr("Export is complited!")); }
};

// ==============================================================================================

#endif // MEASURESAVEDIALOG_H
