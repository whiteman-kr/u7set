#ifndef SIGNALHISTORY_H
#define SIGNALHISTORY_H

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

#include "SignalBase.h"

#include "../../lib/Signal.h"

// ==============================================================================================

class SignalForLog
{

public:

	SignalForLog();
	SignalForLog(const SignalForLog& from);
	SignalForLog(PS::Signal* m_pSignal, double prevState, double state);
	virtual ~SignalForLog();

private:

	mutable QMutex		m_signalMutex;

	QString				m_time;

	PS::Signal*			m_pSignal = nullptr;

	double				m_prevState = 0;
	double				m_state = 0;


public:

	void				clear();

	QString				time() const { return m_time; }
	void				setTime(const QString& time) { m_time = time; }

	PS::Signal*			signalPtr() const { return m_pSignal; }
	void				setSignalPtr(PS::Signal* pSignal) { m_pSignal = pSignal; }

	double				prevState() const { return m_prevState; }
	void				setPrevState(double state) { m_prevState = state; }

	double				state() const { return m_state; }
	void				setState(double state) { m_state = state; }

	QString				stateStr(double state) const;

	SignalForLog&		operator=(const SignalForLog& from);
};


// ==============================================================================================

class SignalHistory : public QObject
{
	Q_OBJECT

public:

	explicit SignalHistory(QObject *parent = nullptr);
	virtual ~SignalHistory();

private:

	mutable QMutex			m_signalMutex;
	QVector<SignalForLog>	m_signalList;

public:

	void					clear();
	int						count() const;


	int						append(const SignalForLog& signalLog);

	SignalForLog*			signalPtr(int index) const;
	SignalForLog			signal(int index) const;

	SignalHistory&			operator=(const SignalHistory& from);

signals:

	void					signalCountChanged();

public slots:
};

// ==============================================================================================

const char* const			SignalHistoryListColumn[] =
{
							QT_TRANSLATE_NOOP("SignalHistory.h", "Time"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "CustomAppSignalID"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "Caption"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "Prev. state"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "State"),
							QT_TRANSLATE_NOOP("SignalHistory.h", "Eng. range"),
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

#endif // SIGNALHISTORY_H
