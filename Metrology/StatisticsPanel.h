#ifndef STATISTICPANEL_H
#define STATISTICPANEL_H

#include <QDebug>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QTableWidget>
#include <QLabel>
#include <QStatusBar>
#include <QKeyEvent>

#include "MeasureBase.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const			StatisticsColumn[] =
{
							QT_TRANSLATE_NOOP("StatisticDialog.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "CustomSignalID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Comparator value"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Comparator No"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Comparator Output ID"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Chassis"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Module"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Place"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Electric range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Electric sensor"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Engineering range"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Signal type"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Connection"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "Measure count"),
							QT_TRANSLATE_NOOP("StatisticDialog.h", "State"),

};

const int					STATISTICS_COLUMN_COUNT				= sizeof(StatisticsColumn)/sizeof(StatisticsColumn[0]);

const int					STATISTICS_COLUMN_APP_ID				= 0,
							STATISTICS_COLUMN_CUSTOM_ID			= 1,
							STATISTICS_COLUMN_EQUIPMENT_ID		= 2,
							STATISTICS_COLUMN_CAPTION			= 3,
							STATISTICS_COLUMN_CMP_VALUE			= 4,
							STATISTICS_COLUMN_CMP_NO				= 5,
							STATISTICS_COLUMN_CMP_OUT_ID			= 6,
							STATISTICS_COLUMN_RACK				= 7,
							STATISTICS_COLUMN_CHASSIS			= 8,
							STATISTICS_COLUMN_MODULE				= 9,
							STATISTICS_COLUMN_PLACE				= 10,
							STATISTICS_COLUMN_EL_RANGE			= 11,
							STATISTICS_COLUMN_EL_SENSOR			= 12,
							STATISTICS_COLUMN_EN_RANGE			= 13,
							STATISTICS_COLUMN_SIGNAL_TYPE		= 14,
							STATISTICS_COLUMN_SIGNAL_CONNECTION	= 15,
							STATISTICS_COLUMN_MEASURE_COUNT		= 16,
							STATISTICS_COLUMN_STATE				= 17;


const int					StatisticsColumnWidth[STATISTICS_COLUMN_COUNT] =
{
							250,	// STATISTICS_COLUMN_APP_ID
							250,	// STATISTICS_COLUMN_CUSTOM_ID
							250,	// STATISTICS_COLUMN_EQUIPMENT_ID
							150,	// STATISTICS_COLUMN_CAPTION
							150,	// STATISTICS_COLUMN_CMP_VALUE
							 50,	// STATISTICS_COLUMN_CMP_NO
							250,	// STATISTICS_COLUMN_CMP_OUT_ID
							100,	// STATISTICS_COLUMN_RACK
							 60,	// STATISTICS_COLUMN_CHASSIS
							 60,	// STATISTICS_COLUMN_MODULE
							 60,	// STATISTICS_COLUMN_PLACE
							150,	// STATISTICS_COLUMN_EL_RANGE
							100,	// STATISTICS_COLUMN_EL_SENSOR
							150,	// STATISTICS_COLUMN_EN_RANGE
							100,	// STATISTICS_COLUMN_SIGNAL_TYPE
							100,	// STATISTICS_COLUMN_SIGNAL_CONNECTION
							100,	// STATISTICS_COLUMN_MEASURE_COUNT
							100,	// STATISTICS_COLUMN_MEASURE_STATE

};

// ==============================================================================================

class StatisticsTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit StatisticsTable(QObject* parent = nullptr);
	virtual ~StatisticsTable();

public:

	void set();
	void clear();

	QString text(int row, int column, const StatisticsItem& si) const;

	void updateSignal(Hash signalHash);

private:

	int m_statisticsItemCount = 0;

	int columnCount(const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;
};

// ==============================================================================================

class StatisticsPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit StatisticsPanel(QWidget* parent = nullptr);
	virtual ~StatisticsPanel();

public:

	void setMeasureBase(MeasureBase* pMeasureBase) { m_pMeasureBase = pMeasureBase; }
	void setViewFont(const QFont& font);

private:

	// elements of interface
	//
	QMainWindow* m_pStatisticsWindow = nullptr;

	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pSignalMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pViewMenu = nullptr;
	QMenu* m_pViewGotoMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pExportAction = nullptr;

	QAction* m_pSelectSignalForMeasure = nullptr;
	QAction* m_pFindSignalInStatisticsList = nullptr;
	QAction* m_pFindSignalInMeasureList = nullptr;

	QAction* m_pFindAction = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;
	QAction* m_pSignalPropertyAction = nullptr;

	QAction* m_pGotoNextNotMeasuredAction = nullptr;
	QAction* m_pGotoNextInvalidAction = nullptr;

	QStatusBar* m_pStatusBar;
	QLabel* m_statusEmpty = nullptr;
	QLabel* m_statusMeasureInavlid = nullptr;
	QLabel* m_statusMeasured = nullptr;

	QTableView* m_pView = nullptr;
	StatisticsTable m_signalTable;

	QAction* m_pColumnAction[STATISTICS_COLUMN_COUNT];
	QMenu* m_headerContextMenu = nullptr;

	MeasureBase* m_pMeasureBase = nullptr;

	static int m_measureType;
	static int m_measureKind;
	static Metrology::ConnectionType m_connectionType;


	void createInterface();
	void createHeaderContexMenu();
	void createContextMenu();
	void createStatusBar();
	void updateStatusBar();

	void updateVisibleColunm();
	void hideColumn(int column, bool hide);

protected:

	bool eventFilter(QObject* object, QEvent* event);

signals:

	void setConnectionType(Metrology::ConnectionType type);
	void setRack(int index);
	void setMeasureSignal(int index);

	void showFindMeasurePanel(const QString& appSignalID);

public slots:

	void measureTypeChanged(int type);
	void measureKindChanged(int kind);
	void connectionTypeChanged(Metrology::ConnectionType type);

	void activeSignalChanged(const MeasureSignal& activeSignal);	// slot informs that signal for measure was selected

	void updateList();												// slots for reload list
	void updateSignalInList(Hash signalHash);						// slots for updating one singal in list

private slots:

	// slots of menu
	//
		// Signal
		//
	void exportSignal();
	void selectSignalForMeasure();
	void findSignalInStatisticsList();
	void findSignalInMeasureList();

		// Edit
		//
	void find();
	void copy();
	void selectAll();
	void onProperty();

		// View
		//
	void gotoNextNotMeasured();
	void gotoNextInvalid();

	void onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void onHeaderContextMenu(QPoint);
	void onColumnAction(QAction* action);

	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&) { selectSignalForMeasure(); }
};

// ==============================================================================================

#endif // STATISTICPANEL_H
