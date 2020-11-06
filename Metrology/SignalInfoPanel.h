#ifndef SIGNALINFOPANEL_H
#define SIGNALINFOPANEL_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "SignalBase.h"
#include "Options.h"

// ==============================================================================================

const char* const			SignalInfoColumn[] =
{
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "CustomSignalID"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "State"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Rack"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Chassis"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Module"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Place"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Electric range"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Electric sensor"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Engineering range"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Calibrator"),
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

class SignalInfoTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalInfoTable(QObject* parent = nullptr);
	virtual ~SignalInfoTable();

private:

	SignalInfoOption		m_signalInfo;

	mutable QMutex			m_signalMutex;
	int						m_signalCount = 0;
	QVector<IoSignalParam>	m_ioParamList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	void					setSignalInfo(const SignalInfoOption& signalInfo);

	int						signalCount() const { return m_signalCount; }
	IoSignalParam			signalParam(int index) const;
	void					set(const QVector<IoSignalParam>& ioParamList);
	void					clear();

	QString					text(int column, const IoSignalParam& ioParam) const;
	QString					signalStateStr(const Metrology::SignalParam& param, const Metrology::SignalState& state) const;

	void					updateColumn(int column);

private slots:

	void					updateSignalParam(const QString& appSignalID);
};

// ==============================================================================================

class SignalInfoPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit SignalInfoPanel(const SignalInfoOption& signalInfo, QWidget* parent = nullptr);
	virtual ~SignalInfoPanel();

private:

	// elements of interface
	//
	QMainWindow*			m_pSignalInfoWindow = nullptr;
	QTableView*				m_pView = nullptr;
	SignalInfoTable			m_signalParamTable;

	QVector<QAction*>		m_pConnectionActionList;
	QMenu*					m_pShowMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;
	QAction*				m_pShowNoValidAction = nullptr;
	QAction*				m_pShowElectricValueAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSignalPropertyAction = nullptr;

	QAction*				m_pColumnAction[SIGNAL_INFO_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	void					createInterface();
	void					createHeaderContexMenu();
	void					initContextMenu();
	void					createContextMenu();
	void					appendSignalConnetionMenu();

	void					hideColumn(int column, bool hide);

	QTimer*					m_updateSignalStateTimer = nullptr;
	void					startSignalStateTimer(int timeout);
	void					stopSignalStateTimer();

	//
	//
	QVector<Metrology::Signal*> m_outputSignalsList;

	int						m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNDEFINED;
	SignalInfoOption		m_signalInfo;

public:

	void					clear() { m_signalParamTable.clear(); }
	void					restartSignalStateTimer(int timeout);

	void					setSignalInfo(const SignalInfoOption& signalInfo);

protected:

	bool					eventFilter(QObject *object, QEvent *event);

signals:

	void					updateActiveOutputSignal(int channel, Metrology::Signal* pOutputSignal);

public slots:

	void					signalConnectionTypeChanged(int type);

	void					activeSignalChanged(const MeasureSignal& activeSignal);		// slot informs that signal for measure was selected
	void					updateSignalState();										// slot informs that signal for measure has updated his state

private slots:

	// slots of menu
	//
	void					onConnectionAction(QAction* action);
	void					showNoValid();
	void					showElectricValue();
	void					copy();
	void					signalProperty();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);
	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) { signalProperty(); }
};

// ==============================================================================================

#endif // SIGNALINFOPANELPANEL_H
