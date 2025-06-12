#pragma once

#include <QStringList>
#include <QTextEdit>
#include <QWidget>
#include <QPushButton>

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
	QPushButton* clearButton = nullptr;

	void updateLogLabel();
	void clearLog();
};
