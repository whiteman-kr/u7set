#ifndef MAINWINDOW_H
#define MAINWINDOW_H

class CentralWidget;
class DbStore;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
	MainWindow(DbStore* dbstore, QWidget* parent);
    ~MainWindow();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

	// Public methods
	//
public:

	// Protected methods
	//
protected:
	void saveWindowState();
	void restoreWindowState();


	// Private methods
	//
private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	CentralWidget* getCentralWidget();

	// Commands
	//
protected slots:
	void exit();

	void userManagement();
	void showLog();
	void showSettings();

	void runConfigurator();
	void showAbout();
	void debug();

private slots:
	void projectOpened();
	void projectClosed();

	void databaseError(QString message);
	void databaseOperationCompleted(QString message);

	// Properties
	//
protected:
	DbStore* dbStore();
    
	// Data
	//
private:
	QAction* m_pExitAction;

	QAction* m_pUsersAction;
	QAction* m_pLogAction;
	QAction* m_pSettingsAction;

	QAction* m_pConfiguratorAction;
	QAction* m_pAboutAction;
	QAction* m_pDebugAction;

	QLabel* m_pStatusBarInfo;
	QLabel* m_pStatusBarConnectionStatistics;
	QLabel* m_pStatusBarConnectionState;

	DbStore* m_pDbStore;
};

#endif // MAINWINDOW_H
