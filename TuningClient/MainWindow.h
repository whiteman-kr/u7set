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
#include "TuningClientObjectManager.h"
#include "TuningClientFilterStorage.h"
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
	TuningClientObjectManager *m_objectManager = nullptr;

	SimpleThread* m_tcpClientThread = nullptr;

	TuningClientFilterStorage m_filterStorage;

	ConfigController m_configController;

	TuningWorkspace* m_tuningWorkspace = nullptr;

	SchemasWorkspace* m_schemasWorkspace = nullptr;

	int m_mainWindowTimerId = -1;

private slots:
	void slot_configurationArrived();
    void slot_presetsEditorClosing(std::vector <int>& signalsTableColumnWidth, std::vector <int>& presetsTreeColumnWidth, QPoint pos, QByteArray geometry);

	void slot_schemasGlobalScriptArrived(QByteArray data);

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

	QString m_globalScript;
};

// Global definitions

extern MainWindow* theMainWindow;

extern LogFile* theLogFile;

extern UserManager theUserManager;

#endif // MAINWINDOW_H
