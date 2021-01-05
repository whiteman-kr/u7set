#include "DialogProgress.h"

DialogProgress::DialogProgress(const QString& caption, int statusLinesCount, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
	// Setup UI

	setMinimumWidth(400);

	setWindowTitle(caption);


	m_progressBar = new QProgressBar();
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);

	QPushButton* buttonAbort = new QPushButton(tr("Abort"));
	connect(buttonAbort, &QPushButton::clicked, this, &DialogProgress::on_cancelButton_clicked);

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

	QTimer* timer = new QTimer(this);
	timer->setInterval(50);

	connect(timer, &QTimer::timeout, this, &DialogProgress::onTimer);

	timer->start();
}

DialogProgress::~DialogProgress()
{
	qDebug() << "DialogProgress deleted";
}

void DialogProgress::setProgressSingle(int progress, int progressMin, int progressMax, const QString& status)
{
	if (m_progressMax != progressMax || m_progressMin != progressMin)
	{
		m_progressMax = progressMax;
		m_progressMin = progressMin;
		m_progressBar->setMaximum(progressMax);
		m_progressBar->setMinimum(progressMin);
	}

	m_progressBar->setValue(progress);

	if (m_labelsStatus.empty() == false)
	{
		m_labelsStatus[0]->setText(status);
	}
}

void DialogProgress::setProgressMultiple(int progress, int progressMin, int progressMax, const QStringList& status)
{
	if (m_progressMax != progressMax || m_progressMin != progressMin)
	{
		m_progressMax = progressMax;
		m_progressMin = progressMin;
		m_progressBar->setMaximum(progressMax);
		m_progressBar->setMinimum(progressMin);
	}

	m_progressBar->setValue(progress);

	int labelIndex = 0;

	for (const QString& s : status)
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

}

void DialogProgress::accept()
{
}

void DialogProgress::reject()
{
}

void DialogProgress::onTimer()
{
	emit getProgress();
}

void DialogProgress::on_cancelButton_clicked()
{
	emit cancelClicked();
}

