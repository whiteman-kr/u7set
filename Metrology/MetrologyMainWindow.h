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

#include "../UtilsLib/SimpleThread.h"

#include "CalibratorBase.h"
#include "ConfigSocket.h"
#include "SignalSocket.h"
#include "TuningSocket.h"
#include "SelectSignalWidget.h"
#include "MeasureView.h"
#include "MeasureThread.h"
#include "PanelFindMeasure.h"
#include "PanelStatistics.h"
#include "PanelSignalInfo.h"
#include "PanelComparatorInfo.h"
#include "DialogCalculator.h"

// ==============================================================================================

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(const SoftwareInfo& softwareInfo, QWidget* parent = nullptr);
	virtual ~MainWindow() override;

public:

	Measure::Type			measureType() const { return m_measureType; }

	// Views
	//
	Measure::View*			activeMeasureView() { return measureView(m_measureType); }
	Measure::View*			measureView(Measure::Type measureType);
	void					appendMeasureView(int measureType, Measure::View* pView);

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

private:

	int						m_measureTimeout = 0;
	Measure::Type			m_measureType = Measure::Type::NoMeasureType;
	Measure::Kind			m_measureKind = Measure::Kind::NoMeasureKind;
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

	QMap<int, Measure::View*> m_measureViewMap;

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
	QAction*				m_pShowConnectionListAction = nullptr;
	QAction*				m_pShowTuningSourceListAction = nullptr;
	QAction*				m_pShowTuningSignalListAction = nullptr;
	QAction*				m_pShowGraphLinElAction = nullptr;
	QAction*				m_pShowGraphLinEnAction = nullptr;
	QAction*				m_pShowGraph20ElAction = nullptr;
	QAction*				m_pShowGraph20EnAction = nullptr;
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
	QMenu*					m_pViewTuningMenu = nullptr;
	QMenu*					m_pViewGraphMenu = nullptr;
	QMenu*					m_pToolsMenu = nullptr;
	QMenu*					m_pInfoMenu = nullptr;

	// Elements of interface - ToolBar
	//
	QToolBar*				m_pMeasureControlToolBar = nullptr;
	QToolBar*				m_pMeasureTimeoutToolBar = nullptr;
	QToolBar*				m_pMeasureKindToolBar = nullptr;
	QToolBar*				m_pConnectionToolBar = nullptr;
	QToolBar*				m_pAnalogSignalToolBar = nullptr;

	// Elements of interface - Items of ToolBars
	//
	QComboBox*				m_pMeasureKindList = nullptr;
	QComboBox*				m_pConnectionTypeList = nullptr;

	QComboBox*				m_pRackCombo = nullptr;
	SelectSignalWidget*		m_pSelectSignalWidget = nullptr;

	// Elements of interface - Pages of Tab
	//
	QTabWidget*				m_pMainTab = nullptr;

	// Elements of interface - Panels
	//
	PanelFindMeasure*		m_pFindMeasurePanel = nullptr;
	PanelStatistics*		m_pStatisticsPanel = nullptr;
	PanelSignalInfo*		m_pSignalInfoPanel = nullptr;
	PanelComparatorInfo*	m_pComparatorInfoPanel = nullptr;

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
	Measure::Base			m_measureBase;

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

	DialogCalculator*		m_pCalculator = nullptr;

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

	void					loadOnToolBar_MeasureKind();
	void					loadOnToolBar_Connection();
	void					loadOnToolBar_Racks();
	void					loadOnToolBar_Signals();

protected:

	void					closeEvent(QCloseEvent* e) override;

signals:

	// from ToolBars
	//
	void					measureViewChanged(Measure::View* pView);							// appear when changing the type of measurement
	void					measureTimeoutChanged(int timeout);									// appear when changing the timeout of measuring
	void					measureTypeChanged(Measure::Type measureType);						// appear when changing the type of measurement
	void					measureKindChanged(Measure::Kind kind);								// appear when changing the kind of measurement
	void					connectionTypeChanged(Metrology::ConnectionType connectionType);	// appear when changing the Metrology::ConnectionType

	// from measureComplite
	//
	void					appendMeasure(Measure::Item*);

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
	void					showConnectionList();
	void					showTuningSourceList();
	void					showTuningSignalList();
	void					showGraphLinEl();
	void					showGraphLinEn();
	void					showGraph20El();
	void					showGraph20En();
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
	void					setConnectionType(int index);
	void					setConnectionTypeFromStatistic(int connectionType);

	// Slots of analog signal toolbar
	//
	void					setRack(int index);
	void					setAcitiveMeasureSignal(int index);
	void					previousMeasureSignal();
	void					nextMeasureSignal();
	bool					setNextMeasureSignalFromModule();
	void					changeActiveDestSignal(int channel, Metrology::Signal* pDestSignal);
	void					changeActiveDestSignals(int channelPrev, int channelNext);

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
	void					measureThreadMsgBox(int type, QString text, int* result = nullptr);
	void					measureComplite(Measure::Item* pMeasurement);

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
