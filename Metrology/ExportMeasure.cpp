#include "ExportMeasure.h"

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QFile>

// -------------------------------------------------------------------------------------------------------------------

ExportMeasure::ExportMeasure(MeasureView* pView, QWidget *parent) :
    QObject(parent),
    m_pView(pView)
{
    m_pDialog = new QDialog(parent);

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
    connect(this, &ExportMeasure::exportThreadFinish, this, &ExportMeasure::exportThreadFinished);
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

    if (m_pView == nullptr)
    {
        return;
    }

    if (m_pView->table().count() == 0)
    {
        QMessageBox::information(m_pDialog, EXPORT_WINDOW_TITLE, tr("Measurements is absent!"));
        return;
    }

    int measureType = m_pView->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    QString filter = tr("CSV files (*.csv);;All Files (*.*)");
    QString fileName = QFileDialog::getSaveFileName(m_pDialog, EXPORT_WINDOW_TITLE, MeasureFileName[measureType], filter);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    m_pDialog->show();

    QtConcurrent::run(ExportMeasure::startExportThread, this, fileName);

    //QFuture<void> result = QtConcurrent::run(ExportMeasure::startExportThread, this, fileName);
    //result.waitForFinished();
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
    if (m_pView == nullptr)
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

    int columnCount = m_pView->table().header().count();
    for(int c = 0; c < columnCount; c++)
    {
        if(m_pView->table().columnIsVisible(c) == false)
        {
            continue;
        }

        file.write(m_pView->table().header().column(c)->title().toLocal8Bit());
        file.write(";");
    }

    file.write("\n");

    int measureCount = m_pView->table().count();

    setRange(0, measureCount);

    for(int m = 0; m < measureCount; m++)
    {
        if (m_exportCancel == true)
        {
            break;
        }

        for(int c = 0; c < columnCount; c++)
        {
            if(m_pView->table().columnIsVisible(c) == false)
            {
                continue;
            }

            file.write(m_pView->table().text(m, c).toLocal8Bit());
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
