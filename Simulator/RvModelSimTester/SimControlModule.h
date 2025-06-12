#pragma once

#include <QWidget>
#include "SimModelPackets.h"
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
	

private:
	QPushButton* startButton = nullptr;
	QPushButton* stopButton = nullptr;
	QPushButton* pauseButton = nullptr;
	QLabel* statusBar = nullptr;
};
