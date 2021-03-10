#ifndef DIALOGRACKLIST_H
#define DIALOGRACKLIST_H

#include "../lib/MetrologySignal.h"

#include "RackBase.h"
#include "DialogList.h"

// ==============================================================================================

const char* const			RackListColumn[] =
{
							QT_TRANSLATE_NOOP("DialogRackList", "Caption"),
							QT_TRANSLATE_NOOP("DialogRackList", "EquipmentID"),
							QT_TRANSLATE_NOOP("DialogRackList", "Group"),
							QT_TRANSLATE_NOOP("DialogRackList", "Channel"),
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

	explicit RackListTable(QObject* parent = nullptr);
	virtual ~RackListTable() override;

public:

	int						rackCount() const;
	Metrology::RackParam*	rack(int index) const;
	void					set(const QVector<Metrology::RackParam*>& list_add);
	void					clear();

	QString					text(int row, int column, const Metrology::RackParam* pRack) const;

	void					setRackGroups(const RackGroupBase& rackGroups) { m_rackGroups = rackGroups; }

private:

	mutable QMutex			m_rackMutex;
	QVector<Metrology::RackParam*> m_rackList;

	RackGroupBase			m_rackGroups;

	int						columnCount(const QModelIndex &parent) const override;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant				data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogRackList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogRackList(QWidget* parent = nullptr);
	virtual ~DialogRackList();

public:

	RackBase&				racks() { return m_rackBase; }

private:

	RackBase				m_rackBase;

	QMenu*					m_pRackMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;

	QAction*				m_pRackGroupsAction = nullptr;

	RackListTable			m_rackTable;

	void					createInterface();
	void					createContextMenu();

public slots:

	void					updateList() override;	// slots for updating

private slots:

	// slots of menu
	//
							// Rack
							//
	void					rackGroups();

							// Edit
							//
	void					onProperties() override;
};

// ==============================================================================================

#endif // DIALOGRACKLISTDIALOG_H
