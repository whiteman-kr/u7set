#include "RackList.h"

#include <QClipboard>

#include "MainWindow.h"
#include "Options.h"
#include "ExportData.h"
#include "FindData.h"
#include "ObjectProperties.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackListTable::RackListTable(QObject*)
{
}

// -------------------------------------------------------------------------------------------------------------------

RackListTable::~RackListTable()
{
	m_rackMutex.lock();

		m_rackList.clear();

	m_rackMutex.unlock();
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
			result = RackListColumn[section];
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

	if (role == Qt::FontRole)
	{
		return theOptions.measureView().font();
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
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.size();

	m_rackMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam* RackListTable::rack(int index) const
{
	Metrology::RackParam* pRack = nullptr;

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			 pRack = m_rackList[index];
		}

	m_rackMutex.unlock();

	return pRack;
}

// -------------------------------------------------------------------------------------------------------------------

void RackListTable::set(const QList<Metrology::RackParam*> list_add)
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

			for(int i = count - 1; i >= 0; i--)
			{
				if (m_rackList[i] != nullptr)
				{
					delete m_rackList[i];
				}
			}

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
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (parent);
	if (pMainWindow != nullptr && pMainWindow->m_pConfigSocket != nullptr)
	{
		connect(pMainWindow->m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &RackListDialog::updateList, Qt::QueuedConnection);
	}

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
	setWindowIcon(QIcon(":/icons/Signals.png"));
	setWindowTitle(tr("Racks"));
	resize(700, 600);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());
	installEventFilter(this);

	m_pMenuBar = new QMenuBar(this);
	m_pRackMenu = new QMenu(tr("&Racks"), this);
	m_pEditMenu = new QMenu(tr("&Edit"), this);

	m_pRackGroupsAction = m_pRackMenu->addAction(tr("&Groups ..."));

	m_pRackMenu->addSeparator();

	m_pPrintAction = m_pRackMenu->addAction(tr("&Print ..."));
	m_pPrintAction->setIcon(QIcon(":/icons/Print.png"));
	m_pPrintAction->setShortcut(Qt::CTRL + Qt::Key_P);

	m_pRackMenu->addSeparator();

	m_pImportAction = m_pRackMenu->addAction(tr("&Import ..."));
	m_pImportAction->setIcon(QIcon(":/icons/Import.png"));
	m_pImportAction->setShortcut(Qt::CTRL + Qt::Key_I);

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

	m_pRackPropertyAction = m_pEditMenu->addAction(tr("Properties ..."));
	m_pRackPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	m_pMenuBar->addMenu(m_pRackMenu);
	m_pMenuBar->addMenu(m_pEditMenu);

	connect(m_pRackGroupsAction, &QAction::triggered, this, &RackListDialog::rackGroups);
	connect(m_pPrintAction, &QAction::triggered, this, &RackListDialog::printRack);
	connect(m_pImportAction, &QAction::triggered, this, &RackListDialog::importRack);
	connect(m_pExportAction, &QAction::triggered, this, &RackListDialog::exportRack);

	connect(m_pFindAction, &QAction::triggered, this, &RackListDialog::find);
	connect(m_pCopyAction, &QAction::triggered, this, &RackListDialog::copy);
	connect(m_pSelectAllAction, &QAction::triggered, this, &RackListDialog::selectAll);
	connect(m_pRackPropertyAction, &QAction::triggered, this, &RackListDialog::rackProperty);

	m_pView = new QTableView(this);
	m_pView->setModel(&m_rackTable);
	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	for(int column = 0; column < RACK_LIST_COLUMN_COUNT; column++)
	{
		m_pView->setColumnWidth(column, RackListColumnWidth[column]);
	}

	m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);

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

	QList<Metrology::RackParam*> rackList;

	int count = m_rackBase.count();
	for(int i = 0; i < count; i++)
	{
		Metrology::RackParam rack = m_rackBase.rack(i);
		if (rack.isValid() == false)
		{
			continue;
		}

		rackList.append(new Metrology::RackParam(rack));
	}

	m_rackTable.set(rackList);
}

// -------------------------------------------------------------------------------------------------------------------

bool RackListDialog::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent *>(event);

		if (keyEvent->key() == Qt::Key_Return)
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

	m_rackBase.groups() = dialog.rackGroups();
	m_rackTable.setRackGroups(dialog.rackGroups());

	m_rackBase.updateParamFromGroups();

	updateList();
}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::printRack()
{

}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::importRack()
{

}

// -------------------------------------------------------------------------------------------------------------------

void RackListDialog::exportRack()
{
	ExportData* dialog = new ExportData(m_pView, tr("Racks"));
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
	QString textClipboard;

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString() + "\t");
		}

		textClipboard.replace(textClipboard.length() - 1, 1, "\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);
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
		QMessageBox::information(this, windowTitle(), tr("No groups have been found.\n"
														 "To create a group of racks, click menu \"Racks\" - \"Groups ...\""));
	}

	RackPropertyDialog dialog(*pRack, m_rackBase);
	if (dialog.exec() != QDialog::Accepted)
	{
		return;
	}

	pRack->setGroupIndex(dialog.rack().groupIndex());
	pRack->setChannel(dialog.rack().channel());

	m_rackBase.setRack(index, *pRack);
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
	theSignalBase.racks() = m_rackBase;

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
