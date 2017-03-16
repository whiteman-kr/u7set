#ifndef SIGNALINFOPANEL_H
#define SIGNALINFOPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QTableView>

#include "SignalBase.h"

// ==============================================================================================

const char* const			SignalInfoColumn[] =
{
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Rack"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "ID"),
							QT_TRANSLATE_NOOP("SignalListDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Sate"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Chassis"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Module"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Place"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Ph. range"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "El. range"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "El. sensor"),
							QT_TRANSLATE_NOOP("SignalInfoMeasure.h", "Calibrator"),
};

const int					SIGNAL_INFO_COLUMN_COUNT		= sizeof(SignalInfoColumn)/sizeof(SignalInfoColumn[0]);

const int					SIGNAL_INFO_COLUMN_RACK			= 0,
							SIGNAL_INFO_COLUMN_ID			= 1,
							SIGNAL_INFO_COLUMN_EQUIPMENT_ID	= 2,
							SIGNAL_INFO_COLUMN_STATE		= 3,
							SIGNAL_INFO_COLUMN_CHASSIS		= 4,
							SIGNAL_INFO_COLUMN_MODULE		= 5,
							SIGNAL_INFO_COLUMN_PLACE		= 6,
							SIGNAL_INFO_COLUMN_CAPTION		= 7,
							SIGNAL_INFO_COLUMN_PH_RANGE		= 8,
							SIGNAL_INFO_COLUMN_EL_RANGE		= 9,
							SIGNAL_INFO_COLUMN_EL_SENSOR	= 10,
							SIGNAL_INFO_COLUMN_CALIBRATOR	= 11;

const int					SignalInfoColumnWidth[SIGNAL_INFO_COLUMN_COUNT] =
{
							100,	// SIGNAL_INFO_COLUMN_RACK
							270,	// SIGNAL_INFO_COLUMN_ID
							250,	// SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150,	// SIGNAL_INFO_COLUMN_STATE
							 60,	// SIGNAL_INFO_COLUMN_CHASSIS
							 60,	// SIGNAL_INFO_COLUMN_MODULE
							 60,	// SIGNAL_INFO_COLUMN_PLACE
							150,	// SIGNAL_INFO_COLUMN_CAPTION
							150,	// SIGNAL_INFO_COLUMN_PH_RANGE
							150,	// SIGNAL_INFO_COLUMN_EL_RANGE
							100,	// SIGNAL_INFO_COLUMN_EL_SENSOR
							150,	// SIGNAL_INFO_COLUMN_CALIBRATOR
};

// ==============================================================================================

class SignalInfoTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalInfoTable(QObject* parent = 0);
	virtual ~SignalInfoTable();

private:

	mutable QMutex			m_signalMutex;
	MeasureMultiParam		m_activeSignalParam[Metrology::ChannelCount];

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const { return Metrology::ChannelCount; }
	MeasureMultiParam		signalParam(int index) const;
	void					set(const MeasureSignal &activeSignal);
	void					clear();

	QString					text(int row, int column, const MeasureMultiParam &measureParam) const;
	QString					signalStateStr(const Metrology::SignalParam& param, const Metrology::SignalState& state) const;

	void					updateColumn(int column);

private slots:

	void					updateSignalParam(const Hash& signalHash);
};

// ==============================================================================================

#define						SIGNAL_INFO_OPTIONS_KEY		"Options/SignalInfo/"

// ==============================================================================================

class SignalInfoPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit SignalInfoPanel(QWidget* parent = 0);
	virtual ~SignalInfoPanel();

private:

	// elements of interface
	//
	QMainWindow*			m_pSignalInfoWindow = nullptr;
	QTableView*				m_pView = nullptr;
	SignalInfoTable			m_signalParamTable;

	QMenu*					m_pShowMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pShowCustomIDAction = nullptr;
	QAction*				m_pShowElectricValueAction = nullptr;
	QAction*				m_pShowAdcValueAction = nullptr;
	QAction*				m_pShowAdcHexValueAction = nullptr;
	QAction*				m_pSignalPropertyAction = nullptr;

	QAction*				m_pColumnAction[SIGNAL_INFO_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();

	void					hideColumn(int column, bool hide);

	QTimer*					m_updateSignalStateTimer = nullptr;
	void					startSignalStateTimer();
	void					stopSignalStateTimer();

public:

	void					clear() { m_signalParamTable.clear(); }

protected:

	bool					eventFilter(QObject *object, QEvent *event);

public slots:

	// slot informs that signal for measure was selected
	//
	void					activeSignalChanged(const MeasureSignal &signal);

	// slot informs that signal for measure has updated his state
	//
	void					updateSignalState();

private slots:

	// slots of menu
	//
	void					copy();
	void					showCustomID();
	void					showElectricValue();
	void					showAdcValue();
	void					showAdcHexValue();
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
