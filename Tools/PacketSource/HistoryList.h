#ifndef SIGNALHISTORYLIST_H
#define SIGNALHISTORYLIST_H

#include <QScreen>
#include <QAbstractTableModel>
#include <QColor>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QValidator>

#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QClipboard>

#include "History.h"

// ==============================================================================================

const char* const			SignalHistoryListColumn[] =
{
							QT_TRANSLATE_NOOP("HistoryList.h", "Time"),
							QT_TRANSLATE_NOOP("HistoryList.h", "CustomAppSignalID"),
							QT_TRANSLATE_NOOP("HistoryList.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("HistoryList.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("HistoryList.h", "Caption"),
							QT_TRANSLATE_NOOP("HistoryList.h", "Prev. state"),
							QT_TRANSLATE_NOOP("HistoryList.h", "State"),
							QT_TRANSLATE_NOOP("HistoryList.h", "Eng. range"),
};

const int					SIGNAL_HISTORY_LIST_COLUMN_COUNT		= sizeof(SignalHistoryListColumn)/sizeof(SignalHistoryListColumn[0]);

const int					SIGNAL_HISTORY_LIST_COLUMN_TIME			= 0,
							SIGNAL_HISTORY_LIST_COLUMN_CUSTOM_ID	= 1,
							SIGNAL_HISTORY_LIST_COLUMN_EQUIPMENT_ID	= 2,
							SIGNAL_HISTORY_LIST_COLUMN_APP_ID		= 3,
							SIGNAL_HISTORY_LIST_COLUMN_CAPTION		= 4,
							SIGNAL_HISTORY_LIST_COLUMN_PREV_STATE	= 5,
							SIGNAL_HISTORY_LIST_COLUMN_STATE		= 6,
							SIGNAL_HISTORY_LIST_COLUMN_EN_RANGE		= 7;

const int					SignalHistoryListColumnWidth[SIGNAL_HISTORY_LIST_COLUMN_COUNT] =
{
							150, //	SIGNAL_HISTORY_LIST_COLUMN_TIME
							200, //	SIGNAL_HISTORY_LIST_COLUMN_CUSTOM_ID
							200, //	SIGNAL_HISTORY_LIST_COLUMN_EQUIPMENT_ID
							200, //	SIGNAL_HISTORY_LIST_COLUMN_APP_ID
							150, //	SIGNAL_HISTORY_LIST_COLUMN_CAPTION
							100, //	SIGNAL_HISTORY_LIST_COLUMN_PREV_STATE
							100, //	SIGNAL_HISTORY_LIST_COLUMN_STATE
							150, //	SIGNAL_HISTORY_LIST_COLUMN_EN_RANGE
};

// ==============================================================================================

class SignalHistoryTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SignalHistoryTable(QObject* parent = nullptr);
	virtual ~SignalHistoryTable();

private:

	mutable QMutex			m_signalMutex;
	QVector<SignalForLog*>	m_signalList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						signalCount() const;
	SignalForLog*			signalPtr(int index) const;
	void					set(const QVector<SignalForLog*> list_add);
	void					clear();

	QString					text(int row, int column, SignalForLog *pSignal) const;

	void					updateColumn(int column);
};

// ==============================================================================================

class SignalHistoryDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalHistoryDialog(SignalHistory* pLog, QWidget *parent = nullptr);
	virtual ~SignalHistoryDialog();

private:

	SignalHistory*			m_pLog = nullptr;

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;

	QTableView*				m_pView = nullptr;
	SignalHistoryTable		m_signalTable;

	QAction*				m_pColumnAction[SIGNAL_HISTORY_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();

	void					hideColumn(int column, bool hide);

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//

							// Edit
							//
	void					copy();
	void					selectAll() { m_pView->selectAll(); }

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);
};

// ==============================================================================================

#endif // SIGNALHISTORYLIST_H
