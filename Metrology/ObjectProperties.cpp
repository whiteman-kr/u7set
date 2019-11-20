#include "ObjectProperties.h"

#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool ProjectPropertyDialog::m_showGroupHeader[PROJECT_PROPERTY_GROUP_COUNT] =
{
	true,	//	PROJECT_PROPERTY_GROUP_INFO
	true,	//	PROJECT_PROPERTY_GROUP_DEVELOP
	true,	//	PROJECT_PROPERTY_GROUP_VERSION
};

// -------------------------------------------------------------------------------------------------------------------

ProjectPropertyDialog::ProjectPropertyDialog(const ProjectInfo& param, QWidget *parent) :
	QDialog(parent)
{
	m_info = param;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

ProjectPropertyDialog::~ProjectPropertyDialog()
{
	if (m_pManager != nullptr)
	{
		delete m_pManager;
		m_pManager = nullptr;
	}

	if (m_pFactory != nullptr)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	if (m_pEditor != nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ProjectPropertyDialog::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Properties"));
	setMinimumSize(600, 300);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());


	setWindowTitle(tr("Project - %1").arg(m_info.projectName()));

	QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
	QStringList electricUnitList;
	for(int u = 0; u < meu.keyCount(); u++)
	{
		electricUnitList.append(meu.key(u));
	}


	QVBoxLayout *mainLayout = new QVBoxLayout;

	// create property list
	//

	QtVariantProperty *item = nullptr;

	m_pManager = new QtVariantPropertyManager;
	m_pFactory = new QtVariantEditorFactory;
	m_pEditor = new QtTreePropertyBrowser;

	// create property groups
	//

		// info group

		QtProperty *infoGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Project information"));

			item = m_pManager->addProperty(QVariant::String, tr("Project name"));
			item->setValue(m_info.projectName());
			item->setAttribute(QLatin1String("readOnly"), true);
			infoGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Project ID"));
			item->setValue(m_info.id());
			item->setAttribute(QLatin1String("readOnly"), true);
			infoGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Build"));
			item->setValue(m_info.release());
			item->setAttribute(QLatin1String("readOnly"), true);
			infoGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Date"));
			item->setValue(m_info.date());
			item->setAttribute(QLatin1String("readOnly"), true);
			infoGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// host group

		QtProperty *hostGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Host"));


			item = m_pManager->addProperty(QVariant::String, tr("User"));
			item->setValue(m_info.user());
			item->setAttribute(QLatin1String("readOnly"), true);
			hostGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Workstation"));
			item->setValue(m_info.workstation());
			item->setAttribute(QLatin1String("readOnly"), true);
			hostGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// position group

		QtProperty *versionGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("File version"));

			item = m_pManager->addProperty(QVariant::Int, tr("Database Version"));
			item->setValue(m_info.dbVersion());
			item->setAttribute(QLatin1String("readOnly"), true);
			versionGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Config File Version"));
			item->setValue(m_info.cfgFileVersion());
			item->setAttribute(QLatin1String("readOnly"), true);
			versionGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

	// show or hide property groups
	//

	m_browserItemList[PROJECT_PROPERTY_GROUP_INFO] = m_pEditor->addProperty(infoGroup);
	m_browserItemList[PROJECT_PROPERTY_GROUP_HOST] = m_pEditor->addProperty(hostGroup);
	m_browserItemList[PROJECT_PROPERTY_GROUP_VERSION] = m_pEditor->addProperty(versionGroup);


	for(int g = 0; g < PROJECT_PROPERTY_GROUP_COUNT; g++)
	{
		if (m_browserItemList[g] == nullptr)
		{
			continue;
		}

		m_pEditor->setExpanded(m_browserItemList[g], m_showGroupHeader[g]);
	}

	//
	//
	m_pEditor->setPropertiesWithoutValueMarked(true);
	m_pEditor->setRootIsDecorated(false);

	// add layouts
	//
	mainLayout->addWidget(m_pEditor);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackPropertyDialog::RackPropertyDialog(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget *parent) :
	QDialog(parent)
{
	if (rack.isValid() == false)
	{
		assert(false);
		return;
	}

	m_rack = rack;
	m_rackBase = rackBase;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

RackPropertyDialog::~RackPropertyDialog()
{
	if (m_pManager != nullptr)
	{
		delete m_pManager;
		m_pManager = nullptr;
	}

	if (m_pFactory != nullptr)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	if (m_pEditor != nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void RackPropertyDialog::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Properties"));
	setMinimumSize(400, 180);
	resize(400, 180);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	if (m_rack.isValid() == false)
	{
		assert(m_rack.isValid() == false);
		return;
	}

	setWindowTitle(tr("Properties - %1").arg(m_rack.caption()));

	QVBoxLayout *mainLayout = new QVBoxLayout;

	// create property list
	//

	QtVariantProperty *item = nullptr;

	m_pManager = new QtVariantPropertyManager;
	m_pFactory = new QtVariantEditorFactory;
	m_pEditor = new QtTreePropertyBrowser;

	//
	//
	QtProperty *rackGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Properties of the rack"));

		item = m_pManager->addProperty(QVariant::String, tr("Caption"));
		item->setValue(m_rack.caption());
		item->setAttribute(QLatin1String("readOnly"), true);
		m_propertyMap.insert(item, RACK_PROPERTY_ITEM_CAPTION);
		rackGroup->addSubProperty(item);

		item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
		item->setValue(m_rack.equipmentID());
		item->setAttribute(QLatin1String("readOnly"), true);
		m_propertyMap.insert(item, RACK_PROPERTY_ITEM_ID);
		rackGroup->addSubProperty(item);

		item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Group"));
		QStringList groupList;
		groupList.append(QString());
		int groupCount = m_rackBase.groups().count();
		for(int g = 0; g < groupCount; g++)
		{
			RackGroup group = m_rackBase.groups().group(g);
			if (group.isValid() == false)
			{
				continue;
			}

			groupList.append(group.caption());
		}
		item->setAttribute(QLatin1String("enumNames"), groupList);
		item->setValue(m_rack.groupIndex() + 1);
		m_propertyMap.insert(item, RACK_PROPERTY_ITEM_GROUP);
		rackGroup->addSubProperty(item);

		item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Channel"));
		QStringList channelList;
		channelList.append(QString());
		for(int ch = 0; ch < Metrology::ChannelCount; ch++)
		{
			channelList.append(QString::number(ch + 1));
		}
		item->setAttribute(QLatin1String("enumNames"), channelList);
		item->setValue(m_rack.channel() + 1);
		m_propertyMap.insert(item, RACK_PROPERTY_ITEM_CHANNEL);
		rackGroup->addSubProperty(item);

	m_pEditor->setFactoryForManager(m_pManager, m_pFactory);
	m_pEditor->addProperty(rackGroup);

	//
	//
	m_pEditor->setPropertiesWithoutValueMarked(true);
	m_pEditor->setRootIsDecorated(false);

	connect(m_pManager, &QtVariantPropertyManager::valueChanged, this, &RackPropertyDialog::onPropertyValueChanged);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &RackPropertyDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &RackPropertyDialog::reject);

	// add layouts
	//
	mainLayout->addWidget(m_pEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void RackPropertyDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyMap.contains(property) == false)
	{
		return;
	}

	int propertyIndex = m_propertyMap[property];
	if (propertyIndex < 0 || propertyIndex >= RACK_PROPERTY_ITEM_COUNT)
	{
		return;
	}

	switch(propertyIndex)
	{
		case RACK_PROPERTY_ITEM_GROUP:

			m_rack.setGroupIndex(value.toInt()-1);

			if (m_rack.groupIndex() == -1)
			{
				m_rack.setChannel(-1);
			}

			break;

		case RACK_PROPERTY_ITEM_CHANNEL:
			m_rack.setChannel(value.toInt()-1);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool RackPropertyDialog::foundDuplicateGroups()
{
	bool result = false;

	int count = m_rackBase.count();
	for(int i = 0; i < count; i ++)
	{
		Metrology::RackParam rack = m_rackBase.rack(i);
		if (rack.isValid() == false)
		{
			continue;
		}

		if (rack.groupIndex() == -1 || rack.channel() == -1)
		{
			continue;
		}

		if (rack.hash() == m_rack.hash())
		{
			continue;
		}

		if (rack.groupIndex() == m_rack.groupIndex() && rack.channel() == m_rack.channel())
		{
			QString alert = tr(	"Another rack (%1) already has the same group or channel.\n"
								"Please choose a different group or channel.").arg(rack.caption());
			QMessageBox::information(this, tr("Same racks"), alert);

			result = true;

			break;
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void RackPropertyDialog::onOk()
{
	if (m_rack.isValid() == false)
	{
		assert(false);
		return;
	}

	if (m_rack.groupIndex() != -1 && m_rack.channel() == -1)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, fill the field \"Channel\""));
		return;
	}

	if (foundDuplicateGroups() == true)
	{
		return;
	}

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackGroupPropertyDialog::RackGroupPropertyDialog(const RackBase& rackBase, QWidget *parent) :
	QDialog(parent)
{
	m_rackBase = rackBase;
	m_groupBase = m_rackBase.groups();

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

RackGroupPropertyDialog::~RackGroupPropertyDialog()
{
	if (m_pManager != nullptr)
	{
		delete m_pManager;
		m_pManager = nullptr;
	}

	if (m_pFactory != nullptr)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	if (m_pEditor != nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Properties - rack groups"));
	setMinimumSize(600, 300);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	// create menu
	//
	m_pMenuBar = new QMenuBar(this);
	m_pGroupMenu = new QMenu(tr("&Group"), this);

	m_pAppendGroupAction = m_pGroupMenu->addAction(tr("&Append"));
	m_pAppendGroupAction->setIcon(QIcon(":/icons/Add.png"));

	m_pRemoveGroupAction = m_pGroupMenu->addAction(tr("&Remove"));
	m_pRemoveGroupAction->setIcon(QIcon(":/icons/Remove.png"));

	m_pMenuBar->addMenu(m_pGroupMenu);

	connect(m_pAppendGroupAction, &QAction::triggered, this, &RackGroupPropertyDialog::appendGroup);
	connect(m_pRemoveGroupAction, &QAction::triggered, this, &RackGroupPropertyDialog::removeGroup);

	// create rack group view
	//
	m_pGroupView = new QTableWidget(this);

	m_pGroupView->setColumnCount(1);
	m_pGroupView->setHorizontalHeaderLabels(QStringList(""));

	m_pGroupView->horizontalHeader()->hide();
	m_pGroupView->verticalHeader()->hide();

	QSize cellSize = QFontMetrics(theOptions.measureView().font()).size(Qt::TextSingleLine,"A");
	m_pGroupView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pGroupView->installEventFilter(this);

	// init context menu
	//
	m_pGroupView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pGroupView, &QTableWidget::customContextMenuRequested, this, &RackGroupPropertyDialog::onContextMenu);

	connect(m_pGroupView, &QTableWidget::cellChanged, this, &RackGroupPropertyDialog::captionGroupChanged);
	connect(m_pGroupView, &QTableWidget::itemSelectionChanged, this, &RackGroupPropertyDialog::groupSelected);

	// create property list
	//
	QtVariantProperty *item = nullptr;

	m_pManager = new QtVariantPropertyManager;
	m_pFactory = new QtVariantEditorFactory;
	m_pEditor = new QtTreePropertyBrowser;

	//
	//
	QtProperty *racks = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Racks"));

		for(int channel = 0; channel < Metrology::ChannelCount; channel++)
		{
			item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Channel %1").arg(channel + 1));

			int rackCount = m_rackBase.count();

			QStringList rackList;
			rackList.append(QString());

			for(int i = 0; i < rackCount; i++)
			{
				Metrology::RackParam rack = m_rackBase.rack(i);
				if (rack.isValid() == false)
				{
					continue;
				}

				rackList.append(rack.caption());
			}

			item->setAttribute(QLatin1String("enumNames"), rackList);
			item->setValue(0);
			m_propertyMap.insert(item, channel);
			racks->addSubProperty(item);

			racks->addSubProperty(item);
		}

	m_pEditor->setFactoryForManager(m_pManager, m_pFactory);
	m_pEditor->addProperty(racks);

	//
	//
	m_pEditor->setPropertiesWithoutValueMarked(true);
	m_pEditor->setRootIsDecorated(false);

	connect(m_pManager, &QtVariantPropertyManager::valueChanged, this, &RackGroupPropertyDialog::onPropertyValueChanged);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &RackGroupPropertyDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &RackGroupPropertyDialog::reject);

	// add layouts
	//
	QHBoxLayout *listLayout = new QHBoxLayout;

	listLayout->addWidget(m_pGroupView);
	listLayout->addWidget(m_pEditor);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addLayout(listLayout);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	updateGroupList();
	updateRackList();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::updateGroupList()
{
	m_pGroupView->blockSignals(true);

	// clear list
	//
	int rowCount = m_pGroupView->rowCount();
	for(int row = 0; row < rowCount; row++)
	{
		QTableWidgetItem *item = m_pGroupView->item(row, RACK_GROUP_COLUMN_CAPTION);
		if (item != nullptr)
		{
			delete item;
		}
	}

	// append new group caption
	//
	int count = m_groupBase.count();

	m_pGroupView->setRowCount(count);

	for(int i = 0; i < count; i++)
	{
		RackGroup group = m_groupBase.group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		QTableWidgetItem* item = new QTableWidgetItem(group.caption());
		item->setTextAlignment(Qt::AlignHCenter);
		m_pGroupView->setItem(i, RACK_GROUP_COLUMN_CAPTION, item);
	}

	m_pGroupView->blockSignals(false);
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::updateRackList()
{
	int index = m_pGroupView->currentIndex().row();
	if (index < 0 || index > m_groupBase.count())
	{
		return;
	}

	RackGroup group = m_groupBase.group(index);
	if (group.isValid() == false)
	{
		return;
	}

	QtVariantProperty *property = nullptr;

	for(int channel = 0; channel < Metrology::ChannelCount; channel++)
	{
		property = dynamic_cast<QtVariantProperty*>(m_propertyMap.key(channel));
		if (property == nullptr)
		{
			continue;
		}

		QString rackID = group.rackID(channel);
		if (rackID.isEmpty() == true)
		{
			property->setValue(0);
			continue;
		}

		Metrology::RackParam rack = m_rackBase.rack(rackID);
		if (rack.isValid() == false)
		{
			property->setValue(0);
			continue;
		}

		property->setValue(rack.index()+1);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::appendGroup()
{
	QString caption = tr("Rack group %1").arg(m_groupBase.count() + 1);

	RackGroup group;

	group.setIndex(m_groupBase.count());
	group.setCaption(caption);

	m_groupBase.append(group);

	updateGroupList();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::removeGroup()
{
	int index = m_pGroupView->currentIndex().row();
	if (index < 0 || index > m_groupBase.count())
	{
		return;
	}

	RackGroup group = m_groupBase.group(index);
	if (group.isValid() == false)
	{
		return;
	}

	if (QMessageBox::question(this, windowTitle(), tr("Do you want delete group \"%1\"?").arg(group.caption())) == QMessageBox::No)
	{
		return;
	}

	m_groupBase.remove(index);

	updateGroupList();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)
	{
		return;
	}

	int index = m_pGroupView->currentIndex().row();
	if (index < 0 || index > m_groupBase.count())
	{
		return;
	}

	RackGroup group = m_groupBase.group(index);
	if (group.isValid() == false)
	{
		return;
	}

	if (m_propertyMap.contains(property) == false)
	{
		return;
	}

	int channel = m_propertyMap[property];
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return;
	}

	int rackIndex = value.toInt() - 1;
	if (rackIndex < 0 || rackIndex >= m_rackBase.count())
	{
		if (group.rackID(channel).isEmpty() == false)
		{
			group.setRackID(channel, QString());
			m_groupBase.setGroup(index, group);
		}

		return;
	}

	Metrology::RackParam rack = m_rackBase.rack(rackIndex);
	if (rack.isValid() == false)
	{
		return;
	}

	if (group.rackID(channel) == rack.equipmentID())
	{
		return;
	}

	group.setRackID(channel, rack.equipmentID());
	m_groupBase.setGroup(index, group);
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupPropertyDialog::event(QEvent* e)
{
	if (e->type() == QEvent::Resize)
	{
		m_pGroupView->setColumnWidth(RACK_GROUP_COLUMN_CAPTION, m_pGroupView->width() - 20);
	}

	return QDialog::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::onContextMenu(QPoint)
{
	m_pGroupMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::captionGroupChanged(int row, int column)
{
	Q_UNUSED(column)

	int index = row;
	if (index < 0 || index >= m_groupBase.count())
	{
		return;
	}

	RackGroup group = m_groupBase.group(index);
	if (group.isValid() == false)
	{
		return;
	}

	QString caption = m_pGroupView->item(row, RACK_GROUP_COLUMN_CAPTION)->text();
	if (caption.isEmpty() == true)
	{
		return;
	}


	int groupCount = m_groupBase.count();
	for(int i = 0; i < groupCount; i++)
	{
		if (m_groupBase.group(i).caption() == caption)
		{
			QMessageBox::information(this, tr("Group caption"), tr("Group caption \"%1\" already exists!").arg(caption));
			return;
		}
	}

	group.setCaption(caption);
	m_groupBase.setGroup(index, group);

	updateGroupList();

	m_pGroupView->setFocus();
	m_pGroupView->selectRow(index);
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::groupSelected()
{
	int index = m_pGroupView->currentIndex().row();
	if (index < 0 || index > m_groupBase.count())
	{
		return;
	}

	RackGroup group = m_groupBase.group(index);
	if (group.isValid() == false)
	{
		return;
	}

	setWindowTitle(tr("Properties - %1").arg(group.caption()));

	updateRackList();
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupPropertyDialog::foundDuplicateRacks()
{
	struct Duplicate
	{
		QString		rackID;
		bool		isDuplicate = false;

		QString		groupCaption1;
		int			channel1 =-1;

		QString		groupCaption2;
		int			channel2 =-1;
	};

	QVector<Duplicate> duplicateList;
	QMap<Hash, int> duplicateMap;

	int groupCount = m_groupBase.count();
	for(int i = 0; i < groupCount; i++)
	{
		RackGroup group = m_groupBase.group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		for(int channel = 0; channel < Metrology::ChannelCount; channel++)
		{
			QString currRackID = group.rackID(channel);

			if (currRackID.isEmpty() == true)
			{
				continue;
			}

			Hash hash = calcHash(currRackID);

			if (duplicateMap.contains(hash) == false)
			{
				Duplicate duplicate;

				duplicate.rackID = currRackID;
				duplicate.groupCaption1 = group.caption();
				duplicate.channel1 = channel+1;

				duplicateList.append(duplicate);

				duplicateMap[hash] = duplicateList.count() - 1;
			}
			else
			{
				int index = duplicateMap[hash];
				if (index >= 0 || index < duplicateList.count())
				{
					Duplicate& duplicate = duplicateList[index];

					duplicate.isDuplicate = true;
					duplicate.groupCaption2 = group.caption();
					duplicate.channel2 = channel+1;
				}
			}
		}
	}

	QString strDuplicates;

	int count = duplicateList.count();
	for(int i = 0; i < count; i ++)
	{
		Duplicate duplicate = duplicateList[i];

		if (duplicate.isDuplicate == true)
		{
			if (duplicate.groupCaption1 == duplicate.groupCaption2)
			{
				strDuplicates.append(tr("%1 - group \"%2\", channels %3 and %4\n")
									.arg(duplicate.rackID)
									.arg(duplicate.groupCaption1)
									.arg(duplicate.channel1).arg(duplicate.channel2));
			}
			else
			{
				strDuplicates.append(tr("%1 - group \"%2\" channel %3 and group \"%4\" channel %5\n")
									.arg(duplicate.rackID)
									.arg(duplicate.groupCaption1).arg(duplicate.channel1)
									.arg(duplicate.groupCaption2).arg(duplicate.channel2));
			}
		}
	}

	if (strDuplicates.isEmpty() == false)
	{
		QString alert = tr("Found same racks:\n\n") + strDuplicates;
		QMessageBox::information(this, tr("Same racks"), alert);

		return true;
	}

	return false;
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupPropertyDialog::onOk()
{
	if (foundDuplicateRacks() == true)
	{
		return;
	}

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool SignalPropertyDialog::m_showGroupHeader[SIGNAL_PROPERTY_GROUP_COUNT] =
{
	true,	//	SIGNAL_PROPERTY_GROUP_ID
	false,	//	SIGNAL_PROPERTY_GROUP_POSITION
	false,	//	SIGNAL_PROPERTY_GROUP_EL_RANGE
	false,	//	SIGNAL_PROPERTY_GROUP_EN_RANGE
};

// -------------------------------------------------------------------------------------------------------------------

SignalPropertyDialog::SignalPropertyDialog(const Metrology::SignalParam& param, QWidget *parent) :
	QDialog(parent)
{
	if (param.isValid() == false)
	{
		assert(false);
		return;
	}

	m_param = param;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalPropertyDialog::~SignalPropertyDialog()
{
	if (m_pManager != nullptr)
	{
		delete m_pManager;
		m_pManager = nullptr;
	}

	if (m_pFactory != nullptr)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	if (m_pEditor != nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Properties"));
	setMinimumSize(600, 300);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	if (m_param.isValid() == false)
	{
		assert(m_param.isValid() == false);
		return;
	}

	setWindowTitle(tr("Properties - %1").arg(m_param.appSignalID()));

	QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
	QStringList electricUnitList;
	for(int u = 0; u < meu.keyCount(); u++)
	{
		electricUnitList.append(meu.key(u));
	}


	QVBoxLayout *mainLayout = new QVBoxLayout;

	// create property list
	//

	QtVariantProperty *item = nullptr;

	m_pManager = new QtVariantPropertyManager;
	m_pFactory = new QtVariantEditorFactory;
	m_pEditor = new QtTreePropertyBrowser;

	// create property groups
	//

		// id group

		QtProperty *signalIdGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Signal ID"));

			item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
			item->setValue(m_param.appSignalID());
			item->setAttribute(QLatin1String("readOnly"), true);
			signalIdGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
			item->setValue(m_param.customAppSignalID());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_CUSTOM_ID);
			signalIdGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Caption"));
			item->setValue(m_param.caption());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_CAPTION);
			signalIdGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// position group

		QtProperty *positionGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Position"));

			item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
			item->setValue(m_param.location().equipmentID());
			item->setAttribute(QLatin1String("readOnly"), true);
			positionGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Rack"));
			item->setValue(m_param.location().rack().caption());
			item->setAttribute(QLatin1String("readOnly"), true);
			positionGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Chassis"));
			item->setValue(m_param.location().chassis());
			item->setAttribute(QLatin1String("readOnly"), true);
			positionGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Module"));
			item->setValue(m_param.location().module());
			item->setAttribute(QLatin1String("readOnly"), true);
			positionGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Place"));
			item->setValue(m_param.location().place());
			item->setAttribute(QLatin1String("readOnly"), true);
			positionGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// electric range group

		QtProperty *electricRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), SignalPropertyGroup[SIGNAL_PROPERTY_GROUP_EL_RANGE] + m_param.electricRangeStr());

			item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
			item->setValue(m_param.electricLowLimit());
			item->setAttribute(QLatin1String("singleStep"), 0.001);
			item->setAttribute(QLatin1String("decimals"), m_param.electricPrecision());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_LOW);
			electricRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
			item->setValue(m_param.electricHighLimit());
			item->setAttribute(QLatin1String("singleStep"), 0.001);
			item->setAttribute(QLatin1String("decimals"), m_param.electricPrecision());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_HIGH);
			electricRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
			item->setAttribute(QLatin1String("enumNames"), electricUnitList);
			item->setValue(m_param.electricUnitID());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_UNIT);
			electricRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Sensor type"));
			QMetaEnum mst = QMetaEnum::fromType<E::SensorType>();
			QStringList sensorList;
			for(int s = 0; s < mst.keyCount(); s++)
			{
				sensorList.append(mst.key(s));
			}
			item->setAttribute(QLatin1String("enumNames"), sensorList);
			item->setValue(m_param.electricSensorType());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_SENSOR);
			electricRangeGroup->addSubProperty(item);

			if (m_param.electricUnitID() == E::ElectricUnit::Ohm)
			{
				item = m_pManager->addProperty(QVariant::Double, tr("R0"));
				item->setValue(m_param.electricR0());
				item->setAttribute(QLatin1String("singleStep"), 1);
				item->setAttribute(QLatin1String("decimals"), 2);
				m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_R0);
				electricRangeGroup->addSubProperty(item);
			}

			item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
			item->setValue(m_param.electricPrecision());
			item->setAttribute(QLatin1String("minimum"), 0);
			item->setAttribute(QLatin1String("maximum"), 10);
			item->setAttribute(QLatin1String("singleStep"), 1);
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EL_RANGE_PRECISION);
			electricRangeGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// engineering range group

		QtProperty *engineeringRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), SignalPropertyGroup[SIGNAL_PROPERTY_GROUP_EN_RANGE] + m_param.engineeringRangeStr());

			item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
			item->setValue(m_param.lowEngineeringUnits());
			item->setAttribute(QLatin1String("singleStep"), 0.001);
			item->setAttribute(QLatin1String("decimals"), m_param.decimalPlaces());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EN_RANGE_LOW);
			engineeringRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
			item->setValue(m_param.highEngineeringUnits());
			item->setAttribute(QLatin1String("singleStep"), 0.001);
			item->setAttribute(QLatin1String("decimals"), m_param.decimalPlaces());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EN_RANGE_HIGH);
			engineeringRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Unit"));
			item->setValue(m_param.unit());
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EN_RANGE_UNIT);
			engineeringRangeGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
			item->setValue(m_param.decimalPlaces());
			item->setAttribute(QLatin1String("minimum"), 0);
			item->setAttribute(QLatin1String("maximum"), 10);
			item->setAttribute(QLatin1String("singleStep"), 1);
			m_propertyMap.insert(item, SIGNAL_PROPERTY_ITEM_EN_RANGE_PRECISION);
			engineeringRangeGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

	// show or hide property groups
	//

	m_browserItemList[SIGNAL_PROPERTY_GROUP_ID] = m_pEditor->addProperty(signalIdGroup);
	m_browserItemList[SIGNAL_PROPERTY_GROUP_POSITION] = m_pEditor->addProperty(positionGroup);

	if (m_param.isAnalog() == true)
	{
		switch (m_param.inOutType())
		{
			case E::SignalInOutType::Input:
			case E::SignalInOutType::Output:


				m_browserItemList[SIGNAL_PROPERTY_GROUP_EL_RANGE] = m_pEditor->addProperty(electricRangeGroup);
				m_browserItemList[SIGNAL_PROPERTY_GROUP_EN_RANGE] = m_pEditor->addProperty(engineeringRangeGroup);

				break;

			case E::SignalInOutType::Internal:

				m_browserItemList[SIGNAL_PROPERTY_GROUP_EN_RANGE] = m_pEditor->addProperty(engineeringRangeGroup);

				break;

			default:
				assert(0);
		}
	}

	for(int g = 0; g < SIGNAL_PROPERTY_GROUP_COUNT; g++)
	{
		if (m_browserItemList[g] == nullptr)
		{
			continue;
		}

		m_pEditor->setExpanded(m_browserItemList[g], m_showGroupHeader[g]);
	}

	//
	//
	m_pEditor->setPropertiesWithoutValueMarked(true);
	m_pEditor->setRootIsDecorated(false);

	connect(m_pManager, &QtVariantPropertyManager::valueChanged, this, &SignalPropertyDialog::onPropertyValueChanged);
	connect(m_pEditor, &QtTreePropertyBrowser::expanded, this, &SignalPropertyDialog::onPropertyExpanded);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertyDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertyDialog::reject);

	// add layouts
	//
	mainLayout->addWidget(m_pEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyMap.contains(property) == false)
	{
		return;
	}

	int index = m_propertyMap[property];
	if (index < 0 || index >= SIGNAL_PROPERTY_ITEM_COUNT)
	{
		return;
	}

	int groupIndex = -1;

	switch(index)
	{
		case SIGNAL_PROPERTY_ITEM_CUSTOM_ID:			m_param.setCustomAppSignalID(value.toString());										groupIndex = SIGNAL_PROPERTY_GROUP_ID;			break;
		case SIGNAL_PROPERTY_ITEM_CAPTION:				m_param.setCaption(value.toString());												groupIndex = SIGNAL_PROPERTY_GROUP_ID;			break;

		// electric limit
		//
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_LOW:			m_param.setElectricLowLimit(value.toDouble());										groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_HIGH:		m_param.setElectricHighLimit(value.toDouble());										groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_UNIT:		m_param.setElectricUnitID(static_cast<E::ElectricUnit>(value.toInt()));				groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_SENSOR:		m_param.setElectricSensorType(static_cast<E::SensorType>(value.toInt()));			groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_R0:			m_param.setElectricR0(value.toDouble());											groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EL_RANGE_PRECISION:	m_param.setElectricPrecision(value.toInt());										groupIndex = SIGNAL_PROPERTY_GROUP_EL_RANGE;	break;

		// engineering limit
		//
		case SIGNAL_PROPERTY_ITEM_EN_RANGE_LOW:			m_param.setLowEngineeringUnits(value.toDouble());									groupIndex = SIGNAL_PROPERTY_GROUP_EN_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EN_RANGE_HIGH:		m_param.setHighEngineeringUnits(value.toDouble());									groupIndex = SIGNAL_PROPERTY_GROUP_EN_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EN_RANGE_UNIT:		m_param.setUnit(value.toString());													groupIndex = SIGNAL_PROPERTY_GROUP_EN_RANGE;	break;
		case SIGNAL_PROPERTY_ITEM_EN_RANGE_PRECISION:	m_param.setDecimalPlaces(value.toInt());											groupIndex = SIGNAL_PROPERTY_GROUP_EN_RANGE;	break;
	}

	if (groupIndex < 0 || groupIndex >= SIGNAL_PROPERTY_GROUP_COUNT)
	{
		return;
	}

	updateGroupHeader(groupIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::updateGroupHeader(int index)
{
	if (index < 0 || index >= SIGNAL_PROPERTY_GROUP_COUNT)
	{
		return;
	}

	QtBrowserItem* browserItem = m_browserItemList[index];
	if (browserItem == nullptr)
	{
		return;
	}

	QString header;

	switch(index)
	{
		case SIGNAL_PROPERTY_GROUP_ID:			header = tr("Signal ID");												break;
		case SIGNAL_PROPERTY_GROUP_EL_RANGE:	header = SignalPropertyGroup[index] + m_param.electricRangeStr();		break;
		case SIGNAL_PROPERTY_GROUP_EN_RANGE:	header = SignalPropertyGroup[index] + m_param.engineeringRangeStr();	break;
		default:								assert(0);
	}

	browserItem->property()->setPropertyName(header);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onPropertyExpanded(QtBrowserItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	if (m_pEditor == nullptr)
	{
		return;
	}

	for(int g = 0; g < SIGNAL_PROPERTY_GROUP_COUNT; g++)
	{
		if (m_browserItemList[g] == item)
		{
			m_showGroupHeader[g] = m_pEditor->isExpanded(item);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onOk()
{
	if (m_param.isValid() == false)
	{
		assert(false);
		return;
	}

	theSignalBase.setSignalParam(m_param.hash(), m_param);

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool ComparatorPropertyDialog::m_showGroupHeader[COMPARATOR_PROPERTY_GROUP_COUNT] =
{
	true,	//	COMPARATOR_PROPERTY_GROUP_INPUT
	false,	//	COMPARATOR_PROPERTY_GROUP_COMPARE
	false,	//	COMPARATOR_PROPERTY_GROUP_HYSTERESIS
	false,	//	COMPARATOR_PROPERTY_GROUP_OUTPUT
};

// -------------------------------------------------------------------------------------------------------------------

ComparatorPropertyDialog::ComparatorPropertyDialog(const Comparator& comparator, QWidget *parent) :
	QDialog(parent)
{
	m_comparator = comparator;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorPropertyDialog::~ComparatorPropertyDialog()
{
	if (m_pManager != nullptr)
	{
		delete m_pManager;
		m_pManager = nullptr;
	}

	if (m_pFactory != nullptr)
	{
		delete m_pFactory;
		m_pFactory = nullptr;
	}

	if (m_pEditor != nullptr)
	{
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorPropertyDialog::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Properties"));
	setMinimumSize(600, 300);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());

	setWindowTitle(tr("Properties"));
	//setWindowTitle(tr("Properties - %1").arg(m_comparator.appSignalID()));

	Metrology::Signal* pInputSignal = theSignalBase.signalPtr(calcHash(m_comparator.input().appSignalID()));

	int precision = m_comparator.precision();
	switch (m_comparator.intAnalogSignalFormat())
	{
		case E::AnalogAppSignalFormat::Float32:		precision = m_comparator.precision();	break;
		case E::AnalogAppSignalFormat::SignedInt32:	precision = 0;							break;
		default:									assert(0);
	}

	QMetaEnum meu = QMetaEnum::fromType<E::CmpType>();
	QStringList cmpTypeList;
	for(int u = 0; u < meu.keyCount(); u++)
	{
		cmpTypeList.append(meu.key(u));
	}

	QVBoxLayout *mainLayout = new QVBoxLayout;

	// create property list
	//

	QtVariantProperty *item = nullptr;

	m_pManager = new QtVariantPropertyManager;
	m_pFactory = new QtVariantEditorFactory;
	m_pEditor = new QtTreePropertyBrowser;

	// create property groups
	//

		// input group

		QtProperty *inputGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Input"));

			if (pInputSignal == nullptr)
			{
				item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
				item->setValue(m_comparator.input().appSignalID());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);
			}
			else
			{
				item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
				item->setValue(pInputSignal->param().appSignalID());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);

				item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
				item->setValue(pInputSignal->param().customAppSignalID());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);

				item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
				item->setValue(pInputSignal->param().equipmentID());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);

				item = m_pManager->addProperty(QVariant::String, tr("Caption"));
				item->setValue(pInputSignal->param().caption());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);

				if (pInputSignal->param().isInput() == true)
				{
					item = m_pManager->addProperty(QVariant::String, tr("Electric range"));
					item->setValue(pInputSignal->param().electricRangeStr());
					item->setAttribute(QLatin1String("readOnly"), true);
					inputGroup->addSubProperty(item);
				}

				item = m_pManager->addProperty(QVariant::String, tr("Engineering range"));
				item->setValue(pInputSignal->param().engineeringRangeStr());
				item->setAttribute(QLatin1String("readOnly"), true);
				inputGroup->addSubProperty(item);
			}

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// compare group

		QtProperty *compareGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), comparator().compare().isConst() == true ? tr("Compare - const") : tr("Compare - dynamic"));

			item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Type"));
			item->setAttribute(QLatin1String("enumNames"), cmpTypeList);
			item->setValue(static_cast<int>(m_comparator.cmpType()));
			m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_CMP_TYPE);
			compareGroup->addSubProperty(item);

			if (comparator().compare().isConst() == true)
			{
				if (pInputSignal != nullptr && pInputSignal->param().isValid() == true && pInputSignal->param().isInput() == true)
				{
					item = m_pManager->addProperty(QVariant::Double, tr("Electric value, ") + pInputSignal->param().electricUnitStr());
					item->setValue(conversion(m_comparator.compare().constValue(), CT_ENGENEER_TO_ELECTRIC, pInputSignal->param()));
					item->setAttribute(QLatin1String("decimals"), pInputSignal->param().electricPrecision());
					m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_CMP_EL_VALUE);
					compareGroup->addSubProperty(item);
				}

				item = m_pManager->addProperty(QVariant::Double, tr("Engineering value, %1").arg(pInputSignal == nullptr ? QString() : pInputSignal->param().unit()));
				item->setValue(m_comparator.compare().constValue());
				item->setAttribute(QLatin1String("decimals"), precision);
				m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_CMP_EN_VALUE);
				compareGroup->addSubProperty(item);
			}
			else
			{
				Metrology::Signal* pCompareSignal = theSignalBase.signalPtr(calcHash(m_comparator.compare().appSignalID()));
				if (pCompareSignal == nullptr)
				{
					item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
					item->setValue(m_comparator.compare().appSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);
				}
				else
				{
					item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
					item->setValue(pCompareSignal->param().appSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
					item->setValue(pCompareSignal->param().customAppSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
					item->setValue(pCompareSignal->param().equipmentID());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("Caption"));
					item->setValue(pCompareSignal->param().caption());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);

					if (pCompareSignal->param().isInput() == true)
					{
						item = m_pManager->addProperty(QVariant::String, tr("Electric range"));
						item->setValue(pCompareSignal->param().electricRangeStr());
						item->setAttribute(QLatin1String("readOnly"), true);
						compareGroup->addSubProperty(item);
					}

					item = m_pManager->addProperty(QVariant::String, tr("Engineering range"));
					item->setValue(pCompareSignal->param().engineeringRangeStr());
					item->setAttribute(QLatin1String("readOnly"), true);
					compareGroup->addSubProperty(item);
				}

			}

			item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
			item->setValue(m_comparator.precision());
			m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_CMP_PRECESION);
			compareGroup->addSubProperty(item);

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// hysteresis group

		QtProperty *hysteresisGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), comparator().hysteresis().isConst() == true ? tr("Hysteresis - const") : tr("Hysteresis - dynamic"));

			if (comparator().hysteresis().isConst() == true)
			{
//				if (pInputSignal != nullptr && pInputSignal->param().isValid() == true && pInputSignal->param().isInput() == true)
//				{
//					item = m_pManager->addProperty(QVariant::Double, tr("Electric value, ") + pInputSignal->param().electricUnitStr());
//					item->setValue(conversion(m_comparator.hysteresis().constValue(), CT_ENGENEER_TO_ELECTRIC, pInputSignal->param()));
//					item->setAttribute(QLatin1String("decimals"), pInputSignal->param().electricPrecision());
//					m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_HYST_EL_VALUE);
//					hysteresisGroup->addSubProperty(item);
//				}

				item = m_pManager->addProperty(QVariant::Double, tr("Engineering value, %1").arg(pInputSignal == nullptr ? QString() : pInputSignal->param().unit()));
				item->setValue(m_comparator.hysteresis().constValue());
				item->setAttribute(QLatin1String("decimals"), precision);
				m_propertyMap.insert(item, COMPARATOR_PROPERTY_ITEM_HYST_EN_VALUE);
				hysteresisGroup->addSubProperty(item);
			}
			else
			{
				Metrology::Signal* pHysteresisSignal = theSignalBase.signalPtr(calcHash(m_comparator.hysteresis().appSignalID()));
				if (pHysteresisSignal == nullptr)
				{
					item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
					item->setValue(m_comparator.hysteresis().appSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);
				}
				else
				{
					item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
					item->setValue(pHysteresisSignal->param().appSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
					item->setValue(pHysteresisSignal->param().customAppSignalID());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
					item->setValue(pHysteresisSignal->param().equipmentID());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);

					item = m_pManager->addProperty(QVariant::String, tr("Caption"));
					item->setValue(pHysteresisSignal->param().caption());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);

					if (pHysteresisSignal->param().isInput() == true)
					{
						item = m_pManager->addProperty(QVariant::String, tr("Electric range"));
						item->setValue(pHysteresisSignal->param().electricRangeStr());
						item->setAttribute(QLatin1String("readOnly"), true);
						hysteresisGroup->addSubProperty(item);
					}

					item = m_pManager->addProperty(QVariant::String, tr("Engineering range"));
					item->setValue(pHysteresisSignal->param().engineeringRangeStr());
					item->setAttribute(QLatin1String("readOnly"), true);
					hysteresisGroup->addSubProperty(item);
				}
			}

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

		// output group

		QtProperty *outputGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Output"));

		Metrology::Signal* pOutputSignal = theSignalBase.signalPtr(calcHash(m_comparator.output().appSignalID()));
		if (pOutputSignal == nullptr)
		{
			item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
			item->setValue(m_comparator.output().appSignalID());
			item->setAttribute(QLatin1String("readOnly"), true);
			outputGroup->addSubProperty(item);
		}
		else
		{
			item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
			item->setValue(pOutputSignal->param().appSignalID());
			item->setAttribute(QLatin1String("readOnly"), true);
			outputGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
			item->setValue(pOutputSignal->param().customAppSignalID());
			item->setAttribute(QLatin1String("readOnly"), true);
			outputGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
			item->setValue(pOutputSignal->param().equipmentID());
			item->setAttribute(QLatin1String("readOnly"), true);
			outputGroup->addSubProperty(item);

			item = m_pManager->addProperty(QVariant::String, tr("Caption"));
			item->setValue(pOutputSignal->param().caption());
			item->setAttribute(QLatin1String("readOnly"), true);
			outputGroup->addSubProperty(item);
		}

		m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

	// show or hide property groups
	//

	m_browserItemList[COMPARATOR_PROPERTY_GROUP_INPUT] = m_pEditor->addProperty(inputGroup);
	m_browserItemList[COMPARATOR_PROPERTY_GROUP_COMPARE] = m_pEditor->addProperty(compareGroup);
	m_browserItemList[COMPARATOR_PROPERTY_GROUP_HYSTERESIS] = m_pEditor->addProperty(hysteresisGroup);
	m_browserItemList[COMPARATOR_PROPERTY_GROUP_OUTPUT] = m_pEditor->addProperty(outputGroup);


	for(int g = 0; g < COMPARATOR_PROPERTY_GROUP_COUNT; g++)
	{
		if (m_browserItemList[g] == nullptr)
		{
			continue;
		}

		m_pEditor->setExpanded(m_browserItemList[g], m_showGroupHeader[g]);
	}

	//
	//
	m_pEditor->setPropertiesWithoutValueMarked(true);
	m_pEditor->setRootIsDecorated(false);

	connect(m_pManager, &QtVariantPropertyManager::valueChanged, this, &ComparatorPropertyDialog::onPropertyValueChanged);
	connect(m_pEditor, &QtTreePropertyBrowser::expanded, this, &ComparatorPropertyDialog::onPropertyExpanded);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ComparatorPropertyDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ComparatorPropertyDialog::reject);

	// add layouts
	//
	mainLayout->addWidget(m_pEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorPropertyDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyMap.contains(property) == false)
	{
		return;
	}

	int index = m_propertyMap[property];
	if (index < 0 || index >= COMPARATOR_PROPERTY_ITEM_COUNT)
	{
		return;
	}

	Metrology::Signal* pInputSignal = theSignalBase.signalPtr(calcHash(m_comparator.input().appSignalID()));
	if (pInputSignal == nullptr || pInputSignal->param().isValid() == false)
	{
		return;
	}

	int groupIndex = -1;

	switch(index)
	{
		// comapre
		//
		case COMPARATOR_PROPERTY_ITEM_CMP_TYPE:
			{
				m_comparator.setCmpType((static_cast<E::CmpType>(value.toInt())));
				groupIndex = COMPARATOR_PROPERTY_GROUP_COMPARE;
			}
			break;

		case COMPARATOR_PROPERTY_ITEM_CMP_EL_VALUE:
			{
				m_comparator.compare().setConstValue(conversion(value.toDouble(), CT_ELECTRIC_TO_ENGENEER, pInputSignal->param()));
				groupIndex = COMPARATOR_PROPERTY_GROUP_COMPARE;

				QtVariantProperty *propertyEn = dynamic_cast<QtVariantProperty*>(m_propertyMap.key(COMPARATOR_PROPERTY_ITEM_CMP_EN_VALUE));
				if (propertyEn != nullptr)
				{
					propertyEn->setValue(m_comparator.compare().constValue());
				}
			}
			break;

		case COMPARATOR_PROPERTY_ITEM_CMP_EN_VALUE:
			{
				m_comparator.compare().setConstValue(value.toDouble());
				groupIndex = COMPARATOR_PROPERTY_GROUP_COMPARE;

				QtVariantProperty *propertyEl = dynamic_cast<QtVariantProperty*>(m_propertyMap.key(COMPARATOR_PROPERTY_ITEM_CMP_EL_VALUE));
				if (propertyEl != nullptr && pInputSignal->param().isInput() == true)
				{
					propertyEl->setValue(conversion(m_comparator.compare().constValue(), CT_ENGENEER_TO_ELECTRIC, pInputSignal->param()));
				}
			}
			break;

		case COMPARATOR_PROPERTY_ITEM_CMP_PRECESION:
			{
				m_comparator.setPrecision(value.toInt());
			}
			break;

		// hysteresis
		//
		case COMPARATOR_PROPERTY_ITEM_HYST_EL_VALUE:
			{
				m_comparator.hysteresis().setConstValue(conversion(value.toDouble(), CT_ELECTRIC_TO_ENGENEER, pInputSignal->param()));
				groupIndex = COMPARATOR_PROPERTY_GROUP_HYSTERESIS;

				QtVariantProperty *propertyEn = dynamic_cast<QtVariantProperty*>(m_propertyMap.key(COMPARATOR_PROPERTY_ITEM_HYST_EN_VALUE));
				if (propertyEn != nullptr)
				{
					propertyEn->setValue(m_comparator.hysteresis().constValue());
				}
			}
			break;

		case COMPARATOR_PROPERTY_ITEM_HYST_EN_VALUE:
			{
				m_comparator.hysteresis().setConstValue(value.toDouble());
				groupIndex = COMPARATOR_PROPERTY_GROUP_HYSTERESIS;

				QtVariantProperty *propertyEl = dynamic_cast<QtVariantProperty*>(m_propertyMap.key(COMPARATOR_PROPERTY_ITEM_HYST_EL_VALUE));
				if (propertyEl != nullptr && pInputSignal->param().isInput() == true)
				{
					propertyEl->setValue(conversion(m_comparator.hysteresis().constValue(), CT_ENGENEER_TO_ELECTRIC, pInputSignal->param()));
				}
			}
			break;
	}

	if (groupIndex < 0 || groupIndex >= COMPARATOR_PROPERTY_GROUP_COUNT)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorPropertyDialog::onPropertyExpanded(QtBrowserItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	if (m_pEditor == nullptr)
	{
		return;
	}

	for(int g = 0; g < COMPARATOR_PROPERTY_GROUP_COUNT; g++)
	{
		if (m_browserItemList[g] == item)
		{
			m_showGroupHeader[g] = m_pEditor->isExpanded(item);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorPropertyDialog::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

