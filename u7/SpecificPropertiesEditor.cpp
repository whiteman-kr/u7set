#include <QTableView>
#include <QAbstractTableModel>
#include <QMessageBox>
#include "SpecificPropertiesEditor.h"

//
// SpecificPropertyDescription
//
SpecificPropertyDescription::SpecificPropertyDescription()
{
	// Add editable properties
	//
	ADD_PROPERTY_GETTER_SETTER(QString, "Caption", true, SpecificPropertyDescription::caption, SpecificPropertyDescription::setCaption);
	ADD_PROPERTY_GETTER_SETTER(QString, "Category", true, SpecificPropertyDescription::category, SpecificPropertyDescription::setCategory);
	ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, SpecificPropertyDescription::description, SpecificPropertyDescription::setDescription);

	ADD_PROPERTY_GETTER_SETTER(E::SpecificPropertyType, "Type", true, SpecificPropertyDescription::type, SpecificPropertyDescription::setType);
	ADD_PROPERTY_GETTER_SETTER(QString, "TypeDynamicEnum", false, SpecificPropertyDescription::typeDynamicEnum, SpecificPropertyDescription::setTypeDynamicEnum);
	ADD_PROPERTY_GETTER_SETTER(QString, "LowLimit", true, SpecificPropertyDescription::lowLimit, SpecificPropertyDescription::setLowLimit);
	ADD_PROPERTY_GETTER_SETTER(QString, "HighLimit", true, SpecificPropertyDescription::highLimit, SpecificPropertyDescription::setHighLimit);
	ADD_PROPERTY_GETTER_SETTER(QString, "Default", true, SpecificPropertyDescription::defaultValue, SpecificPropertyDescription::setDefaultValue);
	ADD_PROPERTY_GETTER_SETTER(int, "Precision", true, SpecificPropertyDescription::precision, SpecificPropertyDescription::setPrecision);
	ADD_PROPERTY_GETTER_SETTER(int, "ViewOrder", true, SpecificPropertyDescription::viewOrder, SpecificPropertyDescription::setViewOrder);

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

quint16 SpecificPropertyDescription::viewOrder() const
{
	return m_viewOrder;
}

void SpecificPropertyDescription::setViewOrder(quint16 value)
{
	m_viewOrder = value;
}

void SpecificPropertyDescription::validateDynamicEnumType(QWidget* parent)
{
	// Show/hide dynamic enum property
	//
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
			//
			setTypeDynamicEnum("A=1, B=2");
			setDefaultValue("A");
		}

		p->setVisible(dynamicEnumVisible);

		emit propertyListChanged();
	}

	// Validate TypeDynamicEnum property
	//
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

std::tuple<quint16, QString, int, QString> SpecificPropertyDescription::tuple_order() const
{
	return std::make_tuple(m_viewOrder, m_caption, static_cast<int>(m_type), m_category);
}

std::tuple<QString, quint16, int, QString> SpecificPropertyDescription::tuple_caption() const
{
	return std::make_tuple(m_caption, m_viewOrder, static_cast<int>(m_type), m_category);
}

std::tuple<int, quint16, QString, QString> SpecificPropertyDescription::tuple_type() const
{
	return std::make_tuple(static_cast<int>(m_type), m_viewOrder, m_caption, m_category);
}

std::tuple<QString, quint16, QString, int> SpecificPropertyDescription::tuple_category() const
{
	return std::make_tuple(m_category, m_viewOrder, m_caption, static_cast<int>(m_type));
}

//
// SpecificPropertyModel
//

SpecificPropertyModel::SpecificPropertyModel(QObject *parent):
	QAbstractTableModel(parent)
{

}

void SpecificPropertyModel::clear()
{
	m_sortedIndexes.clear();
	m_propertyDescriptions.clear();
}

void SpecificPropertyModel::add(std::shared_ptr<SpecificPropertyDescription> spd)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());

	m_propertyDescriptions.push_back(spd);
	m_sortedIndexes.push_back(static_cast<int>(m_propertyDescriptions.size() - 1));

	endInsertRows();
}

void SpecificPropertyModel::remove(QModelIndexList indexes)
{
	if (indexes.isEmpty() == true)
	{
		assert(false);
		return;
	}

	std::vector<int> rowsToDelete;

	rowsToDelete.reserve(indexes.size());

	for (const QModelIndex& mi : indexes)
	{
		if (mi.row() < 0 || mi.row() >= static_cast<int>(m_sortedIndexes.size()))
		{
			assert(false);
			return;
		}

		rowsToDelete.push_back(m_sortedIndexes[mi.row()]);
	}

	std::sort(rowsToDelete.begin(), rowsToDelete.end(),  std::greater<int>());

	beginRemoveRows(QModelIndex(), rowCount() - indexes.size(), rowCount() - 1);

	for (int row : rowsToDelete)
	{
		if (row < 0 || row >= static_cast<int>(m_propertyDescriptions.size()))
		{
			assert(false);
			return;
		}

		m_propertyDescriptions.erase(m_propertyDescriptions.begin() + row);
	}


	// Rebuild sorted indexes and re-sort them
	//

	m_sortedIndexes.resize(m_propertyDescriptions.size());

	for (int i = 0; i < m_sortedIndexes.size(); i++)
	{
		m_sortedIndexes[i] = i;
	}

	sort(m_sortColumn, m_sortOrder);

	//

	endRemoveRows();

}

std::shared_ptr<SpecificPropertyDescription> SpecificPropertyModel::get(int index) const
{
	if (index < 0 || index >= rowCount())
	{
		assert(false);
		return nullptr;
	}

	int row = m_sortedIndexes[index];

	if (row < 0 || row >= rowCount())
	{
		assert(false);
		return nullptr;
	}

	return m_propertyDescriptions[row];

}

int SpecificPropertyModel::count() const
{
	return rowCount();
}

void SpecificPropertyModel::update()
{
	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

void SpecificPropertyModel::sort(int column, Qt::SortOrder order)
{
	if (m_sortedIndexes.empty() == true)
	{
		return;
	}

	if (column < 0 || column >= SpecificPropertyEditorColumns::Count)
	{
		assert(false);
		return;
	}

	m_sortColumn = column;
	m_sortOrder = order;

	std::sort(m_sortedIndexes.begin(), m_sortedIndexes.end(), SpecificPropertyModelSorter(column, order, &m_propertyDescriptions));

	return;
}

QString SpecificPropertyModel::toText() const
{
	QString result;

	for (const std::shared_ptr<SpecificPropertyDescription>& spd : m_propertyDescriptions)
	{
		if (spd == nullptr)
		{
			assert(spd);
			return QString();
		}

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
														spd->lowLimit(),
														spd->highLimit(),
														spd->defaultValue(),
														spd->precision(),
														spd->updateFromPreset(),
														spd->expert(),
														spd->visible(),
														spd->specificEditor(),
														spd->viewOrder());
		result += "\r\n";
	}

	return result;
}

int SpecificPropertyModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(m_propertyDescriptions.size());
}

int SpecificPropertyModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return static_cast<int>(SpecificPropertyEditorColumns::Count);
}

QVariant SpecificPropertyModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.row() < 0 || index.row() >= static_cast<int>(m_sortedIndexes.size()))
		{
			assert(false);
			return QVariant();
		}

		int row = m_sortedIndexes[index.row()];
		if (row < 0 || row >= rowCount())
		{
			assert(false);
			return QVariant();
		}

		const SpecificPropertyDescription* spd = m_propertyDescriptions[row].get();

		if (spd == nullptr)
		{
			assert(spd);
			return QVariant();
		}

		switch (index.column())
		{
		case SpecificPropertyEditorColumns::ViewOrder:
		{
			return QString::number(spd->viewOrder());
		}
		case SpecificPropertyEditorColumns::Caption:
		{
			return spd->caption();
		}
		case SpecificPropertyEditorColumns::Type:
		{
			return E::valueToString<E::SpecificPropertyType>(spd->type());
		}
		case SpecificPropertyEditorColumns::Category:
		{
			return spd->category();
		}
		default:
			return QVariant();
		}
	}
	return QVariant();
}

QVariant SpecificPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case SpecificPropertyEditorColumns::ViewOrder:
			{
				return "#";
			}
			case SpecificPropertyEditorColumns::Caption:
			{
				return QString("Caption");
			}
			case SpecificPropertyEditorColumns::Type:
			{
				return QString("Type");
			}
			case SpecificPropertyEditorColumns::Category:
			{
				return QString("Category");
			}
			}
		}
	}
	return QVariant();
}

SpecificPropertyModelSorter::SpecificPropertyModelSorter(int column, Qt::SortOrder order, std::vector<std::shared_ptr<SpecificPropertyDescription>>* propertyDescriptions):
	m_column(column),
	m_order(order),
	m_propertyDescriptions(propertyDescriptions)
{

}

bool SpecificPropertyModelSorter::sortFunction(int index1, int index2, int column, Qt::SortOrder order) const
{
	if (m_propertyDescriptions == nullptr)
	{
		assert(m_propertyDescriptions);
		return false;
	}

	if (index1 < 0 || index1 >= static_cast<int>(m_propertyDescriptions->size()) ||
		index2 < 0 || index2 >= static_cast<int>(m_propertyDescriptions->size()))
	{
		assert(false);
		return false;
	}

	const SpecificPropertyDescription* spd1 = m_propertyDescriptions->at(index1).get();

	const SpecificPropertyDescription* spd2 = m_propertyDescriptions->at(index2).get();

	if (spd1 == nullptr || spd2 == nullptr)
	{
		assert(spd1);
		assert(spd2);
		return false;
	}

	if (order == Qt::DescendingOrder)
	{
		std::swap(spd1, spd2);
	}

	switch (column)
	{
	case SpecificPropertyEditorColumns::ViewOrder:
	{
		return spd1->tuple_order() < spd2->tuple_order();
	}
	case SpecificPropertyEditorColumns::Caption:
	{
		return spd1->tuple_caption() < spd2->tuple_caption();
	}
	case SpecificPropertyEditorColumns::Type:
	{
		return spd1->tuple_type() < spd2->tuple_type();
	}
	case SpecificPropertyEditorColumns::Category:
	{
		return spd1->tuple_category() < spd2->tuple_category();
	}
	default:
		assert(false);
		return spd1->tuple_caption() < spd2->tuple_caption();
	}

}

//
// SpecificPropertiesEditor
//
SpecificPropertiesEditor::SpecificPropertiesEditor(QWidget* parent):
	PropertyTextEditor(parent),
	m_propertiesModel(this)
{
	m_hasOkCancelButtons = false;

	// Create property list
	//
	m_propertiesTable = new QTableView();
	m_propertiesTable->setModel(&m_propertiesModel);

	m_propertiesTable->verticalHeader()->hide();
	m_propertiesTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_propertiesTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_propertiesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_propertiesTable->setSortingEnabled(true);
	m_propertiesTable->horizontalHeader()->setStretchLastSection(true);
	m_propertiesTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	connect(m_propertiesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpecificPropertiesEditor::tableSelectionChanged);
	connect(m_propertiesTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &SpecificPropertiesEditor::sortIndicatorChanged);



	// Create property editor
	//
	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	m_propertyEditor->setSplitterPosition(200);
	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &SpecificPropertiesEditor::onPropertiesChanged);

	// Buttons
	//
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
	//
	QHBoxLayout* topLayout = new QHBoxLayout();
	topLayout->addWidget(m_propertiesTable);
	topLayout->addWidget(m_propertyEditor);
	topLayout->setContentsMargins(0, 0, 0, 0);

	// Main Layout
	//
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
	static_assert(PropertyObject::m_lastSpecificPropertiesVersion >= 1 && PropertyObject::m_lastSpecificPropertiesVersion <= 5);	// Editor must be reviewed if version is raised

	m_propertiesModel.clear();

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
		//
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

			// --
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

		if (version >= 5)
		{
			if (columns.size() < 14)
			{
				QString message = tr("SpecificProperties: Invalid specific property format: %1").arg(row);
				QMessageBox::critical(this, qAppName(), message);
				return;
			}

			spd->setViewOrder(columns[13].toInt());
		}

		// Show/hide dynamic enum property
		//
		spd->validateDynamicEnumType(this);

		m_propertiesModel.add(spd);

	}

	m_propertiesTable->setColumnWidth(SpecificPropertyEditorColumns::ViewOrder, 50);
	m_propertiesTable->setColumnWidth(SpecificPropertyEditorColumns::Caption, 200);
	m_propertiesTable->setColumnWidth(SpecificPropertyEditorColumns::Type, 100);
	m_propertiesTable->setColumnWidth(SpecificPropertyEditorColumns::Category, 140);

	// Sort

	m_propertiesTable->horizontalHeader()->blockSignals(true);
	m_propertiesTable->sortByColumn(SpecificPropertyEditorColumns::Category, Qt::AscendingOrder);
	m_propertiesTable->horizontalHeader()->blockSignals(false);
}


QString SpecificPropertiesEditor::text()
{
	return m_propertiesModel.toText();
}

void SpecificPropertiesEditor::setReadOnly(bool value)
{
	m_propertyEditor->setReadOnly(value);
	m_removeButton->setEnabled(value == false);
}

void SpecificPropertiesEditor::tableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	QModelIndexList selection = m_propertiesTable->selectionModel()->selectedRows();

	bool cloneEnabled = selection.size() == 1;
	bool removeEnabled = selection.size() > 0;

	if (m_propertyEditor->readOnly() == false)
	{
		m_cloneButton->setEnabled(cloneEnabled);
		m_removeButton->setEnabled(removeEnabled);
	}

	// --
	//
	std::vector<std::shared_ptr<PropertyObject>> objects;

	for (const QModelIndex& mi : selection)
	{
		std::shared_ptr<SpecificPropertyDescription> spd = m_propertiesModel.get(mi.row());
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

	m_propertiesModel.update();
}

void SpecificPropertiesEditor::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_propertiesTable->clearSelection();

	m_propertiesModel.sort(column, order);

	m_propertiesModel.update();

}

void SpecificPropertiesEditor::onAddProperty()
{
	std::shared_ptr<SpecificPropertyDescription> spd = std::make_shared<SpecificPropertyDescription>();

	spd->setCaption(tr("New Property"));
	spd->setCategory(tr("Common"));
	spd->setType(E::SpecificPropertyType::pt_uint32);
	spd->setVisible(true);

	// Add the tree item
	//

	m_propertiesModel.add(spd);
}

void SpecificPropertiesEditor::onCloneProperty()
{
	QModelIndexList selection = m_propertiesTable->selectionModel()->selectedRows();

	if (selection.size() != 1)
	{
		return;
	}

	std::shared_ptr<SpecificPropertyDescription> sourceSpd = m_propertiesModel.get(selection[0].row());

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
	//

	m_propertiesModel.add(spd);

	// Select it
	//

	QModelIndex lastIndex = m_propertiesModel.index(m_propertiesModel.count() - 1, 0);

	if (lastIndex.isValid() == false)
	{
		assert(false);
		return;
	}

	m_propertiesTable->selectionModel()->select(lastIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void SpecificPropertiesEditor::onRemoveProperties()
{
	QModelIndexList selection = m_propertiesTable->selectionModel()->selectedRows();

	if (selection.isEmpty() == true)
	{
		return;
	}

	m_propertiesModel.remove(selection);

	m_propertiesTable->clearSelection();

}

void SpecificPropertiesEditor::onOkClicked()
{
	okButtonPressed();
}

void SpecificPropertiesEditor::onCancelClicked()
{
	cancelButtonPressed();
}
