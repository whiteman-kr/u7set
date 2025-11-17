#include "TuningFilterEditor.h"

using namespace TuningFilters;

//
// ViewTuningSignalsWidget
//

ViewTuningSignalsWidget::ViewTuningSignalsWidget(ClientLib::TuningSignalManager& signalManager, QWidget* parent) :
	QWidget(parent),
	m_signalManager(signalManager)
{
	setWindowTitle(tr("Filter Signals"));

	// Left part
	//

	QHBoxLayout* mainLayout = new QHBoxLayout();

	// Right part
	//

	QVBoxLayout* rightLayout = new QVBoxLayout();

	m_filterValuesTree = new QTreeWidget();
	m_filterValuesTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_filterValuesTree->setWordWrap(false);

	QStringList headers;
	headers << tr("CustomAppSignalID");
	headers << tr("AppSignalID");
	headers << tr("Type");
	headers << tr("Caption");
	headers << tr("Value");

	m_filterValuesTree->setColumnCount(static_cast<int>(headers.size()));
	m_filterValuesTree->setHeaderLabels(headers);
	rightLayout->addWidget(m_filterValuesTree);

	QHBoxLayout* rightGridLayout = new QHBoxLayout();

	rightGridLayout->addStretch();

	m_exportValues = new QPushButton(tr("Export..."));
	connect(m_exportValues, &QPushButton::clicked, this, &ViewTuningSignalsWidget::on_m_exportValues_clicked);
	rightGridLayout->addWidget(m_exportValues);
	m_exportValues->setEnabled(false);

	rightLayout->addLayout(rightGridLayout);

	mainLayout->addLayout(rightLayout);

	setLayout(mainLayout);
}

bool ViewTuningSignalsWidget::readOnly() const
{
	return m_readOnly;
}

void ViewTuningSignalsWidget::setReadOnly(bool value)
{
	m_readOnly = value;

	return;
}

void ViewTuningSignalsWidget::setFilter(std::shared_ptr<TuningFilter> selectedFilter)
{
	m_filter = selectedFilter;

	m_exportValues->setEnabled(selectedFilter != nullptr);

	fillFilterValuesTree();
}

void ViewTuningSignalsWidget::fillFilterValuesTree()
{
	if (m_filterValuesTree == nullptr)
	{
		Q_ASSERT(m_filterValuesTree);
		return;
	}

	m_filterValuesTree->clear();

	if (m_filter != nullptr)
	{
		std::vector<TuningFilterSignal> values = m_filter->getFilterSignals();

		for (const TuningFilterSignal& v : values)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			setFilterValueItemText(item, v);
			m_filterValuesTree->addTopLevelItem(item);
		}

		for (int i = 0; i < m_filterValuesTree->columnCount(); i++)
		{
			m_filterValuesTree->resizeColumnToContents(i);
		}

		m_filterValuesTree->setSortingEnabled(true);
		m_filterValuesTree->sortByColumn(0, Qt::AscendingOrder);
	}
}


void ViewTuningSignalsWidget::on_m_exportValues_clicked()
{
	int columnCount = m_filterValuesTree->columnCount();

	int rowCount = m_filterValuesTree->topLevelItemCount();

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"), path + QDir::separator(), tr("CSV (*.csv)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QFile file(fileName);
	if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
		return;
	}

	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);

	QString csvHeader;

	for (int c = 0; c < columnCount; c++)
	{
		QString str = m_filterValuesTree->headerItem()->text(c);
		csvHeader += str;
		csvHeader += ';';
	}

	out << csvHeader << "\r\n";

	for (int r = 0; r < rowCount; r++)
	{
		QString csvRow;

		QTreeWidgetItem* item = m_filterValuesTree->topLevelItem(r);

		for (int c = 0; c < columnCount; c++)
		{
			QString str = item->text(c);
			csvRow += str;
			csvRow += ';';
		}

		out << csvRow << "\r\n";
	}

	out.flush();

	QMessageBox::information(this, qAppName(), tr("Export complete."));
}

void ViewTuningSignalsWidget::setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterSignal& value)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(static_cast<int>(Columns::AppSignalID), Qt::UserRole, value.appSignalHash());

	if (m_signalManager.signalExists(value.appSignalHash()) == false)
	{
		QStringList l;
		l.push_back("?");
		l.push_back(value.appSignalId());
		l.push_back("?");
		l.push_back("?");
		l.push_back("?");

		int i = 0;
		for (auto s : l)
		{
			item->setText(i++, s);
		}

		return;
	}

	auto asp = m_signalManager.signalParam(value.appSignalHash());
	if (asp.has_value() == false)
	{
		assert(asp.has_value());
		return;
	}

	QStringList l;
	l.push_back(asp->customSignalId());
	l.push_back(value.appSignalId());
	l.push_back(asp->tuningDefaultValue().tuningValueTypeString());
	l.push_back(asp->caption());
	if (value.useValue() == true)
	{
		l.push_back(value.value().toString());
	}
	else
	{
		l.push_back("<Default>");
	}

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

//
// TuningFilterEditor
//

TuningFilterEditor::TuningFilterEditor(TuningFilterStorage& filterStorage,
									   ClientLib::TuningSignalManager& signalManager,
									   bool readOnly,
									   bool typeTreeEnabled,
									   bool typeButtonEnabled,
									   bool typeTabEnabled,
									   bool typeCounterEnabled,
									   bool typeSchemasTabsEnabled,
									   TuningFilter::Source source,
									   QByteArray mainSplitterState,
									   int propertyEditorSplitterPos) :
	m_filterStorage(filterStorage),
	m_signalManager(signalManager),
	m_readOnly(readOnly),
	m_typeTreeEnabled(typeTreeEnabled),
	m_typeButtonEnabled(typeButtonEnabled),
	m_typeTabEnabled(typeTabEnabled),
	m_typeCounterEnabled(typeCounterEnabled),
	m_typeSchemasTabsEnabled(typeSchemasTabsEnabled),
	m_source(source)
{
	initUserInterface(mainSplitterState, propertyEditorSplitterPos);


	// Add presets to tree
	//

	for (int i = 0; i < m_filterStorage.root()->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = m_filterStorage.root()->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			return;
		}

		if (f->source() != m_source)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		addChildTreeObjects(f, item);

		m_presetsTree->addTopLevelItem(item);
	}

	// Set column width

	for (int i = 0; i < m_presetsTree->columnCount(); i++)
	{
		m_presetsTree->resizeColumnToContents(i);

		if (m_presetsTree->columnWidth(i) < 200)
		{
			m_presetsTree->setColumnWidth(i, 200);
		}
	}

	//
}

TuningFilterEditor::~TuningFilterEditor() {}


bool TuningFilterEditor::readOnly() const
{
	return m_readOnly;
}

void TuningFilterEditor::setReadOnly(bool value)
{
	m_readOnly = value;

	// m_ViewTuningSignalsWidget->setReadOnly(m_readOnly);

	on_m_presetsTree_itemSelectionChanged();
}

void TuningFilterEditor::saveUserInterfaceSettings(QByteArray* mainSplitterState, int* propertyEditorSplitterPos)
{
	if (mainSplitterState == nullptr || propertyEditorSplitterPos == nullptr)
	{
		Q_ASSERT(mainSplitterState);
		Q_ASSERT(propertyEditorSplitterPos);
		return;
	}

	if (m_hSplitter != nullptr)
	{
		*mainSplitterState = m_hSplitter->saveState();
	}
	if (m_propertyEditor != nullptr)
	{
		*propertyEditorSplitterPos = m_propertyEditor->splitterPosition();
	}
}

bool TuningFilterEditor::eventFilter(QObject* obj, QEvent* event)
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

void TuningFilterEditor::on_m_addPreset_clicked()
{
	// Get the type of selected filter
	//
	std::shared_ptr<TuningFilter> selectedFilter = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	if (selectedItems.isEmpty() == false)
	{
		QTreeWidgetItem* item = selectedItems[0];

		selectedFilter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

		if (selectedFilter == nullptr)
		{
			assert(selectedFilter);
			return;
		}
	}

	// Allow items

	bool allowTree = (selectedFilter == nullptr || selectedFilter->isTree());
	bool allowTabs = (selectedFilter == nullptr || selectedFilter->isButton());
	bool allowButtons = (selectedFilter == nullptr || selectedFilter->isTab());
	bool allowCounters = (selectedFilter == nullptr);
	bool allowSchemasTabs = (selectedFilter == nullptr);

	if (selectedFilter == nullptr)
	{
		// Buttons and Tabs can't be both added to top level

		for (int i = 0; i < m_presetsTree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = m_presetsTree->topLevelItem(i);

			std::shared_ptr<TuningFilter> f = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

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

	if (m_typeTabEnabled == false && m_typeButtonEnabled == false && m_typeCounterEnabled == false && m_typeSchemasTabsEnabled == false &&
		allowTree == true)
	{
		// This is made for TuningClient

		addPreset(TuningFilter::InterfaceType::Tree);
		return;
	}

	// Create menu

	QMenu menu(this);

	{
		// Tree
		QAction* action = new QAction(tr("Tree"), &menu);

		auto f = [this]() -> void
		{
			addPreset(TuningFilter::InterfaceType::Tree);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowTree);

		menu.addAction(action);
	}

	{
		// Tab
		QAction* action = new QAction(tr("Tab"), &menu);

		auto f = [this]() -> void
		{
			addPreset(TuningFilter::InterfaceType::Tab);
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
			addPreset(TuningFilter::InterfaceType::Button);
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
			addPreset(TuningFilter::InterfaceType::Counter);
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
			addPreset(TuningFilter::InterfaceType::SchemasTab);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowSchemasTabs);

		menu.addAction(action);
	}

	// Run the menu

	menu.exec(QCursor::pos());
}

void TuningFilterEditor::on_m_removePreset_clicked()
{
	if (QMessageBox::warning(this,
							 tr("Remove Filter"),
							 tr("Are you sure you want to remove selected filters?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	while (true)
	{
		// Create the list of selected Presets
		//
		QList<QTreeWidgetItem*> selectedPresets;
		for (auto p : m_presetsTree->selectedItems())
		{
			selectedPresets.push_back(p);
		}

		if (selectedPresets.isEmpty() == true)
		{
			return;
		}

		// Delete the first selected preset
		//
		QTreeWidgetItem* item = selectedPresets[0];
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem == nullptr)
		{
			m_filterStorage.root()->removeChild(filter);

			QTreeWidgetItem* deleteItem = m_presetsTree->takeTopLevelItem(m_presetsTree->indexOfTopLevelItem(item));
			delete deleteItem;
		}
		else
		{
			TuningFilter* parentFilter = filter->parentFilter();
			if (parentFilter == nullptr)
			{
				assert(parentFilter);
				return;
			}
			parentFilter->removeChild(filter);

			QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
			delete deleteItem;
		}

		m_modified = true;
	}
}

void TuningFilterEditor::on_m_moveUpPreset_clicked()
{
	movePresets(-1);
}

void TuningFilterEditor::on_m_moveDownPreset_clicked()
{
	movePresets(1);
}

void TuningFilterEditor::on_m_copyPreset_clicked()
{
	std::vector<std::shared_ptr<TuningFilter>> filters;

	// Create the list of selected Presets
	//
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	for (auto p : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = p->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		filters.push_back(filter);
	}

	if (filters.empty() == true)
	{
		return;
	}

	m_filterStorage.copyToClipboard(filters);
}

void TuningFilterEditor::on_m_pastePreset_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	QTreeWidgetItem* parentItem = nullptr;

	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	if (selectedItems.isEmpty() == false)
	{
		QTreeWidgetItem* firstTreeItem = selectedItems.front();
		Q_ASSERT(firstTreeItem);

		std::shared_ptr<TuningFilter> filter = firstTreeItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			Q_ASSERT(filter);
			return;
		}

		parentFilter = filter;
		parentItem = firstTreeItem;
	}

	std::shared_ptr<TuningFilter> pastedRoot = m_filterStorage.pasteFromClipboard();

	if (pastedRoot == nullptr)
	{
		return;
	}

	int count = pastedRoot->childFiltersCount();
	if (count == 0)
	{
		return;
	}

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> newFilter = pastedRoot->childFilter(i);

		QUuid uid = QUuid::createUuid();
		newFilter->setID(uid.toString());

		QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
		setFilterItemText(newPresetItem, newFilter.get());
		newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));

		addChildTreeObjects(newFilter, newPresetItem);

		if (parentItem == nullptr || parentFilter == nullptr)
		{
			// no item was selected, add top level item
			//
			m_filterStorage.root()->addChild(newFilter);
			m_presetsTree->addTopLevelItem(newPresetItem);
		}
		else
		{
			// an item was selected, add child item
			//
			parentFilter->addChild(newFilter);

			parentItem->addChild(newPresetItem);
		}
	}

	m_modified = true;
}

void TuningFilterEditor::on_m_presetsTree_itemSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

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

	QList<std::shared_ptr<PropertyObject>> selectedFilters;

	std::shared_ptr<TuningFilter> firstFilter = nullptr;


	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		selectedFilters.push_back(filter);

		if (firstFilter == nullptr)
		{
			firstFilter = filter;
		}
	}

	if (selectedItems.size() == 1)
	{
		m_viewTuningSignalsWidget->setFilter(firstFilter);
	}
	else
	{
		m_viewTuningSignalsWidget->setFilter(nullptr);
	}

	m_propertyEditor->setObjects(selectedFilters);

	m_propertyEditor->setReadOnly(m_readOnly);
}

void TuningFilterEditor::on_m_presetsTree_contextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_presetsTreeContextMenu->exec(this->cursor().pos());
}

void TuningFilterEditor::presetPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	QList<std::shared_ptr<PropertyObject>> selectedFilters;

	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> selectedFilter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (selectedFilter == nullptr)
		{
			assert(selectedFilter);
			return;
		}

		selectedFilters.push_back(selectedFilter);

		for (std::shared_ptr<PropertyObject> modifiedFilter : objects)
		{
			TuningFilter* f = dynamic_cast<TuningFilter*>(modifiedFilter.get());

			if (f == nullptr)
			{
				assert(f);
				return;
			}

			f->updateOptionalProperties();

			if (selectedFilter->ID() == f->ID())
			{
				setFilterItemText(item, f);
				break;
			}
		}
	}
}

void TuningFilterEditor::slot_getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok)
{
	emit getCurrentSignalValue(appSignalHash, value, ok);
}

void TuningFilterEditor::initUserInterface(QByteArray mainSplitterState, int propertyEditorSplitterPos)
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

	m_presetsTree = new QTreeWidget();
	m_presetsTree->setExpandsOnDoubleClick(false);

	QStringList headerLabels;
	headerLabels << tr("Caption");
	headerLabels << tr("Type");

	m_presetsTree->setColumnCount(static_cast<int>(headerLabels.size()));
	m_presetsTree->setHeaderLabels(headerLabels);
	m_presetsTree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	connect(m_presetsTree, &QTreeWidget::itemSelectionChanged, this, &TuningFilterEditor::on_m_presetsTree_itemSelectionChanged);

	m_presetsTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_presetsTree, &QTreeWidget::customContextMenuRequested, this, &TuningFilterEditor::on_m_presetsTree_contextMenu);

	leftLayout->addWidget(m_presetsTree);

	QHBoxLayout* leftGridLayout = new QHBoxLayout();

	m_addPreset = new QPushButton(tr("Add Filter"));
	connect(m_addPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_addPreset_clicked);
	leftGridLayout->addWidget(m_addPreset);

	m_removePreset = new QPushButton(tr("Remove Filter"));
	m_removePreset->setEnabled(false);
	connect(m_removePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_removePreset_clicked);
	leftGridLayout->addWidget(m_removePreset);

	leftGridLayout->addStretch();

	m_moveUpPreset = new QPushButton(tr("Up"));
	connect(m_moveUpPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveUpPreset_clicked);
	m_moveUpPreset->setEnabled(false);
	leftGridLayout->addWidget(m_moveUpPreset);

	m_moveDownPreset = new QPushButton(tr("Down"));
	m_moveDownPreset->setEnabled(false);
	connect(m_moveDownPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveDownPreset_clicked);
	leftGridLayout->addWidget(m_moveDownPreset);

	leftGridLayout->addStretch();

	m_copyPreset = new QPushButton(tr("Copy"));
	m_copyPreset->setEnabled(false);
	connect(m_copyPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_copyPreset_clicked);
	leftGridLayout->addWidget(m_copyPreset);

	m_pastePreset = new QPushButton(tr("Paste"));
	connect(m_pastePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_pastePreset_clicked);
	leftGridLayout->addWidget(m_pastePreset);

	leftLayout->addLayout(leftGridLayout);

	m_addPresetAction = new QAction(tr("Add Filter"), this);
	connect(m_addPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_addPreset_clicked);

	m_removePresetAction = new QAction(tr("Remove Filter"), this);
	connect(m_removePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_removePreset_clicked);

	m_moveUpPresetAction = new QAction(tr("Move Up"), this);
	connect(m_moveUpPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_moveUpPreset_clicked);

	m_moveDownPresetAction = new QAction(tr("Move Down"), this);
	connect(m_moveDownPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_moveDownPreset_clicked);

	m_copyPresetAction = new QAction(tr("Copy"), this);
	connect(m_copyPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_copyPreset_clicked);

	m_pastePresetAction = new QAction(tr("Paste"), this);
	connect(m_pastePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_pastePreset_clicked);

	m_presetsTreeContextMenu = new QMenu(this);
	m_presetsTreeContextMenu->addAction(m_addPresetAction);
	m_presetsTreeContextMenu->addAction(m_removePresetAction);
	m_presetsTreeContextMenu->addSeparator();
	m_presetsTreeContextMenu->addAction(m_moveUpPresetAction);
	m_presetsTreeContextMenu->addAction(m_moveDownPresetAction);
	m_presetsTreeContextMenu->addSeparator();
	m_presetsTreeContextMenu->addAction(m_copyPresetAction);
	m_presetsTreeContextMenu->addAction(m_pastePresetAction);
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

	if (propertyEditorSplitterPos > 100)
	{
		m_propertyEditor->setSplitterPosition(propertyEditorSplitterPos);
	}
	else
	{
		m_propertyEditor->setSplitterPosition(100);
	}

	m_hSplitter->restoreState(mainSplitterState);

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &TuningFilterEditor::presetPropertiesChanged);

	tab->addTab(propertyTabWidget, tr("Properties"));

	// Signals Tab

	m_viewTuningSignalsWidget = new ViewTuningSignalsWidget(m_signalManager, this);

	tab->addTab(m_viewTuningSignalsWidget, tr("Signals"));

	//

	m_hSplitter->addWidget(rw);

	//

	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_hSplitter);

	setLayout(mainLayout);
}

void TuningFilterEditor::addPreset(TuningFilter::InterfaceType interfaceType)
{
	std::shared_ptr<TuningFilter> newFilter = std::make_shared<TuningFilter>(interfaceType);

	QUuid uid = QUuid::createUuid();
	newFilter->setID(uid.toString());
	newFilter->setCaption(tr("New Filter"));
	newFilter->setSource(m_source);

	QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
	setFilterItemText(newPresetItem, newFilter.get());
	newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));

	QTreeWidgetItem* parentItem = nullptr;
	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	if (selectedItems.isEmpty() == false)
	{
		parentItem = selectedItems[0];

		parentFilter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (parentFilter == nullptr)
		{
			assert(parentFilter);
			return;
		}
	}

	if (parentItem == nullptr || parentFilter == nullptr)
	{
		// no item was selected, add top level item
		//
		m_filterStorage.root()->addChild(newFilter);

		m_presetsTree->addTopLevelItem(newPresetItem);

		newPresetItem->setSelected(true);
	}
	else
	{
		// an item was selected, add child item
		//
		parentFilter->addChild(newFilter);

		parentItem->addChild(newPresetItem);

		parentItem->setExpanded(true);

		parentItem->setSelected(false);

		newPresetItem->setSelected(true);
	}

	m_modified = true;
}


void TuningFilterEditor::addChildTreeObjects(const std::shared_ptr<TuningFilter>& filter, QTreeWidgetItem* parent)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (parent == nullptr)
	{
		assert(parent);
		return;
	}

	// Add child presets
	//
	for (int i = 0; i < filter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->source() != m_source)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		addChildTreeObjects(f, item);

		parent->addChild(item);
	}
}

void TuningFilterEditor::setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter)
{
	if (item == nullptr || filter == nullptr)
	{
		assert(item);
		assert(filter);
		return;
	}


	QStringList l;
	l << filter->caption();
	l << E::valueToString<TuningFilter::InterfaceType>(filter->interfaceType());

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}


void TuningFilterEditor::movePresets(int direction)
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
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
				QMessageBox::critical(this, tr("Filter Editor"), tr("To change presets order, select presets of the same parent!"));
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
			selectedIndexes.push_back(m_presetsTree->indexOfTopLevelItem(selectedItem));
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
	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	if (parentItem == nullptr)
	{
		parentFilter = m_filterStorage.root();
	}
	else
	{
		parentFilter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	}

	if (parentFilter == nullptr)
	{
		Q_ASSERT(parentFilter);
		return;
	}

	int maxIndex = parentItem == nullptr ? m_presetsTree->topLevelItemCount() : parentItem->childCount();

	for (int index : selectedIndexes)
	{
		int newIndex = index + direction;
		if (newIndex < 0 || newIndex >= maxIndex)
		{
			return;
		}

		// Swap filters

		std::shared_ptr<TuningFilter> childFilter = parentFilter->childFilter(index);
		if (childFilter == nullptr)
		{
			Q_ASSERT(childFilter);
			return;
		}

		parentFilter->removeChild(childFilter);
		parentFilter->insertChild(newIndex, childFilter);

		// Swap tree items

		if (parentItem == nullptr)
		{
			// These are top level items
			//

			QTreeWidgetItem* takeItem = m_presetsTree->takeTopLevelItem(index);
			m_presetsTree->insertTopLevelItem(newIndex, takeItem);
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

//
// IdeTuningFiltersEditor
//

IdeTuningFiltersEditor::IdeTuningFiltersEditor(DbController* dbController, QWidget* parent) :
	PropertyTextEditor(parent),
	m_dbController(dbController),
	m_signals({}, &logFileStub)
{
	AppSignalSet tuningSignalSet;
	::Proto::AppSignalSet appSignalSet;

	// Load tuning signals
	//

	bool ok = m_dbController->getTunableSignals(&tuningSignalSet, parent);

	if (ok == true)
	{
		for (const AppSignal* s : tuningSignalSet)
		{
			Proto::AppSignal* pas = appSignalSet.add_appsignal();
			s->saveToProto(pas);
		}
	}

	m_signals.load(appSignalSet);
}

IdeTuningFiltersEditor::~IdeTuningFiltersEditor()
{
	if (m_tuningFilterEditor != nullptr)
	{
		QByteArray tuningFiltersPropertyEditorSplitterPos;

		int tuningFiltersSplitterPosition;
		m_tuningFilterEditor->saveUserInterfaceSettings(&tuningFiltersPropertyEditorSplitterPos, &tuningFiltersSplitterPosition);

		QSettings().setValue("TuningFiltersEditor/MainSplitterPosition", tuningFiltersSplitterPosition);
		QSettings().setValue("TuningFiltersEditor/PropertyEditorSplitterPos", tuningFiltersPropertyEditorSplitterPos);
	}
}

void IdeTuningFiltersEditor::setText(const QString& text)
{
	if (m_tuningFilterEditor != nullptr)
	{
		assert(false);
		return;
	}

	// Load presets

	QString errorCode;

	QByteArray rawData = text.toUtf8();

	bool ok = m_filters.load(rawData, &errorCode);

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorCode);
	}


	QByteArray m_tuningFiltersSplitterPosition = QSettings().value("TuningFiltersEditor/MainSplitterPosition").toByteArray();
	int m_tuningFiltersPropertyEditorSplitterPos = QSettings().value("TuningFiltersEditor/PropertyEditorSplitterPos").toInt();

	m_tuningFilterEditor = new TuningFilterEditor(m_filters,
												  m_signals,
												  true, /*readOnly*/
												  true, /*typeTreeEnabled*/
												  true, /*typeButtonEnabled*/
												  true, /*typeTabEnabled*/
												  true, /*typeCounterEnabled*/
												  true, /*typeSchemasTabsEnabled*/
												  TuningFilter::Source::Project,
												  m_tuningFiltersSplitterPosition,
												  m_tuningFiltersPropertyEditorSplitterPos);

	QHBoxLayout* l = new QHBoxLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(m_tuningFilterEditor);
}

QString IdeTuningFiltersEditor::text() const
{
	QByteArray data;

	bool ok = m_filters.save(data);

	if (ok == true)
	{
		QString s = QString::fromUtf8(data);

		return s;
	}

	return QString();
}

bool IdeTuningFiltersEditor::readOnly() const
{
	return false;
}

void IdeTuningFiltersEditor::setReadOnly(bool value)
{
	m_tuningFilterEditor->setReadOnly(value);
}

bool IdeTuningFiltersEditor::externalOkCancelButtons() const
{
	return true;
}

bool IdeTuningFiltersEditor::isModified() const
{
	return false;
}