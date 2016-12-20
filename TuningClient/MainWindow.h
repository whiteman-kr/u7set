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
	void userFiltersUpdated();

private slots:
	void slot_configurationArrived(ConfigSettings settings);
	void slot_tuningSourcesArrived();
	void slot_tuningConnectionFailed();

public slots:

	void exit();
	void runUsersEditor();
	void showSettings();
	void runPresetEditor();
	void showTuningSources();

private:

	virtual void timerEvent(QTimerEvent* event) override;

	void removeWorkspace();
	void createWorkspace();

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



 private:
  void loadLanguage(const QString& rLanguage);
  void switchTranslator(QTranslator& translator, const QString& filename);


  QTranslator m_translator; // contains the translations for this application
  QTranslator m_translatorQt; // contains the translations for qt

};

extern MainWindow* theMainWindow;
extern LogFile theLogFile;

extern TuningObjectManager* theObjectManager;

extern TuningFilterStorage theFilters;
extern TuningFilterStorage theUserFilters;

extern UserManager theUserManager;

#endif // MAINWINDOW_H
