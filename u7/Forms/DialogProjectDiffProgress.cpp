#include "DialogProjectDiffProgress.h"
#include "ui_DialogProjectDiffProgress.h"

#include "ProjectDiffGenerator.h"

DialogProjectDiffProgress::DialogProjectDiffProgress(std::shared_ptr<ProjectDiffWorker> worker, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_worker(worker),
	ui(new Ui::DialogProjectDiffProgress)
{
	ui->setupUi(this);

	// Start timer

	QTimer* timer = new QTimer(this);
	timer->setInterval(50);

	connect(timer, &QTimer::timeout, this, &DialogProjectDiffProgress::onTimer);

	timer->start();

}

DialogProjectDiffProgress::~DialogProjectDiffProgress()
{
	qDebug() << "DialogProjectDiffProgress deleted";

	delete ui;
}

void DialogProjectDiffProgress::accept()
{

}

void DialogProjectDiffProgress::reject()
{

}

void DialogProjectDiffProgress::onTimer()
{
	if (m_worker == nullptr)
	{
		return;
	}


	ProjectDiffWorker::WorkerStatus status = m_worker->currentStatus();

	switch (status)
	{
	case ProjectDiffWorker::WorkerStatus::Idle:
		{
			ui->labelCurrentGroup->setText(tr("Idle"));
			ui->labelFilesCount->setText(QString());
			ui->labelCurrentFile->setText(QString());
		}
		break;
	case ProjectDiffWorker::WorkerStatus::Analyzing:
		{
			ui->labelCurrentGroup->setText(tr("Analyzing: %1").arg(m_worker->currentSection()));
			ui->labelFilesCount->setText(tr("Files: %1 / %2, Signals: %3 / %4")
										 .arg(m_worker->fileIndex()).arg(m_worker->filesCount())
										 .arg(m_worker->signalIndex()).arg(m_worker->signalsCount()));

			QString objectName = m_worker->currentObjectName();
			if (objectName.length() > 48)
			{
				objectName = "..." + objectName.right(48);
			}

			ui->labelCurrentFile->setText(tr("%1").arg(objectName));
		}
		break;
	case ProjectDiffWorker::WorkerStatus::Rendering:
		{
			ui->labelCurrentGroup->setText(tr("Creating report..."));
			ui->labelFilesCount->setText(tr("Section: %1 / %2")
										 .arg(m_worker->sectionIndex()).arg(m_worker->sectionCount()));
			ui->labelCurrentFile->setText(QString());
		}
		break;
	case ProjectDiffWorker::WorkerStatus::Printing:
		{
			ui->labelCurrentGroup->setText(tr("Saving report..."));
			ui->labelFilesCount->setText(tr("Page: %1 / %2")
										 .arg(m_worker->pageIndex()).arg(m_worker->pagesCount()));
			ui->labelCurrentFile->setText(QString());
		}
		break;
	}

	return;
}

void DialogProjectDiffProgress::on_cancelButton_clicked()
{
	m_worker->stop();
}
