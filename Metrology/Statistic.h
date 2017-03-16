#ifndef STATISTICDIALOG_H
#define STATISTICDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QStatusBar>

#include "../lib/Signal.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const			StatisticColumn[] =
{
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "ID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Chassis"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Module"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Place"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "ADC range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Input Ph.range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Input El.range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Output type"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Output Ph.range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Output El.range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Measure count"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "State"),

};

const int					STATISTIC_COLUMN_COUNT			= sizeof(StatisticColumn)/sizeof(StatisticColumn[0]);

const int					STATISTIC_COLUMN_RACK			= 0,
							STATISTIC_COLUMN_ID				= 1,
							STATISTIC_COLUMN_EQUIPMENT_ID	= 2,
							STATISTIC_COLUMN_CAPTION		= 3,
							STATISTIC_COLUMN_CHASSIS		= 4,
							STATISTIC_COLUMN_MODULE			= 5,
							STATISTIC_COLUMN_PLACE			= 6,
							STATISTIC_COLUMN_ADC			= 7,
							STATISTIC_COLUMN_IN_PH_RANGE	= 8,
							STATISTIC_COLUMN_IN_EL_RANGE	= 9,
							STATISTIC_COLUMN_OUTPUT_TYPE	= 10,
							STATISTIC_COLUMN_OUT_PH_RANGE	= 11,
							STATISTIC_COLUMN_OUT_EL_RANGE	= 12,
							STATISTIC_COLUMN_MEASURE_COUNT	= 13,
							STATISTIC_COLUMN_STATE			= 14;

const int					StatisticColumnWidth[STATISTIC_COLUMN_COUNT] =
{
							100,	// STATISTIC_COLUMN_RACK
							250,	// STATISTIC_COLUMN_ID
							250,	// STATISTIC_COLUMN_EQUIPMENT_ID
							150,	// STATISTIC_COLUMN_CAPTION
							 60,	// STATISTIC_COLUMN_CHASSIS
							 60,	// STATISTIC_COLUMN_MODULE
							 60,	// STATISTIC_COLUMN_PLACE
							100,	// STATISTIC_COLUMN_ADC
							150,	// STATISTIC_COLUMN_IN_PH_RANGE
							150,	// STATISTIC_COLUMN_IN_EL_RANGE
							100,	// STATISTIC_COLUMN_OUTPUT_TYPE
							150,	// STATISTIC_COLUMN_OUT_PH_RANGE
							150,	// STATISTIC_COLUMN_OUT_EL_RANGE
							100,	// STATISTIC_COLUMN_MEASURE_COUNT
							100,	// STATISTIC_COLUMN_MEASURE_STATE
};

// ==============================================================================================

class StatisticTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit StatisticTable(QObject* parent = 0);
	virtual ~StatisticTable();

private:

	mutable QMutex			m_signalMutex;
	QList<Metrology::Signal*> m_signalList;

	static bool				m_showCustomID;
	static bool				m_showADCInHex;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const;
	Metrology::Signal*		signal(int index) const;
	void					set(const QList<Metrology::Signal*> list_add);
	void					clear();

	QString					text(int row, int column, Metrology::Signal* pSignal) const;

	bool					showCustomID() const { return m_showCustomID; }
	void					setShowCustomID(bool show) { m_showCustomID = show; }

	bool					showADCInHex() const { return m_showADCInHex; }
	void					setShowADCInHex(bool show) { m_showADCInHex = show; }

private slots:

	void					updateSignalParam(const Hash& signalHash);
};

// ==============================================================================================

class StatisticDialog : public QDialog
{
	Q_OBJECT

public:

	explicit StatisticDialog(QWidget *parent = 0);
	virtual ~StatisticDialog();

private:

	QMainWindow*			m_pMainWindow = nullptr;

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewMeasureTypeMenu = nullptr;
	QMenu*					m_pViewShowMenu = nullptr;
	QMenu*					m_pViewGotoMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pPrintAction = nullptr;
	QAction*				m_pExportAction = nullptr;
	QAction*				m_pSelectSignalForMeasure = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;

	QAction*				m_pTypeLinearityAction = nullptr;
	QAction*				m_pTypeComparatorsAction = nullptr;
	QAction*				m_pShowCustomIDAction = nullptr;
	QAction*				m_pShowADCInHexAction = nullptr;
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

	int						m_MeasuredCount = 0;
	int						m_invalidMeasureCount = 0;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();
	void					createStatusBar();
	void					updateStatusBar();

	void					updateVisibleColunm();
	void					hideColumn(int column, bool hide);

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//
							// Signal
							//
	void					printSignal();
	void					exportSignal();
	void					selectSignalForMeasure();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pView->selectAll(); }

							// View
							//
	void					showTypeLinearity();
	void					showTypeComparators();
	void					showCustomID();
	void					showADCInHex();
	void					gotoNextNotMeasured();
	void					gotoNextInvalid();


	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);
};

// ==============================================================================================

#endif // STATISTICDIALOG_H
