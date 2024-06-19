#include "TuningUiEditor.h"
#include <TuningLib/TuningUiItem.h>
#include <UiLib/PropertyEditor.h>

//
// TuningUiEditor
//

using namespace TuningLib;

TuningUiEditor::TuningUiEditor(TuningLib::TuningUiStorage& storage,
							   bool readOnly,
							   bool typeTreeEnabled,
							   bool typeButtonEnabled,
							   bool typeTabEnabled,
							   bool typeCounterEnabled,
							   bool typeSchemasTabsEnabled) :
	m_uiStorage(storage),
	m_readOnly(readOnly),
	m_typeTreeEnabled(typeTreeEnabled),
	m_typeButtonEnabled(typeButtonEnabled),
	m_typeTabEnabled(typeTabEnabled),
	m_typeCounterEnabled(typeCounterEnabled),
	m_typeSchemasTabsEnabled(typeSchemasTabsEnabled)
{
	initUi();

	// Add presets to tree
	//

	for (int i = 0; i < m_uiStorage.root()->childCount(); i++)
	{
		TuningUiItem* uiItem = m_uiStorage.root()->child(i).get();
		if (uiItem == nullptr)
		{
			assert(uiItem);
			return;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setItemText(item, uiItem);
		item->setData(0, Qt::UserRole, uiItem->uuid());
		addChildTreeItems(uiItem, item);

		m_itemsTree->addTopLevelItem(item);
	}

	// Set column width

	for (int i = 0; i < m_itemsTree->columnCount(); i++)
	{
		m_itemsTree->resizeColumnToContents(i);

		if (m_itemsTree->columnWidth(i) < 200)
		{
			m_itemsTree->setColumnWidth(i, 200);
		}
	}
	//
}

TuningUiEditor::~TuningUiEditor()
{
	if (m_hSplitter != nullptr)
	{
		QSettings().setValue("TuningUiEditor/splitterState", m_hSplitter->saveState());
		QSettings().setValue("TuningUiEditor/splitterPosition", m_propertyEditor->splitterPosition());
	}
}


bool TuningUiEditor::readOnly() const
{
	return m_readOnly;
}

void TuningUiEditor::setReadOnly(bool value)
{
	m_readOnly = value;
	onItemSelectionChanged();
}

bool TuningUiEditor::eventFilter(QObject *obj, QEvent *event)
{
	// Filter Enter key press from PropertyEditor, it will close the dialog

	if (obj == m_propertyEditor && event->type() == QEvent::KeyPress)
	{
		return true;
	}
	else
	{
		return QWidget::eventFilter(obj, event);
	}
}

void TuningUiEditor::onAdd()
{
	// Get the type of selected item
	//
	TuningUiItem* selectedUiItem = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();
	if (selectedItems.isEmpty() == false)
	{
		selectedUiItem = m_uiStorage.root()->find(selectedItems[0]->data(0, Qt::UserRole).value<QUuid>()).get();
		if (selectedUiItem == nullptr)
		{
			assert(selectedUiItem);
			return;
		}
	}

	// Allow items

	bool allowTree = (selectedUiItem == nullptr);
	bool allowTabs = (selectedUiItem == nullptr || selectedUiItem->isButton());
	bool allowButtons = (selectedUiItem == nullptr || selectedUiItem->isTab());
	bool allowCounters = (selectedUiItem == nullptr);
	bool allowSchemasTabs = (selectedUiItem == nullptr);

	if (selectedUiItem == nullptr)
	{
		// Buttons and Tabs can't be both added to top level

		for (int i = 0; i < m_itemsTree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = m_itemsTree->topLevelItem(i);

			std::shared_ptr<TuningUiItem> f = m_uiStorage.root()->find(item->data(0, Qt::UserRole).value<QUuid>());
			if (f == nullptr)
			{
				assert(f);
				return;
			}

			if (f->isButton())
			{
				allowTabs = false;
			}

			if (f->isTab())
			{
				allowButtons = false;
			}
		}
	}

	// Disable menu items for custom editor type

	allowTree &= m_typeTreeEnabled;
	allowTabs &= m_typeTabEnabled;
	allowButtons &= m_typeButtonEnabled;
	allowCounters &= m_typeCounterEnabled;
	allowSchemasTabs &= m_typeSchemasTabsEnabled;

	/*
	if (m_typeTabEnabled == false &&
		m_typeButtonEnabled == false &&
		m_typeCounterEnabled == false &&
		m_typeSchemasTabsEnabled == false &&
		allowTree == true)
	{
		addItem(TuningUiItem::InterfaceType::Generic);
		return;
	}*/

	// Create menu

	QMenu menu(this);

	/*
	{
		// Tree
		QAction* action = new QAction(tr("Generic"), &menu);

		auto f = [this]() -> void
		{
			addItem(TuningUiItem::InterfaceType::Generic);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowTree);

		menu.addAction(action);
	}*/

	{
		// Tab
		QAction* action = new QAction(tr("Tab"), &menu);

		auto f = [this]() -> void
		{
				addItem(TuningUiItem::InterfaceType::Tab);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowTabs);

		menu.addAction(action);
	}

	{
		// Tab
		QAction* action = new QAction(tr("Button"), &menu);

		auto f = [this]() -> void
		{
				addItem(TuningUiItem::InterfaceType::Button);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowButtons);

		menu.addAction(action);
	}

	{
		// Counter
		QAction* action = new QAction(tr("Counter"), &menu);

		auto f = [this]() -> void
		{
				addItem(TuningUiItem::InterfaceType::Counter);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowCounters);

		menu.addAction(action);
	}

	{
		// Counter
		QAction* action = new QAction(tr("Schemas Tab"), &menu);

		auto f = [this]() -> void
		{
				addItem(TuningUiItem::InterfaceType::SchemasTab);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowSchemasTabs);

		menu.addAction(action);
	}

	// Run the menu

	menu.exec(QCursor::pos());
}

void TuningUiEditor::onRemove()
{
	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want to remove selected items?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	while (true)
	{
		// Create the list of selected Presets
		//
		QList<QTreeWidgetItem*> selectedItems;
		for (auto p : m_itemsTree->selectedItems())
		{
			selectedItems.push_back(p);
		}

		if (selectedItems.isEmpty() == true)
		{
			return;
		}

		// Delete the first selected preset
		//
		QTreeWidgetItem* item = selectedItems[0];
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		std::shared_ptr<TuningUiItem> uiItem = m_uiStorage.root()->find(item->data(0, Qt::UserRole).value<QUuid>());
		if (uiItem == nullptr)
		{
			assert(uiItem);
			return;
		}

		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem == nullptr)
		{
			m_uiStorage.root()->removeChild(uiItem->uuid());

			QTreeWidgetItem* deleteItem = m_itemsTree->takeTopLevelItem(m_itemsTree->indexOfTopLevelItem(item));
			delete deleteItem;
		}
		else
		{
			TuningUiItem* parentUiUtem = uiItem->parentItem();
			if (parentUiUtem == nullptr)
			{
				assert(parentUiUtem);
				return;
			}
			parentUiUtem->removeChild(uiItem->uuid());

			QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
			delete deleteItem;
		}

		m_modified = true;
	}

}

void TuningUiEditor::onMoveUp()
{
	moveItems(-1);

}

void TuningUiEditor::onMoveDown()
{
	moveItems(1);
}

void TuningUiEditor::onCopy()
{
	std::vector<std::shared_ptr<TuningUiItem>> uiItems;

	// Create the list of selected Presets
	//
	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();
	for (auto item : selectedItems)
	{
		std::shared_ptr<TuningUiItem> uiItem = m_uiStorage.root()->find(item->data(0, Qt::UserRole).value<QUuid>());
		if (uiItem == nullptr)
		{
			assert(uiItem);
			return;
		}

		uiItems.push_back(uiItem);
	}

	if (uiItems.empty() == true)
	{
		return;
	}

	{
		// save data to clipboard
		//
		TuningUiStorage clipboardStorage;

		auto root = clipboardStorage.root();
		root->setInterfaceType(TuningUiItem::InterfaceType::Root);

		for (const auto& item : uiItems)
		{
			root->addChild(item);
		}

		QByteArray data;
		clipboardStorage.save(data);

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(data.toStdString().c_str());
	}
}

void TuningUiEditor::onPaste()
{
	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();

	// Choose selected item
	//
	QTreeWidgetItem* parentItem = nullptr;
	std::shared_ptr<TuningUiItem> parentUiUtem = nullptr;

	if (selectedItems.isEmpty() == false)
	{
		QTreeWidgetItem* firstTreeItem = selectedItems.front();
		Q_ASSERT(firstTreeItem);

		std::shared_ptr<TuningUiItem> uiItem = m_uiStorage.root()->find(firstTreeItem->data(0, Qt::UserRole).value<QUuid>());
		if (uiItem == nullptr)
		{
			Q_ASSERT(uiItem);
			return;
		}

		parentUiUtem = uiItem;
		parentItem = firstTreeItem;
	}


	// Paste items from clipboard
	//
	TuningUiStorage clipboardStorage;

	{
		QClipboard* clipboard = QApplication::clipboard();
		QString clipboardText = clipboard->text();
		if (clipboardText.isEmpty() == true)
		{
			return;
		}

		QByteArray data = clipboardText.toUtf8();


		QString errorMsg;
		if (clipboardStorage.load(data, &errorMsg) == false)
		{
			return;
		}
	}

	int count = clipboardStorage.root()->childCount();
	if (count == 0)
	{
		return;
	}
	// Place new UUIDs to pasted items
	{
		std::vector<TuningUiItem*> pastedItems = clipboardStorage.root()->childernToVector();
		for (const auto& pi : pastedItems) 
		{
			QUuid uuid = QUuid::createUuid();
			pi->setUuid(uuid);
		}
	}

	// Append pasted items to the storage
	//
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningUiItem> newUiItem = clipboardStorage.root()->child(i);

		QTreeWidgetItem* newTreeItem = new QTreeWidgetItem();
		setItemText(newTreeItem, newUiItem.get());
		newTreeItem->setData(0, Qt::UserRole, newUiItem->uuid());
		addChildTreeItems(newUiItem.get(), newTreeItem);
		
		if (parentItem == nullptr || parentUiUtem == nullptr)
		{
			// no item was selected, add top level item
			//
			m_uiStorage.root()->addChild(newUiItem);
			m_itemsTree->addTopLevelItem(newTreeItem);
		}
		else
		{
			// an item was selected, add child item
			//
			parentUiUtem->addChild(newUiItem);
			parentItem->addChild(newTreeItem);
		}
	}

	m_modified = true;
}

void TuningUiEditor::onItemSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();

	qsizetype presetsCount = selectedItems.size();

	m_addPreset->setEnabled(m_readOnly == false);
	m_addPresetAction->setEnabled(m_addPreset->isEnabled());

	m_removePreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_removePresetAction->setEnabled(m_removePreset->isEnabled());

	m_copyPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_copyPresetAction->setEnabled(m_copyPreset->isEnabled());

	m_pastePreset->setEnabled(m_readOnly == false);
	m_pastePresetAction->setEnabled(m_readOnly == false);

	m_moveUpPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_moveUpPresetAction->setEnabled(m_moveUpPreset->isEnabled());

	m_moveDownPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_moveDownPresetAction->setEnabled(m_moveDownPreset->isEnabled());

	//

	QList<std::shared_ptr<PropertyObject>> selectedUiItems;


	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningUiItem> uiItem = m_uiStorage.root()->find(item->data(0, Qt::UserRole).value<QUuid>());
		if (uiItem == nullptr)
		{
			assert(uiItem);
			return;
		}
		selectedUiItems.push_back(uiItem);
	}

	m_propertyEditor->setObjects(selectedUiItems);
	m_propertyEditor->setReadOnly(m_readOnly);
}

void TuningUiEditor::onContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_itemsTreeContextMenu->exec(this->cursor().pos());
}

void TuningUiEditor::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningUiItem> selectedUiItem = m_uiStorage.root()->find(item->data(0, Qt::UserRole).value<QUuid>());
		if (selectedUiItem == nullptr)
		{
			assert(selectedUiItem);
			return;
		}

		for (std::shared_ptr<PropertyObject> modifiedFilter : objects)
		{
			TuningUiItem* uiItem = dynamic_cast<TuningUiItem*>(modifiedFilter.get());
			if (uiItem == nullptr)
			{
				assert(uiItem);
				return;

			}

			uiItem->updateOptionalProperties();

			if (selectedUiItem->uuid() == uiItem->uuid())
			{
				setItemText(item, uiItem);
				break;
			}
		}
	}
}

void TuningUiEditor::initUi()
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);


	// Create splitter control
	//
	m_hSplitter = new QSplitter();


	// Left part
	//

	QWidget* lw = new QWidget();
	QVBoxLayout* leftLayout = new QVBoxLayout(lw);
	leftLayout->setContentsMargins(0, 0, 0, 0);

	m_itemsTree = new QTreeWidget();
	m_itemsTree->setExpandsOnDoubleClick(false);

	QStringList headerLabels;
	headerLabels << tr("Caption");
	headerLabels << tr("Type");

	m_itemsTree->setColumnCount(static_cast<int>(headerLabels.size()));
	m_itemsTree->setHeaderLabels(headerLabels);
	m_itemsTree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	connect(m_itemsTree, &QTreeWidget::itemSelectionChanged, this, &TuningUiEditor::onItemSelectionChanged);

	m_itemsTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_itemsTree, &QTreeWidget::customContextMenuRequested, this, &TuningUiEditor::onContextMenu);

	leftLayout->addWidget(m_itemsTree);

	QHBoxLayout* leftGridLayout = new QHBoxLayout();

	m_addPreset = new QPushButton(tr("Add Ui Item"));
	connect(m_addPreset, &QPushButton::clicked, this, &TuningUiEditor::onAdd);
	leftGridLayout->addWidget(m_addPreset);

	m_removePreset = new QPushButton(tr("Remove Ui Item"));
	m_removePreset->setEnabled(false);
	connect(m_removePreset, &QPushButton::clicked, this, &TuningUiEditor::onRemove);
	leftGridLayout->addWidget(m_removePreset);

	leftGridLayout->addStretch();

	m_moveUpPreset = new QPushButton(tr("Up"));
	connect(m_moveUpPreset, &QPushButton::clicked, this, &TuningUiEditor::onMoveUp);
	m_moveUpPreset->setEnabled(false);
	leftGridLayout->addWidget(m_moveUpPreset);

	m_moveDownPreset = new QPushButton(tr("Down"));
	m_moveDownPreset->setEnabled(false);
	connect(m_moveDownPreset, &QPushButton::clicked, this, &TuningUiEditor::onMoveDown);
	leftGridLayout->addWidget(m_moveDownPreset);

	leftGridLayout->addStretch();

	m_copyPreset = new QPushButton(tr("Copy"));
	m_copyPreset->setEnabled(false);
	connect(m_copyPreset, &QPushButton::clicked, this, &TuningUiEditor::onCopy);
	leftGridLayout->addWidget(m_copyPreset);

	m_pastePreset = new QPushButton(tr("Paste"));
	connect(m_pastePreset, &QPushButton::clicked, this, &TuningUiEditor::onPaste);
	leftGridLayout->addWidget(m_pastePreset);

	leftLayout->addLayout(leftGridLayout);

	m_addPresetAction = new QAction(tr("Add Ui Item"), this);
	connect(m_addPresetAction, &QAction::triggered, this, &TuningUiEditor::onAdd);

	m_removePresetAction = new QAction(tr("Remove Ui Item"), this);
	connect(m_removePresetAction, &QAction::triggered, this, &TuningUiEditor::onRemove);

	m_moveUpPresetAction = new QAction(tr("Move Up"), this);
	connect(m_moveUpPresetAction, &QAction::triggered, this, &TuningUiEditor::onMoveUp);

	m_moveDownPresetAction = new QAction(tr("Move Down"), this);
	connect(m_moveDownPresetAction, &QAction::triggered, this, &TuningUiEditor::onMoveDown);

	m_copyPresetAction = new QAction(tr("Copy"), this);
	connect(m_copyPresetAction, &QAction::triggered, this, &TuningUiEditor::onCopy);

	m_pastePresetAction = new QAction(tr("Paste"), this);
	connect(m_pastePresetAction, &QAction::triggered, this, &TuningUiEditor::onPaste);

	m_itemsTreeContextMenu = new QMenu(this);
	m_itemsTreeContextMenu->addAction(m_addPresetAction);
	m_itemsTreeContextMenu->addAction(m_removePresetAction);
	m_itemsTreeContextMenu->addSeparator();
	m_itemsTreeContextMenu->addAction(m_moveUpPresetAction);
	m_itemsTreeContextMenu->addAction(m_moveDownPresetAction);
	m_itemsTreeContextMenu->addSeparator();
	m_itemsTreeContextMenu->addAction(m_copyPresetAction);
	m_itemsTreeContextMenu->addAction(m_pastePresetAction);
	//

	m_hSplitter->addWidget(lw);

	// Right side

	QWidget* rw = new QWidget();
	QVBoxLayout* rightLayout = new QVBoxLayout(rw);
	rightLayout->setContentsMargins(0, 0, 0, 0);

	QTabWidget* tab = new QTabWidget();
	rightLayout->addWidget(tab);

	// Properties Tab

	QWidget* propertyTabWidget = new QWidget();

	QHBoxLayout* propertyEditorLayout = new QHBoxLayout(propertyTabWidget);

	m_propertyEditor = new ExtWidgets::PropertyEditor(propertyTabWidget);

	propertyEditorLayout->addWidget(m_propertyEditor);

	m_propertyEditor->installEventFilter(this);

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &TuningUiEditor::onPropertiesChanged);

	tab->addTab(propertyTabWidget, tr("Properties"));

	//

	m_hSplitter->addWidget(rw);

	//

	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_hSplitter);

	setLayout(mainLayout);

	// Restore size
	//
	int splitterPosition = QSettings().value("TuningUiEditor/splitterPosition", -1).toInt();
	if (splitterPosition < 0 || splitterPosition > 100)
	{
		m_propertyEditor->setSplitterPosition(splitterPosition);
	}
	else
	{
		m_propertyEditor->setSplitterPosition(100);
	}

	QByteArray ba = QSettings().value("TuningUiEditor/splitterState").toByteArray();
	if (ba.isEmpty() == false)
	{
		m_hSplitter->restoreState(ba);
	}
}

void TuningUiEditor::addItem(TuningLib::TuningUiItem::InterfaceType uiType)
{
	std::shared_ptr<TuningUiItem> uiItem = std::make_shared<TuningUiItem>();
	uiItem->setInterfaceType(uiType);
	uiItem->setUuid(QUuid::createUuid());
	uiItem->setCaption(tr("New Item"));
	uiItem->updateOptionalProperties();

	QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
	setItemText(newPresetItem, uiItem.get());
	newPresetItem->setData(0, Qt::UserRole, uiItem->uuid());

	QTreeWidgetItem* parentItem = nullptr;
	std::shared_ptr<TuningUiItem> parentUiItem = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();
	if (selectedItems.isEmpty() == false)
	{
		parentItem = selectedItems[0];

		parentUiItem = m_uiStorage.root()->find(parentItem->data(0, Qt::UserRole).value<QUuid>());
		if (parentUiItem == nullptr)
		{
			assert(parentUiItem);
			return;
		}
	}

	if (parentItem == nullptr || parentUiItem == nullptr)
	{
		// no item was selected, add top level item
		//
		m_uiStorage.root()->addChild(uiItem);
		m_itemsTree->addTopLevelItem(newPresetItem);
		newPresetItem->setSelected(true);
	}
	else
	{
		// an item was selected, add child item
		//
		parentUiItem->addChild(uiItem);
		parentItem->addChild(newPresetItem);
		parentItem->setExpanded(true);
		parentItem->setSelected(false);
		newPresetItem->setSelected(true);
	}

	m_modified = true;
	return;
}


void TuningUiEditor::addChildTreeItems(TuningUiItem* uiItem, QTreeWidgetItem* parentItem)
{
	if (uiItem == nullptr)
	{
		assert(uiItem);
		return;
	}

	if (parentItem == nullptr)
	{
		assert(parentItem);
		return;
	}

	// Add child presets
	//
	for (int i = 0; i < uiItem->childCount(); i++)
	{
		TuningUiItem* childUiItem = uiItem->child(i).get();
		if (uiItem == nullptr)
		{
			assert(uiItem);
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setItemText(item, childUiItem);
		item->setData(0, Qt::UserRole, childUiItem->uuid());

		addChildTreeItems(childUiItem, item);
		parentItem->addChild(item);
	}
}

void TuningUiEditor::setItemText(QTreeWidgetItem* item, TuningUiItem* uiItem)
{
	if (item == nullptr || uiItem == nullptr)
	{
		assert(item);
		assert(uiItem);
		return;
	}

	QStringList l;
	l << uiItem->caption();
	l << E::valueToString<TuningUiItem::InterfaceType>(uiItem->interfaceType());

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}


void TuningUiEditor::moveItems(int direction)
{
	QList<QTreeWidgetItem*> selectedItems = m_itemsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	// Check if selected items have same parent
	//

	bool first = true;

	QTreeWidgetItem* parentItem = nullptr;

	for (auto selectedItem : selectedItems)
	{
		if (first == true)
		{
			first = false;
			parentItem = selectedItem->parent();
		}
		else
		{
			if (parentItem != selectedItem->parent())
			{
				QMessageBox::critical(this, qAppName(), tr("To change items order, select items of the same parent!"));
				return;
			}
		}
	}

	// Remember indexes
	//
	std::vector<int> selectedIndexes;

	for (auto selectedItem : selectedItems)
	{
		if (parentItem == nullptr)
		{
			 selectedIndexes.push_back(m_itemsTree->indexOfTopLevelItem(selectedItem));
		}
		else
		{
			selectedIndexes.push_back(parentItem->indexOfChild(selectedItem));
		}
	}

	// Sort indexes
	//
	if (direction < 0)
	{
		std::sort(selectedIndexes.begin(), selectedIndexes.end(), std::less<int>());
	}
	else
	{
		std::sort(selectedIndexes.begin(), selectedIndexes.end(), std::greater<int>());
	}

	// Change order
	//
	TuningUiItem* parentUiItem = nullptr;

	if (parentItem == nullptr)
	{
		parentUiItem = m_uiStorage.root();
	}
	else
	{
		parentUiItem = m_uiStorage.root()->find(parentItem->data(0, Qt::UserRole).value<QUuid>()).get();
	}
	if (parentUiItem == nullptr)
	{
		Q_ASSERT(parentUiItem);
		return;
	}

	int maxIndex = parentItem == nullptr ? m_itemsTree->topLevelItemCount() : parentItem->childCount();

	for (int index : selectedIndexes)
	{
		int newIndex = index + direction;
		if (newIndex < 0 || newIndex >= maxIndex)
		{
			return;
		}

		// Swap filters

		std::shared_ptr<TuningUiItem> childUiItem = parentUiItem->child(index);
		if (childUiItem == nullptr)
		{
			Q_ASSERT(childUiItem);
			return;
		}

		parentUiItem->removeChild(childUiItem->uuid());
		parentUiItem->insertChild(newIndex, childUiItem);

		// Swap tree items

		if (parentItem == nullptr)
		{
			// These are top level items
			//

			QTreeWidgetItem* takeItem = m_itemsTree->takeTopLevelItem(index);
			m_itemsTree->insertTopLevelItem(newIndex, takeItem);
			takeItem->setSelected(true);
		}
		else
		{
			QTreeWidgetItem* takeItem = parentItem->takeChild(index);
			parentItem->insertChild(newIndex, takeItem);
			takeItem->setSelected(true);
		}
	}
}


