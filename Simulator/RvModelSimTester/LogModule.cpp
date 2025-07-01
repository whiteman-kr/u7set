#include "LogModule.h"
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

LogModule::LogModule(QWidget* parent) :
	QWidget(parent)
{
	m_logLabel = new QTextEdit(this);
	m_logLabel->setMinimumHeight(60);

	QPushButton* clearButton = new QPushButton(tr("Clear Log"), this);
	connect(clearButton, &QPushButton::clicked, this, &LogModule::clearLog);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_logLabel);
	mainLayout->addWidget(clearButton);
	setLayout(mainLayout);

	logAction(tr("Initialized: Waiting for a response"));
}

// log max 100 actions
void LogModule::logAction(const QString& action)
{
	int size = m_actionLog.size();
	if (size >= 2)
	{
		if (m_actionLog.at(size - 1) == action || m_actionLog.at(size - 2) == action)
			return;
	}

	m_actionLog << action;
	if (size > 100)
	{
		m_actionLog.removeFirst();
	}
	updateLogLabel();
}

void LogModule::updateLogLabel()
{
	m_logLabel->setText(m_actionLog.join("\n"));
	m_logLabel->moveCursor(QTextCursor::End);
}

void LogModule::clearLog()
{
	m_actionLog.clear();
	updateLogLabel();
}