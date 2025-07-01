#pragma once

#include <QStringList>
#include <QWidget>

class QPushButton;
class QTextEdit;

class LogModule : public QWidget
{
	Q_OBJECT
public:
	explicit LogModule(QWidget* parent = nullptr);

public slots:
	void logAction(const QString& action);

private:
	void updateLogLabel();
	void clearLog();

private:
	QStringList m_actionLog;
	QTextEdit* m_logLabel = nullptr;
	QPushButton* m_clearButton = nullptr;
};
