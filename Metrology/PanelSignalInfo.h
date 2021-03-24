#ifndef PANELSIGNALINFO_H
#define PANELSIGNALINFO_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "CalibratorBase.h"
#include "SignalBase.h"
#include "DialogList.h"
#include "Options.h"

// ==============================================================================================

const char* const			SignalInfoColumn[] =
{
							QT_TRANSLATE_NOOP("PanelSignalInfo", "AppSignalID"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "CustomSignalID"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "EquipmentID"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Caption"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "State"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Rack"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Chassis"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Module"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Place"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Electric range"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Electric sensor"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Engineering range"),
							QT_TRANSLATE_NOOP("PanelSignalInfo", "Calibrator"),
};

const int					SIGNAL_INFO_COLUMN_COUNT		= sizeof(SignalInfoColumn)/sizeof(SignalInfoColumn[0]);

const int					SIGNAL_INFO_COLUMN_APP_ID		= 0,
							SIGNAL_INFO_COLUMN_CUSTOM_ID	= 1,
							SIGNAL_INFO_COLUMN_EQUIPMENT_ID	= 2,
							SIGNAL_INFO_COLUMN_CAPTION		= 3,
							SIGNAL_INFO_COLUMN_STATE		= 4,
							SIGNAL_INFO_COLUMN_RACK			= 5,
							SIGNAL_INFO_COLUMN_CHASSIS		= 6,
							SIGNAL_INFO_COLUMN_MODULE		= 7,
							SIGNAL_INFO_COLUMN_PLACE		= 8,
							SIGNAL_INFO_COLUMN_EL_RANGE		= 9,
							SIGNAL_INFO_COLUMN_EL_SENSOR	= 10,
							SIGNAL_INFO_COLUMN_EN_RANGE		= 11,
							SIGNAL_INFO_COLUMN_CALIBRATOR	= 12;

const int					SignalInfoColumnWidth[SIGNAL_INFO_COLUMN_COUNT] =
{
							250,	// SIGNAL_INFO_COLUMN_APP_ID
							250,	// SIGNAL_INFO_COLUMN_CUSTOM_ID
							250,	// SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150,	// SIGNAL_INFO_COLUMN_CAPTION
							150,	// SIGNAL_INFO_COLUMN_STATE
							100,	// SIGNAL_INFO_COLUMN_RACK
							 60,	// SIGNAL_INFO_COLUMN_CHASSIS
							 60,	// SIGNAL_INFO_COLUMN_MODULE
							 60,	// SIGNAL_INFO_COLUMN_PLACE
							150,	// SIGNAL_INFO_COLUMN_EL_RANGE
							100,	// SIGNAL_INFO_COLUMN_EL_SENSOR
							150,	// SIGNAL_INFO_COLUMN_EN_RANGE
							150,	// SIGNAL_INFO_COLUMN_CALIBRATOR
};

// ==============================================================================================

const int					SIGNAL_INFO_UPDATE_TIMER		= 250;	//	250 ms

// ==============================================================================================

class SignalInfoTable : public ListTable<IoSignalParam>
{
	Q_OBJECT

public:

	explicit SignalInfoTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~SignalInfoTable() override {}

public:

	void setSignalInfo(const SignalInfoOption& signalInfo) { m_signalInfo = signalInfo; }

	QString signalStateStr(const Metrology::SignalParam& param, const Metrology::SignalState& state) const;
	QString text(int column, const IoSignalParam& ioParam) const;

private:

	SignalInfoOption m_signalInfo;

	QVariant data(const QModelIndex &index, int role) const override;

public slots:

	void signalParamChanged(const QString& appSignalID);
};

// ==============================================================================================

class PanelSignalInfo : public QDockWidget
{
	Q_OBJECT

public:

	explicit PanelSignalInfo(const SignalInfoOption& signalInfo, QWidget* parent = nullptr);
	virtual ~PanelSignalInfo() override;

public:

	void clear() { m_signalParamTable.clear(); }
	void restartSignalStateTimer(int timeout);

	void setCalibratorBase(CalibratorBase* pCalibratorBase) { m_pCalibratorBase = pCalibratorBase; }
	void setSignalInfo(const SignalInfoOption& signalInfo);

private:

	// elements of interface
	//
	QMainWindow* m_pSignalInfoWindow = nullptr;
	QTableView* m_pView = nullptr;
	SignalInfoTable m_signalParamTable;

	std::vector<QAction*> m_pConnectionActionList;
	QMenu* m_pShowMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;
	QAction* m_pShowNoValidAction = nullptr;
	QAction* m_pShowElectricValueAction = nullptr;
	QAction* m_pShowSignalMoveUpAction = nullptr;
	QAction* m_pShowSignalMoveDownAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSignalPropertyAction = nullptr;

	QAction* m_pColumnAction[SIGNAL_INFO_COLUMN_COUNT];
	QMenu* m_headerContextMenu = nullptr;

	void createInterface();
	void createHeaderContexMenu();
	void initContextMenu();
	void createContextMenu();
	void appendMetrologyConnetionMenu();

	void hideColumn(int column, bool hide);

	QTimer* m_updateSignalStateTimer = nullptr;
	void startSignalStateTimer(int timeout);
	void stopSignalStateTimer();

	//
	//
	std::vector<Metrology::Signal*> m_destSignals;

	CalibratorBase* m_pCalibratorBase = nullptr;
	SignalInfoOption m_signalInfo;

	Measure::Kind m_measureKind = Measure::Kind::NoMeasureKind;
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

protected:

	bool eventFilter(QObject* object, QEvent* event) override;

public slots:

	void measureKindChanged(Measure::Kind measureKind);
	void connectionTypeChanged(Metrology::ConnectionType connectionType);

	void activeSignalChanged(const MeasureSignal& activeSignal);		// slot informs that signal for measure was selected
	void updateSignalState();										// slot informs that signal for measure has updated his state

signals:

	void changeActiveDestSignal(int channel, Metrology::Signal* pDestSignal);
	void changeActiveDestSignals(int channelPrev, int channelNext);

private slots:

	// slots of menu
	//
	void onConnectionAction(QAction* action);
	void showNoValid();
	void showElectricValue();
	void showSignalMoveUp();
	void showSignalMoveDown();
	void copy();
	void signalProperty();

	void onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void onHeaderContextMenu(QPoint);
	void onColumnAction(QAction* action);
	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&) { signalProperty(); }
};

// ==============================================================================================

#endif // PANELSIGNALINFO_H
