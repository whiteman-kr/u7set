#include "DialogObjectProperties.h"

#include "UnitsConverter.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogProjectProperty::DialogProjectProperty(const ProjectInfo& param, QWidget* parent) :
	QDialog(parent)
{
	m_info = param;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogProjectProperty::~DialogProjectProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogProjectProperty::PropertyPattern::PropertyPattern(ProjectInfo* pObject) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	QString categoryInfo = ProjectPropertyCategoryCaption(ProjectPropertyCategory::Info);

	ADD_PROPERTY_GETTER(QString, DialogProjectProperty::tr("Project name"), true, m_pObject->ProjectInfo::projectName)
		->setCategory(categoryInfo)
		.setViewOrder(0);
	ADD_PROPERTY_GETTER(int, DialogProjectProperty::tr("Project ID"), true, m_pObject->ProjectInfo::id)
		->setCategory(categoryInfo)
		.setViewOrder(1);
	ADD_PROPERTY_GETTER(QString, DialogProjectProperty::tr("Date"), true, m_pObject->ProjectInfo::date)
		->setCategory(categoryInfo)
		.setViewOrder(2);

	QString categoryHost = ProjectPropertyCategoryCaption(ProjectPropertyCategory::Host);

	ADD_PROPERTY_GETTER(QString, DialogProjectProperty::tr("User"), true, m_pObject->ProjectInfo::user)
		->setCategory(categoryHost)
		.setViewOrder(0);
	ADD_PROPERTY_GETTER(QString, DialogProjectProperty::tr("Workstation"), true, m_pObject->ProjectInfo::workstation)
		->setCategory(categoryHost)
		.setViewOrder(1);

	QString categoryVersion = ProjectPropertyCategoryCaption(ProjectPropertyCategory::Version);

	ADD_PROPERTY_GETTER(int, DialogProjectProperty::tr("Database Version"), true, m_pObject->ProjectInfo::dbVersion)
		->setCategory(categoryVersion)
		.setViewOrder(0);
	ADD_PROPERTY_GETTER(int, DialogProjectProperty::tr("Config File Version"), true, m_pObject->ProjectInfo::cfgFileVersion)
		->setCategory(categoryVersion)
		.setViewOrder(1);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogProjectProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	move(screen.center() - rect().center());

	setWindowTitle(tr("Project - %1").arg(m_info.projectName()));

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(300);
	m_pPropertyEditor->setReadOnly(true);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(&m_info);
	propertyObjects.push_back(property);

    for(int categoryIndex = 0; categoryIndex < ProjectPropertyCategoryCount; categoryIndex++)
	{
        ProjectPropertyCategory category = static_cast<ProjectPropertyCategory>(categoryIndex);
        if (ERR_PROJECT_PROPERTY_CATEGORY(category) == true)
		{
			continue;
		}

        m_pPropertyEditor->setCategoryViewOrder(ProjectPropertyCategoryCaption(category), categoryIndex);
	}

	m_pPropertyEditor->setObjects(propertyObjects);

	// add layouts
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pPropertyEditor);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

QString ProjectPropertyCategoryCaption(ProjectPropertyCategory category)
{
	QString caption;

	switch (category)
	{
		case ProjectPropertyCategory::Info:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Project");			break;
		case ProjectPropertyCategory::Host:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Host");			break;
		case ProjectPropertyCategory::Version:	caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "File version");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Unknown");
	}

	return qApp->translate("DialogObjectProperty", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogRackProperty::DialogRackProperty(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget* parent) :
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

DialogRackProperty::~DialogRackProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogRackProperty::PropertyPattern::PropertyPattern(Metrology::RackParam* pObject, RackBase* pRackBase) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	if (pRackBase == nullptr)
	{
		return;
	}

	// prepare enum groups
	//
	std::vector<std::pair<QString, int>> enumGroups;

	enumGroups.push_back({QString(), 0});

	int groupCount = pRackBase->groups().count();
	for(int g = 0; g < groupCount; g++)
	{
		RackGroup group = pRackBase->groups().group(g);
		if (group.isValid() == false)
		{
			continue;
		}

		enumGroups.push_back({group.caption(), g + 1});
	}

	QString strDefaultGroup;
	RackGroup defaultGroup = pRackBase->groups().group(m_pObject->groupIndex());
	if (defaultGroup.isValid() == true)
	{
		strDefaultGroup = defaultGroup.caption();
	}

	// prepare enum channels
	//
	std::vector<std::pair<QString, int>> enumChannels;

	enumChannels.push_back({QString(), 0});
	for(int ch = 0; ch < Metrology::CHANNEL_COUNT; ch++)
	{
		enumChannels.push_back({QString::number(ch + 1), ch + 1});
	}

	QString strDefaultChannel;
	int defaultChannel = m_pObject->channel();
	if (ERR_CHANNEL(defaultChannel) == false)
	{
		strDefaultChannel = QString::number(defaultChannel + 1);
	}

	// append properties
	//
	QString categoryInfo = DialogRackProperty::tr("Property of the rack");

	ADD_PROPERTY_GETTER(QString, DialogRackProperty::tr("Caption"), true, m_pObject->Metrology::RackParam::caption)
		->setCategory(categoryInfo)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogRackProperty::tr("EquipmentID"), true, m_pObject->Metrology::RackParam::equipmentID)
		->setCategory(categoryInfo)
		.setViewOrder(1)
		.setReadOnly(true);
	addDynamicEnumProperty(DialogRackProperty::tr("Group"), enumGroups, true)
		->setCategory(categoryInfo)
		.setViewOrder(2)
		.setReadOnly(true)
		.setValue(strDefaultGroup);
	addDynamicEnumProperty(DialogRackProperty::tr("Channel"), enumChannels, true)
		->setCategory(categoryInfo)
		.setViewOrder(3)
		.setReadOnly(true)
		.setValue(strDefaultChannel);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.15), static_cast<int>(screen.height() * 0.15));
	resize(static_cast<int>(screen.width() * 0.15), static_cast<int>(screen.height() * 0.15));
	move(screen.center() - rect().center());

	if (m_rack.isValid() == false)
	{
		assert(m_rack.isValid() == false);
		return;
	}

	setWindowTitle(tr("Property of rack - %1").arg(m_rack.caption()));

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(150);
	m_pPropertyEditor->setReadOnly(true);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogRackProperty::onPropertyValueChanged);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(&m_rack, &m_rackBase);
	propertyObjects.push_back(property);
	m_pPropertyEditor->setObjects(propertyObjects);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogRackProperty::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogRackProperty::reject);

	// add layouts
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pPropertyEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackProperty::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		//
		//
		auto propertyGroup = properties->propertyByCaption(DialogRackProperty::tr("Group"));
		if (propertyGroup != nullptr)
		{
			if (propertyGroup->isEnum() == true)
			{
				m_rack.setGroupIndex(propertyGroup->value().toInt() - 1);
			}
		}

		//
		//
		auto propertyChannel = properties->propertyByCaption(DialogRackProperty::tr("Channel"));
		if (propertyChannel != nullptr)
		{
			if (propertyChannel->isEnum() == true)
			{
				if (m_rack.groupIndex() == -1)
				{
					m_rack.setChannel(-1);
					propertyChannel->setValue(0);
				}
				else
				{
					m_rack.setChannel(propertyChannel->value().toInt() - 1);
				}
			}
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogRackProperty::foundDuplicateGroups()
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
			QString alert = tr(	"Another rack \"%1\" already has the same group or channel.\n"
								"Please choose a different group or channel.").arg(rack.caption());
			QMessageBox::information(this, tr("Same racks"), alert);

			result = true;

			break;
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackProperty::onOk()
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

DialogRackGroupProperty::DialogRackGroupProperty(const RackBase& rackBase, QWidget* parent) :
	QDialog(parent)
{
	m_rackBase = rackBase;
	m_groupBase = m_rackBase.groups();

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogRackGroupProperty::~DialogRackGroupProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogRackGroupProperty::PropertyPattern::PropertyPattern(RackBase* pObject) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	// prepare enum racks
	//
	std::vector<std::pair<QString, int>> enumRacks;

	enumRacks.push_back({QString(), 0});

	int rackCount = m_pObject->count();
	for(int r = 0; r < rackCount; r++)
	{
		Metrology::RackParam rack = m_pObject->rack(r);
		if (rack.isValid() == false)
		{
			continue;
		}

		enumRacks.push_back({rack.caption(), r + 1});
	}

	// append properties
	//
	QString categoryRacks = DialogRackGroupProperty::tr("Racks");

	for(int channel = 0; channel < Metrology::CHANNEL_COUNT; channel++)
	{
		QString strHeader = DialogRackGroupProperty::tr("Channel") + " " + QString::number(channel + 1);

		addDynamicEnumProperty(strHeader, enumRacks, true)
			->setCategory(categoryRacks)
			.setViewOrder(channel)
			.setValue(QString());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property - rack groups"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	move(screen.center() - rect().center());

	// create menu
	//
	m_pMenuBar = new QMenuBar(this);
	m_pGroupMenu = new QMenu(tr("&Group"), this);

	m_pAppendGroupAction = m_pGroupMenu->addAction(tr("&Append"));
	m_pAppendGroupAction->setIcon(QIcon(":/icons/Add.png"));

	m_pRemoveGroupAction = m_pGroupMenu->addAction(tr("&Remove"));
	m_pRemoveGroupAction->setIcon(QIcon(":/icons/Remove.png"));

	m_pMenuBar->addMenu(m_pGroupMenu);

	connect(m_pAppendGroupAction, &QAction::triggered, this, &DialogRackGroupProperty::appendGroup);
	connect(m_pRemoveGroupAction, &QAction::triggered, this, &DialogRackGroupProperty::removeGroup);

	// create rack group view
	//
	m_pGroupView = new QTableWidget(this);

	m_pGroupView->setColumnCount(1);
	m_pGroupView->setHorizontalHeaderLabels(QStringList(""));

	m_pGroupView->horizontalHeader()->hide();
	m_pGroupView->verticalHeader()->hide();

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pGroupView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pGroupView->installEventFilter(this);

	// init context menu
	//
	m_pGroupView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pGroupView, &QTableWidget::customContextMenuRequested, this, &DialogRackGroupProperty::onContextMenu);

	connect(m_pGroupView, &QTableWidget::cellChanged, this, &DialogRackGroupProperty::captionGroupChanged);
	connect(m_pGroupView, &QTableWidget::itemSelectionChanged, this, &DialogRackGroupProperty::groupSelected);

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(150);
	m_pPropertyEditor->setReadOnly(false);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogRackGroupProperty::onPropertyValueChanged);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(&m_rackBase);
	propertyObjects.push_back(property);
	m_pPropertyEditor->setObjects(propertyObjects);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogRackGroupProperty::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogRackGroupProperty::reject);

	// add layouts
	//
	QHBoxLayout* listLayout = new QHBoxLayout;

	listLayout->addWidget(m_pGroupView);
	listLayout->addWidget(m_pPropertyEditor);

	//
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->setMenuBar(m_pMenuBar);
	mainLayout->addLayout(listLayout);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	//
	//
	updateGroupList();
	updateRackList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::updateGroupList(const Hash& hash)
{
	m_pGroupView->blockSignals(true);

	// clear list
	//
	int rowCount = m_pGroupView->rowCount();
	for(int row = 0; row < rowCount; row++)
	{
		QTableWidgetItem* item = m_pGroupView->item(row, RACK_GROUP_COLUMN_CAPTION);
		if (item != nullptr)
		{
			delete item;
		}
	}

	QTableWidgetItem* pSelectItem = nullptr;

	// append new group caption
	//
	int count = m_groupBase.count();

	m_pGroupView->setRowCount(count);

	for(int i = 0; i < count; i++)
	{
		const RackGroup& group = m_groupBase.group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		QTableWidgetItem* item = new QTableWidgetItem(group.caption());
		item->setTextAlignment(Qt::AlignHCenter);
		m_pGroupView->setItem(i, RACK_GROUP_COLUMN_CAPTION, item);

		if (group.hash() == hash)
		{
			pSelectItem = item;
		}
	}

	m_pGroupView->blockSignals(false);

	if (pSelectItem == nullptr)
	{
		if (m_pGroupView->rowCount() > 0 )
		{
			QTableWidgetItem* item = m_pGroupView->item(0, RACK_GROUP_COLUMN_CAPTION);
			if (item != nullptr)
			{
				m_pGroupView->setCurrentItem(item);
			}
		}
	}
	else
	{
		m_pGroupView->setCurrentItem(pSelectItem);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::updateRackList()
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

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : m_pPropertyEditor->objects())
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		for(int channel = 0; channel < Metrology::CHANNEL_COUNT; channel++)
		{
			auto propertyChannel = properties->propertyByCaption(tr("Channel") + " " + QString::number(channel + 1));
			if (propertyChannel == nullptr)
			{
				continue;
			}

			const QString& rackID = group.rackID(channel);
			if (rackID.isEmpty() == true)
			{
				propertyChannel->setValue(0);
				continue;
			}

			const Metrology::RackParam& rack = m_rackBase.rack(rackID);
			if (rack.isValid() == false)
			{
				propertyChannel->setValue(0);
				continue;
			}

			propertyChannel->setValue(rack.index() + 1);
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::appendGroup()
{
	QString caption = QString("Rack group %1").arg(m_groupBase.count() + 1);

	RackGroup group;

	group.setIndex(m_groupBase.count());
	group.setCaption(caption);

	m_groupBase.append(group);

	updateGroupList(group.hash());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::removeGroup()
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

	if (QMessageBox::question(this,
							  windowTitle(),
							  tr("Do you want delete group \"%1\"?").
							  arg(group.caption())) == QMessageBox::No)
	{
		return;
	}

	m_groupBase.remove(index);

	updateGroupList();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
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

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		for(int channel = 0; channel < Metrology::CHANNEL_COUNT; channel++)
		{
			auto propertyChannel = properties->propertyByCaption(tr("Channel") + " " + QString::number(channel + 1));
			if (propertyChannel == nullptr)
			{
				continue;
			}

			int rackIndex = propertyChannel->value().toInt() - 1;
			if (rackIndex < 0 || rackIndex >= m_rackBase.count())
			{
				// clear
				//
				if (group.rackID(channel).isEmpty() == false)
				{
					group.setRackID(channel, QString());
					m_groupBase.setGroup(index, group);
				}

				continue;
			}

			Metrology::RackParam rack = m_rackBase.rack(rackIndex);
			if (rack.isValid() == false)
			{
				continue;
			}

			if (group.rackID(channel) == rack.equipmentID())
			{
				continue;
			}

			rack.setChannel(channel);
			rack.setGroupIndex(group.index());
			m_rackBase.setRack(rack.index(), rack);

			group.setRackID(channel, rack.equipmentID());
			m_groupBase.setGroup(index, group);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogRackGroupProperty::event(QEvent* e)
{
	if (e->type() == QEvent::Resize)
	{
		m_pGroupView->setColumnWidth(RACK_GROUP_COLUMN_CAPTION, m_pGroupView->width() - 20);
	}

	return QDialog::event(e);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::onContextMenu(QPoint)
{
	m_pGroupMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogRackGroupProperty::captionGroupChanged(int row, int column)
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
			QMessageBox::information(this,
									 tr("Group caption"),
									 tr("Group caption \"%1\" already exists!").
									 arg(caption));
			updateGroupList();
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

void DialogRackGroupProperty::groupSelected()
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

	setWindowTitle(tr("Property - %1").arg(group.caption()));

	updateRackList();
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogRackGroupProperty::foundDuplicateRacks()
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

	std::vector<Duplicate> duplicateList;
	QMap<Hash, int> duplicateMap;

	int groupCount = m_groupBase.count();
	for(int i = 0; i < groupCount; i++)
	{
		RackGroup group = m_groupBase.group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		for(int channel = 0; channel < Metrology::CHANNEL_COUNT; channel++)
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

				duplicateList.push_back(duplicate);

				duplicateMap[hash] = TO_INT(duplicateList.size() - 1);
			}
			else
			{
				int index = duplicateMap[hash];
				if (index >= 0 && index < TO_INT(duplicateList.size()))
				{
					Duplicate& duplicate = duplicateList[static_cast<quint64>(index)];

					duplicate.isDuplicate = true;
					duplicate.groupCaption2 = group.caption();
					duplicate.channel2 = channel+1;
				}
			}
		}
	}

	QString strDuplicates;

	for(const Duplicate& duplicate : duplicateList)
	{
		if (duplicate.isDuplicate == true)
		{
			if (duplicate.groupCaption1 == duplicate.groupCaption2)
			{
				strDuplicates.append(tr("%1 - group \"%2\", channels %3 and %4\n")
									.arg(duplicate.rackID, duplicate.groupCaption1)
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

void DialogRackGroupProperty::onOk()
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

QVariant PrComparatorListTable::data(const QModelIndex &index, int role) const
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

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = at(row);
	if (comparatorEx == nullptr)
	{
		return QVariant();
	}

	Metrology::Signal* pInSignal = comparatorEx->inputSignal();
	if (pInSignal == nullptr || pInSignal->param().isValid() == false)
	{
		return QVariant();
	}

	if (role == Qt::ForegroundRole)
	{
		if (comparatorEx->signalsIsValid()  == false)
		{
			return QColor(Qt::red);
		}
		else
		{
			if (comparatorEx->enableMeasure() == false)
			{
				return QColor(Qt::lightGray);
			}
		}

		return QVariant();
	}

	if (role == Qt::BackgroundRole)
	{
		if (column == PR_COMPARATOR_LIST_COLUMN_SETPOINT)
		{
			if (comparatorEx->outputState() == true)
			{
				return theOptions.comparatorInfo().colorStateTrue();
			}
			else
			{
				return theOptions.comparatorInfo().colorStateFalse();
			}
		}
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		return text(row, column, comparatorEx);
	}

	return QVariant();
}

// -------------------------------------------------------------------------------------------------------------------

QString PrComparatorListTable::text(int row, int column, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const
{
	if (row < 0 || row >= count())
	{
		return QString();
	}

	if (column < 0 || column > m_columnCount)
	{
		return QString();
	}

	if (comparatorEx == nullptr)
	{
		return QString();
	}

	//
	//
	QString result;

	switch (column)
	{
		case PR_COMPARATOR_LIST_COLUMN_CMP_TO:		result = comparatorEx->compareTo(Metrology::SignalIDType::CustomID);					break;
		case PR_COMPARATOR_LIST_COLUMN_SETPOINT:	result = comparatorEx->compareOnlineValueStr(Metrology::CmpValueType::SetPoint, true);	break;
		case PR_COMPARATOR_LIST_COLUMN_OUTPUT:		result = comparatorEx->outputSignalID(Metrology::SignalIDType::CustomID);				break;

		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogSignalProperty::DialogSignalProperty(const Metrology::SignalParam& param, QWidget* parent) :
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

DialogSignalProperty::~DialogSignalProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogSignalProperty::PropertyPattern::PropertyPattern(Metrology::SignalParam* pObject) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	QString categorySignalID = SignalPropertyCategoryCaption(SignalPropertyCategory::SignalID);

	ADD_PROPERTY_GETTER_SETTER(QString, DialogSignalProperty::tr("SignalID"), true, m_pObject->Metrology::SignalParam::customAppSignalID, m_pObject->Metrology::SignalParam::setCustomAppSignalID)
		->setCategory(categorySignalID)
		.setViewOrder(0);
	ADD_PROPERTY_GETTER(QString, DialogSignalProperty::tr("AppSignalID"), true, m_pObject->Metrology::SignalParam::appSignalID)
		->setCategory(categorySignalID)
		.setViewOrder(1)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogSignalProperty::tr("EquipmentID"), true, m_pObject->Metrology::SignalParam::equipmentID)
		->setCategory(categorySignalID)
		.setViewOrder(2)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER_SETTER(QString, DialogSignalProperty::tr("Caption"), true, m_pObject->Metrology::SignalParam::caption, m_pObject->Metrology::SignalParam::setCaption)
		->setCategory(categorySignalID)
		.setViewOrder(3);
	ADD_PROPERTY_GETTER(QString, DialogSignalProperty::tr("Signal type"), true, m_pObject->Metrology::SignalParam::signalTypeStr)
		->setCategory(categorySignalID)
		.setViewOrder(4)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogSignalProperty::tr("Count of comparators"), true, m_pObject->Metrology::SignalParam::comparatorCount)
		->setCategory(categorySignalID)
		.setViewOrder(5)
		.setReadOnly(true);


	QString categoryPosition = SignalPropertyCategoryCaption(SignalPropertyCategory::SignalPosition);

	ADD_PROPERTY_GETTER(QString, DialogSignalProperty::tr("Rack"), true, m_pObject->location().Metrology::SignalLocation::rackCaption)
		->setCategory(categoryPosition)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogSignalProperty::tr("Chassis"), true, m_pObject->location().Metrology::SignalLocation::chassis)
		->setCategory(categoryPosition)
		.setViewOrder(1)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogSignalProperty::tr("Module"), true, m_pObject->location().Metrology::SignalLocation::module)
		->setCategory(categoryPosition)
		.setViewOrder(2)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogSignalProperty::tr("Place"), true, m_pObject->location().Metrology::SignalLocation::place)
		->setCategory(categoryPosition)
		.setViewOrder(3)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogSignalProperty::tr("Module type"), true, m_pObject->location().Metrology::SignalLocation::moduleCaption)
		->setCategory(categoryPosition)
		.setViewOrder(4)
		.setReadOnly(true);


	if (pObject->isAnalog() == true)
	{
		if (pObject->isInput() == true || pObject->isOutput() == true)
		{
			QString categoryElectricLimit = SignalPropertyCategoryCaption(SignalPropertyCategory::ElectricLimit);

			ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Electric low limit"), true, m_pObject->Metrology::SignalParam::electricLowLimit, m_pObject->Metrology::SignalParam::setElectricLowLimit)
				->setCategory(categoryElectricLimit)
				.setViewOrder(0)
				.setPrecision(m_pObject->electricPrecision());
			ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Electric high limit"), true, m_pObject->Metrology::SignalParam::electricHighLimit, m_pObject->Metrology::SignalParam::setElectricHighLimit)
				->setCategory(categoryElectricLimit)
				.setViewOrder(1)
				.setPrecision(m_pObject->electricPrecision());
			ADD_PROPERTY_GETTER_SETTER(E::ElectricUnit, DialogSignalProperty::tr("Electric unit"), true, m_pObject->Metrology::SignalParam::electricUnitID, m_pObject->Metrology::SignalParam::setElectricUnitID)
				->setCategory(categoryElectricLimit)
				.setViewOrder(2);
			ADD_PROPERTY_GETTER_SETTER(E::SensorType, DialogSignalProperty::tr("Electric sensor type"), true, m_pObject->Metrology::SignalParam::electricSensorType, m_pObject->Metrology::SignalParam::setElectricSensorType)
				->setCategory(categoryElectricLimit)
				.setViewOrder(3);

			switch (m_pObject->electricUnitID())
			{
				case E::ElectricUnit::mA:

					if (m_pObject->sensorType() != E::SensorType::V_0_5 && m_pObject->sensorType() != E::SensorType::V_m10_p10)
					{
						break;
					}

					ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Electric RLoad"), true, m_pObject->Metrology::SignalParam::electricRLoad, m_pObject->Metrology::SignalParam::setElectricRLoad)
							->setCategory(categoryElectricLimit)
							.setViewOrder(4)
							.setPrecision(0);
					break;

				case E::ElectricUnit::Ohm:

					if (m_pObject->sensorType() == E::SensorType::NoSensor || m_pObject->sensorType() == E::SensorType::Ohm_Raw)
					{
						break;
					}

					ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Electric R0"), true, m_pObject->Metrology::SignalParam::electricR0, m_pObject->Metrology::SignalParam::setElectricR0)
							->setCategory(categoryElectricLimit)
							.setViewOrder(4)
							.setPrecision(0);
					break;

				default:
					break;
			}

			ADD_PROPERTY_GETTER_SETTER(int, DialogSignalProperty::tr("Electric precision"), true, m_pObject->Metrology::SignalParam::electricPrecision, m_pObject->Metrology::SignalParam::setElectricPrecision)
				->setCategory(categoryElectricLimit)
				.setViewOrder(5);
		}


		QString categoryEngineeringLimit = SignalPropertyCategoryCaption(SignalPropertyCategory::EngineeringLimit);

		ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Engineering low limit"), true, m_pObject->Metrology::SignalParam::lowEngineeringUnits, m_pObject->Metrology::SignalParam::setLowEngineeringUnits)
			->setCategory(categoryEngineeringLimit)
			.setViewOrder(0)
			.setPrecision(m_pObject->decimalPlaces());
		ADD_PROPERTY_GETTER_SETTER(double, DialogSignalProperty::tr("Engineering high limit"), true, m_pObject->Metrology::SignalParam::highEngineeringUnits, m_pObject->Metrology::SignalParam::setHighEngineeringUnits)
			->setCategory(categoryEngineeringLimit)
			.setViewOrder(1)
			.setPrecision(m_pObject->decimalPlaces());
		ADD_PROPERTY_GETTER_SETTER(QString, DialogSignalProperty::tr("Engineering unit"), true, m_pObject->Metrology::SignalParam::unit, m_pObject->Metrology::SignalParam::setUnit)
			->setCategory(categoryEngineeringLimit)
			.setViewOrder(2);
		ADD_PROPERTY_GETTER_SETTER(int, DialogSignalProperty::tr("Engineering precision"), true, m_pObject->Metrology::SignalParam::decimalPlaces, m_pObject->Metrology::SignalParam::setDecimalPlaces)
			->setCategory(categoryEngineeringLimit)
			.setViewOrder(3);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::createContextMenu()
{
	if (m_pComparatorView == nullptr)
	{
		return;
	}

	// create context menu
	//
	m_pContextMenu = new QMenu(tr(""), this);


	m_pCopyAction = m_pContextMenu->addAction(tr("&Copy"));
	m_pCopyAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pCopyCellAction = m_pContextMenu->addAction(tr("Copy cell"));
	m_pCopyCellAction->setIcon(QIcon(":/icons/Copy.png"));

	m_pContextMenu->addSeparator();

	m_pComparatorPropertyAction = m_pContextMenu->addAction(tr("Propertу ..."));
	m_pComparatorPropertyAction->setIcon(QIcon(":/icons/Property.png"));

	connect(m_pCopyAction, &QAction::triggered, this, &DialogSignalProperty::onCopy);
	connect(m_pCopyCellAction, &QAction::triggered, this, &DialogSignalProperty::onCopyCell);
	connect(m_pComparatorPropertyAction, &QAction::triggered, this, &DialogSignalProperty::onComparatorProperty);

	// init context menu
	//
	m_pComparatorView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_pComparatorView, &QTableWidget::customContextMenuRequested, this, &DialogSignalProperty::onContextMenu);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	resize(static_cast<int>(screen.width() * 0.38), static_cast<int>(screen.height() * 0.52));
	move(screen.center() - rect().center());

	if (m_param.isValid() == false)
	{
		assert(m_param.isValid() == false);
		return;
	}

	setWindowTitle(tr("Property of signal - %1").arg(m_param.customAppSignalID()));

	// create tab
	//
	m_pTab = new QTabWidget();
	m_pTab->setTabPosition(QTabWidget::North);

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(350);
	m_pPropertyEditor->setReadOnly(false);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogSignalProperty::onPropertyValueChanged);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(&m_param);
	propertyObjects.push_back(property);

    for(int categoryIndex = 0; categoryIndex < SignalPropertyCategoryCount; categoryIndex++)
	{
        SignalPropertyCategory category = static_cast<SignalPropertyCategory>(categoryIndex);
        if (ERR_SIGNAL_PROPERTY_CATEGORY(category) == true)
		{
			continue;
		}

        m_pPropertyEditor->setCategoryViewOrder(SignalPropertyCategoryCaption(category), categoryIndex);
	}

	m_pPropertyEditor->setMaxDecimaplPlaces(15);

	m_pPropertyEditor->setObjects(propertyObjects);

	// create compartor list
	//
	m_pComparatorView = new QTableView(this);
	m_comparatorTable.setColumnCaption(DialogSignalProperty::metaObject()->className(), PR_COMPARATOR_LIST_COLUMN_COUNT, PrComparatorListColumn);
	m_pComparatorView->setModel(&m_comparatorTable);

	QSize cellSize = QFontMetrics(font()).size(Qt::TextSingleLine,"A");
	m_pComparatorView->verticalHeader()->setDefaultSectionSize(cellSize.height());

	m_pComparatorView->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_pComparatorView->setWordWrap(false);

	for(int column = 0; column < PR_COMPARATOR_LIST_COLUMN_COUNT; column++)
	{
		m_pComparatorView->setColumnWidth(column, PrComparatorListColumnWidth[column]);
	}

	createContextMenu();

	// load comparators
	//
	std::vector<std::shared_ptr<Metrology::ComparatorEx>> comparatorList;

	int comparatorCount = m_param.comparatorCount();
	for (int c = 0; c < comparatorCount; c++)
	{
		std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_param.comparator(c);
		if (comparatorEx == nullptr)
		{
			continue;
		}

		comparatorList.push_back(comparatorEx);

		if (comparatorEx->compare().isConst() == false)
		{
			Metrology::Signal* pCmpSignal = comparatorEx->compareSignal();
			if (pCmpSignal != nullptr && pCmpSignal->param().isValid() == true)
			{
				m_requestStateList.insert(pCmpSignal->param().hash());
			}
		}

		if (comparatorEx->hysteresis().isConst() == false)
		{
			Metrology::Signal* pHysSignal = comparatorEx->hysteresisSignal();
			if (pHysSignal != nullptr && pHysSignal->param().isValid() == true)
			{
				m_requestStateList.insert(pHysSignal->param().hash());
			}
		}

		Metrology::Signal* pOutSignal = comparatorEx->outputSignal();
		if (pOutSignal != nullptr && pOutSignal->param().isValid() == true)
		{
			m_requestStateList.insert(pOutSignal->param().hash());
		}
	}

	m_comparatorTable.set(comparatorList);
	theSignalBase.appendHashForRequestState(m_requestStateList);

	// start timer for updating comparator state
	//
	m_updateComparatorStateTimer = new QTimer(this);
	connect(m_updateComparatorStateTimer, &QTimer::timeout, this, &DialogSignalProperty::updateComparatorState);
	m_updateComparatorStateTimer->start(theOptions.comparatorInfo().timeForUpdate());

	// add tab items
	//
	m_pTab->addTab(m_pPropertyEditor, tr("Signal"));
	m_pTab->addTab(m_pComparatorView, tr("Comparators"));

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogSignalProperty::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogSignalProperty::onCancel);

	// add layouts
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pTab);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		auto propertyLEl = properties->propertyByCaption(DialogSignalProperty::tr("Electric low limit"));
		if (propertyLEl != nullptr)
		{
			propertyLEl->setPrecision(m_param.electricPrecision());
		}

		auto propertyHEl = properties->propertyByCaption(DialogSignalProperty::tr("Electric high limit"));
		if (propertyHEl != nullptr)
		{
			propertyHEl->setPrecision(m_param.electricPrecision());
		}

		auto propertyLEn = properties->propertyByCaption(DialogSignalProperty::tr("Engineering low limit"));
		if (propertyLEn != nullptr)
		{
			propertyLEn->setPrecision(m_param.decimalPlaces());
		}

		auto propertyHEn = properties->propertyByCaption(DialogSignalProperty::tr("Engineering high limit"));
		if (propertyHEn != nullptr)
		{
			propertyHEn->setPrecision(m_param.decimalPlaces());
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onContextMenu(QPoint)
{
	m_pContextMenu->exec(QCursor::pos());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onCopy()
{
	CopyData copyData(m_pComparatorView, false);
	copyData.exec();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onCopyCell()
{
	if (m_pComparatorView == nullptr)
	{
		return;
	}

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(m_pComparatorView->model()->data(m_pComparatorView->currentIndex()).toString());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onComparatorProperty()
{
	int index = m_pComparatorView->currentIndex().row();
	if (index < 0 || index >= m_comparatorTable.count())
	{
		return;
	}

	std::shared_ptr<Metrology::ComparatorEx> comparatorEx = m_comparatorTable.at(index);
	if (comparatorEx == nullptr)
	{
		return;
	}

	DialogComparatorProperty dialog(*comparatorEx, this);
	int result = dialog.exec();
	if (result != QDialog::Accepted)
	{
		return;
	}

	*comparatorEx = dialog.comparator();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::updateComparatorState()
{
	m_comparatorTable.updateColumn(PR_COMPARATOR_LIST_COLUMN_SETPOINT);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onOk()
{
	if (m_param.isValid() == false)
	{
		assert(false);
		return;
	}

	theSignalBase.setSignalParam(m_param.hash(), m_param);

	theSignalBase.removeLastHashForRequestState(TO_INT(m_requestStateList.size()));

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::onCancel()
{
	theSignalBase.removeLastHashForRequestState(TO_INT(m_requestStateList.size()));

	reject();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogSignalProperty::closeEvent(QCloseEvent* e)
{
	theSignalBase.removeLastHashForRequestState(TO_INT(m_requestStateList.size()));

	QDialog::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalPropertyCategoryCaption(SignalPropertyCategory category)
{
	QString caption;

	switch (category)
	{
		case SignalPropertyCategory::SignalID:			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Signal ID");			break;
		case SignalPropertyCategory::SignalPosition:	caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Position");			break;
		case SignalPropertyCategory::ElectricLimit:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Electric range");		break;
		case SignalPropertyCategory::EngineeringLimit:	caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Engineering range");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Unknown");
	}

	return qApp->translate("DialogObjectProperty", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogComparatorProperty::DialogComparatorProperty(const Metrology::ComparatorEx& comparatorEx, QWidget* parent) :
	QDialog(parent)
{
	m_comparatorEx = comparatorEx;

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogComparatorProperty::~DialogComparatorProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogComparatorProperty::PropertyPattern::PropertyPattern(Metrology::ComparatorEx* pObject) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	QString electricUnit, engineeringUnit;

	QString categorySchemaID = ComparatorPropertyCategoryCaption(ComparatorPropertyCategory::Schema);

	ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("SchemaID"), true, m_pObject->Metrology::ComparatorEx::schemaID)
		->setCategory(categorySchemaID)
		.setViewOrder(0)
		.setReadOnly(true);


	QString categoryInput = ComparatorPropertyCategoryCaption(ComparatorPropertyCategory::Input);

	if (m_pObject->inputSignal() == nullptr)
	{
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - in"), true, m_pObject->inputSignal()->param().appSignalID)
			->setCategory(categoryInput)
			.setViewOrder(0)
			.setReadOnly(true);
	}
	else
	{
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("SignalID - in"), true, m_pObject->inputSignal()->param().customAppSignalID)
			->setCategory(categoryInput)
			.setViewOrder(0)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - in"), true, m_pObject->inputSignal()->param().appSignalID)
			->setCategory(categoryInput)
			.setViewOrder(1)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("EquipmentID - in"), true, m_pObject->inputSignal()->param().equipmentID)
			->setCategory(categoryInput)
			.setViewOrder(2)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Caption - in"), true, m_pObject->inputSignal()->param().caption)
			->setCategory(categoryInput)
			.setViewOrder(3)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Signal type - in"), true, m_pObject->inputSignal()->param().signalTypeStr)
			->setCategory(categoryInput)
			.setViewOrder(4)
			.setReadOnly(true);

		if (m_pObject->inputSignal()->param().isInput() == true && m_pObject->inputSignal()->param().electricRangeIsValid() == true)
		{
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Electric range - in"), true, m_pObject->inputSignal()->param().electricRangeStr)
				->setCategory(categoryInput)
				.setViewOrder(5)
				.setReadOnly(true);
		}

		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Engineering range - in"), true, m_pObject->inputSignal()->param().engineeringRangeStr)
			->setCategory(categoryInput)
			.setViewOrder(6)
			.setReadOnly(true);

		electricUnit = m_pObject->inputSignal()->param().electricUnitStr();
		if(electricUnit.isEmpty() == false)
		{
			electricUnit.insert(0, " ,");
		}

		engineeringUnit = m_pObject->inputSignal()->param().unit();
		if(engineeringUnit.isEmpty() == false)
		{
			engineeringUnit.insert(0, " ,");
		}
	}


	QString categoryCompare = ComparatorPropertyCategoryCaption(ComparatorPropertyCategory::Comapre);

	ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Compare to"), true, PropertyPattern::comapreTo)
		->setCategory(categoryCompare)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER_SETTER(E::CmpType, DialogComparatorProperty::tr("Type"), true, m_pObject->Metrology::ComparatorEx::cmpType, m_pObject->Metrology::ComparatorEx::setCmpType)
		->setCategory(categoryCompare)
		.setViewOrder(1);

	if (m_pObject->compare().isConst() == true)
	{
		if (m_pObject->inputSignal() != nullptr)
		{
			if (m_pObject->inputSignal()->param().isInput() == true && m_pObject->inputSignal()->param().electricRangeIsValid() == true)
			{
				ADD_PROPERTY_GETTER(double, DialogComparatorProperty::tr("Electric value") + electricUnit, true, PropertyPattern::electricConstValue)
					->setCategory(categoryCompare)
					.setViewOrder(2)
					.setReadOnly(true)
					.setPrecision( m_pObject->inputSignal()->param().electricPrecision());
			}
		}

		ADD_PROPERTY_GETTER_SETTER(double, DialogComparatorProperty::tr("Engineering value") + engineeringUnit, true, m_pObject->Metrology::ComparatorEx::compareConstValue, m_pObject->compare().setConstValue)
			->setCategory(categoryCompare)
			.setViewOrder(3)
			.setReadOnly(m_pObject->deviation() != Metrology::ComparatorEx::DeviationType::Unused)
			.setPrecision(m_pObject->valuePrecision());
	}
	else
	{
		if (m_pObject->compareSignal() == nullptr)
		{
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - cmp"), true, m_pObject->compareSignal()->param().appSignalID)
				->setCategory(categoryCompare)
				.setViewOrder(2)
				.setReadOnly(true);
		}
		else
		{
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("SignalID - cmp"), true, m_pObject->compareSignal()->param().customAppSignalID)
				->setCategory(categoryCompare)
				.setViewOrder(2)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - cmp"), true, m_pObject->compareSignal()->param().appSignalID)
				->setCategory(categoryCompare)
				.setViewOrder(3)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("EquipmentID - cmp"), true, m_pObject->compareSignal()->param().equipmentID)
				->setCategory(categoryCompare)
				.setViewOrder(4)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Caption  - cmp"), true, m_pObject->compareSignal()->param().caption)
				->setCategory(categoryCompare)
				.setViewOrder(5)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Signal type - cmp"), true, m_pObject->compareSignal()->param().signalTypeStr)
				->setCategory(categoryCompare)
				.setViewOrder(6)
				.setReadOnly(true);

			if (m_pObject->compareSignal()->param().isInput() == true && m_pObject->compareSignal()->param().electricRangeIsValid() == true)
			{
				ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Electric range - cmp"), true, m_pObject->compareSignal()->param().electricRangeStr)
					->setCategory(categoryCompare)
					.setViewOrder(7)
					.setReadOnly(true);
			}

			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Engineering range - cmp"), true, m_pObject->compareSignal()->param().engineeringRangeStr)
				->setCategory(categoryCompare)
				.setViewOrder(8)
				.setReadOnly(true);
		}
	}

	if (m_pObject->inAnalogSignalFormat() == E::AnalogAppSignalFormat::Float32)
	{
		ADD_PROPERTY_GETTER_SETTER(int, DialogComparatorProperty::tr("Precision"), true, m_pObject->Metrology::ComparatorEx::precision, m_pObject->Metrology::ComparatorEx::setPrecision)
			->setCategory(categoryCompare)
			.setViewOrder(9);
	}


	QString categoryHysteresis = ComparatorPropertyCategoryCaption(ComparatorPropertyCategory::Hysteresis);

	if (m_pObject->hysteresis().isConst() == true)
	{
		ADD_PROPERTY_GETTER_SETTER(double, DialogComparatorProperty::tr("Engineering value - hyst") + engineeringUnit, true, m_pObject->Metrology::ComparatorEx::hysteresisOnlineValue, m_pObject->hysteresis().setConstValue)
			->setCategory(categoryHysteresis)
			.setViewOrder(0)
			.setPrecision(m_pObject->valuePrecision())
			.setReadOnly(m_pObject->deviation() != Metrology::ComparatorEx::DeviationType::Unused);
	}
	else
	{
		if (m_pObject->hysteresisSignal() == nullptr)
		{
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - hyst"), true, m_pObject->hysteresisSignal()->param().appSignalID)
				->setCategory(categoryHysteresis)
				.setViewOrder(0)
				.setReadOnly(true);
		}
		else
		{
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("SignalID - hyst"), true, m_pObject->hysteresisSignal()->param().customAppSignalID)
				->setCategory(categoryHysteresis)
				.setViewOrder(0)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - hyst"), true, m_pObject->hysteresisSignal()->param().appSignalID)
				->setCategory(categoryHysteresis)
				.setViewOrder(3)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("EquipmentID - hyst"), true, m_pObject->hysteresisSignal()->param().equipmentID)
				->setCategory(categoryHysteresis)
				.setViewOrder(4)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Caption  - hyst"), true, m_pObject->hysteresisSignal()->param().caption)
				->setCategory(categoryHysteresis)
				.setViewOrder(5)
				.setReadOnly(true);
			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Signal type - hyst"), true, m_pObject->hysteresisSignal()->param().signalTypeStr)
				->setCategory(categoryHysteresis)
				.setViewOrder(6)
				.setReadOnly(true);

			if (m_pObject->hysteresisSignal()->param().isInput() == true && m_pObject->hysteresisSignal()->param().electricRangeIsValid() == true)
			{
				ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Electric range - hyst"), true, m_pObject->hysteresisSignal()->param().electricRangeStr)
					->setCategory(categoryHysteresis)
					.setViewOrder(7)
					.setReadOnly(true);
			}

			ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Engineering range - hyst"), true, m_pObject->hysteresisSignal()->param().engineeringRangeStr)
				->setCategory(categoryHysteresis)
				.setViewOrder(8)
				.setReadOnly(true);
		}
	}


	QString categoryOutput = ComparatorPropertyCategoryCaption(ComparatorPropertyCategory::Output);

	if (m_pObject->outputSignal() == nullptr)
	{
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - out"), true, m_pObject->outputSignal()->param().appSignalID)
			->setCategory(categoryOutput)
			.setViewOrder(0)
			.setReadOnly(true);
	}
	else
	{
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("SignalID - out"), true, m_pObject->outputSignal()->param().customAppSignalID)
			->setCategory(categoryOutput)
			.setViewOrder(0)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("AppSignalID - out"), true, m_pObject->outputSignal()->param().appSignalID)
			->setCategory(categoryOutput)
			.setViewOrder(1)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("EquipmentID - out"), true, m_pObject->outputSignal()->param().equipmentID)
			->setCategory(categoryOutput)
			.setViewOrder(2)
			.setReadOnly(true);
		ADD_PROPERTY_GETTER(QString, DialogComparatorProperty::tr("Caption - out"), true, m_pObject->outputSignal()->param().caption)
			->setCategory(categoryOutput)
			.setViewOrder(3)
			.setReadOnly(true);
	}
}

// -------------------------------------------------------------------------------------------------------------------

QString DialogComparatorProperty::PropertyPattern::comapreTo()
{
	if (m_pObject == nullptr)
	{
		return QString();
	}

	if (m_pObject->compare().isConst() == false)
	{
		return DialogComparatorProperty::tr("Signal");
	}

	return DialogComparatorProperty::tr("Value");
}

// -------------------------------------------------------------------------------------------------------------------

double DialogComparatorProperty::PropertyPattern::electricConstValue()
{
	if (m_pObject == nullptr)
	{
		return 0;
	}

	if (m_pObject->inputSignal() == nullptr)
	{
		return 0;
	}

	if (m_pObject->inputSignal()->param().isValid() == false)
	{
		return 0;
	}

	UnitsConverter uc;

	return uc.conversion(m_pObject->compareConstValue(), UnitsConvertType::PhysicalToElectric, m_pObject->inputSignal()->param());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.45));
	move(screen.center() - rect().center());

	setWindowTitle(tr("Property of comparator"));

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(250);
	m_pPropertyEditor->setReadOnly(false);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogComparatorProperty::onPropertyValueChanged);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(&m_comparatorEx);
	propertyObjects.push_back(property);

    for(int categoryIndex = 0; categoryIndex < ComparatorPropertyCategoryCount; categoryIndex++)
	{
        ComparatorPropertyCategory category = static_cast<ComparatorPropertyCategory>(categoryIndex);
        if (ERR_COMPARATOR_PROPERTY_CATEGORY(category) == true)
		{
			continue;
		}

        m_pPropertyEditor->setCategoryViewOrder(ComparatorPropertyCategoryCaption(category), categoryIndex);
	}

	m_pPropertyEditor->setObjects(propertyObjects);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogComparatorProperty::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogComparatorProperty::reject);

	// add layouts
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pPropertyEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorProperty::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	QString strEngineeringvalue = DialogSignalProperty::tr("Engineering value");

	if (m_comparatorEx.inputSignal() != nullptr && m_comparatorEx.inputSignal()->param().isValid() == true)
	{
		QString engineeringUnit = m_comparatorEx.inputSignal()->param().unit();
		if(engineeringUnit.isEmpty() == false)
		{
			engineeringUnit.insert(0, " ,");
			strEngineeringvalue.append(engineeringUnit);
		}
	}

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		auto propertyEnV = properties->propertyByCaption(strEngineeringvalue);
		if (propertyEnV != nullptr)
		{
			propertyEnV->setPrecision(m_comparatorEx.valuePrecision());
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogComparatorProperty::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------

QString ComparatorPropertyCategoryCaption(ComparatorPropertyCategory category)
{
	QString caption;

	switch (category)
	{
		case ComparatorPropertyCategory::Schema:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Schema");		break;
		case ComparatorPropertyCategory::Input:			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Input");		break;
		case ComparatorPropertyCategory::Comapre:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Compare");		break;
		case ComparatorPropertyCategory::Hysteresis:	caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Hysteresis");	break;
		case ComparatorPropertyCategory::Output:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Output");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Unknown");
	}

	return qApp->translate("DialogObjectProperty", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DialogMeasureProperty::DialogMeasureProperty(Measure::Item* pMeasurement, QWidget* parent) :
	QDialog(parent),
	m_pMeasurement(pMeasurement)

{
	if (pMeasurement == nullptr)
	{
		assert(false);
		return;
	}

	createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

DialogMeasureProperty::~DialogMeasureProperty()
{
}

// -------------------------------------------------------------------------------------------------------------------

DialogMeasureProperty::PropertyPattern::PropertyPattern(Measure::Item* pObject) : m_pObject(pObject)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	QString categorySignalID = MeasurePropertyCategoryCaption(MeasurePropertyCategory::MeasureID);

	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("SignalID"), true, m_pObject->Measure::Item::customAppSignalID)
		->setCategory(categorySignalID)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("AppSignalID"), true, m_pObject->Measure::Item::appSignalID)
		->setCategory(categorySignalID)
		.setViewOrder(1)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("EquipmentID"), true, m_pObject->Measure::Item::equipmentID)
		->setCategory(categorySignalID)
		.setViewOrder(2)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Caption"), true, m_pObject->Measure::Item::caption)
		->setCategory(categorySignalID)
		.setViewOrder(3)
		.setReadOnly(true);


	QString categoryPosition = MeasurePropertyCategoryCaption(MeasurePropertyCategory::MeasurePosition);

	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Module SN"), true, m_pObject->location().Metrology::SignalLocation::moduleSerialNoStr)
		->setCategory(categoryPosition)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Module type"), true, m_pObject->location().Metrology::SignalLocation::moduleCaption)
		->setCategory(categoryPosition)
		.setViewOrder(1)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Rack"), true, m_pObject->location().Metrology::SignalLocation::rackCaption)
		->setCategory(categoryPosition)
		.setViewOrder(2)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogMeasureProperty::tr("Chassis"), true, m_pObject->location().Metrology::SignalLocation::chassis)
		->setCategory(categoryPosition)
		.setViewOrder(3)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogMeasureProperty::tr("Module"), true, m_pObject->location().Metrology::SignalLocation::module)
		->setCategory(categoryPosition)
		.setViewOrder(4)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(int, DialogMeasureProperty::tr("Place"), true, m_pObject->location().Metrology::SignalLocation::place)
		->setCategory(categoryPosition)
		.setViewOrder(5)
		.setReadOnly(true);


	QString categoryLimits = MeasurePropertyCategoryCaption(MeasurePropertyCategory::Limits);

	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Engineering range"), true, PropertyPattern::engineeringLimitStr)
		->setCategory(categoryLimits)
		.setViewOrder(0)
		.setReadOnly(true);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Electric range"), true, PropertyPattern::electricLimitStr)
		->setCategory(categoryLimits)
		.setViewOrder(1)
		.setReadOnly(true);


	QString categoryErrors = MeasurePropertyCategoryCaption(MeasurePropertyCategory::Errors);

	ADD_PROPERTY_GETTER_SETTER(double, DialogMeasureProperty::tr("Limit of error (%)"), true, PropertyPattern::errorLimit, PropertyPattern::setErrorLimit)
		->setCategory(categoryErrors)
		.setViewOrder(0)
		.setPrecision(3);
	ADD_PROPERTY_GETTER(QString, DialogMeasureProperty::tr("Measurement time"), true, m_pObject->Measure::Item::measureTimeStr)
		->setCategory(categoryErrors)
		.setViewOrder(1)
		.setReadOnly(true);
}

QString DialogMeasureProperty::PropertyPattern::engineeringLimitStr()
{
	if (m_pObject == nullptr)
	{
		return QString();
	}

	return m_pObject->limitStr(Measure::LimitType::Engineering);
}

QString DialogMeasureProperty::PropertyPattern::electricLimitStr()
{
	if (m_pObject == nullptr)
	{
		return QString();
	}

	return m_pObject->limitStr(Measure::LimitType::Electric);
}

double DialogMeasureProperty::PropertyPattern::errorLimit()
{
	if (m_pObject == nullptr)
	{
		return 0;
	}

	return m_pObject->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce);
}

void DialogMeasureProperty::PropertyPattern::setErrorLimit(double value)
{
	if (m_pObject == nullptr)
	{
		return;
	}

	return m_pObject->calcErrorLimit(value);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMeasureProperty::createPropertyList()
{
	setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Property.png"));
	setWindowTitle(tr("Property"));

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.25));
	resize(static_cast<int>(screen.width() * 0.3), static_cast<int>(screen.height() * 0.38));
	move(screen.center() - rect().center());

	if (m_pMeasurement == nullptr)
	{
		assert(0);
		return;
	}

	setWindowTitle(tr("Property of measurement - %1").arg(m_pMeasurement->customAppSignalID()));

	// create property list
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(300);
	m_pPropertyEditor->setReadOnly(false);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogMeasureProperty::onPropertyValueChanged);

	//
	//
	QList<std::shared_ptr<PropertyObject>> propertyObjects;
	std::shared_ptr<PropertyPattern> property = std::make_shared<PropertyPattern>(m_pMeasurement);
	propertyObjects.push_back(property);

    for(int categoryIndex = 0; categoryIndex < MeasurePropertyCategoryCount; categoryIndex++)
	{
        MeasurePropertyCategory category = static_cast<MeasurePropertyCategory>(categoryIndex);
        if (ERR_MEASURE_PROPERTY_CATEGORY(category) == true)
		{
			continue;
		}

        m_pPropertyEditor->setCategoryViewOrder(MeasurePropertyCategoryCaption(category), categoryIndex);
	}

	m_pPropertyEditor->setObjects(propertyObjects);

	// create buttons ok and cancel
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &DialogMeasureProperty::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &DialogMeasureProperty::reject);

	// add layouts
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addWidget(m_pPropertyEditor);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogMeasureProperty::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		if (modifiedFilter.get() == nullptr)
		{
			assert(0);
			continue;
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasurePropertyCategoryCaption(MeasurePropertyCategory category)
{
	QString caption;

	switch (category)
	{
		case MeasurePropertyCategory::MeasureID:		caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Signal ID");	break;
		case MeasurePropertyCategory::MeasurePosition:	caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Position");	break;
		case MeasurePropertyCategory::Limits:			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Limits");		break;
		case MeasurePropertyCategory::Errors:			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Errors");		break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogObjectProperty", "Unknown");
	}

	return qApp->translate("DialogObjectProperty", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

