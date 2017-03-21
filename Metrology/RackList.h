#ifndef RACKLISTDIALOG_H
#define RACKLISTDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>

#include "SignalBase.h"

// ==============================================================================================

const char* const			RackListColumn[] =
{
							QT_TRANSLATE_NOOP("RackListDialog.h", "Caption"),
							QT_TRANSLATE_NOOP("RackListDialog.h", "ID"),
							QT_TRANSLATE_NOOP("RackListDialog.h", "Group"),
							QT_TRANSLATE_NOOP("RackListDialog.h", "Channel"),
};

const int					RACK_LIST_COLUMN_COUNT			= sizeof(RackListColumn)/sizeof(RackListColumn[0]);

const int					RACK_LIST_COLUMN_CAPTION		= 0,
							RACK_LIST_COLUMN_ID				= 1,
							RACK_LIST_COLUMN_GROUP			= 2,
							RACK_LIST_COLUMN_CHANNEL		= 3;

const int					RackListColumnWidth[RACK_LIST_COLUMN_COUNT] =
{
							150,	// RACK_LIST_COLUMN_CAPTION
							250,	// RACK_LIST_COLUMN_ID
							100,	// RACK_LIST_COLUMN_GROUP
							100,	// RACK_LIST_COLUMN_CHANNEL
};

// ==============================================================================================

class RackListTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit RackListTable(QObject* parent = 0);
	virtual ~RackListTable();

private:

	mutable QMutex			m_rackMutex;
	QList<Metrology::RackParam*> m_rackList;

	RackGroupBase			m_rackGroups;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						rackCount() const;
	Metrology::RackParam*	rack(int index) const;
	void					set(const QList<Metrology::RackParam*> list_add);
	void					clear();

	QString					text(int row, int column, const Metrology::RackParam* pRack) const;

	void					setRackGroups(const RackGroupBase& rackGroups) { m_rackGroups = rackGroups; }

private slots:

};

// ==============================================================================================

class RackListDialog : public QDialog
{
	Q_OBJECT

public:

	explicit RackListDialog(QWidget *parent = 0);
	virtual ~RackListDialog();

private:

	RackBase				m_rackBase;

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pRackMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QAction*				m_pRackGroupsAction = nullptr;
	QAction*				m_pImportAction = nullptr;
	QAction*				m_pExportAction = nullptr;

	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pRackPropertyAction = nullptr;

	QTableView*				m_pView = nullptr;
	RackListTable			m_rackTable;

	QDialogButtonBox*		m_buttonBox = nullptr;

	QAction*				m_pColumnAction[RACK_LIST_COLUMN_COUNT];
	QMenu*					m_headerContextMenu = nullptr;

	void					createInterface();
	void					createContextMenu();

protected:

	bool					eventFilter(QObject *object, QEvent *event);

signals:

private slots:

	// slots for updating
	//
	void					updateList();

	// slots of menu
	//
							// Rack
							//
	void					rackGroups();
	void					importRack();
	void					exportRack();

							// Edit
							//
	void					find();
	void					copy();
	void					selectAll() { m_pView->selectAll(); }
	void					rackProperty();

	// slots for list
	//
	void					onContextMenu(QPoint);
	void					onListDoubleClicked(const QModelIndex&);

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // RACKLISTDIALOG_H
