#include <QTreeWidget>
#include <QMessageBox>
#include "SpecificPropertiesEditor.h"

//
// SpecificPropertyDescription
//
SpecificPropertyDescription::SpecificPropertyDescription()
{
	// Add editable properties

	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, SpecificPropertyDescription::caption, SpecificPropertyDescription::setCaption);
	ADD_PROPERTY_GETTER_SETTER(QString, "Category", true, SpecificPropertyDescription::category, SpecificPropertyDescription::setCategory);
	ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, SpecificPropertyDescription::description, SpecificPropertyDescription::setDescription);

	ADD_PROPERTY_GETTER_SETTER(E::SpecificPropertyType, "Type", true, SpecificPropertyDescription::type, SpecificPropertyDescription::setType);
	ADD_PROPERTY_GETTER_SETTER(QString, "TypeDynamicEnum", false, SpecificPropertyDescription::typeDynamicEnum, SpecificPropertyDescription::setTypeDynamicEnum);
	ADD_PROPERTY_GETTER_SETTER(QString, "LowLimit", true, SpecificPropertyDescription::lowLimit, SpecificPropertyDescription::setLowLimit);
	ADD_PROPERTY_GETTER_SETTER(QString, "HighLimit", true, SpecificPropertyDescription::highLimit, SpecificPropertyDescription::setHighLimit);
	ADD_PROPERTY_GETTER_SETTER(QString, "Default", true, SpecificPropertyDescription::defaultValue, SpecificPropertyDescription::setDefaultValue);
	ADD_PROPERTY_GETTER_SETTER(int, "Precision", true, SpecificPropertyDescription::precision, SpecificPropertyDescription::setPrecision);

	auto propBool = ADD_PROPERTY_GETTER_SETTER(bool, "UpdateFromPreset", true, SpecificPropertyDescription::updateFromPreset, SpecificPropertyDescription::setUpdateFromPreset);
	propBool->setCategory(tr("Flags"));
	propBool = ADD_PROPERTY_GETTER_SETTER(bool, "Expert", true, SpecificPropertyDescription::expert, SpecificPropertyDescription::setExpert);
	propBool->setCategory(tr("Flags"));
	propBool = ADD_PROPERTY_GETTER_SETTER(bool, "Visible", true, SpecificPropertyDescription::visible, SpecificPropertyDescription::setVisible);
	propBool->setCategory(tr("Flags"));

	auto propSpecific = ADD_PROPERTY_GETTER_SETTER(E::PropertySpecificEditor, "SpecificEditor", true, SpecificPropertyDescription::specificEditor, SpecificPropertyDescription::setSpecificEditor);
	propSpecific->setCategory(tr("Miscellaneous"));
}

SpecificPropertyDescription::SpecificPropertyDescription(const SpecificPropertyDescription& source) : SpecificPropertyDescription()
{
	m_caption = source.m_caption;
	m_category = source.m_category;
	m_description = source.m_description;
	m_type = source.m_type;
	m_typeDynamicEnum = source.m_typeDynamicEnum;
	m_lowLimit = source.m_lowLimit;
	m_highLimit = source.m_highLimit;
	m_defaultValue = source.m_defaultValue;
	m_precision = source.m_precision;
	m_updateFromPreset = source.m_updateFromPreset;
	m_expert = source.m_expert;
	m_visible = source.m_visible;
	m_specificEditor = source.m_specificEditor;
}


QString SpecificPropertyDescription::caption() const
{
	return m_caption;
}

void SpecificPropertyDescription::setCaption(const QString& value)
{
	m_caption = value;
}

QString SpecificPropertyDescription::category() const
{
	return m_category;
}

void SpecificPropertyDescription::setCategory(const QString& value)
{
	m_category = value;
}


QString SpecificPropertyDescription::description() const
{
	return m_description;
}

void SpecificPropertyDescription::setDescription(const QString& value)
{
	m_description = value;
}


E::SpecificPropertyType SpecificPropertyDescription::type() const
{
	return m_type;
}

void SpecificPropertyDescription::setType(E::SpecificPropertyType value)
{
	m_type = value;
}


QString SpecificPropertyDescription::typeDynamicEnum() const
{
	return m_typeDynamicEnum;
}

void SpecificPropertyDescription::setTypeDynamicEnum(const QString& value)
{
	m_typeDynamicEnum = value;
}

QString SpecificPropertyDescription::lowLimit() const
{
	return m_lowLimit;
}

void SpecificPropertyDescription::setLowLimit(const QString& value)
{
	m_lowLimit = value;
}


QString SpecificPropertyDescription::highLimit() const
{
	return m_highLimit;
}

void SpecificPropertyDescription::setHighLimit(const QString& value)
{
	m_highLimit = value;
}


QString SpecificPropertyDescription::defaultValue() const
{
	return m_defaultValue;
}

void SpecificPropertyDescription::setDefaultValue(const QString& value)
{
	m_defaultValue = value;
}


int SpecificPropertyDescription::precision() const
{
	return m_precision;
}

void SpecificPropertyDescription::setPrecision(int value)
{
	m_precision = value;
}


bool SpecificPropertyDescription::updateFromPreset() const
{
	return m_updateFromPreset;
}

void SpecificPropertyDescription::setUpdateFromPreset(bool value)
{
	m_updateFromPreset = value;
}

bool SpecificPropertyDescription::expert() const
{
	return m_expert;
}

void SpecificPropertyDescription::setExpert(bool value)
{
	m_expert = value;
}


bool SpecificPropertyDescription::visible() const
{
	return m_visible;
}

void SpecificPropertyDescription::setVisible(bool value)
{
	m_visible = value;
}


E::PropertySpecificEditor SpecificPropertyDescription::specificEditor() const
{
	return m_specificEditor;
}

void SpecificPropertyDescription::setSpecificEditor(E::PropertySpecificEditor value)
{
	m_specificEditor = value;
}

void SpecificPropertyDescription::validateDynamicEnumType(QWidget* parent)
{
	// Show/hide dynamic enum property

	std::shared_ptr<Property> p = propertyByCaption("TypeDynamicEnum");
	if (p == nullptr)
	{
		assert(p);
		return;
	}

	bool dynamicEnumVisible = type() == E::SpecificPropertyType::pt_dynamicEnum;

	if (p->visible() != dynamicEnumVisible)
	{
		if (dynamicEnumVisible == true && typeDynamicEnum().isEmpty() == true)
		{
			// Create an example on showing

			setTypeDynamicEnum("A=1, B=2");
			setDefaultValue("A");
		}

		p->setVisible(dynamicEnumVisible);

		emit propertyListChanged();
	}

	// Validate TypeDynamicEnum property

	if (dynamicEnumVisible == true)
	{
		bool propertyOk = false;

		QString strType = tr("DynamicEnum [%1]").arg(typeDynamicEnum());

		PropertyObject::parseSpecificPropertyTypeDynamicEnum(strType, &propertyOk);

		if (propertyOk == false)
		{
			QString message = tr("SpecificProperties: Dynamic enum property error: %1").arg(strType);
			QMessageBox::critical(parent, qAppName(), message);
		}
	}
}

//
// SpecificPropertiesEditor
//

SpecificPropertiesEditor::SpecificPropertiesEditor(QWidget* parent):
	PropertyTextEditor(parent)
{
	m_hasOkCancelButtons = false;

	// Create property list

	m_propertiesList = new QTreeWidget();

	QStringList l;
	l << tr("Caption");
	l << tr("Type");
	l << tr("Category");

	m_propertiesList->setColumnCount(l.size());
	m_propertiesList->setHeaderLabels(l);

	int il = 0;
	m_propertiesList->setColumnWidth(il++, 140);
	m_propertiesList->setSortingEnabled(true);
	m_propertiesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_propertiesList->setContextMenuPolicy(Qt::CustomContextMenu);
	m_propertiesList->setRootIsDecorated(false);

	connect(m_propertiesList, &QTreeWidget::itemSelectionChanged, this, &SpecificPropertiesEditor::onTreeSelectionChanged);

	// Create property editor

	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	m_propertyEditor->setSplitterPosition(200);
	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SpecificPropertiesEditor::onPropertiesChanged);

	// Buttons

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	m_addButton = new QPushButton(tr("Add Property"));
	connect(m_addButton, &QPushButton::clicked, this, &SpecificPropertiesEditor::onAddProperty);
	buttonLayout->addWidget(m_addButton);

	m_cloneButton = new QPushButton(tr("Clone Property"));
	m_cloneButton->setEnabled(false);
	connect(m_cloneButton, &QPushButton::clicked, this, &SpecificPropertiesEditor::onCloneProperty);
	buttonLayout->addWidget(m_cloneButton);

	m_removeButton = new QPushButton(tr("Remove Property"));
	m_removeButton->setEnabled(false);
	connect(m_removeButton, &QPushButton::clicked, this, &SpecificPropertiesEditor::onRemoveProperties);
	buttonLayout->addWidget(m_removeButton);

	buttonLayout->addStretch();

	QPushButton* okButton = new QPushButton(tr("OK"));
	okButton->setDefault(true);
	connect(okButton, &QPushButton::clicked, this, &SpecificPropertiesEditor::onOkClicked);
	buttonLayout->addWidget(okButton);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::clicked, this, &SpecificPropertiesEditor::onCancelClicked);
	buttonLayout->addWidget(cancelButton);

	// Top Layout

	QHBoxLayout* topLayout = new QHBoxLayout();
	topLayout->addWidget(m_propertiesList);
	topLayout->addWidget(m_propertyEditor);
	topLayout->setContentsMargins(0, 0, 0, 0);

	// Main Layout

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(buttonLayout);
	mainLayout->setContentsMargins(0, 0, 0, 0);
}

SpecificPropertiesEditor::~SpecificPropertiesEditor()
{

}

void SpecificPropertiesEditor::setText(const QString& text)
{

	static_assert(PropertyObject::m_lastSpecificPropertiesVersion >= 1 && PropertyObject::m_lastSpecificPropertiesVersion <= 4);	// Editor must be reviewed if version is raised

	m_propertiesList->clear();
	m_propertyDescriptionsMap.clear();

	QStringList rows = text.split(QChar::LineFeed, QString::SkipEmptyParts);

	for (QString row : rows)
	{
		row = row.trimmed();
		if (row.isEmpty() == true)
		{
			continue;
		}

		std::shared_ptr<SpecificPropertyDescription> spd = std::make_shared<SpecificPropertyDescription>();

		// Initialize fields

		QStringList columns = row.split(';');

		for (QString& col : columns)
		{
			col = col.trimmed();
		}

		QString strVersion(columns[0]);
		bool ok = false;
		int version = strVersion.toInt(&ok);

		if (version < 1 || version > PropertyObject::m_lastSpecificPropertiesVersion)
		{
			QString message = tr("SpecificProperties: Specific property version is not recognized: %1").arg(version);
			QMessageBox::critical(this, qAppName(), message);
			return;
		}

		if (version >= 1)
		{
			if (columns.size() < 9)
			{
				QString message = tr("SpecificProperties: Invalid specific property format: %1").arg(row);
				QMessageBox::critical(this, qAppName(), message);
				return;
			}

			QString strType = columns[3];

			//

			bool startedFromDynamicEnum = strType.trimmed().startsWith(QLatin1String("DynamicEnum"), Qt::CaseInsensitive);
			int openBrace = strType.indexOf('[');
			int closeBrace = strType.lastIndexOf(']');

			if (openBrace != -1 &&
					closeBrace != -1 &&
					openBrace < closeBrace &&
					startedFromDynamicEnum == true)
			{
				QString valuesString = strType.mid(openBrace + 1, closeBrace - openBrace - 1);
				valuesString.remove(' ');

				spd->setTypeDynamicEnum(valuesString);
				spd->setType(E::SpecificPropertyType::pt_dynamicEnum);
			}
			else
			{
				auto [propertyType, propertyOk] = PropertyObject::parseSpecificPropertyType(strType);

				if (propertyOk == false)
				{
					QString message = tr("SpecificProperties: Specific property type is not recognized: %1").arg(strType);
					QMessageBox::critical(this, qAppName(), message);
					return;
				}

				spd->setType(propertyType);
			}

			spd->setCaption(columns[1]);
			spd->setCategory(columns[2]);
			spd->setLowLimit(columns[4]);
			spd->setHighLimit(columns[5]);
			spd->setDefaultValue(columns[6]);
			spd->setPrecision(columns[7].toInt());
			spd->setUpdateFromPreset(columns[8].compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
		}

		if (version >= 2)
		{
			if (columns.size() < 11)
			{
				QString message = tr("SpecificProperties: Invalid specific property format: %1").arg(row);
				QMessageBox::critical(this, qAppName(), message);
				return;
			}
			spd->setExpert(columns[9].compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
			spd->setDescription(columns[10]);
		}

		if (version >= 3)
		{
			if (columns.size() < 12)
			{
				QString message = tr("SpecificProperties: Invalid specific property format: %1").arg(row);
				QMessageBox::critical(this, qAppName(), message);
				return;
			}
			spd->setVisible(columns[11].compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
		}

		if (version >= 4)
		{
			if (columns.size() < 13)
			{
				QString message = tr("SpecificProperties: Invalid specific property format: %1").arg(row);
				QMessageBox::critical(this, qAppName(), message);
				return;
			}

			auto[editorType, editorOk] = E::stringToValue<E::PropertySpecificEditor>(columns[12]);
			if (editorOk == true)
			{
				spd->setSpecificEditor(editorType);
			}
			else
			{
				assert(false);
				return;
			}
		}

		// Show/hide dynamic enum property

		spd->validateDynamicEnumType(this);

		// Add the tree item

		QTreeWidgetItem* item = new QTreeWidgetItem();

		updatePropetyListItem(item, spd.get());

		m_propertyDescriptionsMap[item] = spd;

		m_propertiesList->addTopLevelItem(item);
	}

	m_propertiesList->sortByColumn(m_columnCaption, Qt::AscendingOrder);
}


QString SpecificPropertiesEditor::text()
{
	QString result;

	for (int i = 0; i < m_propertiesList->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_propertiesList->topLevelItem(i);
		if (item == nullptr)
		{
			assert(false);
			continue;
		}

		if (m_propertyDescriptionsMap.find(item) == m_propertyDescriptionsMap.end())
		{
			assert(false);
			continue;
		}

		const std::shared_ptr<SpecificPropertyDescription>& spd = m_propertyDescriptionsMap[item];

		QString strType;

		switch (spd->type())
		{
		case E::SpecificPropertyType::pt_int32:			strType = "int32";					break;
		case E::SpecificPropertyType::pt_uint32:		strType = "uint32";					break;
		case E::SpecificPropertyType::pt_double:		strType = "double";					break;
		case E::SpecificPropertyType::pt_string:		strType = "string";					break;
		case E::SpecificPropertyType::pt_bool:			strType = "bool";					break;
		case E::SpecificPropertyType::pt_e_channel:		strType = "E::Channel";				break;
		case E::SpecificPropertyType::pt_dynamicEnum:
		{
			strType = tr("DynamicEnum [%1]").arg(spd->typeDynamicEnum());
			break;
		}
		default:
			assert(false);
		}

		result += PropertyObject::createSpecificPropertyStruct(spd->caption(),
														spd->category(),
														spd->description(),
														strType,
														&spd->lowLimit(),
														&spd->highLimit(),
														&spd->defaultValue(),
														spd->precision(),
														spd->updateFromPreset(),
														spd->expert(),
														spd->visible(),
														spd->specificEditor());
		result += "\r\n";
	}

	return result;
}

void SpecificPropertiesEditor::setReadOnly(bool value)
{
	m_propertyEditor->setReadOnly(value);
	m_removeButton->setEnabled(value == false);
}

void SpecificPropertiesEditor::onTreeSelectionChanged()
{
	bool cloneEnabled = m_propertiesList->selectedItems().size() == 1;
	bool removeEnabled = m_propertiesList->selectedItems().size() > 0;

	if (m_propertyEditor->readOnly() == false)
	{
		m_cloneButton->setEnabled(cloneEnabled);
		m_removeButton->setEnabled(removeEnabled);
	}

	//

	std::vector<std::shared_ptr<PropertyObject>> objects;

	for (QTreeWidgetItem* item : m_propertiesList->selectedItems())
	{

		if (m_propertyDescriptionsMap.find(item) == m_propertyDescriptionsMap.end())
		{
			assert(false);
			return;
		}

		std::shared_ptr<SpecificPropertyDescription> spd = m_propertyDescriptionsMap[item];
		if (spd == nullptr)
		{
			assert(spd);
			return;
		}

		objects.push_back(spd);
	}

	if (objects.empty() == true)
	{
		m_propertyEditor->clear();
		return;
	}

	m_propertyEditor->setObjects(objects);
}

void SpecificPropertiesEditor::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	Q_UNUSED(objects);

	bool updatePropertiesList = false;

	for (QTreeWidgetItem* item : m_propertiesList->selectedItems())
	{
		if (m_propertyDescriptionsMap.find(item) == m_propertyDescriptionsMap.end())
		{
			assert(false);
			return;
		}

		std::shared_ptr<SpecificPropertyDescription> spd = m_propertyDescriptionsMap[item];
		if (spd == nullptr)
		{
			assert(spd);
			return;
		}

		spd->validateDynamicEnumType(this);

		updatePropetyListItem(item, spd.get());
	}

	if (updatePropertiesList == true)
	{

	}
}

void SpecificPropertiesEditor::onAddProperty()
{
	std::shared_ptr<SpecificPropertyDescription> spd = std::make_shared<SpecificPropertyDescription>();

	spd->setCaption(tr("New Property"));
	spd->setType(E::SpecificPropertyType::pt_uint32);
	spd->setVisible(true);

	// Add the tree item

	QTreeWidgetItem* item = new QTreeWidgetItem();

	updatePropetyListItem(item, spd.get());

	m_propertyDescriptionsMap[item] = spd;

	m_propertiesList->addTopLevelItem(item);

	m_propertiesList->clearSelection();

	m_propertiesList->setCurrentItem(item);
}

void SpecificPropertiesEditor::onCloneProperty()
{
	QList<QTreeWidgetItem*> selected = m_propertiesList->selectedItems();

	if (selected.size() != 1)
	{
		return;
	}

	QTreeWidgetItem* sourceItem = selected[0];

	auto it = m_propertyDescriptionsMap.find(sourceItem);

	if (it == m_propertyDescriptionsMap.end())
	{
		assert(false);
		return;
	}

	std::shared_ptr<SpecificPropertyDescription> sourceSpd = it->second;
	if (sourceSpd == nullptr)
	{
		assert(sourceSpd);
		return;
	}

	std::shared_ptr<SpecificPropertyDescription> spd = std::make_shared<SpecificPropertyDescription>(*sourceSpd);
	if (spd == nullptr)
	{
		assert(spd);
		return;
	}

	spd->setCaption(spd->caption() + tr(" - clone"));

	// Add the tree item

	QTreeWidgetItem* item = new QTreeWidgetItem();

	updatePropetyListItem(item, spd.get());

	m_propertyDescriptionsMap[item] = spd;

	m_propertiesList->addTopLevelItem(item);

	m_propertiesList->clearSelection();

	m_propertiesList->setCurrentItem(item);
}

void SpecificPropertiesEditor::onRemoveProperties()
{
	QList<QTreeWidgetItem*> selected = m_propertiesList->selectedItems();

	if (selected.empty() == true)
	{
		return;
	}

	if (QMessageBox::question(this, qAppName(), tr("Are you sure you want to remove selected properties?"), QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	for (QTreeWidgetItem* item : selected)
	{
		if (m_propertyDescriptionsMap.find(item) == m_propertyDescriptionsMap.end())
		{
			assert(false);
			return;
		}

		int index = m_propertiesList->indexOfTopLevelItem(item);
		if (index == -1)
		{
			assert(false);
			return;
		}

		m_propertyDescriptionsMap.erase(item);

		QTreeWidgetItem* itemToDelete = m_propertiesList->takeTopLevelItem(index);
		delete itemToDelete;
	}
}

void SpecificPropertiesEditor::onOkClicked()
{
	okButtonPressed();
}

void SpecificPropertiesEditor::onCancelClicked()
{
	cancelButtonPressed();
}

void SpecificPropertiesEditor::updatePropetyListItem(QTreeWidgetItem* item, SpecificPropertyDescription* spd)
{
	item->setText(m_columnCaption, spd->caption());
	item->setText(m_columnType, E::valueToString<E::SpecificPropertyType>(spd->type()));
	item->setText(m_columnCategory, spd->category());
}
