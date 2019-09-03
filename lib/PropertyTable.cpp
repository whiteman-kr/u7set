#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "PropertyTable.h"
#include "TuningValue.h"

#include "Stable.h"

namespace ExtWidgets
{
	PropertyTableItemDelegate::PropertyTableItemDelegate(PropertyTable* propertyTable, PropertyTableProxyModel* proxyModel) :
		QItemDelegate(propertyTable),
		m_propertyTable(propertyTable),
		m_proxyModel(proxyModel)
	{
	}

	QWidget* PropertyTableItemDelegate::createEditor(QWidget *parent,
													 const QStyleOptionViewItem &option,
													 const QModelIndex &index) const
	{
		Q_UNUSED(option);

		if (m_proxyModel == nullptr)
		{
			Q_ASSERT(m_proxyModel);
			return new QWidget(parent);
		}

		int row = -1;

		std::shared_ptr<Property> p = m_proxyModel->propertyByIndex(index, &row);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return new QWidget(parent);
		}

		m_cellEditor = m_propertyTable->createCellRowEditor(p, row, true, p->readOnly() == true || m_propertyTable->isReadOnly() == true, parent);

		connect(m_cellEditor, &PropertyEditCellWidget::valueChanged, this, &PropertyTableItemDelegate::onValueChanged);

		return m_cellEditor;
	}

	void PropertyTableItemDelegate::setEditorData(QWidget *editor,
												  const QModelIndex &index) const
	{
		if (m_proxyModel == nullptr)
		{
			Q_ASSERT(m_proxyModel);
			return;
		}

		std::shared_ptr<Property> p = m_proxyModel->propertyByIndex(index, nullptr);
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
	// PropertyTableProxyModel
	//

	PropertyTableProxyModel::PropertyTableProxyModel(QObject *parent):
		QSortFilterProxyModel(parent)
	{

	}

	std::shared_ptr<PropertyObject> PropertyTableProxyModel::propertyObjectByIndex(const QModelIndex& mi) const
	{
		PropertyTableModel* sm = dynamic_cast<PropertyTableModel*>(sourceModel());
		if (sm == nullptr)
		{
			Q_ASSERT(sm);
			return nullptr;
		}

		return sm->propertyObjectByIndex(mapToSource(mi));
	}

	std::shared_ptr<Property> PropertyTableProxyModel::propertyByIndex(const QModelIndex& mi, int* propertyRow) const
	{
		PropertyTableModel* sm = dynamic_cast<PropertyTableModel*>(sourceModel());
		if (sm == nullptr)
		{
			Q_ASSERT(sm);
			return nullptr;
		}

		return sm->propertyByIndex(mapToSource(mi), propertyRow);
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

			int count = 0;

			for (const PropertyTableObject& pto : tableObjects)
			{
				count += pto.rowCount;

			}

			beginInsertRows(QModelIndex(), 0, count - 1);

			m_tableObjects.swap(tableObjects);

			endInsertRows();


		}
	}

	std::shared_ptr<PropertyObject> PropertyTableModel::propertyObjectByIndex(const QModelIndex& mi) const
	{
		int row = 0;

		for (const PropertyTableObject& pto : m_tableObjects)
		{
			if (row + pto.rowCount > mi.row())
			{

				return pto.propertyObject;
			}

			row += pto.rowCount;
		}

		Q_ASSERT(false);
		return nullptr;
	}

	std::shared_ptr<Property> PropertyTableModel::propertyByIndex(const QModelIndex& mi, int* propertyRow) const
	{
		int row = 0;

		for (const PropertyTableObject& pto : m_tableObjects)
		{
			if (row + pto.rowCount > mi.row())
			{
				if (propertyRow != nullptr)
				{
					*propertyRow = mi.row() - row;
				}

				if (mi.column() < 0 || mi.column() >= static_cast<int>(pto.properties.size()))
				{
					Q_ASSERT(false);
					return nullptr;
				}

				return pto.properties[mi.column()];
			}

			row += pto.rowCount;
		}

		Q_ASSERT(false);
		return nullptr;
	}

	void PropertyTableModel::recalculateRowCount(std::shared_ptr<PropertyObject> object)
	{
		for (PropertyTableObject& pto : m_tableObjects)
		{
			if (pto.propertyObject.get() == object.get())
			{
				int oldRowCount = pto.rowCount;

				pto.rowCount = 1;

				for (auto p : pto.properties)
				{
					int type = p.get()->value().userType();

					if (type == QVariant::StringList)
					{
						QStringList l = p.get()->value().toStringList();

						if (pto.rowCount < static_cast<int>(l.size()))
						{
							pto.rowCount = static_cast<int>(l.size());
						}
					}
				}

				if (oldRowCount > pto.rowCount)
				{
					// Rows were deleted

					int rowsRemoved = oldRowCount - pto.rowCount;

					beginRemoveRows(QModelIndex(), 0, rowsRemoved - 1);
					endRemoveRows();

				}
				else
				{
					if (oldRowCount < pto.rowCount)
					{
						// Rows were added

						int rowsAdded = pto.rowCount - oldRowCount;

						beginInsertRows(QModelIndex(), 0, rowsAdded - 1);
						endInsertRows();
					}
				}

				return;
			}
		}

		Q_ASSERT(false);
		return;
	}

	int PropertyTableModel::rowCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		int result = 0;

		for (const PropertyTableObject& pto : m_tableObjects)
		{
			result += pto.rowCount;
		}

		return result;
	}

	int PropertyTableModel::columnCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		int result = static_cast<int>(m_propertyNames.size());
		return result;
	}

	QVariant PropertyTableModel::data(const QModelIndex &index, int role) const
	{
		if (role== Qt::DecorationRole)
		{
			int row = -1;

			std::shared_ptr<Property> p = propertyByIndex(index, &row);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			if (m_propertyTable->expandValuesToAllRows() == false &&
					p->value().userType() != QVariant::StringList &&  row > 0)
			{
				return QVariant();
			}

			bool enabled = m_propertyTable->isReadOnly() == false && p->readOnly() == false;

			return PropertyEditorBase::propertyIcon(p.get(), /*sameValue*/true, enabled);
		}

		if (role == Qt::DisplayRole)
		{
			int row = -1;

			std::shared_ptr<Property> p = propertyByIndex(index, &row);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return QVariant();
			}

			if (m_propertyTable->expandValuesToAllRows() == false &&
					p->value().userType() != QVariant::StringList &&  row > 0)
			{
				return QVariant();
			}

			return PropertyEditorBase::propertyValueText(p.get(), row);
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

	void PropertyTableView::keyPressEvent(QKeyEvent *event)
	{
		if (event->key() == Qt::Key_F2 || event->key() == Qt::Key_Return)
		{
			emit editKeyPressed();
			return;
		}

		QTableView::keyPressEvent(event);
	}

	//
	// PropertyTable
	//

	PropertyTable::PropertyTable(QWidget *parent) :
		QWidget(parent),
		m_tableModel(this),
		PropertyEditorBase()
	{
		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		// Property Mask

		m_editPropertyMask = new QLineEdit();
		connect(m_editPropertyMask, &QLineEdit::editingFinished, this, &PropertyTable::onPropertyMaskChanged);

		// Toolbar

		QHBoxLayout* toolsLayout = new QHBoxLayout();

		toolsLayout->addWidget(m_editPropertyMask);
		toolsLayout->addStretch();

		mainLayout->addLayout(toolsLayout);

		// Model

		m_proxyModel.setSourceModel(&m_tableModel);

		// Table View

		m_tableView = new PropertyTableView();
		mainLayout->addWidget(m_tableView);

		m_tableView->setModel(&m_proxyModel);
		m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_tableView->setTabKeyNavigation(false);

		m_tableView->setSortingEnabled(true);
		m_tableView->sortByColumn(1, Qt::AscendingOrder);

		m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(m_tableView, &PropertyTableView::customContextMenuRequested, this, &PropertyTable::onTableContextMenuRequested);

		connect(m_tableView, &QTableView::doubleClicked, this, &PropertyTable::onCellDoubleClicked);

		connect(m_tableView, &PropertyTableView::editKeyPressed, this, &PropertyTable::onCellEditKeyPressed);

		// Edit Delegate

		m_itemDelegate = new PropertyTableItemDelegate(this, &m_proxyModel);
		connect(m_itemDelegate, &PropertyTableItemDelegate::valueChanged, this, &PropertyTable::onValueChanged);
		connect(m_itemDelegate, &QItemDelegate::closeEditor, this, &PropertyTable::onCellEditorClosed);

		m_tableView->setItemDelegate(m_itemDelegate);
	}

	void PropertyTable::clear()
	{
		m_tableModel.clear();
	}

	void PropertyTable::closeCurrentEditor()
	{
		m_tableView->closeCurrentEditorIfOpen();
	}

	const QList<std::shared_ptr<PropertyObject>>& PropertyTable::objects() const
	{
		return m_objects;
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

	QString PropertyTable::propertyMask() const
	{
		return m_propertyMasks.join(';');
	}

	void PropertyTable::setPropertyMask(const QString& propertyMask)
	{
		if (m_editPropertyMask == nullptr)
		{
			Q_ASSERT(m_editPropertyMask);
			return;
		}

		m_editPropertyMask->blockSignals(true);
		m_editPropertyMask->setText(propertyMask);
		m_editPropertyMask->blockSignals(false);

		m_propertyMasks = propertyMask.split(';', QString::SkipEmptyParts);

		fillProperties();
	}

	bool PropertyTable::expandValuesToAllRows() const
	{
		return m_expandValuesToAllRows;
	}

	void PropertyTable::setExpandValuesToAllRows(bool value)
	{
		m_expandValuesToAllRows = value;
	}

	void PropertyTable::valueChanged(QMap<QString, std::pair<std::shared_ptr<PropertyObject>, QVariant>> modifiedObjectsData)
	{
		// Set the new property value in all objects
		//
		QString errorString;

		QList<std::shared_ptr<PropertyObject>> modifiedObjects;

		for (const QString& propertyName : modifiedObjectsData.keys())
		{
			QList<std::pair<std::shared_ptr<PropertyObject>, QVariant>> objectsData = modifiedObjectsData.values(propertyName);

			for (auto objectData : objectsData)
			{
				std::shared_ptr<PropertyObject> object = objectData.first;
				QVariant value = objectData.second;

				// Do not set property, if it has same value

				QVariant oldValue = object->propertyValue(propertyName);

				if (oldValue == value)
				{
					continue;
				}

				// Warning!!! If property changing changes the list of properties (e.g. SpecificProperties),
				// property pointer becomes unusable! So next calls to property-> will cause crash

				object->setPropertyValue(propertyName, value);

				QVariant newValue = object->propertyValue(propertyName);

				if (oldValue == newValue && errorString.isEmpty() == true)
				{
					errorString = QString("Property: %1 - incorrect input value")
							.arg(propertyName);
				}

				modifiedObjects.append(object);
			}
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

	void PropertyTable::updatePropertiesValues()
	{
		// Force redraw all cells

		m_tableView->viewport()->update();
	}

	void PropertyTable::onCellDoubleClicked(const QModelIndex &index)
	{
		Q_UNUSED(index);
		startEditProperty();
	}

	void PropertyTable::onCellEditKeyPressed()
	{
		startEditProperty();
	}

	void PropertyTable::onShowErrorMessage (QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

	void PropertyTable::onPropertyMaskChanged()
	{
		QString str = m_editPropertyMask->text().trimmed();

		m_propertyMasks = str.split(';', QString::SkipEmptyParts);

		fillProperties();
	}

	void PropertyTable::onTableContextMenuRequested(const QPoint &pos)
	{
		Q_UNUSED(pos);

		QMenu menu(this);

		// Determine if row operations are avaliable
		//

		bool addRowOperationsEnabled = false;
		bool removeRowOperationsEnabled = false;

		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.size() == 1)
		{
			std::shared_ptr<Property> po = m_proxyModel.propertyByIndex(selectedIndexes[0], nullptr);
			if (po == nullptr)
			{
				Q_ASSERT(po);
				return;
			}

			if (po->value().userType() == QVariant::StringList)
			{
				addRowOperationsEnabled = true;

				QStringList strings = po->value().toStringList();

				if (strings.isEmpty() == false)
				{
					removeRowOperationsEnabled = true;
				}
			}
		}

		if (addRowOperationsEnabled == true)
		{
			QAction* a = menu.addAction(tr("Insert String Before..."));
			connect(a, &QAction::triggered, this, &PropertyTable::onInsertStringBefore);

			a = menu.addAction(tr("Insert String After..."));
			connect(a, &QAction::triggered, this, &PropertyTable::onInsertStringAfter);
		}

		if (removeRowOperationsEnabled == true)
		{
			QAction* a = menu.addAction(tr("Remove String..."));
			connect(a, &QAction::triggered, this, &PropertyTable::onRemoveString);
		}

		if (addRowOperationsEnabled == true || removeRowOperationsEnabled == true)
		{
			menu.addSeparator();
		}

		//

		QAction* a = menu.addAction(tr("Expand Values to all Rows"));
		a->setCheckable(true);
		a->setChecked(expandValuesToAllRows());
		connect(a, &QAction::triggered, this, &PropertyTable::onUniqueRowValuesChanged);

		if (menu.actions().empty() == true)
		{
			return;
		}

		menu.exec(QCursor::pos());
	}

	void PropertyTable::onInsertStringBefore()
	{
		addString(false);
	}

	void PropertyTable::onInsertStringAfter()
	{
		addString(true);
	}

	void PropertyTable::onRemoveString()
	{
		removeString();
	}

	void PropertyTable::onUniqueRowValuesChanged()
	{
		setExpandValuesToAllRows(!expandValuesToAllRows());

		updatePropertiesValues();
	}

	void PropertyTable::onValueChanged(QVariant value)
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		QMap<QString, std::pair<std::shared_ptr<PropertyObject>, QVariant>> modifiedObjectsData;

		for (const QModelIndex& mi : selectedIndexes)
		{
			QVariant newValue = value;

			std::shared_ptr<PropertyObject> po = m_proxyModel.propertyObjectByIndex(mi);

			if (po == nullptr)
			{
				Q_ASSERT(po);
				return;
			}

			int row = -1;

			std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(mi, &row);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return;
			}

			if (p->value().userType() == QVariant::StringList && newValue.userType() == QVariant::String)
			{
				QStringList l = p->value().toStringList();

				if (row < 0 || row >= static_cast<int>(l.size()))
				{
					Q_ASSERT(false);
					return;
				}

				l[row] = newValue.toString();

				newValue = l;
			}

			modifiedObjectsData.insertMulti(p->caption(), std::make_pair(po, newValue));
		}

		if (modifiedObjectsData.empty() == false)
		{
			valueChanged(modifiedObjectsData);
		}

		// Force redraw all selected cells

		for (const QModelIndex& mi : selectedIndexes)
		{
			m_tableView->update(mi);
		}

		return;
	}

	void PropertyTable::onCellEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
	{
		Q_UNUSED(editor);
		Q_UNUSED(hint);

		m_tableView->setFocus();
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

					const QString& propertyName = p->caption();

					if (m_propertyMasks.empty() == false)
					{
						bool maskMatch = false;

						for (const QString& mask : m_propertyMasks)
						{
							if (propertyName.contains(mask) == true)
							{
								maskMatch = true;
								break;
							}
						}

						if (maskMatch == false)
						{
							continue;
						}
					}

					propertyItems.insertMulti(propertyName, p);

					if (propertyNames.indexOf(propertyName) == -1)
					{
						propertyNames.append(propertyName);
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
				int type = -1;

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

		m_tableModel.setTableObjects(tableObjects);
	}

	void PropertyTable::startEditProperty()
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
				int row = -1;

				std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(mi, &row);
				if (p == nullptr)
				{
					Q_ASSERT(p);
					return;
				}

				if (expandValuesToAllRows() == false &&
						p->value().userType() != QVariant::StringList &&  row > 0)
				{
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
						QMessageBox::critical(this, qAppName(), tr("Please select properties of same type."));
						return;
					}
				}
			}
		}

		QModelIndex index = m_tableView->currentIndex();

		m_tableView->edit(index);
	}

	void PropertyTable::addString(bool after)
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.size() != 1)
		{
			Q_ASSERT(false);
			return;
		}

		int row = -1;

		std::shared_ptr<PropertyObject> po = m_proxyModel.propertyObjectByIndex(selectedIndexes[0]);
		if (po == nullptr)
		{
			Q_ASSERT(po);
			return;
		}

		std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(selectedIndexes[0], &row);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		if (p->value().userType() != QVariant::StringList)
		{
			Q_ASSERT(false);
			return;
		}

		bool ok = false;
		QString text = QInputDialog::getText(this, qAppName(),
											 tr("Enter the value:"),
											 QLineEdit::Normal,
											 tr("NewLine"),
											 &ok);

		if (ok == false || text.isEmpty() == true)
		{
			return;
		}

		if (after == true)
		{
			row++;
		}

		QStringList strings = p->value().toStringList();

		strings.insert(row, text);

		onValueChanged(strings);

		m_tableModel.recalculateRowCount(po);

		updatePropertiesValues();

		return;
	}

	void PropertyTable::removeString()
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.size() != 1)
		{
			Q_ASSERT(false);
			return;
		}

		int row = -1;

		std::shared_ptr<PropertyObject> po = m_proxyModel.propertyObjectByIndex(selectedIndexes[0]);
		if (po == nullptr)
		{
			Q_ASSERT(po);
			return;
		}

		std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(selectedIndexes[0], &row);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		if (p->value().userType() != QVariant::StringList)
		{
			Q_ASSERT(false);
			return;
		}

		QStringList strings = p->value().toStringList();

		if (strings.isEmpty() == true)
		{
			return;
		}

		if (row >= static_cast<int>(strings.size()))
		{
			Q_ASSERT(false);
			return;
		}

		strings.erase(strings.begin() + row);

		onValueChanged(strings);

		m_tableModel.recalculateRowCount(po);

		updatePropertiesValues();

		return;
	}
}
