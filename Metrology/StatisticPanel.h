#ifndef STATISTICPANEL_H
#define STATISTICPANEL_H

#include <QDebug>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QTableWidget>
#include <QLabel>
#include <QStatusBar>

#include "SignalBase.h"

// ==============================================================================================

const char* const			StatisticColumn[] =
{
							QT_TRANSLATE_NOOP("StatisticDialog.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "CustomSignalID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Chassis"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Module"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Place"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "ADC range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Electric range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Physical range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Engineering range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Measure count"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "State"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Signal connection"),
};

const int					STATISTIC_COLUMN_COUNT				= sizeof(StatisticColumn)/sizeof(StatisticColumn[0]);

const int					STATISTIC_COLUMN_APP_ID				= 0,
							STATISTIC_COLUMN_CUSTOM_ID			= 1,
							STATISTIC_COLUMN_EQUIPMENT_ID		= 2,
							STATISTIC_COLUMN_CAPTION			= 3,
							STATISTIC_COLUMN_RACK				= 4,
							STATISTIC_COLUMN_CHASSIS			= 5,
							STATISTIC_COLUMN_MODULE				= 6,
							STATISTIC_COLUMN_PLACE				= 7,
							STATISTIC_COLUMN_ADC				= 8,
							STATISTIC_COLUMN_EL_RANGE			= 9,
							STATISTIC_COLUMN_PH_RANGE			= 10,
							STATISTIC_COLUMN_EN_RANGE			= 11,
							STATISTIC_COLUMN_MEASURE_COUNT		= 12,
							STATISTIC_COLUMN_STATE				= 13,
							STATISTIC_COLUMN_SIGNAL_CONNECTION	= 14;

const int					StatisticColumnWidth[STATISTIC_COLUMN_COUNT] =
{
							250,	// STATISTIC_COLUMN_APP_ID
							250,	// STATISTIC_COLUMN_CUSTOM_ID
							250,	// STATISTIC_COLUMN_EQUIPMENT_ID
							150,	// STATISTIC_COLUMN_CAPTION
							100,	// STATISTIC_COLUMN_RACK
							 60,	// STATISTIC_COLUMN_CHASSIS
							 60,	// STATISTIC_COLUMN_MODULE
							 60,	// STATISTIC_COLUMN_PLACE
							100,	// STATISTIC_COLUMN_ADC
							150,	// STATISTIC_COLUMN_IN_EL_RANGE
							150,	// STATISTIC_COLUMN_IN_PH_RANGE
							150,	// STATISTIC_COLUMN_IN_EN_RANGE
							100,	// STATISTIC_COLUMN_MEASURE_COUNT
							100,	// STATISTIC_COLUMN_MEASURE_STATE
							100,	// STATISTIC_COLUMN_SIGNAL_CONNECTION
};

// ==============================================================================================

class StatisticTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit StatisticTable(QObject* parent = nullptr);
	virtual ~StatisticTable();

private:

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	void					set();
	void					clear();

	QString					text(int row, int column, Metrology::Signal* pSignal) const;

	void					updateSignal(Hash signalHash);
};

// ==============================================================================================

class StatisticPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit StatisticPanel(QWidget* parent = nullptr);
	virtual ~StatisticPanel();

private:

	// elements of interface
	//
	QMainWindow*			m_pMainWindow = nullptr;
	QMainWindow*			m_pStatisticWindow = nullptr;

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewGotoMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pExportAction = nullptr;
	QAction*				m_pSelectSignalForMeasure = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pSignalPropertyAction = nullptr;

	QAction*				m_pGotoNextNotMeasuredAction = nullptr;
	QAction*				m_pGotoNextInvalidAction = nullptr;

	QStatusBar*				m_pStatusBar;
	QLabel*					m_statusEmpty = nullptr;
	QLabel*					m_statusMeasureInavlid = nullptr;
	QLabel*					m_statusMeasured = nullptr;


	QTableView*				m_pView = nullptr;
	StatisticTable			m_signalTable;

	QAction*				m_pColumnAction[STATISTIC_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	static int				m_measureType;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();
	void					createStatusBar();
	void					updateStatusBar();

	void					updateVisibleColunm();
	void					hideColumn(int column, bool hide);

protected:

	bool					eventFilter(QObject *object, QEvent *event);

public slots:

	void					changedMeasureType(int type);
	void					changedSignalConnectionType(int type);

	void					activeSignalChanged(const MeasureSignal& activeSignal);	// slot informs that signal for measure was selected

	void					updateList();											// slots for reload list
	void					updateSignalInList(Hash signalHash);					// slots for updating one singal in list

private slots:

	// slots of menu
	//
							// Signal
							//
	void					exportSignal();
	void					selectSignalForMeasure();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll();
	void					signalProperty();

							// View
							//
	void					gotoNextNotMeasured();
	void					gotoNextInvalid();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) { selectSignalForMeasure(); }
};

// ==============================================================================================

#endif // STATISTICPANEL_H
