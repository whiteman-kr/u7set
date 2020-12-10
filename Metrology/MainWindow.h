#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QTabWidget>
#include <QTableView>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>

#include "../lib/SimpleThread.h"

#include "CalibratorBase.h"
#include "ConfigSocket.h"
#include "SignalSocket.h"
#include "TuningSocket.h"
#include "SelectSignalWidget.h"
#include "MeasureView.h"
#include "MeasureThread.h"
#include "FindMeasurePanel.h"
#include "StatisticsPanel.h"
#include "SignalInfoPanel.h"
#include "ComparatorInfoPanel.h"
#include "Calculator.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
	virtual ~MainWindow();

private:

	int						m_measureType = MEASURE_TYPE_UNDEFINED;
	int						m_measureKind = MEASURE_KIND_UNDEFINED;
	int						m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;
	int						m_measureTimeout = 0;

	QMap<int, MeasureView*> m_measureViewMap;

	// Actions of main menu
	//
							// menu - Measure
							//
	QAction*				m_pStartMeasureAction = nullptr;
	QAction*				m_pStopMeasureAction = nullptr;
	QAction*				m_pExportMeasureAction = nullptr;
	QAction*				m_pPreviousSignalAction = nullptr;
	QAction*				m_pNextSignalAction = nullptr;

							// menu - Edit
							//
	QAction*				m_pCopyMeasureAction = nullptr;
	QAction*				m_pRemoveMeasureAction = nullptr;
	QAction*				m_pSelectAllMeasureAction = nullptr;

							// menu - View
							//
	QAction*				m_pShowRackListAction = nullptr;
	QAction*				m_pShowSignalListAction = nullptr;
	QAction*				m_pShowComparatorsListAction = nullptr;
	QAction*				m_pShowTuningSignalListAction = nullptr;
	QAction*				m_pShowSignalConnectionListAction = nullptr;
	QAction*				m_pShowStatisticsAction = nullptr;

							// menu - Tools
							//
	QAction*				m_pCalibratorsAction = nullptr;
	QAction*				m_pShowCalculatorAction = nullptr;
	QAction*				m_pOptionsAction;

							// menu - ?
							//
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
	QMenu*					m_pToolsMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_pMeasureControlToolBar = nullptr;
	QToolBar*				m_pMeasureTimeoutToolBar = nullptr;
	QToolBar*				m_pMeasureKindToolBar = nullptr;
	QToolBar*				m_pSignalConnectionToolBar = nullptr;
	QToolBar*				m_pAnalogSignalToolBar = nullptr;

	// Elements of interface - Items of ToolBars
	//
	QComboBox*				m_pMeasureKindList = nullptr;
	QComboBox*				m_pSignalConnectionTypeList = nullptr;

	QComboBox*				m_pRackCombo = nullptr;
	SelectSignalWidget*		m_pSelectSignalWidget = nullptr;

	// Elements of interface - Pages of Tab
	//
	QTabWidget*				m_pMainTab = nullptr;

	// Elements of interface - Panels
	//
	FindMeasurePanel*		m_pFindMeasurePanel = nullptr;
	StatisticsPanel*		m_pStatisticsPanel = nullptr;
	SignalInfoPanel*		m_pSignalInfoPanel = nullptr;
	ComparatorInfoPanel*	m_pComparatorInfoPanel = nullptr;
	QTableView*				m_pComparatorInfoView = nullptr;

	// Elements of interface - StatusBar
	//
	QLabel*					m_statusEmpty = nullptr;
	QProgressBar*			m_statusLoadSignals = nullptr;
	QLabel*					m_statusMeasureThreadInfo = nullptr;
	QProgressBar*			m_statusMeasureTimeout = nullptr;
	QLabel*					m_statusMeasureThreadState = nullptr;
	QLabel*					m_statusCalibratorCount = nullptr;
	QLabel*					m_statusConnectToConfigServer = nullptr;
	QLabel*					m_statusConnectToAppDataServer = nullptr;
	QLabel*					m_statusConnectToTuningServer = nullptr;

private:

	SoftwareInfo			m_softwareInfo;

	CalibratorBase			m_calibratorBase;
	MeasureBase				m_measureBase;

	ConfigSocket*			m_pConfigSocket = nullptr;
	void					runConfigSocket();
	void					stopConfigSocket();
	QString					configSocketConnectedStateStr();

	SignalSocket*			m_pSignalSocket = nullptr;
	SimpleThread*			m_pSignalSocketThread = nullptr;
	void					runSignalSocket();
	void					stopSignalSocket();

	TuningSocket*			m_pTuningSocket = nullptr;
	SimpleThread*			m_pTuningSocketThread = nullptr;
	void					runTuningSocket();
	void					stopTuningSocket();
	QString					tuningSocketConnectedStateStr();

	MeasureThread			m_measureThread;
	void					runMeasureThread();
	void					stopMeasureThread();

private:

	Calculator*				m_pCalculator = nullptr;

	void					loadSettings();
	void					saveSettings();

private:

	bool					createInterface();

	void					createActions();
	void					createMenu();
	bool					createToolBars();
	void					createPanels();
	void					createMeasureViews();
	void					createStatusBar();
	void					createContextMenu();

	void					loadMeasureKindToolBar();
	void					loadSignalConnectionToolBar();
	void					loadRacksOnToolBar();
	void					loadSignalsOnToolBar();

public:

	int						measureType() const { return m_measureType; }

	// Views
	//
	MeasureView*			activeMeasureView() { return measureView(m_measureType); }
	MeasureView*			measureView(int measureType);
	void					appendMeasureView(int measureType, MeasureView* pView);

	// Sockets
	//
	ConfigSocket*			configSocket() { return m_pConfigSocket; }
	SignalSocket*			signalSocket() { return m_pSignalSocket; }
	bool					signalSocketIsConnected();
	TuningSocket*			tuningSocket() { return m_pTuningSocket; }
	bool					tuningSocketIsConnected();

	// Threads
	//
	MeasureThread&			measureThread() { return m_measureThread; }

	bool					signalSourceIsValid(bool showMsg);
	bool					changeInputSignalOnInternal(const MeasureSignal& activeSignal);
	bool					signalIsMeasured(const MeasureSignal& activeSignal, QString& signalID);
	bool					inputsOfmoduleIsSame(const MeasureSignal& activeSignal);					// only for mode "Single module"
	int						getMaxComparatorCount(const MeasureSignal& activeSignal);

protected:

	void					closeEvent(QCloseEvent* e);

signals:

	// from ToolBars
	//
	void					measureViewChanged(MeasureView* pView);	// appear when changing the type of measurement
	void					measureTimeoutChanged(int timeout);		// appear when changing the timeout of measuring
	void					measureTypeChanged(int type);			// appear when changing the type of measurement
	void					measureKindChanged(int kind);			// appear when changing the kind of measurement
	void					signalConnectionTypeChanged(int type);	// appear when changing the SignalConnectionType

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
	void					showRackList();
	void					showSignalList();
	void					showComparatorsList();
	void					showTuningSignalList();
	void					showSignalConnectionList();
	void					showStatistics();

	// menu - Tools
	//
	void					showCalibrators();
	void					showCalculator();
	void					showOptions();

	// menu - ?
	//
	void					aboutConnection();
	void					aboutApp();

	// Slots of tab -- page measure type
	//
	void					setMeasureType(int measureType);

	// Slots of control panels
	//
	void					setMeasureTimeout(QString value);
	void					setMeasureKind(int index);
	void					setSignalConnectionType(int index);

	// Slots of analog signal toolbar
	//
	void					setRack(int index);
	void					setAcitiveMeasureSignal(int index);
	void					previousMeasureSignal();
	void					nextMeasureSignal();
	bool					setNextMeasureSignalFromModule();
	void					changeActiveSignalOutput(int channel, Metrology::Signal* pOutputSignal);
	void					changeActiveSignalOutputs(int channelPrev, int channelNext);

	// Slots of contex menu
	//
	void					onContextMenu(QPoint);

	// Slots of calibrator base
	//
	void					calibratorConnectedChanged(int count);

	// Slots of configSocket
	//
	void					configSocketConnected();
	void					configSocketDisconnected();
	void					configSocketUnknownClient();
	void					configSocketConfigurationLoaded();
	void					configSocketSignalBaseLoading(int persentage);
	void					configSocketSignalBaseLoaded();

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
	void					measureThreadInfo(const MeasureThreadInfo& info);
	void					measureThreadMsgBox(int type, QString text, int *result = nullptr);
	void					measureComplite(Measurement* pMeasurement);

	// Slots of measure base
	//
	void					updateMeasureView();

	// Slots for enable measuring
	//
	void					updateStartStopActions();
	void					updatePrevNextSignalActions(int signalIndex);

	// Slots for panels
	//
	void					showFindMeasurePanel(const QString& appSignalID);
};

// ==============================================================================================

#endif // MAINWINDOW_H
