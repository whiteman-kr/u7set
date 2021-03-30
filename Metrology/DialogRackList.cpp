#include "DialogRackList.h"

#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

QVariant RackListTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= count())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > m_columnCount)
	{
		return QVariant();
	}

	Metrology::RackParam* pRack = at(row);
	if (pRack == nullptr)
	{
		return QVariant();
	}

	if (pRack->isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		int result = Qt::AlignLeft;

		switch (column)
		{
			case RACK_LIST_COLUMN_CAPTION:	result = Qt::AlignLeft;		break;
			case RACK_LIST_COLUMN_ID:		result = Qt::AlignLeft;		break;
			case RACK_LIST_COLUMN_GROUP:	result = Qt::AlignCenter;	break;
			case RACK_LIST_COLUMN_CHANNEL:	result = Qt::AlignCenter;	break;
			default:						assert(0);
		}

		return result;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, pRack);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString RackListTable::text(int row, int column, const Metrology::RackParam* pRack) const
{
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	if (pRack == nullptr)
	{
		return QString();
	}

	if (pRack->isValid() == false)
	{
		return QString();
	}

	QString groupCaption;

	int index = pRack->groupIndex();
	if (index >= 0 && index < m_rackGroups.count())
	{
		groupCaption = m_rackGroups.group(index).caption();
	}

	QString result;

	switch (column)
	{
		case RACK_LIST_COLUMN_CAPTION:	result = pRack->caption();		break;
		case RACK_LIST_COLUMN_ID:		result = pRack->equipmentID();	break;
		case RACK_LIST_COLUMN_GROUP:	result = groupCaption;			break;
		case RACK_LIST_COLUMN_CHANNEL:	result = pRack->channelStr();	break;
		default:						assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogRackList::DialogRackList(QWidget* parent) :
	DialogList(0.35, 0.4, true, parent)
{
	m_rackBase = theSignalBase.racks();
	m_rackTable.setRackGroups(m_rackBase.groups());

	createInterface();
	DialogRackList::updateList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogRackList::~DialogRackList()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackList::createInterface()
{
	setWindowTitle(tr("Racks"));

	// menu
	//
	m_pRackMenu = new QMenu(tr("&Racks"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	// action
	//
	m_pRackGroupsAction = m_pRackMenu->addAction(tr("&Groups ..."));
	m_pRackMenu->addSeparator();
	m_pRackMenu->addAction(m_pExportAction);
	m_pEditMenu->addAction(m_pFindAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pCopyAction);
	m_pEditMenu->addAction(m_pSelectAllAction);
	m_pEditMenu->addSeparator();
	m_pEditMenu->addAction(m_pPropertyAction);

	//
	//
	addMenu(m_pRackMenu);
	addMenu(m_pEditMenu);

	// connect
	//
	connect(m_pRackGroupsAction, &QAction::triggered, this, &DialogRackList::rackGroups);

	//
	//
	m_rackTable.setColumnCaption(DialogRackList::metaObject()->className(), RACK_LIST_COLUMN_COUNT, RackListColumn);
	setModel(&m_rackTable);

	//
	//
	DialogList::createHeaderContexMenu(RACK_LIST_COLUMN_COUNT, RackListColumn, RackListColumnWidth);
	createContextMenu();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackList::createContextMenu()
{
	addContextAction(m_pCopyAction);
	addContextSeparator();
	addContextAction(m_pPropertyAction);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackList::updateList()
{
	m_rackTable.clear();

	std::vector<Metrology::RackParam*> rackList;

	int count = m_rackBase.count();
	for(int i = 0; i < count; i++)
	{
		Metrology::RackParam* pRack = m_rackBase.rackPtr(i);
		if (pRack == nullptr || pRack->isValid() == false)
		{
			continue;
		}

		rackList.push_back(pRack);
	}

	m_rackTable.set(rackList);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackList::rackGroups()
{
	DialogRackGroupProperty dialog(m_rackBase, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	m_rackBase = dialog.racks();
	m_rackBase.groups() = dialog.rackGroups();
	m_rackTable.setRackGroups(m_rackBase.groups());

	m_rackBase.updateParamFromGroups();

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackList::onProperties()
{
	QTableView* pView = view();
	if (pView == nullptr)
	{
		return;
	}

	int index = pView->currentIndex().row();
	if (index < 0 || index >= m_rackTable.count())
	{
		return;
	}

	Metrology::RackParam* pRack = m_rackTable.at(index);
	if (pRack == nullptr || pRack->isValid() == false)
	{
		return;
	}

	if (m_rackBase.groups().count() == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("No rack groups have been found.\nTo create a group of racks, click menu \"Racks\" - \"Groups ...\""));
		return;
	}

	DialogRackProperty dialog(*pRack, m_rackBase, this);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	int groupIndex = dialog.rack().groupIndex();
	if (groupIndex < 0 || groupIndex >= m_rackBase.groups().count())
	{
		return;
	}

	int channel = dialog.rack().channel();
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return;
	}

	// update rack
	//
	pRack->setGroupIndex(groupIndex);
	pRack->setChannel(channel);

	m_rackBase.setRack(index,* pRack);

	// update group rack
	//
	RackGroup group = m_rackBase.groups().group(groupIndex);
	if (group.isValid() == false)
	{
		return;
	}

	group.setRackID(channel, pRack->equipmentID());

	m_rackBase.groups().setGroup(groupIndex, group);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
