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

	if (m_worker->totalFiles() == 0)
	{
		ui->labelFilesCount->setText(tr("Counting..."));
	}
	else
	{
		ui->labelFilesCount->setText(tr("Files: %1 / %2, Signals: %3 / %4")
									 .arg(m_worker->currentFileIndex()).arg(m_worker->totalFiles())
									 .arg(m_worker->currentSignalIndex()).arg(m_worker->totalSignals()));
	}

	ui->labelCurrentGroup->setText(tr("%1").arg(m_worker->currentSection()));

	QString objectName = m_worker->currentObjectName();
	if (objectName.length() > 64)
	{
		objectName = "..." + objectName.right(64);
	}

	ui->labelCurrentFile->setText(tr("%1").arg(objectName));

	return;
}

void DialogProjectDiffProgress::on_cancelButton_clicked()
{
	m_worker->stop();
}
