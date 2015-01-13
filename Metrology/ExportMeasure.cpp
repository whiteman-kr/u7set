#include "ExportMeasure.h"

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QFile>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

ExportMeasure::ExportMeasure(MeasureView* pMeasureView) :
    QObject(pMeasureView),
    m_pMeasureView(pMeasureView)
{
    m_pDialog = new QDialog(pMeasureView);

    m_pDialog->setWindowFlags(Qt::Drawer);
    m_pDialog->setFixedSize(300, 70);
    m_pDialog->setWindowTitle(EXPORT_WINDOW_TITLE);
    m_pDialog->setWindowIcon(QIcon(":/icons/Export.png"));

        m_progress = new QProgressBar;
        m_progress->setTextVisible(false);
        m_progress->setRange(0, 100);
        m_progress->setFixedHeight(10);

        m_cancelButton = new QPushButton;
        m_cancelButton->setText(tr("Cancel"));

        QHBoxLayout *buttonLayout = new QHBoxLayout ;

        buttonLayout->addStretch();
        buttonLayout->addWidget(m_cancelButton);
        buttonLayout->addStretch();

        QVBoxLayout *mainLayout = new QVBoxLayout ;

        mainLayout->addWidget(m_progress);
        mainLayout->addLayout(buttonLayout);

    m_pDialog->setLayout(mainLayout);

    connect(this, &ExportMeasure::setValue, m_progress, &QProgressBar::setValue);
    connect(this, &ExportMeasure::setRange, m_progress, &QProgressBar::setRange);

    connect(this, &ExportMeasure::exportThreadFinish, m_pDialog, &QDialog::reject);
    connect(this, &ExportMeasure::exportThreadFinish, this, &ExportMeasure::exportComplited);
    connect(m_cancelButton, &QPushButton::clicked, m_pDialog, &QDialog::reject);
    connect(m_pDialog, &QDialog::rejected, this, &ExportMeasure::exportCancel);
}

// -------------------------------------------------------------------------------------------------------------------

ExportMeasure::~ExportMeasure()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ExportMeasure::exec()
{
    if (m_pDialog == nullptr)
    {
        return;
    }

    if (m_pMeasureView == nullptr)
    {
        return;
    }

    if (m_pMeasureView->table().count() == 0)
    {
        QMessageBox::information(m_pDialog, EXPORT_WINDOW_TITLE, tr("Measurements is absent!"));
        return;
    }

    int measureType = m_pMeasureView->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    QString filter = tr("CSV files (*.csv);;PDF files (*.pdf)");

    QString fileName = QFileDialog::getSaveFileName(m_pDialog, EXPORT_WINDOW_TITLE, MeasureFileName[measureType], filter);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    QString fileExt = fileName.right(fileName.count() - fileName.lastIndexOf(".") - 1);
    if (fileExt.isEmpty() == true)
    {
        return;
    }

    if (fileExt == "csv")
    {
        m_pDialog->show();
        QtConcurrent::run(ExportMeasure::startExportThread, this, fileName);

        //QFuture<void> result = QtConcurrent::run(ExportMeasure::startExportThread, this, fileName);
        //result.waitForFinished();
    }

    if (fileExt == "pdf")
    {
        int reportType = theOptions.report().reportTypeByMeasureType(measureType);
        if (reportType != REPORT_TYPE_UNKNOWN)
        {
            if (ReportView::exportToPDF(reportType, fileName) == true)
            {
                exportComplited();
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void ExportMeasure::startExportThread(ExportMeasure* pThis, QString fileName)
{
    if (pThis == nullptr)
    {
        return;
    }

    pThis->saveFile(fileName);

    emit pThis->exportThreadFinish();
}

// -------------------------------------------------------------------------------------------------------------------

bool ExportMeasure::saveFile(QString fileName)
{
    if (m_pMeasureView == nullptr)
    {
        return false;
    }

    if (fileName.isEmpty() == true)
    {
        return false;
    }

    m_exportCancel = false;

    QFile file;
    file.setFileName(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        return false;
    }

    int columnCount = m_pMeasureView->table().header().count();
    for(int c = 0; c < columnCount; c++)
    {
        if(m_pMeasureView->table().columnIsVisible(c) == false)
        {
            continue;
        }

        file.write(m_pMeasureView->table().header().column(c)->title().toLocal8Bit());
        file.write(";");
    }

    file.write("\n");

    int measureCount = m_pMeasureView->table().count();

    setRange(0, measureCount);

    for(int m = 0; m < measureCount; m++)
    {
        if (m_exportCancel == true)
        {
            break;
        }

        for(int c = 0; c < columnCount; c++)
        {
            if(m_pMeasureView->table().columnIsVisible(c) == false)
            {
                continue;
            }

            file.write(m_pMeasureView->table().text(m, c).toLocal8Bit());
            file.write(";");
        }

        file.write("\n");
        file.flush();

        setValue(m);
    }

    file.close();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------
