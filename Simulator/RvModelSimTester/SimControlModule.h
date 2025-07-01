#pragma once

#include <QWidget>

class QPushButton;
class QLabel;

class SimControlModule : public QWidget
{
	Q_OBJECT
public:
	explicit SimControlModule(QWidget* parent = nullptr);

signals:
	void simStatusChanged(const QString& status);
	void simAction(const QString& action);
	void simControlMode(const QString& mode);

public slots:
	void onSimStateReady(int errorCode, int stateCode);

private slots:
	void onStartClicked();
	void onStopClicked();
	void onPauseClicked();
	void onResumeClicked();


private:
	QPushButton* m_startButton = nullptr;
	QPushButton* m_stopButton = nullptr;
	QPushButton* m_pauseButton = nullptr;
	QPushButton* m_resumeButton = nullptr;
	QLabel* m_statusBar = nullptr;
};
