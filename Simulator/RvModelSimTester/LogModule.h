#pragma once

#include <QTextEdit>
#include <QStringList>
#include <QWidget>

class LogModule : public QWidget
{
	Q_OBJECT
public:
	explicit LogModule(QWidget* parent = nullptr);

public slots:
	void logAction(const QString& action);

private:
	QTextEdit* m_logLabel = nullptr;
	QStringList actionLog;
	void updateLogLabel();
};
