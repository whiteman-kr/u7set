#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTimer>

#include "SourceBase.h"
#include "FindSignalTextPanel.h"

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
	QMenu*					m_sourceContextMenu = nullptr;
	QMenu*					m_signalContextMenu = nullptr;
	QMenu*					m_infoMenu = nullptr;

	// Actions of main menu
	//
							// menu - Sources
							//
	QAction*				m_sourceStartAction = nullptr;
	QAction*				m_sourceStopAction = nullptr;
	QAction*				m_sourceSelectAllAction = nullptr;
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
	FindSignalTextPanel*		m_pFindSignalTextPanel = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusServer = nullptr;

private:

	// interface
	//
	bool					createInterface();

	void					createActions();
	void					createMenu();
	bool					createToolBars();
	void					createViews();
	void					createContextMenu();
	void					createHeaderContexMenu();
	void					createStatusBar();

	//

	SignalBase				m_signalBase;
	SourceBase				m_sourceBase;

	// update lists
	//
	QTableView*				m_pSourceView = nullptr;
	SourceTable				m_sourceTable;
	QAction*				m_pSourceColumnAction[SOURCE_LIST_COLUMN_COUNT];
	QMenu*					m_sourceHeaderContextMenu = nullptr;

	void					hideSourceColumn(int column, bool hide);

	QTableView*				m_pSignalView = nullptr;
	SignalTable				m_signalTable;
	QAction*				m_pSignalColumnAction[SIGNAL_LIST_COLUMN_COUNT];
	QMenu*					m_signalHeaderContextMenu = nullptr;

	void					hideSignalColumn(int column, bool hide);

	QTableView*				m_pFrameDataView = nullptr;
	FrameDataTable			m_frameDataTable;

	// update timers
	//
	QTimer*					m_updateSourceListTimer = nullptr;
	void					startUpdateSourceListTimer();
	void					stopUpdateSourceListTimer();

	//

	void					updateSignalList(PS::Source* pSource);
	void					updateFrameDataList(PS::Source* pSource);

public:

	QTableView*				sourceView() { return m_pSourceView; }
	QTableView*				signalView() { return m_pSignalView; }
	QTableView*				frameDataView() { return m_pFrameDataView; }

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
	void					selectAllSource();
	void					optionSource();
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
	void					initSignalsInSources();

	// slot of lists
	//
	void					updateSourceState();

	void					onSourceListClicked(const QModelIndex& index);
	void					onSignalListDoubleClicked(const QModelIndex& index);
	void					onFrameDataListDoubleClicked(const QModelIndex& index);
};

// ==============================================================================================

#endif // MAINWINDOW_H
