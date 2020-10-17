#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "PropertyTable.h"
#include "TuningValue.h"

#include "Stable.h"

namespace ExtWidgets
{
	//
	// DialogReplace
	//

	DialogReplace::DialogReplace(const QString& what, const QString& to, bool caseSensitive, QWidget* parent):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
		m_what(what),
		m_to(to),
		m_caseSensitive(caseSensitive)
	{
		QGridLayout* gl = new QGridLayout();

		QLabel* l = new QLabel(tr("Find what:"));
		gl->addWidget(l, 0, 0);

		m_editWhat = new QLineEdit();
		m_editWhat->setText(what);
		gl->addWidget(m_editWhat, 0, 1);

		l = new QLabel(tr("Replace with:"));
		gl->addWidget(l, 1, 0);

		m_editTo = new QLineEdit();
		m_editTo->setText(to);
		gl->addWidget(m_editTo, 1, 1);

		m_checkCase = new QCheckBox(tr("Case Sensitive"));
		m_checkCase->setChecked(caseSensitive);

		gl->addWidget(m_checkCase, 2, 1);

		//

		QHBoxLayout* hl = new QHBoxLayout();
		hl->addStretch();

		QPushButton* b = new QPushButton(tr("OK"));
		connect(b, &QPushButton::clicked, this, &QDialog::accept);
		hl->addWidget(b);

		b = new QPushButton(tr("Cancel"));
		connect(b, &QPushButton::clicked, this, &QDialog::reject);
		hl->addWidget(b);

		//

		QVBoxLayout* vl = new QVBoxLayout();
		vl->addLayout(gl);
		vl->addLayout(hl);

		setLayout(vl);
	}

	const QString DialogReplace::what() const
	{
		return m_what;
	}

	const QString DialogReplace::to() const
	{
		return m_to;
	}

	bool DialogReplace::caseSensitive() const
	{
		return m_caseSensitive;
	}

	void DialogReplace::accept()
	{
		if (m_editWhat->text().isEmpty() == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Please fill the \"Find What\" field!"));
			m_editWhat->setFocus();
			return;
		}
		m_what = m_editWhat->text();

		if (m_editTo->text().isEmpty() == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Please fill the \"Replace With\" field!"));
			m_editTo->setFocus();
			return;
		}
		m_to = m_editTo->text();

		m_caseSensitive = m_checkCase->isChecked();

		QDialog::accept();
	}


	//
	// PropertyTableItemDelegate
	//


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

		bool readOnly = p->readOnly() == true || m_propertyTable->isReadOnly() == true;

		cellEditor->setValue(p, readOnly);

		if (m_initText.isEmpty() == false)
		{
			if (readOnly == false)
			{
				cellEditor->setInitialText(m_initText);
			}

			m_initText.clear();
		}

		return;
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

	void PropertyTableItemDelegate::setInitText(const QString& text)
	{
		m_initText = text;
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

	bool PropertyTableProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
	{
		PropertyTableModel* sm = dynamic_cast<PropertyTableModel*>(sourceModel());
		if (sm == nullptr)
		{
			Q_ASSERT(sm);
			return false;
		}

		int leftPropertyRow = -1;
		int rightPropertyRow = -1;

		std::shared_ptr<Property> leftProperty = sm->propertyByIndex(left, &leftPropertyRow);
		std::shared_ptr<Property> rightProperty = sm->propertyByIndex(right, &rightPropertyRow);

		if (leftProperty == nullptr || rightProperty == nullptr)
		{
			Q_ASSERT(leftProperty);
			Q_ASSERT(rightProperty);
			return false;
		}

		QVariant lv = leftProperty->value();
		QVariant rv = rightProperty->value();

		if (leftProperty->isEnum() == true && rightProperty->isEnum() == true)
		{
			return leftProperty->value().toInt() < rightProperty->value().toInt();
		}

		if (lv.userType() != rv.userType())
		{
			Q_ASSERT(false);
			return lv.userType() < rv.userType();
		}

		if (lv.userType() == TuningValue::tuningValueTypeId())
		{
			TuningValue tv1 = lv.value<TuningValue>();
			TuningValue tv2 = rv.value<TuningValue>();

			return tv1.toDouble() < tv2.toDouble();
		}

		switch(lv.userType())
		{
		case QVariant::Int:
		case QVariant::UInt:
		case QMetaType::Float:
		case QVariant::Double:
			{
				return lv.toDouble() < rv.toDouble();
				break;
			}
		default:
			return lv.toString() < rv.toString();
		}
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

	void PropertyTableModel::setTableObjects(std::vector<PropertyTableObject>& tableObjects, bool showCategory)
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
				QString propertyName = p->caption();

				if (showCategory == true)
				{
					QString propertyCategory = p->category();

					if (propertyCategory.isEmpty() == true)
					{
						propertyCategory = PropertyEditorBase::m_commonCategoryName;
					}

					propertyName = p->caption() + QChar::LineFeed + propertyCategory;
				}

				m_propertyNames.push_back(propertyName);
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

	bool PropertyTableModel::hasMultiRows() const
	{
		for (const PropertyTableObject& pto : m_tableObjects)
		{
			if (pto.rowCount > 1)
			{
				return true;
			}
		}

		return false;
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
		if (selectedIndexes().size() > 0)
		{
			if (event->key() == Qt::Key_Escape)
			{
				clearSelection();
				return;
			}

			if (event->key() == Qt::Key_F2 || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
			{
				emit editKeyPressed();
				return;
			}

			if (event->key() == Qt::Key_Space)
			{
				emit spaceKeyPressed();
				return;
			}

			if ((event->key() == Qt::Key_Up ||
				 event->key() == Qt::Key_Down ||
				 event->key() == Qt::Key_PageUp ||
				 event->key() == Qt::Key_PageDown)
				&& indexWidget(currentIndex()) != nullptr)
			{
				return;
			}

			if (event->text().isEmpty() == false)
			{
				emit symbolKeyPressed(event->text());
				return;
			}

		}
		else
		{
			if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
			{
				return;
			}

		}

		QTableView::keyPressEvent(event);
		return;
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
		mainLayout->setContentsMargins(1, 1, 1, 1);
		mainLayout->setSpacing(2);

		// Property Filter
		m_editPropertyFilter = new QLineEdit();
		connect(m_editPropertyFilter, &QLineEdit::editingFinished, this, &PropertyTable::onPropertyFilterChanged);

		// Property Filter Help

		m_editPropertyFilter->setToolTip(tr("To filter properties, enter a caption fragment. Multiple fragments can be separated by semicolons."));

		QPushButton* filterHelpButton = new QPushButton("?");
		filterHelpButton->setDefault(false);
		filterHelpButton->setAutoDefault(false);
		connect(filterHelpButton, &QPushButton::clicked, [this](){
			QString filterHelp = tr("Property Filter\n\nTo filter properties, enter a caption fragment.\n\nMultiple fragments can be separated by semicolons.\n\nExample: Caption;Enable");
			QMessageBox::information(this, qAppName(), filterHelp);
		});

		// GroupByCategory
		m_buttonGroupByCategory = new QPushButton(tr("Group by Category"));
		m_buttonGroupByCategory->setCheckable(true);
		m_buttonGroupByCategory->setDefault(false);
		m_buttonGroupByCategory->setAutoDefault(false);
		connect(m_buttonGroupByCategory, &QPushButton::toggled, this, &PropertyTable::onGroupByCategoryToggled);

		// Toolbar

		QHBoxLayout* toolsLayout = new QHBoxLayout();

		toolsLayout->addWidget(new QLabel(tr("Property Filter:")));
		toolsLayout->addWidget(m_editPropertyFilter);
		toolsLayout->addWidget(filterHelpButton);
		toolsLayout->addStretch();
		toolsLayout->addWidget(m_buttonGroupByCategory);

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
		connect(m_tableView, &PropertyTableView::symbolKeyPressed, this, &PropertyTable::onCellSymbolKeyPressed);
		connect(m_tableView, &PropertyTableView::spaceKeyPressed, this, &PropertyTable::onCellToggleKeyPressed);

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

	const QList<std::shared_ptr<PropertyObject>>& PropertyTable::objects() const
	{
		return m_objects;
	}

	void PropertyTable::setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects)
	{
		QList<std::shared_ptr<PropertyObject>> list =
		        QList<std::shared_ptr<PropertyObject>>::fromVector(QVector<std::shared_ptr<PropertyObject>>{objects.begin(), objects.end()});

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

	QString PropertyTable::propertyFilter() const
	{
		return m_propertyFilters.join(';');
	}

	void PropertyTable::setPropertyFilter(const QString& propertyFilter)
	{
		if (m_editPropertyFilter == nullptr)
		{
			Q_ASSERT(m_editPropertyFilter);
			return;
		}

		m_editPropertyFilter->blockSignals(true);
		m_editPropertyFilter->setText(propertyFilter);
		m_editPropertyFilter->blockSignals(false);

		m_propertyFilters = propertyFilter.split(';', Qt::SkipEmptyParts);

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

	const QMap<QString, int>& PropertyTable::getColumnsWidth()
	{
		saveColumnsWidth();

		return m_columnsWidth;
	}

	void PropertyTable::setColumnsWidth(const QMap<QString, int>& columnsWidth)
	{
		m_columnsWidth = columnsWidth;
	}

	bool PropertyTable::groupByCategory() const
	{
		if (m_buttonGroupByCategory == nullptr)
		{
			Q_ASSERT(m_buttonGroupByCategory);
			return false;
		}

		return m_buttonGroupByCategory->isChecked() == true;
	}

	void PropertyTable::setGroupByCategory(bool value)
	{
		if (m_buttonGroupByCategory == nullptr)
		{
			Q_ASSERT(m_buttonGroupByCategory);
			return;
		}

		m_buttonGroupByCategory->setChecked(value);
	}

	void PropertyTable::valueChanged(const ModifiedObjectsData& modifiedObjectsData)
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

	void PropertyTable::hideEvent(QHideEvent* event)
	{
		if (event->type() == QEvent::Hide)
		{
			m_tableView->closeCurrentEditorIfOpen();
		}
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
		startEditing();
	}

	void PropertyTable::onCellEditKeyPressed()
	{
		if (getSelectionType() != QVariant::Bool)
		{
			startEditing();
		}
	}

	void PropertyTable::onCellSymbolKeyPressed(QString key)
	{
		if (getSelectionType() != QVariant::Bool && isSelectionReadOnly() == false)
		{
			m_itemDelegate->setInitText(key);

			startEditing();
		}
	}

	void PropertyTable::onCellToggleKeyPressed()
	{
		if (getSelectionType() == QVariant::Bool)
		{
			toggleSelected();
		}
	}

	void PropertyTable::onShowErrorMessage (QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

	void PropertyTable::onPropertyFilterChanged()
	{
		QString str = m_editPropertyFilter->text().trimmed();

		m_propertyFilters = str.split(';', Qt::SkipEmptyParts);

		fillProperties();
	}

	void PropertyTable::onTableContextMenuRequested(const QPoint &pos)
	{
		Q_UNUSED(pos);

		QMenu menu(this);

		if (isSelectionReadOnly() == false && isReadOnly() == false)
		{
			// Non-ReadOnly operations

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

			// Replace

			int selectionType = getSelectionType();

			if (selectionType == QVariant::String ||
					selectionType == QVariant::StringList)
			{
				QAction* a = menu.addAction(tr("Replace..."));
				connect(a, &QAction::triggered, this, &PropertyTable::onReplace);
			}

			// End of Non-ReadOnly operations
		}

		if (m_tableModel.hasMultiRows() == true)
		{
			menu.addSeparator();

			// Expand
			QAction* a = menu.addAction(tr("Expand Values to all Rows"));
			a->setCheckable(true);
			a->setChecked(expandValuesToAllRows());
			connect(a, &QAction::triggered, this, &PropertyTable::onUniqueRowValuesChanged);
		}

		if (menu.actions().empty() == true)
		{
			return;
		}

		menu.exec(QCursor::pos());
	}

	void PropertyTable::onGroupByCategoryToggled(bool value)
	{
		Q_UNUSED(value);

		fillProperties();
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

	void PropertyTable::onReplace()
	{
		int selectionType = getSelectionType();

		if (selectionType != QVariant::String &&
				selectionType != QVariant::StringList)
		{
			return;
		}

		static bool caseSensitive = true;
		static QString replaceWhat;
		static QString replaceTo;

		DialogReplace d(replaceWhat, replaceTo, caseSensitive, this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		replaceWhat = d.what();
		replaceTo = d.to();
		caseSensitive = d.caseSensitive();

		//

		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		ModifiedObjectsData modifiedObjectsData;

		std::map<std::pair<QString, std::shared_ptr<PropertyObject>>, QVariant> multiRowValues;

		for (const QModelIndex& mi : selectedIndexes)
		{
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

			if (p->value().userType() == QVariant::String)
			{
				QString s = p->value().toString();

				s = s.replace(replaceWhat, replaceTo, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

				modifiedObjectsData.insertMulti(p->caption(), std::make_pair(po, s));
			}

			if (p->value().userType() == QVariant::StringList)
			{
				// StringList property can have more than one modified string.
				// Because of this we need to take already added value from multiRowValues and make new modification.
				// If it is not added yet, we take value from the property

				QVariant value;

				auto propertyKey = std::make_pair(p->caption(), po);

				auto it = multiRowValues.find(propertyKey);
				if (it == multiRowValues.end())
				{
					value = p->value();
				}
				else
				{
					value = it->second;
				}

				QStringList l = value.toStringList();

				if (row < 0 || row >= static_cast<int>(l.size()))
				{
					Q_ASSERT(false);
					return;
				}

				QString s = l[row];

				s = s.replace(replaceWhat, replaceTo, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

				l[row] = s;

				multiRowValues[propertyKey] = l;
			}
		}

		// Fill stringListValues

		for (auto it : multiRowValues)
		{
			std::pair<QString, std::shared_ptr<PropertyObject>> key = it.first;

			const QString& propertyName = key.first;
			const std::shared_ptr<PropertyObject>& po = key.second;
			const QVariant& value = it.second;

			modifiedObjectsData.insertMulti(propertyName, std::make_pair(po, value));
		}

		if (modifiedObjectsData.empty() == true)
		{
			return;
		}

		valueChanged(modifiedObjectsData);

		// Force redraw all selected cells

		for (const QModelIndex& mi : selectedIndexes)
		{
			m_tableView->update(mi);
		}

		return;
	}

	void PropertyTable::onValueChanged(QVariant value)
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		ModifiedObjectsData modifiedObjectsData;

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

		if (modifiedObjectsData.empty() == true)
		{
			return;
		}

		valueChanged(modifiedObjectsData);

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
		saveColumnsWidth();

		clear();

		std::map<QString, QString> commonPropertyNames;	// contains map [NameCategory, Name]

		bool groupByCategory = m_buttonGroupByCategory->isChecked() == true;

		{
			QMap<QString, std::shared_ptr<Property>> propertyItems;

			std::map<QString, QString> propertyNames; // contains map [NameCategory, Name]

			// Create a map with all properties
			//

			for (std::shared_ptr<PropertyObject> pobject : m_objects)
			{
				PropertyObject* object = pobject.get();

				for (std::shared_ptr<Property> p : object->properties())
				{
					// Filter the property

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

					const QString& propertyCaption = p->caption();

					if (m_propertyFilters.empty() == false)
					{
						bool maskMatch = false;

						for (const QString& filter : m_propertyFilters)
						{
							if (propertyCaption.contains(filter) == true)
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

					QString propertyName = propertyCaption;

					// Add category if required

					if (groupByCategory == true)
					{
						QString propertyCategory = p->category();

						if (propertyCategory.isEmpty() == true)
						{
							propertyCategory = m_commonCategoryName;
						}

						propertyName = propertyCategory + '\\' + propertyCaption;
					}

					//

					propertyItems.insertMulti(propertyName, p);

					if (propertyNames.find(propertyName) == propertyNames.end())
					{
						propertyNames[propertyName] = propertyCaption;
					}
				}
			}

			// add only common properties with same type
			//
			for (const auto& name : propertyNames)
			{
				const QString& propertyName = name.first;
				const QString& propertyCaption = name.second;

				// take all properties witn the same name
				//
				QList<std::shared_ptr<Property>> propsByName = propertyItems.values(propertyName);
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

				commonPropertyNames[propertyName] = propertyCaption;
			}
		}

		//commonPropertyNames is sorted by key

		std::vector<PropertyTableObject> tableObjects;

		for (std::shared_ptr<PropertyObject> pobject : m_objects)
		{
			PropertyObject* object = pobject.get();

			PropertyTableObject pto;
			pto.propertyObject = pobject;

			for (const auto& commonProperty : commonPropertyNames)
			{
				const QString& propertyCaption = commonProperty.second;

				std::shared_ptr<Property> p = object->propertyByCaption(propertyCaption);
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

		m_tableModel.setTableObjects(tableObjects, groupByCategory);

		restoreColumnsWidth();
	}

	// returns -1 if no type is selected or they are different
	//
	int PropertyTable::getSelectionType()
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			return -1;
		}

		// Check if selected cells have the same type
		bool firstCell = true;
		int type = -1;

		for (const QModelIndex& mi : selectedIndexes)
		{
			int row = -1;

			std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(mi, &row);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return -1;
			}

			if (expandValuesToAllRows() == false &&
					p->value().userType() != QVariant::StringList &&
					row > 0)
			{
				// empty cell with no-repeated value is selected
				return -1;
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
					return -1;
				}
			}
		}

		return type;
	}

	bool PropertyTable::isSelectionReadOnly()
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			return false;
		}

		for (const QModelIndex& mi : selectedIndexes)
		{

			std::shared_ptr<Property> p = m_proxyModel.propertyByIndex(mi, nullptr);
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return false;
			}

			if (p->readOnly() == true)
			{
				return true;
			}
		}

		return false;
	}

	void PropertyTable::startEditing()
	{
		if (getSelectionType() == -1)
		{
			return;
		}

		QModelIndex index = m_tableView->currentIndex();
		m_tableView->edit(index);
	}

	void PropertyTable::toggleSelected()
	{
		QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			return;
		}

		ModifiedObjectsData modifiedObjectsData;

		for (const QModelIndex& mi : selectedIndexes)
		{
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

			if (p->value().userType() == QVariant::Bool)
			{
				bool b = p->value().toBool();

				modifiedObjectsData.insertMulti(p->caption(), std::make_pair(po, !b));
			}
		}

		if (modifiedObjectsData.empty() == true)
		{
			return;
		}

		valueChanged(modifiedObjectsData);

		// Force redraw all selected cells

		for (const QModelIndex& mi : selectedIndexes)
		{
			m_tableView->update(mi);
		}
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

	void PropertyTable::saveColumnsWidth()
	{
		if (m_tableView == nullptr)
		{
			Q_ASSERT(m_tableView);
			return;
		}

		int columnCount = m_tableView->model()->columnCount();
		for (int i = 0; i < columnCount; i++)
		{
			QString colName = m_tableView->model()->headerData(i, Qt::Horizontal).toString();

			int nPos = colName.indexOf(QChar::LineFeed);
			if (nPos != -1)
			{
				colName = colName.left(nPos);
			}

			m_columnsWidth[colName] = m_tableView->columnWidth(i);
		}
	}

	void PropertyTable::restoreColumnsWidth()
	{
		if (m_tableView == nullptr)
		{
			Q_ASSERT(m_tableView);
			return;
		}
		for (int i = 0; i < m_tableView->model()->columnCount(); i++)
		{
			QString colName = m_tableView->model()->headerData(i, Qt::Horizontal).toString();

			int nPos = colName.indexOf(QChar::LineFeed);
			if (nPos != -1)
			{
				colName = colName.left(nPos);
			}

			auto it = m_columnsWidth.find(colName);
			if (it != m_columnsWidth.end())
			{
				m_tableView->setColumnWidth(i, it.value());
			}
		}
	}

}
