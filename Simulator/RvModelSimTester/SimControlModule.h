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
