#include "DialogProgress.h"

DialogProgress::DialogProgress(const QString& caption, int statusLinesCount, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	// Setup UI

	setMinimumWidth(400);
	setWindowTitle(caption);
	setAttribute(Qt::WA_DeleteOnClose);

	m_progressBar = new QProgressBar();
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);

	QPushButton* buttonAbort = new QPushButton(tr("Abort"));
	connect(buttonAbort, &QPushButton::clicked, this, &DialogProgress::onCancelClicked);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(buttonAbort);

	QVBoxLayout* mainLayout = new QVBoxLayout();

	if (statusLinesCount > 5)
	{
		statusLinesCount = 5;
	}

	for (int i = 0; i < statusLinesCount; i++)
	{
		QLabel* labelStatus = new QLabel();
		mainLayout->addWidget(labelStatus);
		m_labelsStatus.push_back(labelStatus);
	}

	mainLayout->addWidget(m_progressBar);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	// Start timer

	m_timer = new QTimer(this);
	m_timer->setInterval(100);
	connect(m_timer, &QTimer::timeout, this, &DialogProgress::onTimer);
	m_timer->start();
}

DialogProgress::~DialogProgress()
{
	qDebug() << "DialogProgress deleted";
}

void DialogProgress::setProgressSingle(int progress, int progressMin, int progressMax, const QString& status)
{
	QMutexLocker l(&m_mutex);

	m_status = status.split('\n', Qt::SkipEmptyParts);
	m_progressMax = progressMax;
	m_progressMin = progressMin;
	m_progressValue = progress;
}

void DialogProgress::setProgressMultiple(int progress, int progressMin, int progressMax, const QStringList& status)
{
	QMutexLocker l(&m_mutex);

	m_status = status;
	m_progressMax = progressMax;
	m_progressMin = progressMin;
	m_progressValue = progress;
}

void DialogProgress::setErrorMessage(const QString& message)
{
	QMutexLocker l(&m_mutex);
	m_errorMessage = message;
}

void DialogProgress::accept()
{
}

void DialogProgress::reject()
{
}

void DialogProgress::onTimer()
{
	// Display error message and quit
	//
	QMutexLocker l(&m_mutex);

	if (m_errorMessage.isEmpty() == false)
	{
		QString errorMsg = m_errorMessage;

		l.unlock();
		m_timer->stop();

		QMessageBox::critical(this, qAppName(), errorMsg);

		QDialog::accept();
		return;
	}

	// Update progress
	//
	if (m_progressMax != m_progressBar->maximum() || m_progressMin != m_progressBar->minimum())
	{
		m_progressBar->setMaximum(m_progressMax);
		m_progressBar->setMinimum(m_progressMin);
	}

	m_progressBar->setValue(m_progressValue);

	int labelIndex = 0;

	for (const QString& s : m_status)
	{
		if (labelIndex >= m_labelsStatus.size())
		{
			break;
		}

		m_labelsStatus[labelIndex++]->setText(s);
	}

	while (labelIndex < m_labelsStatus.size())
	{
		m_labelsStatus[labelIndex++]->setText(QString());
	}

	l.unlock();

	// Request progress update
	//
	emit getProgress();
}

void DialogProgress::onCancelClicked()
{
	emit cancelClicked();
}

