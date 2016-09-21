#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Stable.h"

#include "TuningWorkspace.h"
#include "ConfigController.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	ConfigController m_configController;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;

	int m_updateStatusBarTimerId = -1;

private slots:
	void slot_configurationArrived(bool updateFilters, bool updateSchemas, bool updateSignals);


private:
	void createStatusBar();

	virtual void timerEvent(QTimerEvent* event) override;
};

#endif // MAINWINDOW_H
