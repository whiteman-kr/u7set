#include "LogModule.h"
#include <QVBoxLayout>

LogModule::LogModule(QWidget* parent) :
	QWidget(parent)
{
	m_logLabel = new QTextEdit(this);
	m_logLabel->setMinimumHeight(60);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_logLabel);
	setLayout(mainLayout);

	logAction("Initialized: Run");
}

// log max 100 actions
void LogModule::logAction(const QString& action)
{
	actionLog << action;
	if (actionLog.size() > 100)
	{
		actionLog.removeFirst();
	}
	updateLogLabel();
}

void LogModule::updateLogLabel()
{
	m_logLabel->setText(actionLog.join("\n"));
}