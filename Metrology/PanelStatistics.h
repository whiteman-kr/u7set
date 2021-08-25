#ifndef PANELSTATISTICS_H
#define PANELSTATISTICS_H

#include <QDebug>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QTableWidget>
#include <QLabel>
#include <QStatusBar>
#include <QKeyEvent>
#include <QClipboard>

#include "MeasureBase.h"
#include "SignalBase.h"

// ==============================================================================================

const char* const			StatisticsColumn[] =
{
							QT_TRANSLATE_NOOP("PanelStatistics", "SignalID"),
							QT_TRANSLATE_NOOP("PanelStatistics", "AppSignalID"),
							QT_TRANSLATE_NOOP("PanelStatistics", "EquipmentID"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Caption"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Comparator No"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Set point"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Rack"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Chassis"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Module"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Place"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Engineering range"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Electric range"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Electric sensor"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Signal type"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Connection"),
							QT_TRANSLATE_NOOP("PanelStatistics", "Measure count"),
							QT_TRANSLATE_NOOP("PanelStatistics", "State"),
};


const int					STATISTICS_COLUMN_COUNT				= sizeof(StatisticsColumn)/sizeof(StatisticsColumn[0]);

const int					STATISTICS_COLUMN_CUSTOM_ID			= 0,
							STATISTICS_COLUMN_APP_ID			= 1,
							STATISTICS_COLUMN_EQUIPMENT_ID		= 2,
							STATISTICS_COLUMN_CAPTION			= 3,
							STATISTICS_COLUMN_CMP_NO			= 4,
							STATISTICS_COLUMN_CMP_VALUE			= 5,
							STATISTICS_COLUMN_RACK				= 6,
							STATISTICS_COLUMN_CHASSIS			= 7,
							STATISTICS_COLUMN_MODULE			= 8,
							STATISTICS_COLUMN_PLACE				= 9,
							STATISTICS_COLUMN_EN_RANGE			= 10,
							STATISTICS_COLUMN_EL_RANGE			= 11,
							STATISTICS_COLUMN_EL_SENSOR			= 12,
							STATISTICS_COLUMN_SIGNAL_TYPE		= 13,
							STATISTICS_COLUMN_SIGNAL_CONNECTION	= 14,
							STATISTICS_COLUMN_MEASURE_COUNT		= 15,
							STATISTICS_COLUMN_STATE				= 16;


const int					StatisticsColumnWidth[STATISTICS_COLUMN_COUNT] =
{
							250,	// STATISTICS_COLUMN_CUSTOM_ID
							250,	// STATISTICS_COLUMN_APP_ID
							250,	// STATISTICS_COLUMN_EQUIPMENT_ID
							150,	// STATISTICS_COLUMN_CAPTION
							 50,	// STATISTICS_COLUMN_CMP_NO
							150,	// STATISTICS_COLUMN_CMP_VALUE
							100,	// STATISTICS_COLUMN_RACK
							 60,	// STATISTICS_COLUMN_CHASSIS
							 60,	// STATISTICS_COLUMN_MODULE
							 60,	// STATISTICS_COLUMN_PLACE
							150,	// STATISTICS_COLUMN_EN_RANGE
							150,	// STATISTICS_COLUMN_EL_RANGE
							100,	// STATISTICS_COLUMN_EL_SENSOR
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
	virtual ~StatisticsTable() override;

public:

	void set();
	void clear();

	QString text(int row, int column, const StatisticsItem& si) const;

	void updateSignal(Hash signalHash);

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;

private:

	int m_statisticsItemCount = 0;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;
};

// ==============================================================================================

class PanelStatistics : public QDockWidget
{
	Q_OBJECT

public:

	explicit PanelStatistics(QWidget* parent = nullptr);
	virtual ~PanelStatistics() override;

public:

	void setMeasureBase(Measure::Base* pMeasureBase) { m_pMeasureBase = pMeasureBase; }
	void setViewFont(const QFont& font);

private:

	// elements of interface
	//
	QMainWindow* m_pStatisticsWindow = nullptr;

	QMenuBar* m_pMenuBar = nullptr;
	QToolBar* m_pToolBar = nullptr;

	QMenu* m_pSignalMenu = nullptr;
	QMenu* m_pEditMenu = nullptr;
	QMenu* m_pViewMenu = nullptr;
	QMenu* m_pViewGotoMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pExportAction = nullptr;

	QAction* m_pSelectSignalForMeasure = nullptr;
	QAction* m_pFindSignalInMeasureList = nullptr;

	QAction* m_pCopyAction = nullptr;
	QAction* m_pCopyCellAction = nullptr;
	QAction* m_pSelectAllAction = nullptr;
	QAction* m_pSignalPropertyAction = nullptr;

	QAction* m_pShowSearchToolBarAction = nullptr;
	QAction* m_pGotoNextNotMeasuredAction = nullptr;
	QAction* m_pGotoNextInvalidAction = nullptr;

	QLineEdit* m_pFindTextEdit = nullptr;
	QAction* m_pFindPreviousAction = nullptr;
	QAction* m_pFindNextAction = nullptr;

	QStatusBar* m_pStatusBar;
	QLabel* m_statusEmpty = nullptr;
	QLabel* m_statusMeasureInavlid = nullptr;
	QLabel* m_statusMeasured = nullptr;

	QTableView* m_pView = nullptr;
	StatisticsTable m_signalTable;

	QMenu* m_headerContextMenu = nullptr;
	QAction* m_pColumnAction[STATISTICS_COLUMN_COUNT];
	QMap<QString, int> m_columnsWidth;

	Measure::Base* m_pMeasureBase = nullptr;

	static Measure::Type m_measureType;
	static Measure::Kind m_measureKind;
	static Metrology::ConnectionType m_connectionType;


	void createInterface();
	void createHeaderContexMenu();
	void createContextMenu();
	void createStatusBar();
	void updateStatusBar();

	void updateVisibleColunm();
	void hideColumn(int column, bool hide);
	void restoreColumnsWidth();
	void saveColumnsWidth();
	int firstVisibleColumn();

protected:

	bool eventFilter(QObject* object, QEvent* event) override;

signals:

	void setConnectionType(int connectionType);
	void setRack(int index);
	void setMeasureSignal(int index);

	void showFindMeasurePanel(const QString& signalID);

public slots:

	void measureTypeChanged(Measure::Type measureType);
	void measureKindChanged(Measure::Kind measureKind);
	void connectionTypeChanged(Metrology::ConnectionType connectionType);

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
	void findSignalInMeasureList();

		// Edit
		//
	void copy();
	void copyCell();
	void selectAll();
	void onProperty();

		// View
		//
	void showSearchToolBar();
	void gotoNextNotMeasured();
	void gotoNextInvalid();

	void onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void onHeaderContextMenu(QPoint);
	void onColumnAction(QAction* action);
	void onColumnResized(int, int, int);

	// slots for list
	//
	void onListDoubleClicked(const QModelIndex&);

	// slots for find
	//
	int findText(int startRow, bool reverseFind);
	void onFindTextChanged(const QString& text);
	void onFindPrevious();
	void onFindNext();
	void updateFindActions(int foundRow);
};

// ==============================================================================================

#endif // PANELSTATISTICS_H
