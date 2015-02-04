#ifndef REPORTVIEW_H
#define REPORTVIEW_H

#include <QAction>

#include "Options.h"
#include "MeasureView.h"

#include "ncreport.h"
#include "ncreportoutput.h"
#include "ncreportpreviewoutput.h"
#include "ncreportpreviewwindow.h"

// ==============================================================================================

#define             REPORT_MENU_FILE        0
#define             REPORT_MENU_NAVIGATE    2
#define             REPORT_MENU_ABOUT       4

// ==============================================================================================

class ReportView : public QObject
{
    Q_OBJECT
public:
    explicit        ReportView(QObject *parent = 0);
                    ~ReportView();

    bool            preview(MeasureView* pMeasureView);

    static bool     exportToPDF(int reportType, const QString &filename);

private:

    QAction*        m_pExportAction = nullptr;

    MeasureView*    m_pMeasureView = nullptr;

signals:

    void            exportComplited();

public slots:

    bool            exportReport();

};

// ==============================================================================================

#endif // REPORTVIEW_H
