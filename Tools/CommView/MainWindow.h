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

#include "SerialPortWorker.h"
#include "SerialPortList.h"
#include "WorkerBase.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	virtual ~MainWindow();

private:

	// Elements of interface - Menu
	//
	QMenu*					m_portsMenu = nullptr;
	QMenu*					m_viewMenu = nullptr;
	QMenu*					m_commStateContextMenu = nullptr;
	QMenu*					m_commDataContextMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Actions of main menu
	//
							// menu - Ports
							//
	QAction*				m_portStartAction = nullptr;
	QAction*				m_portReconnectAction = nullptr;
	QAction*				m_portReconnectAllAction = nullptr;
	QAction*				m_portOptionAction = nullptr;

							// menu - View
							//
	QAction*				m_showHeaderAction = nullptr;
	QAction*				m_showInWordAction = nullptr;
	QAction*				m_showInHexAction = nullptr;
	QAction*				m_showInFloatAction = nullptr;

							// menu - ?
							//
	QAction*				m_optionAppAction = nullptr;
	QAction*				m_aboutAppAction = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_mainToolBar = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusPortConnectedCount = nullptr;

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

	// Serial port threads
	//
	SerialPortWorker*		m_pWorker[SERIAL_PORT_COUNT];

	bool					runSerialPortThread(int index);
	void					runSerialPortThreads();
	void					stopSerialPortThreads();

	// update lists
	//
	QTableView*				m_pCommStateView = nullptr;
	CommStateTable			m_commStateTable;
	QAction*				m_pColumnAction[COMM_STATE_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	QTableView*				m_pCommHeaderView = nullptr;
	CommHeaderTable			m_commHeaderTable;

	QTableView*				m_pCommDataView = nullptr;
	CommDataTable			m_commDataTable;

	void					updateCommStateList();
	void					updateCommHeaderList();
	void					updateCommDataList();

	void					hideColumn(int column, bool hide);

	// update timers
	//
	QTimer*					m_updateCommStateTimer = nullptr;
	void					startCommStateTimer();
	void					stopCommStateTimer();

	QTimer*					m_updateCommHeaderTimer = nullptr;
	void					startCommHeaderTimer();
	void					stopCommHeaderTimer();

	QTimer*					m_updateCommDataTimer = nullptr;
	void					startCommDataTimer();
	void					stopCommDataTimer();

	void					writeTestResultToFile();

protected:

	void					closeEvent(QCloseEvent* e);

signals:

private slots:

	// slots of menu
	//
							// Ports
							//
	void					startTest();
	void					reconnectSerialPort();
	void					reconnectAllSerialPort();
	void					optionSerialPort();

							// View
							//
	void					showHeader();
	void					showInWord();
	void					showInHex();
	void					showInFloat();

							// ContextMenu
							//
	void					onContextCommStateMenu(QPoint);
	void					onContextCommDataMenu(QPoint);
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

							// menu - ?
							//
	void					optionApp();
	void					aboutApp();

	// slot for update lists
	//
	void					updateCommState();
	void					updateCommHeader();
	void					updateCommData();

	// Slots of SerialPortThread
	//
	void					portConnectedChanged();
	void					testFinished();
};

// ==============================================================================================

#endif // MAINWINDOW_H
