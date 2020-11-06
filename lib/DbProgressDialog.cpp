#include <QProgressBar>
#include <QTime>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QThread>
#include <assert.h>
#include "../lib/DbProgressDialog.h"
#include "../lib/DbProgress.h"

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

	// Create all widgets and layouts
	//
    QVBoxLayout* vLayout = new QVBoxLayout;

	m_label = new QLabel;
	m_currentOperation = new QLabel;

    m_progressBar = new QProgressBar();
    m_progressBar->setTextVisible(false);
    m_progressBar->setRange(0, 100);

    m_cancelButton = new QPushButton(tr("Cancel"));

    vLayout->addWidget(m_label);
	vLayout->addWidget(m_currentOperation);
    vLayout->addWidget(m_progressBar);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_cancelButton, 0, Qt::AlignRight | Qt::AlignBottom);

    vLayout->addLayout(hLayout);

    setLayout(vLayout);

	setMinimumWidth(400);
	setMinimumHeight(minimumSizeHint().height());

	connect(m_cancelButton, &QPushButton::clicked, this, &ProgressDialog::reject);

	m_timerId = startTimer(10);

	return;
}

void ProgressDialog::showProgressDialog(QWidget* parent, const QString& description, DbProgress* progress)
{
	// Do not show the dialog if the process completes in showDialogDelayMs ms
	//
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

	assert(progress->completed() == true);

	return;
}

void ProgressDialog::reject()
{
	m_progress->setCancel(true);	// DbWorker must set Completed to finish work, the dialog will be closed in timerEvent
	m_cancelButton->setEnabled(false);
	return;
}

void ProgressDialog::timerEvent(QTimerEvent*)
{
    assert(m_progress);

    if (m_progress->completed() == true)
    {
        accept();
    }

	if (m_progress->wasCanceled() == true)
	{
		m_label->setText(m_description + QString(" - Waiting for cancelling..."));
	}
	else
	{
		m_label->setText(m_description + QString(" - %1%").arg(m_progress->value()));

		QString curOp = m_progress->currentOperation();
		if (curOp != m_currentOperation->text())
		{
			m_currentOperation->setText(curOp);
		}

		m_progressBar->setValue(m_progress->value());
	}

    return;
}

