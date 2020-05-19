#include "SimAppLogicSchemasPage.h"
#include "../lib/Ui/TagSelectorWidget.h"

//
//
//		SimSchemaListView - Tree View
//
//
SimSchemaListView::SimSchemaListView(SimIdeSimulator* simulator, QWidget* parent) :
	QTreeWidget{parent},
	m_simulator(simulator)
{
	assert(m_simulator);

	setUniformRowHeights(false);		// Helps to show multiline schema cations
	setWordWrap(false);
	setExpandsOnDoubleClick(true);		// DoubleClick signal is used

	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	QStringList columns;

	columns << tr("SchemaID");
	columns << tr("Caption");
	columns << tr("Tags");
	columns << tr("Module(s)");

	setHeaderLabels(columns);

	// --
	//
	connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimSchemaListView::slot_projectUpdated);

	connect(this, &QTreeWidget::doubleClicked, this, &SimSchemaListView::slot_doubleClicked);

	// --
	//
	QByteArray lastState = QSettings{}.value("Simulator/SimSchemaListView/State").toByteArray();
	header()->restoreState(lastState);

	// --
	//
	fillList();

	return;
}

SimSchemaListView::~SimSchemaListView()
{
	QSettings{}.setValue("Simulator/SimSchemaListView/State", header()->saveState());
	return;
}

class SortableSchemaTreeWidgetItem : public QTreeWidgetItem
{
public:
	SortableSchemaTreeWidgetItem(QTreeWidget* _parent, const QStringList& _strings, std::shared_ptr<VFrame30::SchemaDetails> _details, int _type = Type) :
		QTreeWidgetItem(_parent, _strings, _type),
		details(_details)
	{
		if (_type >= 100000)
		{
			isFolder = true;
		}
		else
		{
			assert(details);
		}
	}

	SortableSchemaTreeWidgetItem(QTreeWidgetItem* _parent, const QStringList& _strings, std::shared_ptr<VFrame30::SchemaDetails> _details, int _type = Type) :
		QTreeWidgetItem(_parent, _strings, _type),
		details(_details)
	{
		if (_type >= 100000)
		{
			isFolder = true;
		}
		else
		{
			assert(details);
		}
	}

	virtual bool operator<(const QTreeWidgetItem& other) const override
	{
		try
		{
			const SortableSchemaTreeWidgetItem& that = dynamic_cast<const SortableSchemaTreeWidgetItem&>(other);

			// All folders alway at top
			//
			bool leftIsFolder = this->isFolder;
			bool rightIsFolder = that.isFolder;

			bool result = false;

			if ((leftIsFolder == true && rightIsFolder == true) ||
				(leftIsFolder == false && rightIsFolder == false))
			{
				int sortColumn = treeWidget()->header()->sortIndicatorSection();
				return this->text(sortColumn) < that.text(sortColumn);
			}
			else
			{
				// Relying on sort order helps to kepp folders always at the top
				//
				if (treeWidget()->header()->sortIndicatorOrder() == Qt::AscendingOrder)
				{
					result = (leftIsFolder == true && rightIsFolder == false);
				}
				else
				{
					result = (leftIsFolder == false && rightIsFolder == true);
				}
			}

			return result;
		}
		catch(...)
		{
			assert(false);
			return false;
		}
	}

	bool isFolder = false;
	std::shared_ptr<VFrame30::SchemaDetails> details;
};

void SimSchemaListView::fillList()
{
static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");

	assert(m_simulator);

	if (m_simulator->isLoaded() == false)
	{
		return;
	}

	// Save seleted items
	//
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

	QStringList selectedSchemaIds;
	selectedSchemaIds.reserve(selectedItems.size());

	for (QTreeWidgetItem* item : selectedItems)
	{
		SortableSchemaTreeWidgetItem* si = dynamic_cast<SortableSchemaTreeWidgetItem*>(item);
		assert(si);

		if (si->isFolder == false && si->details != nullptr)
		{
			selectedSchemaIds.push_back(si->details->m_schemaId);
		}
	}

	// Fill tree
	//
	this->clear();

	const VFrame30::SchemaDetailsSet& detailsSet = m_simulator->schemaDetails();

	std::map<QString, QTreeWidgetItem*> createdPath;

	int filtered = 0;
	int filteredTags = 0;
	int count = detailsSet.schemaCount();

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<VFrame30::SchemaDetails> schema = detailsSet.schemaDetails(i);
		assert(schema);

		// Apply filter
		//
		if (m_filter.isEmpty() == false)
		{
			if (schema->m_schemaId.contains(m_filter, Qt::CaseInsensitive) == false &&
				schema->searchForString(m_filter) == false)
			{
				// Schema filtered
				//
				filtered ++;
				continue;
			}
		}

		// Apply tag filter
		//
		if (m_tagFilter.isEmpty() == false)
		{
			if (schema->hasTag(m_tagFilter) == false)
			{
				// Schema filtered
				//
				filteredTags ++;
				continue;
			}
		}

		// --
		//
		QString tags;
		for (const QString& t : schema->m_tags)
		{
			if (tags.isEmpty() == true)
			{
				tags = t;
			}
			else
			{
				tags += QStringLiteral(" ") + t;
			}
		}

		QStringList sl;
		sl << schema->m_schemaId;
		sl << schema->m_caption;
		sl << tags;
		sl << schema->m_equipmentId;

		// --
		//
		QTreeWidgetItem* addedItemTreeItem = nullptr;

		QStringList pathList = schema->m_path.split(QChar('/'), QString::SplitBehavior::SkipEmptyParts);

		if (pathList.isEmpty() == true)
		{
			// Top level item
			//
			addedItemTreeItem = new SortableSchemaTreeWidgetItem{this, sl, schema};
			this->addTopLevelItem(addedItemTreeItem);
		}
		else
		{
			// Check path already exists, if not then create it
			//
			QString pathString;
			QTreeWidgetItem* parentTreeItem = nullptr;	// Nullptr is a top item
			for (const QString& p : pathList)
			{
				pathString += QChar('/') + p;

				if (auto cpi = createdPath.find(pathString);
					cpi == createdPath.end())
				{
					// pathString is not found, 'p' must be created in parentTreeItem
					//
					QTreeWidgetItem* treeItem = nullptr;

					if (parentTreeItem == nullptr)
					{
						treeItem = new SortableSchemaTreeWidgetItem{this, QStringList{} << p, {}, QTreeWidgetItem::UserType + 100000 + i};
						this->addTopLevelItem(treeItem);
					}
					else
					{
						treeItem = new SortableSchemaTreeWidgetItem{parentTreeItem,  QStringList{} << p, {}, QTreeWidgetItem::UserType + 100000 + i};
					}

					treeItem->setIcon(0, staticFolderIcon);

					createdPath[pathString] = treeItem;

					parentTreeItem = treeItem;
				}
				else
				{
					parentTreeItem = cpi->second;
				}
			}

			// Create item
			//
			assert(createdPath.count(schema->m_path) != 0);

			if (parentTreeItem == nullptr)
			{
				addedItemTreeItem = new SortableSchemaTreeWidgetItem{this, sl, schema};
				this->addTopLevelItem(addedItemTreeItem);
			}
			else
			{
				addedItemTreeItem = new SortableSchemaTreeWidgetItem{parentTreeItem, sl, schema};
			}
		}

		assert(addedItemTreeItem);
		if (addedItemTreeItem != nullptr && selectedSchemaIds.contains(schema->m_schemaId) == true)
		{
			addedItemTreeItem->setSelected(true);
		}
	}

	m_filterCount = count - filtered;

	if (filtered != 0 ||
		filteredTags != 0)
	{
		// If filter is applied then expand all
		//
		this->expandAll();
	}
	else
	{
		// Expand all top level items
		//
		int topLevelCount = this->topLevelItemCount();
		for (int i = 0; i < topLevelCount; i++)
		{
			this->expandItem(this->topLevelItem(i));
		}
	}

	return;
}

void SimSchemaListView::slot_projectUpdated()
{
	fillList();
	return;
}

void SimSchemaListView::slot_doubleClicked(const QModelIndex&)
{
	SortableSchemaTreeWidgetItem* si = dynamic_cast<SortableSchemaTreeWidgetItem*>(currentItem());
	assert(si);

	if (si != nullptr && si->isFolder == false)
	{
		assert(si->details);

		if (si->details != nullptr)
		{
			emit openSchemaRequest(si->details->m_schemaId);
			return;
		}
	}

	return;
}

QString SimSchemaListView::filter() const
{
	return m_filter;
}

void SimSchemaListView::setFilter(QString value)
{
	m_filter = value;
	fillList();

	return;
}

void SimSchemaListView::setTagFilter(const QStringList& tags)
{
	m_tagFilter = tags;
	fillList();
	return;
}

int SimSchemaListView::filterCount() const
{
	return m_filterCount;
}

std::vector<QTreeWidgetItem*> SimSchemaListView::searchFor(QString searchText)
{
	std::vector<QTreeWidgetItem*> result;

	std::function<bool(SortableSchemaTreeWidgetItem*, const QString&)> itemHasText =
			[](SortableSchemaTreeWidgetItem* item, const QString& searchText) -> bool
			{
				if (item->isFolder == false &&
					item->text(0).contains(searchText, Qt::CaseInsensitive) == true)
				{
					return true;
				}

				if (item->details != nullptr)
				{
					return item->details->searchForString(searchText);
				}

				return false;
			};

	std::function<void(QTreeWidgetItem*)> visitTreeItems =
			[&itemHasText, &visitTreeItems, &searchText, &result](QTreeWidgetItem* treeItem)
			{
				for(int i = 0; i < treeItem->childCount(); i++)
				{
					SortableSchemaTreeWidgetItem* item = dynamic_cast<SortableSchemaTreeWidgetItem*>(treeItem->child(i));
					assert(item);

					if (item != nullptr &&
						itemHasText(item, searchText))
					{
						result.push_back(item);
					}

					visitTreeItems(item);
				}
			};

	std::function<void(const QTreeWidget*)> visitTopTreeItems =
			[&itemHasText, &visitTreeItems, &searchText, &result](const QTreeWidget* treeWidget)
			{
				for(int i=0; i < treeWidget->topLevelItemCount(); i++)
				{
					SortableSchemaTreeWidgetItem* item = dynamic_cast<SortableSchemaTreeWidgetItem*>(treeWidget->topLevelItem(i));
					assert(item);

					if (item != nullptr &&
						itemHasText(item, searchText))
					{
						result.push_back(item);
					}

					visitTreeItems(item);
				}
			};

	// Result is forming now
	//
	visitTopTreeItems(this);

	return result;
}

void SimSchemaListView::searchAndSelect(QString searchText)
{
	clearSelection();

	std::vector<QTreeWidgetItem*> matched = searchFor(searchText);
	if (matched.empty() == true)
	{
		return;
	}

	for (QTreeWidgetItem* item : matched)
	{
		item->setSelected(true);
		this->scrollToItem(item);
	}

	QMessageBox::information(this, qAppName(), tr("Found %1 schema(s)").arg(matched.size()));

	return;
}

//
//
//		SimAppLogicSchemasPage - Tab Page
//
//
SimAppLogicSchemasPage::SimAppLogicSchemasPage(SimIdeSimulator* simulator, QWidget* parent) :
	SimBasePage(simulator, parent)
{
	setBackgroundRole(QPalette::Light);

	m_schemasView = new SimSchemaListView{m_simulator, this};
	m_schemasView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	m_searchAction = new QAction(tr("Edit Search"), this);
	m_searchAction->setShortcut(QKeySequence::Find);
	addAction(m_searchAction);

	m_searchEdit = new QLineEdit{this};
	m_searchEdit->setPlaceholderText(tr("Search Text"));
	m_searchEdit->setClearButtonEnabled(true);

	m_filterEdit = new QLineEdit(this);
	m_filterEdit->setPlaceholderText(tr("Filter Text"));
	m_filterEdit->setClearButtonEnabled(true);

	QStringList completerStringList = QSettings{}.value("Simulator/SimAppLogicSchemasPage/SearchCompleter").toStringList();
	m_searchCompleter = new QCompleter(completerStringList, this);
	m_searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	m_searchEdit->setCompleter(m_searchCompleter);
	m_filterEdit->setCompleter(m_searchCompleter);

	m_searchButton = new QPushButton(tr("Search"));
	m_filterButton = new QPushButton(tr("Filter"));

	m_resetFilterButton = new QPushButton(tr("Reset Filter"));
	m_resetFilterButton->setDisabled(true);

	m_tagSelector = new TagSelectorWidget{this};
	m_tagSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(m_tagSelector, &TagSelectorWidget::changed, this, &SimAppLogicSchemasPage::tagSelectorHasChanges);

	// --
	//
	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(m_schemasView, 0, 0, 1, 6);

	layout->addWidget(m_searchEdit, 1, 0, 1, 2);
	layout->addWidget(m_searchButton, 1, 2, 1, 1);

	layout->addWidget(m_filterEdit, 2, 0, 1, 2);
	layout->addWidget(m_filterButton, 2, 2, 1, 1);
	layout->addWidget(m_resetFilterButton, 2, 3, 1, 1);
	layout->addWidget(m_tagSelector, 1, 4, 2, 2);

	layout->setColumnStretch(0, 2);
	layout->setColumnStretch(4, 2);
	layout->setColumnStretch(5, 2);

	layout->setRowStretch(0, 2);
	layout->setRowStretch(1, 0);

	setLayout(layout);

	// --
	//
	connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimAppLogicSchemasPage::updateData);

	connect(m_searchAction, &QAction::triggered, this, &SimAppLogicSchemasPage::ctrlF);
	connect(m_searchEdit, &QLineEdit::returnPressed, this, &SimAppLogicSchemasPage::search);
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &SimAppLogicSchemasPage::filter);
	connect(m_searchButton, &QPushButton::clicked, this, &SimAppLogicSchemasPage::search);
	connect(m_filterButton, &QPushButton::clicked, this, &SimAppLogicSchemasPage::filter);
	connect(m_resetFilterButton, &QPushButton::clicked, this, &SimAppLogicSchemasPage::resetFilter);

	connect(m_schemasView, &SimSchemaListView::openSchemaRequest, this, &SimAppLogicSchemasPage::openSchemaRequest);
	connect(m_schemasView, &QTreeWidget::customContextMenuRequested, this, &SimAppLogicSchemasPage::treeContextMenu);

	// --
	//
	updateData();

	return;
}

void SimAppLogicSchemasPage::updateData()
{
	std::set<QString> tags;
	const VFrame30::SchemaDetailsSet& detailsSet = m_simulator->schemaDetails();
	int count = detailsSet.schemaCount();

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<VFrame30::SchemaDetails> schema = detailsSet.schemaDetails(i);
		assert(schema);

		const std::set<QString>& schemaTags = schema->tags();

		for (const QString& t : schemaTags)
		{
			tags.insert(t);
		}
	}

	m_tagSelector->setTags(tags);

	return;
}

void SimAppLogicSchemasPage::ctrlF()
{
	assert(m_searchEdit);

	m_searchEdit->setFocus();
	m_searchEdit->selectAll();

	return;
}

void SimAppLogicSchemasPage::search()
{
	// Search for text in schemas
	//
	assert(m_schemasView);
	assert(m_searchEdit);

	QString searchText = m_searchEdit->text().trimmed();

	if (searchText.isEmpty() == true)
	{
		m_schemasView->clearSelection();
		return;
	}

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("Simulator/SimAppLogicSchemasPage/SearchCompleter").toStringList();

	if (completerStringList.contains(searchText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(searchText);
		QSettings{}.setValue("Simulator/SimAppLogicSchemasPage/SearchCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_searchCompleter->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	// Search for text and select schemas with it
	//
	m_schemasView->searchAndSelect(searchText);

	m_schemasView->setFocus();

	return;
}

void SimAppLogicSchemasPage::filter()
{
	// Search for text in schemas
	//
	assert(m_schemasView);
	assert(m_filterEdit);

	QString filterText = m_filterEdit->text().trimmed();

	// Save completer
	//
	QStringList completerStringList = QSettings{}.value("Simulator/SimAppLogicSchemasPage/SearchCompleter").toStringList();

	if (completerStringList.contains(filterText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(filterText);
		QSettings{}.setValue("Simulator/SimAppLogicSchemasPage/SearchCompleter", completerStringList);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_searchCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(completerStringList);
		}
	}

	// Search for text and select schemas with it
	//
	m_schemasView->setFilter(filterText);
	m_schemasView->setFocus();

	int schemaFiletrCount = m_schemasView->filterCount();

	if (filterText.trimmed().isEmpty() == false)
	{
		m_filterButton->setText(tr("Filter: %1 found").arg(schemaFiletrCount));

		QFont font = m_filterButton->font();
		font.setBold(true);
		m_filterButton->setFont(font);
	}
	else
	{
		m_filterButton->setText(tr("Filter"));

		QFont font = m_filterButton->font();
		font.setBold(false);
		m_filterButton->setFont(font);
	}

	m_resetFilterButton->setDisabled(filterText.trimmed().isEmpty());

	return;
}

void SimAppLogicSchemasPage::resetFilter()
{
	assert(m_schemasView);

	m_filterEdit->clear();

	m_schemasView->setFilter({});
	m_schemasView->setFocus();

	m_filterButton->setText(tr("Filter"));
	QFont font = m_filterButton->font();
	font.setBold(false);
	m_filterButton->setFont(font);

	m_resetFilterButton->setDisabled(true);

	return;
}

void SimAppLogicSchemasPage::tagSelectorHasChanges()
{
	// Filter schemas by tags
	//
	QStringList selectedTags = m_tagSelector->selectedTags();

	m_schemasView->setTagFilter(selectedTags);
	m_schemasView->setFocus();

	return;
}

void SimAppLogicSchemasPage::treeContextMenu(const QPoint& pos)
{
	SortableSchemaTreeWidgetItem* currentItem = dynamic_cast<SortableSchemaTreeWidgetItem*>(m_schemasView->currentItem());
	if (currentItem == nullptr ||
		currentItem->isFolder == true ||
		currentItem->details == nullptr)
	{
		return;
	}

	QMenu m;

	m.addAction(tr("Open..."),
				[this, currentItem]()
				{
					emit this->openSchemaRequest(currentItem->details->m_schemaId);
				});

	m.exec(m_schemasView->mapToGlobal(pos));

	return;
}

