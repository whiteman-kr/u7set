#include "Stable.h"
#include "EquipmentTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"

//
//
// EquipmentModel
//
//


EquipmentModel::EquipmentModel(std::shared_ptr<Hardware::DeviceRoot> root, QObject* parent) :
	QAbstractItemModel(parent),
	m_root(root)
{
	assert(root.get() != nullptr);

//    std::shared_ptr<DeviceObject> c1 = std::make_shared<DeviceSystem>();
//    std::shared_ptr<DeviceObject> c2 = std::make_shared<DeviceSystem>();
//    std::shared_ptr<DeviceObject> c3 = std::make_shared<DeviceSystem>();

//	c1->setCaption("c1");
//	c2->setCaption("c2");
//	c3->setCaption("c3");

//	c2->addChild(c3);

	/*for (int i = 0; i < 32; i++)
	{
        auto d1 = std::make_shared<DeviceCase>();
		d1->setCaption(QString("c1 item %1").arg(i));
		c1->addChild(d1);

		for (int j = 0; j < 1024; j++)
		{
            auto d11 = std::make_shared<DeviceObject>();
			d11->setCaption(QString("cdwd1 item %1").arg(j));
			d1->addChild(d11);

            for (int k = 0; k < 32; k++)
			{
				auto d111 = std::make_shared<DeviceBase>();
				d111->setCaption(QString("ll1 item %1").arg(k));
				d11->addChild(d111);
            }
		}

        auto d2 = std::make_shared<DeviceObject>();
		d2->setCaption(QString("c2 item %1").arg(i));
		c2->addChild(d2);

        auto d3 = std::make_shared<DeviceObject>();
		d3->setCaption(QString("c3 item %1").arg(i));
		c3->addChild(d3);
	}*/

//	m_root.addChild(c1);
//	m_root.addChild(c2);
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
	const Hardware::DeviceObject* parent = nullptr;

	if (parentIndex.isValid() == false)
	{
		parent = m_root.get();
	}
	else
	{
		parent = static_cast<const Hardware::DeviceObject*>(parentIndex.internalPointer());
	}

	assert(parent != nullptr);
	return parent->childrenCount() > 0;
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
	/*QModelIndexList indexes = selectedIndexes();

	if (indexes.isEmpty() == false)
	{
		// Nothing is selected, add new system to the root
		//
		return;
	}
	else
	{
		//
	}*/

	std::shared_ptr<Hardware::DeviceSystem> system = std::make_shared<Hardware::DeviceSystem>();

	system->setStrId("STRID");
	system->setCaption(tr("New System"));

	bool result = dbController()->addSystem(system.get(), this);

	//if (result == true)
	{
		// Add system to the model m_equipmentModel
		//
		//m_root->addChild(system);

		// !!!!!!!!!!!!!!!!!!! emmit here message about model changing............
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
	m_root = std::make_shared<Hardware::DeviceRoot>();

	// Equipment View
	//
	m_equipmentView = new EquipmentView(dbcontroller);

	m_equipmentModel = new EquipmentModel(m_root, this);
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

	/*auto s1 = std::make_shared<DeviceSystem>();
	s1->setCaption("SDS I");
	s1->setStrId("1SDS1");

	auto s2 = std::make_shared<DeviceSystem>();
	s2->setCaption("SDS II");
	s2->setStrId("1SDS2");

	auto r1 = std::make_shared<DeviceRack>();
	r1->setCaption("1SHFS1");
	r1->setStrId("HS017");

	auto r2 = std::make_shared<DeviceRack>();
	r2->setCaption("2SHFS1");
	r2->setStrId("HS018");

	auto r3 = std::make_shared<DeviceRack>();
	r3->setCaption("3SHFS1");
	r3->setStrId("HS019");

	s1->addChild(r1);
	s1->addChild(r2);
	s1->addChild(r3);

	m_root->addChild(s1);
	m_root->addChild(s2);*/

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
