#include "RackList.h"

#include "ProcessData.h"
#include "ObjectProperties.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackListTable::RackListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

RackListTable::~RackListTable()
{
	QMutexLocker l(&m_rackMutex);

	m_rackList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int RackListTable::columnCount(const QModelIndex&) const
{
	return RACK_LIST_COLUMN_COUNT;
}

// -------------------------------------------------------------------------------------------------------------------

int RackListTable::rowCount(const QModelIndex&) const
{
	return rackCount();
}

// -------------------------------------------------------------------------------------------------------------------

QVariant RackListTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < RACK_LIST_COLUMN_COUNT)
		{
			result = qApp->translate("RackListDialog.h", RackListColumn[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QVariant RackListTable::data(const QModelIndex &index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	int row = index.row();
	if (row < 0 || row >= rackCount())
	{
		return QVariant();
	}

	int column = index.column();
	if (column < 0 || column > RACK_LIST_COLUMN_COUNT)
	{
		return QVariant();
	}

	Metrology::RackParam* pRack = rack(row);
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
	if (row < 0 || row >= rackCount())
	{
		return QString();
	}

	if (column < 0 || column > RACK_LIST_COLUMN_COUNT)
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

int RackListTable::rackCount() const
{
	QMutexLocker l(&m_rackMutex);

	return m_rackList.count();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam* RackListTable::rack(int index) const
{
	QMutexLocker l(&m_rackMutex);

	if (index < 0 || index >= m_rackList.count())
	{
		return nullptr;
	}

	return m_rackList[index];
}

// -------------------------------------------------------------------------------------------------------------------

void RackListTable::set(const QVector<Metrology::RackParam*>& list_add)
{
	int count = list_add.count();
	if (count == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, count - 1);

		m_rackMutex.lock();

			m_rackList = list_add;

		m_rackMutex.unlock();

	endInsertRows();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListTable::clear()
{
	int count = rackCount();
	if (count == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, count - 1);

		m_rackMutex.lock();

			m_rackList.clear();

		m_rackMutex.unlock();

	endRemoveRows();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackListDialog::RackListDialog(QWidget *parent) :
	QDialog(parent)
{
	m_rackBase = theSignalBase.racks();
	m_rackTable.setRackGroups(m_rackBase.groups());

	createInterface();
	createContextMenu();
	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

RackListDialog::~RackListDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::createInterface()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Rack.png"));
	setWindowTitle(tr("Racks"));
	resize(700, 600);
	move(QGuiApplication::primaryScreen()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pRackMenu = new QMenu(tr("&Racks"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	m_pRackGroupsAction = m_pRackMenu->addAction(tr("&Groups ..."));

	m_pRackMenu->addSeparator();

	m_pExportAction = m_pRackMenu->addAction(tr("&Export ..."));
	m_pExportAction->setIcon(QIcon(":/icons/Export.png"));
	m_pExportAction->setShortcut(Qt::CTRL + Qt::Key_E);

	m_pFindAction = m_pEditMenu->addAction(tr("&Find ..."));
	m_pFindAction->setIcon(QIcon(":/icons/Find.png"));
	m_pFindAction->setShortcut(Qt::CTRL + Qt::Key_F);

	m_pEditMenu->addSeparator();

	m_pCopyAction = m_pEditMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));
	m_pCopyAction->setShortcut(Qt::CTRL + Qt::Key_C);

	m_pSelectAllAction = m_pEditMenu->addAction(tr("Select &All"));
	m_pSelectAllAction->setIcon(QIcon(":/icons/SelectAll.png"));
	m_pSelectAllAction->setShortcut(Qt::CTRL + Qt::Key_A);

	m_pEditMenu->addSeparator();

	m_pRackPropertyAction = m_pEditMenu->addAction(tr("Propertу ..."));
	m_pRackPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pMenuBar->addMenu(m_pRackMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pRackGroupsAction, &QAction::triggered, this, &RackListDialog::rackGroups);
	connect(m_pExportAction, &QAction::triggered, this, &RackListDialog::exportRacks);

	connect(m_pFindAction, &QAction::triggered, this, &RackListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &RackListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &RackListDialog::selectAll);
	connect(m_pRackPropertyAction, &QAction::triggered, this, &RackListDialog::rackProperty);

	m_pView = new QTableView(this);
	m_pView->setModel(&m_rackTable);
	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < RACK_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, RackListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pView->setWordWrap(false);

	connect(m_pView, &QTableView::doubleClicked , this, &RackListDialog::onListDoubleClicked);


	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &RackListDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &RackListDialog::reject);


	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addWidget(m_pView);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::createContextMenu()
{
	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);

	m_pContextMenu->addAction(m_pCopyAction);
	m_pContextMenu->addSeparator();
	m_pContextMenu->addAction(m_pRackPropertyAction);

	// init context menu
	//
	m_pView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pView, &QTableWidget::customContextMenuRequested, this, &RackListDialog::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::updateList()
{
	m_rackTable.clear();

	QVector<Metrology::RackParam*> rackList;

	int count = m_rackBase.count();
	for(int i = 0; i < count; i++)
	{
		Metrology::RackParam* pRack = m_rackBase.rackPtr(i);
		if (pRack == nullptr || pRack->isValid() == false)
		{
			continue;
		}

		rackList.append(pRack);
	}

	m_rackTable.set(rackList);
}

// -------------------------------------------------------------------------------------------------------------------

bool RackListDialog::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
		{
			rackProperty();
		}
	}

	return QObject::eventFilter(object, event);
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::rackGroups()
{
	RackGroupPropertyDialog dialog(m_rackBase);
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

void RackListDialog::exportRacks()
{
	ExportData* dialog = new ExportData(m_pView, false, "Racks");
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::find()
{
	FindData* dialog = new FindData(m_pView);
	dialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::copy()
{
	CopyData copyData(m_pView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::rackProperty()
{
	int index = m_pView->currentIndex().row();
	if (index < 0 || index >= m_rackTable.rackCount())
	{
		return;
	}

	Metrology::RackParam* pRack = m_rackTable.rack(index);
	if (pRack == nullptr || pRack->isValid() == false)
	{
		return;
	}

	if (m_rackBase.groups().count() == 0)
	{
		QMessageBox::information(this, windowTitle(), tr("No rack groups have been found.\nTo create a group of racks, click menu \"Racks\" - \"Groups ...\""));
	}

	RackPropertyDialog dialog(*pRack, m_rackBase);
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

	m_rackBase.setRack(index, *pRack);

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

void RackListDialog::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::onListDoubleClicked(const QModelIndex&)
{
	rackProperty();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
