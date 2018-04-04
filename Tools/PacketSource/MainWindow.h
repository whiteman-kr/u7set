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

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	explicit MainWindow(QMainWindow* parent = 0);
	virtual ~MainWindow();

private:

	// Elements of interface - Menu
	//
	QMenu*					m_sourceMenu = nullptr;
	QMenu*					m_viewMenu = nullptr;
	QMenu*					m_sourceContextMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Actions of main menu
	//
							// menu - Sources
							//
	QAction*				m_sourceStartAction = nullptr;
	QAction*				m_sourceStopAction = nullptr;
	QAction*				m_sourceSelectAllAction = nullptr;
	QAction*				m_sourceOptionAction = nullptr;

							// menu - ?
							//
	QAction*				m_pAboutAppAction = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_mainToolBar = nullptr;

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
	void					loadSources();

	// update lists
	//
	QTableView*				m_pSourceView = nullptr;
	SourceTable				m_sourceTable;
	QAction*				m_pColumnAction[SOURCE_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	void					hideColumn(int column, bool hide);

	// update timers
	//
	QTimer*					m_updateSourceListTimer = nullptr;
	void					startUpdateSourceListTimer();
	void					stopUpdateSourceListTimer();

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

							// ContextMenu
							//
	void					onContextSourceMenu(QPoint);
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

							// menu - ?
							//
	void					aboutApp();

	// slot for update lists
	//
	void					updateSourceState();
};

// ==============================================================================================

#endif // MAINWINDOW_H
