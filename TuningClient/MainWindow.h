#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>

#include "Stable.h"

#include "TuningWorkspace.h"
#include "SchemasWorkspace.h"
#include "ConfigController.h"
#include "LogFile.h"
#include "UserManager.h"
#include "../lib/Tuning/TuningObjectManager.h"
#include "SchemaStorage.h"

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

	SchemaStorage m_schemaStorage;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	SchemasWorkspace* m_schemasWorkspace = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

    int m_mainWindowTimerId = -1;

private slots:
	void slot_configurationArrived();
    void slot_presetsEditorClosing(std::vector <int>& signalsTableColumnWidth, std::vector <int>& presetsTreeColumnWidth, QPoint pos, QByteArray geometry);

public slots:
	void exit();
    void runPresetEditor();
	void runUsersEditor();
	void showSettings();
	void showTuningSources();
    void showAbout();

private:

	virtual void timerEvent(QTimerEvent* event) override;

    void createWorkspace(const TuningObjectStorage *objects);

	QAction* m_pExitAction = nullptr;
	QAction* m_pPresetEditorAction = nullptr;
	QAction* m_pUsersAction = nullptr;
	QAction* m_pSettingsAction = nullptr;
	QAction* m_pTuningSourcesAction = nullptr;
	QAction* m_pLogAction = nullptr;
	QAction* m_pAboutAction = nullptr;

	QLabel* m_statusBarInfo = nullptr;
	QLabel* m_statusBarConfigConnection = nullptr;
	QLabel* m_statusBarTuningConnection = nullptr;




};

extern MainWindow* theMainWindow;
extern LogFile* theLogFile;

extern TuningObjectManager* theObjectManager;

extern TuningFilterStorage theFilters;

extern UserManager theUserManager;

#endif // MAINWINDOW_H
