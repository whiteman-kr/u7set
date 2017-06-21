#ifndef TUNINGSIGNALLISTDIALOG_H
#define TUNINGSIGNALLISTDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>

#include "TuningSignalBase.h"

// ==============================================================================================

const char* const			TuningSourceColumn[] =
{
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "IP"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Channel"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Subsytem"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "LM number"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "isReply"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Request count"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Reply count"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Cmd queue size"),
};

const int					TUN_SOURCE_LIST_COLUMN_COUNT		= sizeof(TuningSourceColumn)/sizeof(TuningSourceColumn[0]);

const int					TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID	= 0,
							TUN_SOURCE_LIST_COLUMN_CAPTION		= 1,
							TUN_SOURCE_LIST_COLUMN_IP			= 2,
							TUN_SOURCE_LIST_COLUMN_CHANNEL		= 3,
							TUN_SOURCE_LIST_COLUMN_SUBSYSTEM	= 4,
							TUN_SOURCE_LIST_COLUMN_LM_NUMBER	= 5,
							TUN_SOURCE_LIST_COLUMN_IS_REPLY		= 6,
							TUN_SOURCE_LIST_COLUMN_REQUESTS		= 7,
							TUN_SOURCE_LIST_COLUMN_REPLIES		= 8,
							TUN_SOURCE_LIST_COLUMN_COMMANDS		= 9;

const int					TuningSourceColumnWidth[TUN_SOURCE_LIST_COLUMN_COUNT] =
{
							250,	 // TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID
							150,	 // TUN_SOURCE_LIST_COLUMN_CAPTION
							150,	 // TUN_SOURCE_LIST_COLUMN_IP
							100,	 // TUN_SOURCE_LIST_COLUMN_CHANNEL
							100,	 // TUN_SOURCE_LIST_COLUMN_SUBSYSTEM
							100,	 // TUN_SOURCE_LIST_COLUMN_LM_NUMBER
							100,	 // TUN_SOURCE_LIST_COLUMN_IS_REPLY
							100,	 // TUN_SOURCE_LIST_COLUMN_REQUESTS
							100,	 // TUN_SOURCE_LIST_COLUMN_REPLIES
							100,	 // TUN_SOURCE_LIST_COLUMN_COMMANDS
};


// ==============================================================================================

class TuningSourceTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit TuningSourceTable(QObject* parent = 0);
	virtual ~TuningSourceTable();

private:

	mutable QMutex			m_sourceMutex;
	QList<TuningSource>		m_sourceIdList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						sourceCount() const;
	TuningSource			source(int index) const;
	void					set(const QList<TuningSource> list_add);
	void					clear();

	QString					text(int row, int column, const TuningSource& source, const TuningSourceState& state) const;

	void					updateColumn(int column);

};

// ==============================================================================================

const char* const			TuningSignalColumn[] =
{
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Rack"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "AppSignalID"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "CustomSignalID"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "EquipmentID"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "State"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Default"),
							QT_TRANSLATE_NOOP("TuningSignalListDialog.h", "Range"),
};

const int					TUN_SIGNAL_LIST_COLUMN_COUNT		= sizeof(TuningSignalColumn)/sizeof(TuningSignalColumn[0]);

const int					TUN_SIGNAL_LIST_COLUMN_RACK			= 0,
							TUN_SIGNAL_LIST_COLUMN_APP_ID		= 1,
							TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID	= 2,
							TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID	= 3,
							TUN_SIGNAL_LIST_COLUMN_CAPTION		= 4,
							TUN_SIGNAL_LIST_COLUMN_STATE		= 5,
							TUN_SIGNAL_LIST_COLUMN_DEFAULT		= 6,
							TUN_SIGNAL_LIST_COLUMN_RANGE		= 7;

const int					TuningSignalColumnWidth[TUN_SIGNAL_LIST_COLUMN_COUNT] =
{
							100,	 // TUN_SIGNAL_LIST_COLUMN_RACK
							250,	 // TUN_SIGNAL_LIST_COLUMN_APP_ID
							250,	 // TUN_SIGNAL_LIST_COLUMN_CUSTOM_ID
							250,	 // TUN_SIGNAL_LIST_COLUMN_EQUIPMENT_ID
							150,	 // TUN_SIGNAL_LIST_COLUMN_CAPTION
							100,	 // TUN_SIGNAL_LIST_COLUMN_VALUE
							100,	 // TUN_SIGNAL_LIST_COLUMN_DEFAULT
							150,	 // TUN_SIGNAL_LIST_COLUMN_RANGE
};


// ==============================================================================================

class TuningSignalTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit TuningSignalTable(QObject* parent = 0);
	virtual ~TuningSignalTable();

private:

	mutable QMutex			m_signalMutex;
	QList<Metrology::Signal*>	m_signalList;

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
	QString					signalStateStr(Metrology::Signal* pSignal) const;

	void					updateColumn(int column);
};

// ==============================================================================================

class TuningSignalListDialog : public QDialog
{
	Q_OBJECT

public:

	explicit TuningSignalListDialog(QWidget *parent = 0);
	virtual ~TuningSignalListDialog();

private:

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pSignalMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pViewMenu = nullptr;
	QMenu*					m_pViewTypeADMenu = nullptr;
	QMenu*					m_pViewShowMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pExportAction = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;

	QAction*				m_pTypeAnalogAction = nullptr;
	QAction*				m_pTypeDiscreteAction = nullptr;
	QAction*				m_pTypeBusAction = nullptr;
	QAction*				m_pShowSoucreAction = nullptr;

	QTableView*				m_pSourceView = nullptr;
	TuningSourceTable		m_sourceTable;

	QTableView*				m_pSignalView = nullptr;
	TuningSignalTable		m_signalTable;

	QAction*				m_pColumnAction[TUN_SIGNAL_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	static E::SignalType	m_typeAD;
	static bool				m_showSource;

	Hash					m_selectedSignalHash = 0;

	void					createInterface();
	void					createHeaderContexMenu();
	void					createContextMenu();

	void					updateVisibleColunm();
	void					hideColumn(int column, bool hide);

	QTimer*					m_updateSignalStateTimer = nullptr;
	void					startSignalStateTimer();
	void					stopSignalStateTimer();

public:

	Hash					selectedSignalHash() const { return m_selectedSignalHash; }

protected:

	bool					eventFilter(QObject *object, QEvent *event);

signals:

private slots:

	// slots for updating source signal list
	//
	void					updateSourceList();
	void					updateSignalList();

	// slot informs that signal for measure has updated his state
	//
	void					updateState();

	// slots of menu
	//
							// Signal
							//
	void					exportSignal();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pSignalView->selectAll(); }

							// View
							//
	void					showTypeAnalog();
	void					showTypeDiscrete();
	void					showTypeBus();
	void					showSources();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&) {}

};

// ==============================================================================================

#endif // TUNINGSIGNALLISTDIALOG_H
