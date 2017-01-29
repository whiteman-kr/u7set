#include "ReportView.h"

#include <QApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>

#include "ExportData.h"

// -------------------------------------------------------------------------------------------------------------------

ReportView::ReportView(QObject *parent) :
    QObject(parent)
{
//    m_pExportAction = new QAction(tr("Export the report"), this);
//    m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);
//    m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
//    m_pExportAction->setToolTip(tr("Export"));
//    connect(m_pExportAction, &QAction::triggered, this, &ReportView::exportReport);
}

// -------------------------------------------------------------------------------------------------------------------

ReportView::~ReportView()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool ReportView::preview(MeasureView* pMeasureView)
{
    if (pMeasureView == nullptr)
    {
        return false;
    }

//    int measureType = pMeasureView->measureType();
//    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
//    {
//        return false;
//    }

//    int reportType = theOptions.report().reportTypeByMeasureType(measureType);
//    if (reportType < 0 || reportType >= REPORT_TYPE_COUNT)
//    {
//        return false;
//    }

//    QString reportFileName = ReportFileName[reportType];
//    if (reportFileName.isEmpty() == true)
//    {
//        return false;
//    }

//    QString reportPath = theOptions.report().m_path + QDir::separator() + reportFileName;

//    // create report
//    //
//    NCReport* report = new NCReport;

//    report->setCurrentLanguage(tr("en"));
//    report->setReportSource( NCReportSource::File );                        // set report source type
//    report->setReportFile(reportPath);                                      // set the report filename fullpath or relative to dir
//    report->runReportToPreview();                                           // run to preview output

//    if( report->hasError() == true)
//    {
//        QMessageBox::critical(m_pMeasureView, tr("Preview error"), report->lastErrorMsg());
//        delete report;

//        return false;
//    }

//    m_pMeasureView = pMeasureView;

//    // create preview window
//    //
//    NCReportPreviewWindow *pv = new NCReportPreviewWindow(m_pMeasureView);  // create preview window

//        pv->setWindowIcon(QIcon(":/icons/Preview.png"));
//        pv->setWindowTitle(tr("Preview report: ") + ReportType[reportType]);

//        pv->setOutput( (NCReportPreviewOutput*)report->output() );              // add output to the window
//        pv->setReport(report);
//        pv->setWindowModality(Qt::ApplicationModal );                           // set modality
//        pv->setAttribute( Qt::WA_DeleteOnClose );                               // set attrib

//        // correct menu actions
//        //
//        pv->menuBar()->actions().at(REPORT_MENU_FILE)->menu()->insertAction(pv->actionPdf(), m_pExportAction);
//        QAction* openAction = pv->menuBar()->actions().at(REPORT_MENU_FILE)->menu()->actions().first();
//        pv->menuBar()->actions().at(REPORT_MENU_FILE)->menu()->removeAction(openAction);
//        pv->menuBar()->actions().at(REPORT_MENU_FILE)->menu()->removeAction(pv->actionPdf());
//        pv->menuBar()->actions().at(REPORT_MENU_FILE)->menu()->removeAction(pv->actionSvg());

//        pv->menuBar()->actions().at(REPORT_MENU_NAVIGATE)->menu()->removeAction(pv->actionNextReport());
//        pv->menuBar()->actions().at(REPORT_MENU_NAVIGATE)->menu()->removeAction(pv->actionPreviousReport());
//        pv->menuBar()->actions().at(REPORT_MENU_NAVIGATE)->menu()->removeAction(pv->actionClose());

//        QAction* aboutAction = pv->menuBar()->actions().at(REPORT_MENU_ABOUT);
//        pv->menuBar()->removeAction( aboutAction );

//        // correct toolbar actions
//        //
//        pv->toolBarFile()->insertAction(pv->actionPdf(), m_pExportAction);
//        pv->toolBarFile()->removeAction(pv->actionPdf());
//        pv->toolBarFile()->removeAction(pv->actionSvg());

//        pv->toolBarNavigation()->removeAction(pv->actionNextReport());
//        pv->toolBarNavigation()->removeAction(pv->actionPreviousReport());
//        pv->toolBarNavigation()->removeAction(pv->actionClose());

//    // exec report preview
//    //
//    pv->exec();                                                         // run like modal dialog

//    delete report;

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ReportView::exportToPDF(int reportType, const QString &filename)
{
    if (reportType < 0 || reportType >= REPORT_TYPE_COUNT)
    {
        return false;
    }

    if (filename.isEmpty() == true)
    {
        return false;
    }

//    QString reportFileName = ReportFileName[reportType];
//    if (reportFileName.isEmpty() == true)
//    {
//        return false;
//    }

//    QString reportPath = theOptions.report().m_path + "/" + reportFileName;

//    bool exportResult = true;

//    NCReport* report = new NCReport;

//    report->setReportSource( NCReportSource::File );                                     // set report source type
//    report->setReportFile(reportPath);                                                   // set the report filename fullpath or relative to dir
//    report->runReportToPDF(filename);                                                    // export to PDF file

//    if( report->hasError() == true)
//    {
//        exportResult = false;
//        QMessageBox::critical(nullptr, tr("Export error"), report->lastErrorMsg());
//    }

//    delete report;

//    return exportResult;

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ReportView::exportReport()
{
//    if (m_pMeasureView == nullptr)
//    {
//        return false;
//    }

//    ExportMeasure* dialog = new ExportMeasure(m_pMeasureView);
//    dialog->exec();

//    emit exportComplited();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------
