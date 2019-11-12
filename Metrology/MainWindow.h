#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QToolBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QClipboard>

#include "../lib/SimpleThread.h"

#include "MeasureView.h"
#include "ConfigSocket.h"
#include "SignalSocket.h"
#include "TuningSocket.h"
#include "MeasureThread.h"
#include "FindMeasurePanel.h"
#include "StatisticPanel.h"
#include "SignalInfoPanel.h"
#include "Calculator.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
	virtual ~MainWindow();

private:

	int						m_measureType = MEASURE_TYPE_UNKNOWN;

	QMap<int, MeasureView*> m_measureViewMap;

	// Actions of main menu
	//
							// menu - Measure
							//
	QAction*				m_pStartMeasureAction = nullptr;
	QAction*				m_pStopMeasureAction = nullptr;
	QAction*				m_pExportMeasureAction = nullptr;

							// menu - Edit
							//
	QAction*				m_pCopyMeasureAction = nullptr;
	QAction*				m_pRemoveMeasureAction = nullptr;
	QAction*				m_pSelectAllMeasureAction = nullptr;

							// menu - View
							//

							// menu - Tools
							//
	QAction*				m_pCalibratorsAction = nullptr;
	QAction*				m_pShowRackListAction = nullptr;
	QAction*				m_pShowSignalListAction = nullptr;
	QAction*				m_pShowComparatorsListAction = nullptr;
	QAction*				m_pShowOutputSignalListAction = nullptr;
	QAction*				m_pShowTuningSignalListAction = nullptr;
	QAction*				m_pShowCalculatorAction = nullptr;
	QAction*				m_pOptionsAction;

							// menu - ?
							//
	QAction*				m_pShowStatisticAction = nullptr;
	QAction*				m_pAboutConnectionAction = nullptr;
	QAction*				m_pAboutAppAction = nullptr;

	QMenu*					m_pContextMenu = nullptr;

private:

	// Elements of interface - Menu
	//
	QMenu*					m_pMeasureMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewPanelMenu = nullptr;
	QMenu*					m_pSettingMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_pMeasureControlToolBar = nullptr;
	QToolBar*				m_pMeasureTimeout = nullptr;
	QToolBar*				m_pMeasureKind = nullptr;
	QToolBar*				m_pOutputSignalToolBar = nullptr;
	QToolBar*				m_pAnalogSignalToolBar = nullptr;

	// Elements of interface - Items of ToolBars
	//
	QComboBox*				m_measureKindList = nullptr;
	QComboBox*				m_outputSignalTypeList = nullptr;

	QComboBox*				m_asRackCombo = nullptr;
	QComboBox*				m_asSignalCombo = nullptr;
	QComboBox*				m_asChassisCombo = nullptr;
	QComboBox*				m_asModuleCombo = nullptr;
	QComboBox*				m_asPlaceCombo = nullptr;

	// Elements of interface - Pages of Tab
	//
	QTabWidget*				m_pMainTab = nullptr;

	// Elements of interface - Panels
	//
	FindMeasurePanel*		m_pFindMeasurePanel = nullptr;
	StatisticPanel*			m_pStatisticPanel = nullptr;
	SignalInfoPanel*		m_pSignalInfoPanel = nullptr;
	QDockWidget*			m_pComparatorInfoPanel = nullptr;
	QTableView*				m_pComparatorInfoView = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusMeasureThreadInfo = nullptr;
	QProgressBar*			m_statusMeasureTimeout = nullptr;
	QLabel*					m_statusMeasureThreadState = nullptr;
	QLabel*					m_statusCalibratorCount = nullptr;
	QLabel*					m_statusConnectToConfigServer = nullptr;
	QLabel*					m_statusConnectToAppDataServer = nullptr;
	QLabel*					m_statusConnectToTuningServer = nullptr;

private:

	ConfigSocket*			m_pConfigSocket = nullptr;

	SignalSocket*			m_pSignalSocket = nullptr;
	SimpleThread*			m_pSignalSocketThread = nullptr;

	TuningSocket*			m_pTuningSocket = nullptr;
	SimpleThread*			m_pTuningSocketThread = nullptr;

	MeasureThread			m_measureThread;

	Calculator*				m_pCalculator = nullptr;

	void					loadSettings();
	void					saveSettings();

public:

	int						measureType() const { return m_measureType; }

	bool					createInterface();

	void					createActions();
	void					createMenu();
	bool					createToolBars();
	void					createPanels();
	void					createMeasureViews();
	void					createStatusBar();
	void					createContextMenu();

	void					updateRacksOnToolBar();
	void					updateSignalsOnToolBar();

	QComboBox*				rackCombo() { return m_asRackCombo; }
	QComboBox*				signalCombo() { return m_asSignalCombo; }

	MeasureView*			activeMeasureView() { return measureView(m_measureType); }
	MeasureView*			measureView(int measureType);
	void					appendMeasureView(int measureType, MeasureView* pView);
	StatisticPanel*			statisticPanel() { return m_pStatisticPanel; }

	ConfigSocket*			configSocket() { return m_pConfigSocket; }
	SignalSocket*			signalSocket() { return m_pSignalSocket; }
	bool					signalSocketIsConnected();
	TuningSocket*			tuningSocket() { return m_pTuningSocket; }
	bool					tuningSocketIsConnected();

	MeasureThread&			measureThread() { return m_measureThread; }

	bool					signalSourceIsValid(bool showMsg);

protected:

	void					closeEvent(QCloseEvent* e);

signals:


	//
	void					changedMeasureType(int type);		// appear when changing the type of measurement
	void					changedOutputSignalType(int type);	// appear when changing the OutputSignalType

	// from measureComplite
	//
	void					appendMeasure(Measurement*);

private slots:

	// Slots of main menu
	//

	// menu - Measure
	//
	void					startMeasure();
	void					stopMeasure();
	void					exportMeasure();

	// menu - Edit
	//
	void					copyMeasure();
	void					removeMeasure();
	void					selectAllMeasure();

	// menu - View
	//

	// menu - Tools
	//
	void					calibrators();
	void					showRackList();
	void					showSignalList();
	void					showComparatorsList();
	void					showOutputSignalList();
	void					showTuningSignalList();
	void					showCalculator();
	void					showOptions();

	// menu - ?
	//
	void					showStatistic();
	void					aboutConnection() {}
	void					aboutApp();

	// Slots of tab -- page measure type
	//
	void					setMeasureType(int measureType);

	// Slots of control panels
	//
	void					setMeasureKind(int index);
	void					setMeasureTimeout(QString value);
	void					setOutputSignalType(int index);

	// Slots of analog signal toolbar
	//
	void					setRack(int index);
	void					setMeasureSignal(int index);
	void					setChassis(int index);
	void					setModule(int index);
	void					setPlace(int index);
	void					setMetrologySignalByPosition(int index);

	// Slots of contex menu
	//
	void					onContextMenu(QPoint);

	// Slots of calibrator base
	//
	void					calibratorConnectedChanged(int);

	// Slots of configSocket
	//
	void					configSocketConnected();
	void					configSocketDisconnected();
	void					configSocketConfigurationLoaded();

	// Slots of signalSocket
	//
	void					signalSocketConnected();
	void					signalSocketDisconnected();

	// Slots of tuningSocket
	//
	void					tuningSocketConnected();
	void					tuningSocketDisconnected();
	void					tuningSignalsCreated();

	// Slots of measure thread
	//
	void					measureThreadStarted();
	void					measureThreadStoped();
	void					setMeasureThreadInfo(QString msg);
	void					setMeasureThreadInfo(int timeout);
	void					measureThreadMsgBox(int type, QString text, int *result = nullptr);
	void					setNextMeasureSignal(bool& measureNextSignal);
	void					measureComplite(Measurement* pMeasurement);

	// Slots for enable measuring
	//
	void					updateStartStopActions();
};

// ==============================================================================================

#endif // MAINWINDOW_H
