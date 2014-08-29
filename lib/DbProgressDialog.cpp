#include <QProgressBar>
#include <QTime>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include "../include/DbProgressDialog.h"

//
//
// ProgressDialog
//
//

ProgressDialog::ProgressDialog(QWidget* parent, const QString& description, DbProgress* progress) :
    QDialog(parent, Qt::CustomizeWindowHint),
    m_description(description),
    m_progress(progress)
{
    assert(m_progress);

    setWindowTitle(tr("u7"));
    setMinimumWidth(400);

    // create all widgets and layouts

    QVBoxLayout* vLayout = new QVBoxLayout;

    m_label = new QLabel();

    m_progressBar = new QProgressBar();
    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 100);

    m_cancelButton = new QPushButton(tr("Cancel"));

    vLayout->addWidget(m_label);
    vLayout->addWidget(m_progressBar);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_cancelButton, 0, Qt::AlignRight | Qt::AlignBottom);

    vLayout->addLayout(hLayout);

    setLayout(vLayout);

    // connect signals and slots

    connect(this, &QDialog::rejected, this, &ProgressDialog::cancel);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);


    startTimer(10);
}

void ProgressDialog::ShowProgressDialog(QWidget* parent, const QString& description, DbProgress* progress)
{
    // do not show the dialog if the process completes in showDialogDelayMs milliseconds

    const int showDialogDelayMs = 200;

    QTime endTime = QTime::currentTime().addMSecs(showDialogDelayMs);
    while (QTime::currentTime() < endTime)
    {
        QThread::yieldCurrentThread();

        if (progress->completed() == true)
        {
            return;
        }
    }

    ProgressDialog progressDialog(parent, description, progress);
    progressDialog.exec();
}


void ProgressDialog::cancel()
{
    m_progress->setCancel(true);
}

void ProgressDialog::timerEvent(QTimerEvent*)
{
    assert(m_progress);

    if (m_progress->completed() == true)
    {
        accept();
    }

    m_label->setText(m_description + QString(" - %1%").arg(m_progress->value()));
    m_progressBar->setValue(m_progress->value());

    return;
}

