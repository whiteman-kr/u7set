#include "LogModule.h"
#include <QVBoxLayout>

LogModule::LogModule(QWidget* parent) :
	QWidget(parent)
{
	m_logLabel = new QTextEdit(this);
	m_logLabel->setMinimumHeight(60);

	QPushButton* clearButton = new QPushButton("Clear Log", this);
	connect(clearButton, &QPushButton::clicked, this, &LogModule::clearLog);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_logLabel);
	mainLayout->addWidget(clearButton);
	setLayout(mainLayout);

	logAction("Initialized: Wainting to response");
}

// log max 100 actions
void LogModule::logAction(const QString& action)
{
	int size = actionLog.size();
	if (size >= 2)
	{
		if (actionLog.at(size - 1) == action || actionLog.at(size - 2) == action)
			return;
	}

	actionLog << action;
	if (size > 100)
	{
		actionLog.removeFirst();
	}
	updateLogLabel();
}

void LogModule::updateLogLabel()
{
	m_logLabel->setText(actionLog.join("\n"));
	m_logLabel->moveCursor(QTextCursor::End);
}

void LogModule::clearLog()
{
	actionLog.clear();
	updateLogLabel();
}