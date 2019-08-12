#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "PropertyTable.h"
#include "TuningValue.h"

#include "Stable.h"

namespace ExtWidgets
{
	PropertyTableItemDelegate::PropertyTableItemDelegate(QObject *parent) :
		QItemDelegate(parent)
	{
	}

	// TableView need to create an Editor
	// Create Editor when we construct PropertyTableItemDelegate
	// and return the Editor
	QWidget* PropertyTableItemDelegate::createEditor(QWidget *parent,
													 const QStyleOptionViewItem &option,
													 const QModelIndex &index) const
	{

		QSpinBox *editor = new QSpinBox(parent);
		editor->setMinimum(0);
		editor->setMaximum(100);
		return editor;
	}

	// Then, we set the Editor
	// Gets the data from Model and feeds the data to Editor
	void PropertyTableItemDelegate::setEditorData(QWidget *editor,
												  const QModelIndex &index) const
	{
		// Get the value via index of the Model
		int value = index.model()->data(index, Qt::EditRole).toInt();

		// Put the value into the SpinBox
		QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
		spinbox->setValue(value);
	}

	// When we modify data, this model reflect the change
	void PropertyTableItemDelegate::setModelData(QWidget *editor,
												 QAbstractItemModel *model,
												 const QModelIndex &index) const
	{
		/*QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
		spinbox->interpretText();
		int value = spinbox->value();
		model->setData(index, value, Qt::EditRole);*/
	}

	// Give the SpinBox the info on size and location
	void PropertyTableItemDelegate::updateEditorGeometry(QWidget *editor,
														 const QStyleOptionViewItem &option,
														 const QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
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

	Property* PropertyTableModel::propertyByIndex(const QModelIndex& mi) const
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

		return pto.properties[mi.column()].get();
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
			Property* p = propertyByIndex(index);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			bool enabled = m_propertyTable->readOnly() == false && p->readOnly() == false;

			return PropertyEditorBase::propertyIcon(p, /*sameValue*/true, enabled);
		}

		if (role == Qt::DisplayRole)
		{
			Property* p = propertyByIndex(index);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			return PropertyEditorBase::propertyValueText(p);
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
	// PropertyTable
	//

	PropertyTable::PropertyTable(QWidget *parent) :
		QWidget(parent),
		PropertyEditorBase()
	{
		QHBoxLayout* l = new QHBoxLayout(this);
		l->setContentsMargins(0, 0, 0, 0);

		// Table View

		m_tableView = new QTableView();
		l->addWidget(m_tableView);

		m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

		connect(m_tableView, &QTableView::doubleClicked, this, &PropertyTable::onCellDoubleClicked);

		m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

		// Model

		m_tableModel = new PropertyTableModel(this);
		m_tableView->setModel(m_tableModel);

		// Edit Delegate

		m_itemDelegate = new PropertyTableItemDelegate(this);
		m_tableView->setItemDelegate(m_itemDelegate);

		//
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

	void PropertyTable::clear()
	{
		m_tableModel->clear();
	}


	void PropertyTable::updatePropertyValues(const QString& propertyName)
	{

	}

	const QList<std::shared_ptr<PropertyObject>>& PropertyTable::objects() const
	{
		return m_objects;
	}

	bool PropertyTable::expertMode() const
	{
		return m_expertMode;
	}

	void PropertyTable::setExpertMode(bool expertMode)
	{
		m_expertMode = expertMode;
	}

	bool PropertyTable::readOnly() const
	{
		return m_readOnly;
	}

	void PropertyTable::setReadOnly(bool readOnly)
	{
		m_readOnly = readOnly;
	}

	void PropertyTable::updatePropertiesList()
	{
		setVisible(false);

		fillProperties();

		setVisible(true);

		return;
	}

	void PropertyTable::updatePropertiesValues()
	{
		updatePropertyValues(QString());
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
				Property* p = m_tableModel->propertyByIndex(mi);
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

		Property* cellProperty = m_tableModel->propertyByIndex(index);
		if (cellProperty == nullptr)
		{
			Q_ASSERT(cellProperty);
			return;
		}

		m_tableView->edit(index);
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

					if (p->expert() && m_expertMode == false)
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
