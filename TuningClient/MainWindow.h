#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Stable.h"

#include "TuningWorkspace.h"
#include "TcpTuningClient.h"
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
	void createActions();
	void createMenu();
	void createStatusBar();


private:
	ConfigController m_configController;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

	int m_updateStatusBarTimerId = -1;

signals:
	void filtersUpdated();

private slots:
	void slot_configurationArrived(ConfigSettings settings);
	void slot_tuningSourcesArrived();
	void slot_tuningConnectionFailed();

	void exit();
	void showSettings();
	void runPresetEditor();
	void showTuningSources();

private:

	virtual void timerEvent(QTimerEvent* event) override;

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;
};

#endif // MAINWINDOW_H
