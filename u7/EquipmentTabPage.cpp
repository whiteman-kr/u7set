#include "Stable.h"
#include "EquipmentTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"

//
//
// EquipmentModel
//
//


EquipmentModel::EquipmentModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent) :
	QAbstractItemModel(parent),
	m_dbController(dbcontroller),
	m_parentWidget(parentWidget),
	m_root(std::make_shared<Hardware::DeviceRoot>())
{
	assert(dbcontroller);
	assert(m_root.get() != nullptr);

	connect(dbcontroller, &DbController::projectOpened, this, &EquipmentModel::projectOpened);
	connect(dbcontroller, &DbController::projectClosed, this, &EquipmentModel::projectClosed);
}

EquipmentModel::~EquipmentModel()
{
}

QModelIndex EquipmentModel::index(int row, int column, const QModelIndex& parentIndex) const
{
	if (hasIndex(row, column, parentIndex) == false)
	{
		return QModelIndex();
	}

	// Is it request for the root's items?
	//
	if (parentIndex.isValid() == false)
	{
		return createIndex(row, column, const_cast<Hardware::DeviceObject*>(m_root->child(row)));
	}

	Hardware::DeviceObject* parent = static_cast<Hardware::DeviceObject*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(parent);
		return QModelIndex();
	}

	return createIndex(row, column, parent->child(row));
}

QModelIndex EquipmentModel::parent(const QModelIndex& childIndex) const
{
	if (childIndex.isValid() == false)
	{
		return QModelIndex();
	}

	Hardware::DeviceObject* child = static_cast<Hardware::DeviceObject*>(childIndex.internalPointer());
	if (child == nullptr)
	{
		assert(child != nullptr);
		return QModelIndex();
	}

	if (child->parent() == nullptr || child->parent() == m_root.get())
	{
		return QModelIndex();
	}

	// Determine the position of the parent in the parent's parent
	//
	if (child->parent()->parent() == nullptr)
	{
		int row = m_root->childIndex(child);
		if (row == -1)
		{
			assert(row != -1);
			return QModelIndex();
		}

		return createIndex(row, 0, child->parent());
	}
	else
	{
		int row = child->parent()->parent()->childIndex(child->parent());
		if (row == -1)
		{
			assert(row != -1);
			return QModelIndex();
		}

		return createIndex(row, 0, child->parent());
	}
}

int EquipmentModel::rowCount(const QModelIndex& parentIndex) const
{
	if (parentIndex.isValid() == false)
	{
		return m_root->childrenCount();
	}

	const Hardware::DeviceObject* parent = static_cast<const Hardware::DeviceObject*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(false);
		return 0;
	}

	return parent->childrenCount();
}

int EquipmentModel::columnCount(const QModelIndex& /*parentIndex*/) const
{
	return ColumnCount;		// Always the same
}

QVariant EquipmentModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	Hardware::DeviceObject* device = static_cast<Hardware::DeviceObject*>(index.internalPointer());
	assert(device != nullptr);

	switch (role)
	{
	case Qt::DisplayRole:
		{
			QVariant v;

			switch (index.column())
			{
			case ObjectNameColumn:
				v.setValue<QString>(device->caption());
				break;

			case ObjectStrIdColumn:
				break;

			case ObjectStateColumn:
				break;

			case ObjectUserColumn:
				break;

			default:
				assert(false);
			}

			return v;
		}
		break;
	}

	return QVariant();
}

QVariant EquipmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case ObjectNameColumn:
				return QObject::tr("Object");

			case ObjectStrIdColumn:
				return QObject::tr("StrId");

			case ObjectStateColumn:
				return QObject::tr("State");

			case ObjectUserColumn:
				return QObject::tr("User");

			default:
				assert(false);
			}
		}
	}

	return QVariant();
}

bool EquipmentModel::hasChildren(const QModelIndex& parentIndex ) const
{
	const Hardware::DeviceObject* parent = deviceObject(parentIndex);

	return parent->childrenCount() > 0;
}

bool EquipmentModel::canFetchMore(const QModelIndex& parent) const
{
	if (dbController()->isProjectOpened() == false)
	{
		return false;
	}

	if (parent == QModelIndex())
	{
		return true;
	}

	return true;
}

void EquipmentModel::fetchMore(const QModelIndex& parentIndex)
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	if (parentIndex == QModelIndex())
	{
		// Get top level files
		//
		std::vector<DbFileInfo> files;

		bool ok = dbController()->getFileList(&files, dbController()->hcFileId(), m_parentWidget);

		if (ok == true)
		{
			for (auto& fi : files)
			{
				std::shared_ptr<DbFile> file;

				if (fi.state() == VcsState::CheckedOut &&
					fi.user() == dbController()->currentUser())
				{
					dbController()->getWorkcopy(fi, &file, m_parentWidget);
				}
				else
				{
					// Get lateset copy
					//
					assert(false);
				}

				if (file == false)
				{
					continue;
				}

				Hardware::DeviceObject* object = Hardware::DeviceObject::Create(file->data());
				assert(object);

				if (object == nullptr)
				{
					continue;
				}

				std::shared_ptr<Hardware::DeviceObject> sp(object);
				m_root->addChild(sp);
			}
		}
	}
	else
	{
		// Get file id of parent and get its children
		//
		assert(false);
	}

	// TODO:: sort files in parent DeviceObject !!!!!!!!!!!
	//

	return;
}

bool EquipmentModel::insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex)
{
	// TODO: This function should take into consideration sort property!!!
	//

	Hardware::DeviceObject* parent = deviceObject(parentIndex);

	beginInsertRows(parentIndex, parent->childrenCount(), parent->childrenCount());
	parent->addChild(object);
	endInsertRows();

	return true;
}

Hardware::DeviceObject* EquipmentModel::deviceObject(QModelIndex& index)
{
	Hardware::DeviceObject* object = nullptr;

	if (index.isValid() == false)
	{
		object = m_root.get();
	}
	else
	{
		object = static_cast<Hardware::DeviceObject*>(index.internalPointer());
	}

	assert(object != nullptr);
	return object;
}

const Hardware::DeviceObject* EquipmentModel::deviceObject(const QModelIndex& index) const
{
	const Hardware::DeviceObject* object = nullptr;

	if (index.isValid() == false)
	{
		object = m_root.get();
	}
	else
	{
		object = static_cast<const Hardware::DeviceObject*>(index.internalPointer());
	}

	assert(object != nullptr);
	return object;
}

void EquipmentModel::projectOpened()
{
	// read all childer for HC file
	//
	beginResetModel();
	m_root = std::make_shared<Hardware::DeviceRoot>();
	endResetModel();
	return;
}

void EquipmentModel::projectClosed()
{
	// Release all children
	//
	beginResetModel();
	m_root = std::make_shared<Hardware::DeviceRoot>();
	endResetModel();
	return;
}

DbController* EquipmentModel::dbController()
{
	return m_dbController;
}

DbController* EquipmentModel::dbController() const
{
	return m_dbController;
}

//
//
// EquipmentView
//
//
EquipmentView::EquipmentView(DbController* dbcontroller) :
	m_dbController(dbcontroller)
{
}

EquipmentView::~EquipmentView()
{
}

DbController* EquipmentView::dbController()
{
	return m_dbController;
}

void EquipmentView::addSystem()
{
	// Add new system to the root
	//

	std::shared_ptr<Hardware::DeviceObject> system = std::make_shared<Hardware::DeviceSystem>();

	system->setStrId("SYSTEMSTRID");
	system->setCaption(tr("New System"));

	bool result = dbController()->addDeviceObject(system.get(), dbController()->hcFileId(), this);

	if (result == true)
	{
		equipmentModel()->insertDeviceObject(system, QModelIndex());
	}

	return;
}

void EquipmentView::addCase()
{
	assert(false);
}

void EquipmentView::addSubblock()
{
	assert(false);
}

void EquipmentView::addBlock()
{
	assert(false);
}

EquipmentModel* EquipmentView::equipmentModel()
{
	EquipmentModel* result = dynamic_cast<EquipmentModel*>(model());
	assert(result);
	return result;
}


//
//
// EquipmentTabPage
//
//
EquipmentTabPage::EquipmentTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	//
	// Controls
	//

	// Equipment View
	//
	m_equipmentView = new EquipmentView(dbcontroller);
	m_equipmentModel = new EquipmentModel(dbcontroller, this, this);
	m_equipmentView->setModel(m_equipmentModel);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	m_equipmentView->addAction(m_addSystemAction);
	m_equipmentView->addAction(m_addCaseAction);
	m_equipmentView->addAction(m_addSubblockAction);
	m_equipmentView->addAction(m_addBlockAction);

	// Property View
	//
	m_propertyView = new QTextEdit();

	// Splitter
	//
	m_splitter = new QSplitter();

	m_splitter->addWidget(m_equipmentView);
	m_splitter->addWidget(m_propertyView);

	m_splitter->setStretchFactor(0, 2);
	m_splitter->setStretchFactor(1, 1);

	m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	//
	// Layouts
	//

	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &EquipmentTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &EquipmentTabPage::projectClosed);


//	connect(m_filesView, &ConfigurationFileView::openFileSignal, this, &ConfigurationsTabPage::openFiles);
//	connect(m_filesView, &ConfigurationFileView::viewFileSignal, this, &ConfigurationsTabPage::viewFiles);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

EquipmentTabPage::~EquipmentTabPage()
{
	theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
	theSettings.writeUserScope();
}

void EquipmentTabPage::CreateActions()
{
	m_addSystemAction = new QAction(tr("Add System"), this);
	m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
	//m_addSystemAction->setEnabled(false);
	connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

	m_addCaseAction = new QAction(tr("Add Case"), this);
	m_addCaseAction->setStatusTip(tr("Add case to the configuration..."));
	//m_addCaseAction->setEnabled(false);
	connect(m_addCaseAction, &QAction::triggered, m_equipmentView, &EquipmentView::addCase);

	m_addSubblockAction = new QAction(tr("Add Subblock"), this);
	m_addSubblockAction->setStatusTip(tr("Add subblock to the configuration..."));
	//m_addSubblockAction->setEnabled(false);
	connect(m_addSubblockAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSubblock);

	m_addBlockAction = new QAction(tr("Add Block"), this);
	m_addBlockAction->setStatusTip(tr("Add block to the configuration..."));
	//m_addBlockAction->setEnabled(false);
	connect(m_addBlockAction, &QAction::triggered, m_equipmentView, &EquipmentView::addBlock);

	return;
}

void EquipmentTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void EquipmentTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void EquipmentTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}
