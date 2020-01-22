#pragma once

#include <QMainWindow>
#include <QDesktopWidget>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QDockWidget>
#include <QCloseEvent>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include <QClipboard>

#include "UalTesterServer.h"
#include "SignalBase.h"
#include "SourceBase.h"
#include "FrameDataPanel.h"
#include "FindSignalPanel.h"
#include "History.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	explicit MainWindow(QMainWindow* parent = nullptr);
	virtual ~MainWindow();

private:

	// Elements of interface - Menu
	//
	QMenu*					m_sourceMenu = nullptr;
	QMenu*					m_signalMenu = nullptr;
	QMenu*					m_sourceContextMenu = nullptr;
	QMenu*					m_signalContextMenu = nullptr;
	QMenu*					m_infoMenu = nullptr;

	// Actions of main menu
	//
							// menu - Sources
							//
	QAction*				m_sourceStartAction = nullptr;
	QAction*				m_sourceStopAction = nullptr;
	QAction*				m_sourceReloadAction = nullptr;
	QAction*				m_sourceSelectAllAction = nullptr;
	QAction*				m_signalSetStateAction = nullptr;
	QAction*				m_signalInitAction = nullptr;
	QAction*				m_signalHistoryAction = nullptr;
	QAction*				m_signalSaveStatesAction = nullptr;
	QAction*				m_signalRestoreStatesAction = nullptr;
	QAction*				m_signalSelectAllAction = nullptr;
	QAction*				m_optionAction = nullptr;
	QAction*				m_sourceTextCopyAction = nullptr;
	QAction*				m_signalTextCopyAction = nullptr;

							// menu - ?
							//
	QAction*				m_pAboutAppAction = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_mainToolBar = nullptr;

	// Elements of interface - Panels
	//
	FrameDataPanel*			m_pFrameDataPanel = nullptr;
	FindSignalPanel*		m_pFindSignalPanel = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusUalTesterClient = nullptr;

private:

	// interface
	//
	bool					createInterface();

	void					createActions();
	void					createMenu();
	bool					createToolBars();
	void					createPanels();
	void					createViews();
	void					createContextMenu();
	void					createHeaderContexMenu();
	void					createStatusBar();

	//
	//
	UalTesterServer*		m_ualTesterSever = nullptr;
	UalTesterServerThread*	m_ualTesterServerThread = nullptr;
	void					runUalTesterServerThread();
	void					stopUalTesterServerThread();

	//
	//
	SignalBase				m_signalBase;
	SourceBase				m_sourceBase;
	SignalHistory			m_signalHistory;

	// update lists
	//
	QTableView*				m_pSourceView = nullptr;
	SourceTable				m_sourceTable;
	QAction*				m_pSourceColumnAction[SOURCE_LIST_COLUMN_COUNT];
	QMenu*					m_sourceHeaderContextMenu = nullptr;
	QModelIndex				m_selectedSourceIndex;

	void					hideSourceColumn(int column, bool hide);

	QTableView*				m_pSignalView = nullptr;
	SignalTable				m_signalTable;
	QAction*				m_pSignalColumnAction[SIGNAL_LIST_COLUMN_COUNT];
	QMenu*					m_signalHeaderContextMenu = nullptr;

	void					hideSignalColumn(int column, bool hide);

	//
	//
	QTimer*					m_updateSourceListTimer = nullptr;
	void					startUpdateSourceListTimer();
	void					stopUpdateSourceListTimer();

	//
	//
	void					updateSignalList(PS::Source* pSource);
	void					updateFrameDataList(PS::Source* pSource);

	//
	//
	QTimer*					m_updateBuildFilesTimer = nullptr;
	void					startUpdateBuildFilesTimer();
	void					stopUpdateBuildFilesTimer();

	//
	//
	void					saveWindowState();
	void					restoreWindowState();



public:

	QTableView*				sourceView() { return m_pSourceView; }
	QTableView*				signalView() { return m_pSignalView; }

protected:

	void					closeEvent(QCloseEvent* e);

signals:

private slots:

	// slots of menu
	//
							// Sources
							//
	void					startSource();
	void					stopSource();
	void					reloadSource();
	void					selectAllSources();
	void					setSignalState();
	void					initSignalsState();
	void					history();
	void					saveSignalsState();
	void					restoreSignalsState();
	void					selectAllSignals();
	void					onOptions();
	void					copyText(QTableView* pView);
	void					copySourceText();
	void					copySignalText();


							// ContextMenu
							//
	void					onSourceContextMenu(QPoint);
	void					onSourceHeaderContextMenu(QPoint);
	void					onSourceColumnAction(QAction* action);
	void					onSignalContextMenu(QPoint);
	void					onSignalHeaderContextMenu(QPoint);
	void					onSignalColumnAction(QAction* action);


							// menu - ?
							//
	void					aboutApp();

	// slot of data
	//
	void					loadSources();
	void					loadSignals();
	void					loadSignalsInSources();

	// slot of lists
	//
	void					updateSourceState();

	void					onSourceListClicked(const QModelIndex& index);
	void					onSignalListDoubleClicked(const QModelIndex& index);

	//
	//
	void					updateBuildFiles();

	// slot of UalTesterServer
	//
	void					ualTesterSocketConnect(bool isConnect);
	void					signalStateChanged(Hash hash, double prevState, double state);
};
