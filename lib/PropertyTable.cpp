#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "PropertyTable.h"
#include "TuningValue.h"

#include "Stable.h"

namespace ExtWidgets
{
	PropertyTableItemDelegate::PropertyTableItemDelegate(PropertyTable* propertyTable, PropertyTableModel* model) :
		QItemDelegate(propertyTable),
		m_propertyTable(propertyTable),
		m_model(model)
	{
	}

	QWidget* PropertyTableItemDelegate::createEditor(QWidget *parent,
													 const QStyleOptionViewItem &option,
													 const QModelIndex &index) const
	{
		Q_UNUSED(option);

		if (m_model == nullptr)
		{
			Q_ASSERT(m_model);
			return new QWidget(parent);
		}

		std::shared_ptr<Property> p = m_model->propertyByIndex(index);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return new QWidget(parent);
		}

		m_cellEditor = m_propertyTable->createCellEditor(p, true, p->readOnly() == true || m_propertyTable->isReadOnly() == true, parent);

		connect(m_cellEditor, &PropertyEditCellWidget::valueChanged, this, &PropertyTableItemDelegate::onValueChanged);
		connect(this, &PropertyTableItemDelegate::valueChanged, m_propertyTable, &PropertyTable::onValueChanged);

		return m_cellEditor;
	}

	void PropertyTableItemDelegate::setEditorData(QWidget *editor,
												  const QModelIndex &index) const
	{
		if (m_model == nullptr)
		{
			Q_ASSERT(m_model);
			return;
		}

		std::shared_ptr<Property> p = m_model->propertyByIndex(index);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		PropertyEditCellWidget *cellEditor = dynamic_cast<PropertyEditCellWidget*>(editor);
		if (cellEditor == nullptr)
		{
			Q_ASSERT(cellEditor);
			return;
		}
		cellEditor->setValue(p, p->readOnly() == true || m_propertyTable->isReadOnly() == true);
	}

	void PropertyTableItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		Q_UNUSED(editor);
		Q_UNUSED(model);
		Q_UNUSED(index);

		// This function is called when user press Enter or changes selection
	}

	void PropertyTableItemDelegate::updateEditorGeometry(QWidget *editor,
														 const QStyleOptionViewItem &option,
														 const QModelIndex &index) const
	{
		Q_UNUSED(index);
		editor->setGeometry(option.rect);
	}

	void PropertyTableItemDelegate::onValueChanged(QVariant value)
	{
		emit valueChanged(value);
	}

	//
	// PropertyTableModel
	//


	PropertyTableModel::PropertyTableModel(PropertyTable* propertyTable):
		m_propertyTable(propertyTable)
	{

	}

	PropertyTableModel::~PropertyTableModel()
	{

	}

	void PropertyTableModel::clear()
	{
		if (rowCount() > 0)
		{
			beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
			endRemoveRows();
		}

		if (columnCount() > 0)
		{
			beginRemoveColumns(QModelIndex(), 0, columnCount() - 1);
			endRemoveColumns();
		}

		m_tableObjects.clear();
		m_propertyNames.clear();
	}

	void PropertyTableModel::setTableObjects(std::vector<PropertyTableObject>& tableObjects)
	{
		clear();

		m_tableObjects.clear();
		m_propertyNames.clear();

		if (tableObjects.empty() == false)
		{
			// Construct header names

			const PropertyTableObject& to = tableObjects[0];

			for (auto p : to.properties)
			{
				m_propertyNames.push_back(p->caption());
			}

			beginInsertColumns(QModelIndex(), 0, static_cast<int>(m_propertyNames.size()) - 1);

			endInsertColumns();

			// Copy objects

			int count = static_cast<int>(tableObjects.size());

			beginInsertRows(QModelIndex(), 0, count - 1);

			m_tableObjects.swap(tableObjects);

			endInsertRows();


		}
	}

	std::shared_ptr<PropertyObject> PropertyTableModel::propertyObjectByIndex(const QModelIndex& mi) const
	{
		if (mi.row() < 0 || mi.row() >= static_cast<int>(m_tableObjects.size()))
		{
			Q_ASSERT(false);
			return nullptr;
		}

		const PropertyTableObject& pto = m_tableObjects[mi.row()];

		return pto.propertyObject;
	}

	std::shared_ptr<Property> PropertyTableModel::propertyByIndex(const QModelIndex& mi) const
	{
		if (mi.row() < 0 || mi.row() >= static_cast<int>(m_tableObjects.size()))
		{
			Q_ASSERT(false);
			return nullptr;
		}

		const PropertyTableObject& pto = m_tableObjects[mi.row()];

		if (mi.column() < 0 || mi.column() >= static_cast<int>(pto.properties.size()))
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return pto.properties[mi.column()];
	}

	int PropertyTableModel::rowCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		int result = static_cast<int>(m_tableObjects.size());
		qDebug() << result;
		return result;
	}

	int PropertyTableModel::columnCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		int result = static_cast<int>(m_propertyNames.size());
		qDebug() << result;
		return result;
	}

	QVariant PropertyTableModel::data(const QModelIndex &index, int role) const
	{
		if (role== Qt::DecorationRole)
		{
			std::shared_ptr<Property> p = propertyByIndex(index);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			bool enabled = m_propertyTable->isReadOnly() == false && p->readOnly() == false;

			return PropertyEditorBase::propertyIcon(p.get(), /*sameValue*/true, enabled);
		}

		if (role == Qt::DisplayRole)
		{
			std::shared_ptr<Property> p = propertyByIndex(index);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			return PropertyEditorBase::propertyValueText(p.get());
		}

		return QVariant();
	}

	QVariant PropertyTableModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
	{

		if (role == Qt::DisplayRole)
		{
			if (orientation == Qt::Horizontal)
			{
				if (section < 0 || section >= static_cast<int>(m_propertyNames.size()))
				{
					Q_ASSERT(false);
					return QVariant();
				}
				return m_propertyNames[section];
			}
		}

		return QVariant();
	}

	Qt::ItemFlags PropertyTableModel::flags(const QModelIndex &index) const
	{
		return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
	}


	//
	// PropertyTableView
	//

	void PropertyTableView::closeCurrentEditorIfOpen()
	{
		QWidget* editWidget = indexWidget(currentIndex());
		if (editWidget != nullptr)
		{
			closeEditor(editWidget, QAbstractItemDelegate::RevertModelCache);
		}

		return;
	}

	//
	// PropertyTable
	//

	PropertyTable::PropertyTable(QWidget *parent) :
		QWidget(parent),
		PropertyEditorBase()
	{
		QHBoxLayout* l = new QHBoxLayout(this);
		l->setContentsMargins(0, 0, 0, 0);

		// Table View

		m_tableView = new PropertyTableView();
		l->addWidget(m_tableView);

		m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

		connect(m_tableView, &QTableView::doubleClicked, this, &PropertyTable::onCellDoubleClicked);

		m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_tableView->setTabKeyNavigation(false);

		// Model

		m_tableModel = new PropertyTableModel(this);
		m_tableView->setModel(m_tableModel);

		// Edit Delegate

		m_itemDelegate = new PropertyTableItemDelegate(this, m_tableModel);
		m_tableView->setItemDelegate(m_itemDelegate);

		//
	}

	void PropertyTable::clear()
	{
		m_tableModel->clear();
	}

	void PropertyTable::closeCurrentEditor()
	{
		m_tableView->closeCurrentEditorIfOpen();
	}

	void PropertyTable::setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects)
	{
		QList<std::shared_ptr<PropertyObject>> list =
				QList<std::shared_ptr<PropertyObject>>::fromVector(QVector<std::shared_ptr<PropertyObject>>::fromStdVector(objects));

		return setObjects(list);
	}

	void PropertyTable::setObjects(const QList<std::shared_ptr<PropertyObject>>& objects)
	{
		setVisible(false);

		// Disconnect updatePropertiesList slot from previous objects

		for (std::shared_ptr<PropertyObject> po : m_objects)
		{
			bool ok = disconnect(po.get(), &PropertyObject::propertyListChanged, this, &PropertyTable::updatePropertiesList);
			if (ok == false)
			{
				assert(false);
			}
		}

		m_objects = objects;

		fillProperties();

		// Connect updatePropertiesList slot to new objects

		for (std::shared_ptr<PropertyObject> po : m_objects)
		{
			bool ok = connect(po.get(), &PropertyObject::propertyListChanged, this, &PropertyTable::updatePropertiesList);
			if (ok == false)
			{
				assert(false);
			}
		}

		setVisible(true);

		return;
	}


	const QList<std::shared_ptr<PropertyObject>>& PropertyTable::objects() const
	{
		return m_objects;
	}

	void PropertyTable::valueChanged(std::vector<std::pair<std::shared_ptr<PropertyObject>, QString>> modifiedObjectsData, const QVariant& value)
	{
		// Set the new property value in all objects
		//
		QString errorString;

		QList<std::shared_ptr<PropertyObject>> modifiedObjects;

		for (auto i : modifiedObjectsData)
		{
			std::shared_ptr<PropertyObject> pObject = i.first;
			const QString& propertyName = i.second;

			// Do not set property, if it has same value

			QVariant oldValue = pObject->propertyValue(propertyName);

			if (oldValue == value)
			{
				continue;
			}

			// Warning!!! If property changing changes the list of properties (e.g. SpecificProperties),
			// property pointer becomes unusable! So next calls to property-> will cause crash

			pObject->setPropertyValue(propertyName, value);

			QVariant newValue = pObject->propertyValue(propertyName);

			if (oldValue == newValue && errorString.isEmpty() == true)
			{
				errorString = QString("Property: %1 - incorrect input value")
							  .arg(propertyName);
			}

			modifiedObjects.append(pObject);
		}

		if (errorString.isEmpty() == false)
		{
			emit showErrorMessage(errorString);
		}

		if (modifiedObjects.count() > 0)
		{
			emit propertiesChanged(modifiedObjects);
		}

		return;
	}

	void PropertyTable::updatePropertiesList()
	{
		setVisible(false);

		fillProperties();

		setVisible(true);

		return;
	}



	void PropertyTable::onCellDoubleClicked(const QModelIndex &index)
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			return;
		}

		// Check if selected cells have the same type
		{
			bool firstCell = true;
			int type = -1;

			for (const QModelIndex& mi : selectedIndexes)
			{
				std::shared_ptr<Property> p = m_tableModel->propertyByIndex(mi);
				if (p == nullptr)
				{
					Q_ASSERT(p);
					return;
				}

				if (firstCell == true)
				{
					type = p->value().userType();
					firstCell = false;
				}
				else
				{
					if (type != p->value().userType())
					{
						return;
					}
				}
			}
		}

		std::shared_ptr<Property> cellProperty = m_tableModel->propertyByIndex(index);
		if (cellProperty == nullptr)
		{
			Q_ASSERT(cellProperty);
			return;
		}

		m_tableView->edit(index);
	}

	void PropertyTable::onShowErrorMessage (QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}


	void PropertyTable::onValueChanged(QVariant value)
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		std::vector<std::pair<std::shared_ptr<PropertyObject>, QString>> modifiedObjectsData;

		for (const QModelIndex& mi : selectedIndexes)
		{
			std::shared_ptr<PropertyObject> po = m_tableModel->propertyObjectByIndex(mi);

			if (po == nullptr)
			{
				Q_ASSERT(po);
				return;
			}

			std::shared_ptr<Property> p = m_tableModel->propertyByIndex(mi);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return;
			}

			modifiedObjectsData.emplace_back(std::make_pair(po, p->caption()));

		}

		if (modifiedObjectsData.empty() == false)
		{
			valueChanged(modifiedObjectsData, value);
		}

		// Force redraw all selected cells

		for (const QModelIndex& mi : selectedIndexes)
		{
			m_tableView->update(mi);
		}

		return;
	}

	void PropertyTable::fillProperties()
	{
		clear();

		QStringList commonProperties;

		{

			QMap<QString, std::shared_ptr<Property>> propertyItems;
			QList<QString> propertyNames;

			// Create a map with all properties
			//

			for (std::shared_ptr<PropertyObject> pobject : m_objects)
			{
				PropertyObject* object = pobject.get();

				for (std::shared_ptr<Property> p : object->properties())
				{
					if (p->visible() == false)
					{
						continue;
					}

					if (p->disableTableEditor() == true)
					{
						continue;
					}

					if (p->expert() && expertMode() == false)
					{
						continue;
					}

					propertyItems.insertMulti(p->caption(), p);

					if (propertyNames.indexOf(p->caption()) == -1)
					{
						propertyNames.append(p->caption());
					}
				}
			}

			// add only common properties with same type
			//
			for (auto name : propertyNames)
			{
				// take all properties witn the same name
				//
				QList<std::shared_ptr<Property>> propsByName = propertyItems.values(name);
				if (propsByName.size() != m_objects.size() || propsByName.size() == 0)
				{
					continue;   // this property is not in all objects
				}

				// now check if all properties have the same type and values
				//
				int type;

				bool sameType = true;

				for (auto p = propsByName.begin(); p != propsByName.end(); p++)
				{
					if (p == propsByName.begin())
					{
						// remember the first item params
						//
						type = p->get()->value().userType();
					}
					else
					{
						// compare with next item params
						//
						if (p->get()->value().userType() != type)
						{
							sameType = false;
							break;
						}
					}
				}

				if (sameType == false)
				{
					continue;   // properties are not the same type
				}

				commonProperties.push_back(name);
			}
		}

		std::sort(commonProperties.begin(), commonProperties.end());

		std::vector<PropertyTableObject> tableObjects;

		for (std::shared_ptr<PropertyObject> pobject : m_objects)
		{
			PropertyObject* object = pobject.get();

			PropertyTableObject pto;
			pto.propertyObject = pobject;

			for (const QString& propertyName : commonProperties)
			{
				std::shared_ptr<Property> p = object->propertyByCaption(propertyName);
				if (p == nullptr)
				{
					Q_ASSERT(p);
					return;
				}

				int type = p.get()->value().userType();

				if (type == QVariant::StringList)
				{
					QStringList l = p.get()->value().toStringList();

					if (pto.rowCount < static_cast<int>(l.size()))
					{
						pto.rowCount = static_cast<int>(l.size());
					}
				}

				/*
				 * Add these later
				if (variantIsPropertyVector(value) == true || variantIsPropertyList(value) == true)
						case QVariant::ByteArray:
						case QVariant::Image:
							*/

				if (p->isEnum() == true ||
						type == TuningValue::tuningValueTypeId() ||
						type == FilePathPropertyType::filePathTypeId() ||
						type == QVariant::Int ||
						type == QVariant::UInt ||
						type == QMetaType::Float ||
						type == QVariant::Double ||
						type == QVariant::Bool ||
						type == QVariant::Color ||
						type == QVariant::Uuid ||
						type == QVariant::String ||
						type == QVariant::StringList)
					{
						pto.properties.push_back(p);
					}
			}

			if (pto.properties.size() > 0)
			{
				tableObjects.push_back(pto);
			}
		}

		m_tableModel->setTableObjects(tableObjects);
	}
}
