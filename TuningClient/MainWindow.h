#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Stable.h"

#include "TuningWorkspace.h"
#include "ConfigController.h"
#include "LogFile.h"
#include "UserManager.h"
#include "TuningObjectManager.h"
#include <QTranslator>


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
	void signalsUpdated();

private slots:
	void slot_configurationArrived(ConfigSettings settings);
	void slot_tuningSourcesArrived();
	void slot_tuningConnectionFailed();

public slots:

	void exit();
	void runUsersEditor();
	void showSettings();
	void showTuningSources();
    void showAbout();

private:

	virtual void timerEvent(QTimerEvent* event) override;

	void removeWorkspace();
    void createWorkspace(const TuningObjectStorage *objects);

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pUsersAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConnectionStatistics = nullptr;
	QLabel* m_statusBarConnectionState = nullptr;




};

extern MainWindow* theMainWindow;
extern LogFile* theLogFile;

extern TuningObjectManager* theObjectManager;

extern TuningFilterStorage theFilters;

extern UserManager theUserManager;

#endif // MAINWINDOW_H
