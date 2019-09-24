#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

#include <QButtonGroup>
#include <QTreeWidget>
#include <QTreeWidgetItem>

//
// FilterButton
//

FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, bool check, QWidget* parent)
	:QPushButton(filter->caption(), parent)
{
	Q_ASSERT(filter);

	m_filter = filter;

	setCheckable(true);

	if (check == true)
	{
		setChecked(true);
	}

	setMinimumSize(100, 25);

	update(0);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
{
	return m_filter;
}

int FilterButton::counter() const
{
	return m_discreteCounter;
}

void FilterButton::update(int discreteCounter)
{
	// Text

	QString newCaption;

	if (discreteCounter == 0)
	{
		newCaption = m_filter->caption();
	}
	else
	{
		newCaption = QString(" %1 [%2] ").arg(m_filter->caption()).arg(discreteCounter);
	}

	m_discreteCounter = discreteCounter;

	if (text() != newCaption)
	{
		setText(newCaption);
	}

	// Color

	QColor backColor = Qt::lightGray;
	QColor textColor = Qt::white;

	QColor backSelectedColor = Qt::darkGray;
	QColor textSelectedColor = Qt::white;

	if (m_filter->useColors() == true)
	{
		if (counter() != 0 && m_filter->backAlertedColor() != m_filter->textAlertedColor())
		{
			// Alerted state

			backColor = m_filter->backAlertedColor();
			textColor = m_filter->textAlertedColor();

			backSelectedColor = backColor;
			textSelectedColor = textColor;
		}
		else
		{
			if (m_filter->backColor() != m_filter->textColor())
			{
				backColor = m_filter->backColor();
				textColor = m_filter->textColor();
			}

			if (m_filter->backSelectedColor() != m_filter->textSelectedColor())
			{
				backSelectedColor = m_filter->backSelectedColor();
				textSelectedColor = m_filter->textSelectedColor();
			}
		}
	}

	QString style = tr("\
					   QPushButton {   \
						   background-color: %1;\
						   color: %2;    \
					   }   \
					   QPushButton:checked{\
						   background-color: %3;\
						   color: %4;    \
						   border: none;\
					   }\
					   ").arg(backColor.name())
					   .arg(textColor.name())
					   .arg(backSelectedColor.name())
					   .arg(textSelectedColor.name());


	if (styleSheet() != style)
	{
		setStyleSheet(style);
	}
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_filter);
	}

}

//
// TuningWorkspace
//

int TuningWorkspace::m_instanceCounter = 0;

TuningWorkspace::TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, TuningClientFilterStorage* tuningFilterStorage, QWidget* parent) :
	QWidget(parent),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient),
	m_tuningFilterStorage(tuningFilterStorage),
	m_workspaceFilter(workspaceFilter),
	m_treeFilter(treeFilter)
{
	//qDebug() << "TuningWorkspace::TuningWorkspace m_instanceCounter = " << m_instanceCounter;
	m_instanceCounter++;

	//assert(m_treeFilter); // Can be nullptr
	assert(m_workspaceFilter);
	assert(m_tuningSignalManager);
	assert(m_tuningTcpClient);
	assert(m_tuningFilterStorage);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* rightWidget = new QWidget();

	m_rightLayout = new QVBoxLayout(rightWidget);

	//

	updateFiltersTree(m_workspaceFilter);

	//

	createButtons();

	if (m_buttonsLayout != nullptr)
	{
		m_rightLayout->addLayout(m_buttonsLayout);
	}

	//

	createTabPages();

	//

	if (m_treeLayoutWidget != nullptr)
	{
		// Create splitter control
		//
		m_hSplitter = new QSplitter();

		m_hSplitter->addWidget(m_treeLayoutWidget);

		m_hSplitter->addWidget(rightWidget);

		mainLayout->addWidget(m_hSplitter);

		// Restore splitter size
		//

		m_hSplitter->restoreState(theSettings.m_tuningWorkspaceSplitterState);
	}
	else
	{
		mainLayout->addWidget(rightWidget);
	}

	// Color

	if (workspaceFilter->useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Background, workspaceFilter->backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
		show();
	}
}

TuningWorkspace::~TuningWorkspace()
{
	m_instanceCounter--;
	//qDebug() << "TuningWorkspace::~TuningWorkspace m_instanceCounter = " << m_instanceCounter;

	if (m_filterTree != nullptr)
	{
		QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

		if (m_columnNameIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnNameIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnIndex", width);

		}
		if (m_columnAccessIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnAccessIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnsAccess", width);
		}

		for (int i = 0; i < static_cast<int>(m_columnDiscreteCountIndexes.size()); i++)
		{
			int width = m_filterTree->columnWidth(m_columnDiscreteCountIndexes[i]);
			settings.setValue(tr("TuningWorkspace/FilterTreeColumnCounter%1").arg(i), width);
		}

		if (m_columnStatusIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnStatusIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnStatus", width);
		}
		if (m_columnSorIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnSorIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnSor", width);
		}
	}

	if (m_hSplitter != nullptr)
	{
		theSettings.m_tuningWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

bool TuningWorkspace::hasPendingChanges()
{
	for (auto it : m_tuningPagesMap)
	{
		TuningPage* tp = it.second;

		if (tp->hasPendingChanges() == true)
		{
			return true;
		}
	}

	for (auto it : m_tuningWorkspacesMap)
	{
		TuningWorkspace* tw = it.second;

		if (tw->hasPendingChanges() == true)
		{
			return true;
		}
	}

	return false;
}

bool TuningWorkspace::askForSavePendingChanges()
{
	for (auto it : m_tuningPagesMap)
	{
		TuningPage* tp = it.second;

		if (tp->askForSavePendingChanges() == false)
		{
			return false;
		}
	}

	for (auto it : m_tuningWorkspacesMap)
	{
		TuningWorkspace* tw = it.second;

		if (tw->askForSavePendingChanges() == false)
		{
			return false;
		}
	}

	return true;
}

void TuningWorkspace::onTimer()
{
	updateTabsButtonsCounters();

	updateTreeItemsStatus();

	for (auto it : m_tuningWorkspacesMap)
	{
		TuningWorkspace* tw = it.second;

		static int dp = 0;

		dp++;

		tw->onTimer();

		dp--;
	}
}

void TuningWorkspace::updateFilters(std::shared_ptr<TuningFilter> rootFilter)
{
	updateFiltersTree(rootFilter);

	for (auto swp : m_switchPresetPages)
	{
		if (swp == nullptr)
		{
			Q_ASSERT(swp);
			return;
		}

		swp->updateFilters(rootFilter);
	}
}

void TuningWorkspace::updateFiltersTree(std::shared_ptr<TuningFilter> rootFilter)
{
	// Fill the filter tree
	//
	if (rootFilter == nullptr)
	{
		assert(rootFilter);
		return;
	}

	QString mask;
	if (m_treeMask != nullptr)
	{
		mask = m_treeMask->text();
	}

	QStringList l;
	l << rootFilter->caption();

	QTreeWidgetItem* rootItem = new QTreeWidgetItem(l);
	rootItem->setData(0, Qt::UserRole, QVariant::fromValue(rootFilter));

	addChildTreeObjects(rootFilter, rootItem, mask);

	if (rootItem->childCount() == 0)
	{
		delete rootItem;
		return;
	}

	// Create tree control
	//
	if (m_filterTree == nullptr)
	{
		m_filterTree = new QTreeWidget();
		m_filterTree->setSortingEnabled(true);
		m_filterTree->setObjectName("FilterTreeWidget");

		m_filterTree->viewport()->installEventFilter(this);
		m_filterTree->installEventFilter(this);

		m_filterTree->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);
		connect(m_filterTree, &QWidget::customContextMenuRequested, this, &TuningWorkspace::slot_treeContextMenuRequested);

		int columnIndex = m_columnNameIndex;

		QStringList headerLabels;

		headerLabels << tr("Caption");
		columnIndex++;

		// Access (?)

		if (theConfigSettings.useAccessFlag == true)
		{
			headerLabels << tr("Access");
			m_columnAccessIndex = columnIndex;
			columnIndex++;
		}

		// SOR (?)

		if (theConfigSettings.showSOR == true)
		{
			headerLabels << tr("SOR");
			m_columnSorIndex = columnIndex;
			columnIndex++;
		}

		// Counters ()

		int counerColumnsCount = m_tuningFilterStorage->schemaCounterFiltersCount();
		const QStringList& schemaCounterFiltersNames = m_tuningFilterStorage->schemaCounterFiltersNames();

		if (static_cast<int>(schemaCounterFiltersNames.size()) != counerColumnsCount)
		{
			Q_ASSERT(false);
			return;
		}

		for (int i = 0; i < counerColumnsCount; i++)
		{
			headerLabels << schemaCounterFiltersNames.at(i);
			m_columnDiscreteCountIndexes.push_back(columnIndex++);
		}

		// Status

		headerLabels << tr("Status");
		m_columnStatusIndex = columnIndex;
		columnIndex++;

		//

		headerLabels << tr("");

		m_filterTree->setColumnCount(headerLabels.size());
		m_filterTree->setHeaderLabels(headerLabels);

		// Set column width

		const int columnMaxWidth = 500;

		QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

		if (m_columnNameIndex != -1)
		{
			const int defaultWidth = 200;

			int width = settings.value("TuningWorkspace/FilterTreeColumnIndex", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(m_columnNameIndex, width);
		}

		if (m_columnAccessIndex != -1)
		{
            const int defaultWidth = 50;

			int width = settings.value("TuningWorkspace/FilterTreeColumnsAccess", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(m_columnAccessIndex, width);

		}

		if (m_columnSorIndex != -1)
		{
			const int defaultWidth = 80;

			int width = settings.value("TuningWorkspace/FilterTreeColumnSor", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(m_columnSorIndex, width);
		}

		for (int i = 0; i < counerColumnsCount; i++)
		{
			const int defaultWidth = 40;

			int width = settings.value(QString("TuningWorkspace/FilterTreeColumnCounter%1").arg(i), defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(m_columnDiscreteCountIndexes[i], width);
		}

		if (m_columnStatusIndex != -1)
		{
			const int defaultWidth = 80;

			int width = settings.value("TuningWorkspace/FilterTreeColumnStatus", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(m_columnStatusIndex, width);
		}

		//

		m_treeMask = new QLineEdit();
		connect(m_treeMask, &QLineEdit::returnPressed, this, &TuningWorkspace::slot_maskReturnPressed);

		m_treeMaskApply = new QPushButton(tr("Filter"));
		connect(m_treeMaskApply, &QPushButton::clicked, this, &TuningWorkspace::slot_maskApply);

		QHBoxLayout* searchLayout = new QHBoxLayout();
		searchLayout->addWidget(m_treeMask);
		searchLayout->addWidget(m_treeMaskApply);

		m_treeLayoutWidget = new QWidget();

		QVBoxLayout* treeLayout = new QVBoxLayout(m_treeLayoutWidget);

		treeLayout->addWidget(m_filterTree);
		treeLayout->addLayout(searchLayout);
	}
	else
	{
		m_filterTree->clear();
	}

	// Fill filters control
	//

	m_filterTree->addTopLevelItem(rootItem);

	// Restore selection

	if (m_treeFilter == nullptr)
	{
		rootItem->setSelected(true);
	}
	else
	{
		// Find a pointer to previously selected tree filter (remember we are working with shared_ptrs)

		m_treeFilter = rootFilter->findFilterById(m_treeFilter->ID());

		if (m_treeFilter == nullptr)
		{
			// No such filter - select root

			rootItem->setSelected(true);
		}
		else
		{
			// Find a tree item for restored selected filter and select it

			QTreeWidgetItem* treeFilterWidget = findFilterWidget(m_treeFilter->ID(), rootItem);

			if (treeFilterWidget == nullptr)
			{
				assert(treeFilterWidget);
			}
			else
			{
				treeFilterWidget->setSelected(true);

				// Expand all parents

				QTreeWidgetItem* parent = treeFilterWidget->parent();
				while (parent != nullptr && parent != rootItem)
				{
					parent->setExpanded(true);
					parent = parent->parent();
				}
			}
		}
	}

	// Expand root item

	rootItem->setExpanded(true);

	// Expand "Equipment" item

	for (int i = 0; i < rootItem->childCount(); i++)
	{
		QTreeWidgetItem* rootChildItem = rootItem->child(i);

		std::shared_ptr<TuningFilter> filter = rootChildItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		if (filter->isEmpty() == true && filter->isSourceEquipment())
		{
			rootChildItem->setExpanded(true);
			break;
		}

	}

	m_filterTree->sortItems(0, Qt::AscendingOrder);

	if (mask.isEmpty() == false)
	{
		m_filterTree->expandAll();
	}
}

void TuningWorkspace::createButtons()
{
	if (m_workspaceFilter == nullptr)
	{
		assert(m_workspaceFilter);
		return;
	}

	// Buttons
	//
	m_filterButtons.clear();

	bool firstButton = true;

	for (int i = 0; i < m_workspaceFilter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = m_workspaceFilter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f, firstButton);
		m_filterButtons.push_back(button);

		button->installEventFilter(this);

		if (firstButton)
		{
			firstButton = false;
		}

		connect(button, &FilterButton::filterButtonClicked, this, &TuningWorkspace::slot_filterButtonClicked);

	}

	if (m_filterButtons.empty() == false)
	{
		QButtonGroup* filterButtonGroup = new QButtonGroup();

		filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: m_filterButtons)
		{
			filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();

		m_currentbuttonFilter = m_filterButtons[0]->filter();
	}
}

void TuningWorkspace::createTabPages()
{
	if (m_workspaceFilter == nullptr)
	{
		assert(m_workspaceFilter);
		return;
	}

	// Fill tab pages
	//

	std::vector<std::pair<QWidget*, std::shared_ptr<TuningFilter>>> tuningPages;

	m_tabsFilters.clear();

	// Workspace level tabs

	for (int i = 0; i < m_workspaceFilter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = m_workspaceFilter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTab() == false)
		{
			continue;
		}

		QWidget* tp = createTuningPageOrWorkspace(f);

		tuningPages.push_back(std::make_pair(tp, f));

		m_tabsFilters.push_back(f);
	}

	// Buttons level tabs

	if (m_currentbuttonFilter != nullptr)
	{
		for (int i = 0; i < m_currentbuttonFilter->childFiltersCount(); i++)
		{
			std::shared_ptr<TuningFilter> f = m_currentbuttonFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			if (f->isTab() == false)
			{
				continue;
			}

			QWidget* tp = createTuningPageOrWorkspace(f);

			tuningPages.push_back(std::make_pair(tp, f));

			m_tabsFilters.push_back(f);
		}
	}

	if (tuningPages.empty() == false)
	{
		// Create tab control and add pages
		//
		if (m_tab == nullptr)
		{
			m_tab = new QTabWidget();
			m_tab->setObjectName("TuningTabWidget");

			m_rightLayout->addWidget(m_tab);

			m_tab->tabBar()->installEventFilter(this);

			m_tab->setVisible(false);
		}
		else
		{
			m_tab->setVisible(false);

			m_tab->clear();
		}

		if (m_singleTuningPage != nullptr)
		{
			m_singleTuningPage->setVisible(false);
		}

		for (auto t : tuningPages)
		{
			QWidget* w = new QWidget();

			QHBoxLayout* l = new QHBoxLayout(w);

			QWidget* tp = t.first;

			l->addWidget(tp);

			m_tab->addTab(w, t.second->caption());
		}

		m_tab->setVisible(true);

		// set the active tab

		if (m_currentbuttonFilter != nullptr)
		{
			auto it = m_activeTabPagesMap.find(m_currentbuttonFilter->ID());
			if (it != m_activeTabPagesMap.end())
			{
				int index = m_activeTabPagesMap[m_currentbuttonFilter->ID()];
				m_tab->setCurrentIndex(index);
			}
		}
	}
	else
	{
		// No tab pages, create only one page
		//
		if (m_singleTuningPage == nullptr)
		{
			std::shared_ptr<TuningFilter> singlePageFilter = nullptr;

			if (m_currentbuttonFilter != nullptr)
			{
				// If a button is pressed - set button filter as page filter

				singlePageFilter = m_currentbuttonFilter;
			}
			else
			{
				// Otherwise set workspace filter to page filter

				singlePageFilter = std::make_shared<TuningFilter>();

				singlePageFilter->setCaption(m_workspaceFilter->caption());

				// Copy signals' hashes from parent filter to single page's filter

				singlePageFilter->setSignalsHashes(m_workspaceFilter->signalsHashes());
			}

			QWidget* tp = createTuningPageOrWorkspace(singlePageFilter);

			m_rightLayout->addWidget(tp);

			m_singleTuningPage = (TuningPage*)tp;
		}

		if (m_tab != nullptr)
		{
			m_tab->setVisible(false);
		}

		m_singleTuningPage->setVisible(true);
	}
}

QWidget* TuningWorkspace::createTuningPageOrWorkspace(std::shared_ptr<TuningFilter> childWorkspaceFilter)
{
	if (childWorkspaceFilter == nullptr)
	{
		assert(childWorkspaceFilter);
		return new QWidget();
	}

	QString childWorkspaceFilterId = childWorkspaceFilter->ID();

	bool createChildWorkspace = false;

	for (int c = 0; c < childWorkspaceFilter->childFiltersCount(); c++)
	{
		std::shared_ptr<TuningFilter> cf = childWorkspaceFilter->childFilter(c);
		if (cf == nullptr)
		{
			assert(cf);
			continue;
		}

		if (cf->isTab() == true || cf->isButton() == true || cf->isTree() == true)
		{
			createChildWorkspace = true;
			break;
		}
	}

	if (createChildWorkspace == true)
	{
		// We have to create nested workspace
		//
		auto it = m_tuningWorkspacesMap.find(childWorkspaceFilterId);
		if (it == m_tuningWorkspacesMap.end())
		{
			TuningWorkspace* tw = new TuningWorkspace(m_treeFilter, childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient, m_tuningFilterStorage, this/*parent*/);

			m_tuningWorkspacesMap[childWorkspaceFilterId] = tw;

			connect(this, &TuningWorkspace::treeFilterSelectionChanged, tw, &TuningWorkspace::slot_parentTreeFilterChanged);

			return tw;
		}
		else
		{
			return it->second;
		}
	}
	else
	{
		if (childWorkspaceFilter->isTab() && childWorkspaceFilter->tabType() == TuningFilter::TabType::FiltersSwitch )
		{
			// We have to create Presets Switch page
			//
			SwitchFiltersPage* swp = new SwitchFiltersPage(childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient, m_tuningFilterStorage);
			m_switchPresetPages.push_back(swp);
			return swp;
		}
		else
		{
			// We have to create tuning page
			//
			auto it = m_tuningPagesMap.find(childWorkspaceFilterId);
			if (it == m_tuningPagesMap.end())
			{
				TuningPage* tp = new TuningPage(m_treeFilter, childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient, m_tuningFilterStorage);

				m_tuningPagesMap[childWorkspaceFilterId] = tp;

				connect(this, &TuningWorkspace::treeFilterSelectionChanged, tp, &TuningPage::slot_treeFilterSelectionChanged);

				if (childWorkspaceFilter->isButton() == true)
				{
					// Connect button filter event only if this tuning page is selected by button, not tab

					connect(this, &TuningWorkspace::buttonFilterSelectionChanged, tp, &TuningPage::slot_pageFilterChanged);
				}

				return tp;
			}
			else
			{
				return it->second;
			}
		}
	}
}

void TuningWorkspace::addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask)
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

	for (int i = 0; i < filter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTree() == false)
		{
			continue;
		}

		QString caption = f->caption();

		if (mask.isEmpty() == false)
		{
			if (f->childFiltersCount() == 0 && caption.contains(mask, Qt::CaseInsensitive) == false)
			{
				continue;
			}
		}


		//if (f->isSourceSchema() == true || f->isSourceEquipment() == true)
		//{
			//caption += QString(" [+%1 DEBUG counters]").arg(f->childFiltersCount());
		//}

		static QString equipmentString = tr("Equipment");
		static QString schemasString = tr("Schemas");
		Q_UNUSED(equipmentString);
		Q_UNUSED(schemasString);

		QStringList l;
		l << tr(caption.toUtf8().data());	// Try to translate filter name!

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		parent->addChild(item);

		addChildTreeObjects(f, item, mask);
	}
}

void TuningWorkspace::updateTabsButtonsCounters()
{
	// Tab counters

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		int tabFiltersCount = static_cast<int>(m_tabsFilters.size());

		if (m_tab->count() != tabFiltersCount)
		{
			//qDebug() << m_tab->count();
			//qDebug() << static_cast<int>(m_tabsFilters.size());
			assert(m_tab->count() == tabFiltersCount);
		}

		for (int ti = 0; ti < tabFiltersCount; ti++)
		{
			std::shared_ptr<TuningFilter> f = m_tabsFilters[ti];

			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			if (f->isTab() == false)
			{
				assert(false);
				continue;
			}

			if (f->hasDiscreteCounter() == false)
			{
				continue;
			}

			int discreteCount = f->counters().discreteCounter;

			QString newCaption;
			if (discreteCount == 0)
			{
				newCaption = f->caption();
			}
			else
			{
				newCaption = QString(" %1 [%2] ").arg(f->caption()).arg(discreteCount);
			}

			if (m_tab->tabText(ti) != newCaption)
			{
				m_tab->setTabText(ti, newCaption);
			}

			// Tab text color

			if (f->useColors() == true)
			{
				QColor tabTextColor;

				if (discreteCount > 0)
				{
					tabTextColor = f->textAlertedColor();
				}
				else
				{
					tabTextColor = f->textColor();
				}

				if (m_tab->tabBar()->tabTextColor(ti) != tabTextColor)
				{
					m_tab->tabBar()->setTabTextColor(ti, tabTextColor);
				}
			}
		}
	}

	// Buttons counters

	for (FilterButton* button : m_filterButtons)
	{
		if (button == nullptr)
		{
			assert(button);
			return;
		}

		std::shared_ptr<TuningFilter> f = button->filter();

		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->hasDiscreteCounter() == false)
		{
			continue;
		}

		int discreteCount = f->counters().discreteCounter;

		if (discreteCount != button->counter())
		{
			button->update(discreteCount);
		}
	}
}

void TuningWorkspace::updateTreeItemsStatus(QTreeWidgetItem* treeItem)
{
	if (m_filterTree == nullptr)
	{
		return;
	}

	if (treeItem == nullptr)
	{

		if (m_filterTree->topLevelItemCount() == 0)
		{
			return;
		}

		treeItem = m_filterTree->topLevelItem(0);
	}

	std::shared_ptr<TuningFilter> filter = treeItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (filter->isEmpty() == false)
	{
		updateTreeItemCounters(treeItem, filter.get());

		// Counters column

		TuningCounters counters = filter->counters();

		// Status column

		if (filter->isSourceEquipment() == true)
		{
			updateTuningSourceTreeItem(treeItem, filter.get());
		}
		else
		{
			assert(m_columnStatusIndex != -1);

			QColor backColor;
			QColor textColor;
			QString text;

			if (counters.errorCounter == 0)
			{
				backColor = Qt::white;
				textColor = Qt::black;
			}
			else
			{
				text = QString("E: %1").arg(counters.errorCounter);
				backColor = redColor;
				textColor = Qt::white;
			}

			if (treeItem->text(m_columnStatusIndex) != text)
			{
				treeItem->setText(m_columnStatusIndex, text);
			}

			if (treeItem->backgroundColor(m_columnStatusIndex) != backColor)
			{
				treeItem->setBackgroundColor(m_columnStatusIndex, backColor);
			}

			if (treeItem->textColor(m_columnStatusIndex) != textColor)
			{
				treeItem->setTextColor(m_columnStatusIndex, textColor);
			}
		}

		// SOR Column

		if (m_columnSorIndex != -1 && theConfigSettings.showSOR == true)
		{
			QColor backColor;
			QColor textColor;
			QString text;

			if (counters.sorActive == false)
			{
				// Inactive
				backColor = Qt::white;
				textColor = Qt::black;
			}
			else
			{
				if (counters.sorValid == false)
				{
					text = "?";
					backColor = redColor;
					textColor = Qt::white;
				}
				else
				{
					if (counters.sorCounter == 0)
					{
						// Sor NO
						backColor = Qt::white;
						textColor = Qt::black;
					}
					else
					{
						if (counters.sorCounter == 1)
						{
							text = QString("SOR");
						}
						else
						{
							text = QString("SOR [%1]").arg(counters.sorCounter);
						}
						backColor = redColor;
						textColor = Qt::white;
					}
				}
			}

			if (treeItem->text(m_columnSorIndex) != text)
			{
				treeItem->setText(m_columnSorIndex, text);
			}

			if (treeItem->backgroundColor(m_columnSorIndex) != backColor)
			{
				treeItem->setBackgroundColor(m_columnSorIndex, backColor);
			}

			if (treeItem->textColor(m_columnSorIndex) != textColor)
			{
				treeItem->setTextColor(m_columnSorIndex, textColor);
			}
		}
	}

	int count = treeItem->childCount();
	for (int i = 0; i < count; i++)
	{
		updateTreeItemsStatus(treeItem->child(i));
	}
}

void TuningWorkspace::updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	std::vector<Hash> equipmentHashes = filter->equipmentHashes();

	if (equipmentHashes.size() != 1)
	{
		Q_ASSERT(filter);
		return;
	}

	Hash hash = equipmentHashes[0];

	assert(m_columnStatusIndex != -1);

	int errorCounter = filter->counters().errorCounter;

	TuningSource ts;

	QString status;
	bool valid = false;
	bool controlIsEnabled = false;
	bool hasUnappliedParams = false;

    bool access = false;

	if (m_tuningTcpClient->tuningSourceInfo(hash, &ts) == false)
	{
		status = tr("Unknown");
	}
	else
	{
		valid = ts.valid();
		controlIsEnabled = ts.state.controlisactive();
		hasUnappliedParams = ts.state.hasunappliedparams();

        if (theConfigSettings.useAccessFlag == true &&
            valid == true &&
            controlIsEnabled == true &&
            ts.state.isreply() == true)
			{
                access = ts.state.writingdisabled() == false;
			}

		if (valid == false)
		{
			status = tr("Unknown");
		}
		else
		{
			if (controlIsEnabled == false)
			{
				status = tr("Inactive");
			}
			else
			{
				if (ts.state.isreply() == false)
				{
					status = tr("No Reply");
				}
				else
				{
					if (errorCounter > 0)
					{
						status = tr("E: %1").arg(errorCounter);
					}
					else
					{
						if (hasUnappliedParams == true)
						{
							status = tr("Unapplied [%1 replies]").arg(ts.state.replycount());
						}
						else
						{
							status = tr("Active [%1 replies]").arg(ts.state.replycount());
						}
					}
				}
			}
		}
	}

    // Access column

	if (m_columnAccessIndex != -1)
	{
		QColor accessBackColor = Qt::white;
		QColor accessTextColor = Qt::black;

        if (access == true)
        {
			accessBackColor = QColor(0, 128, 0);
			accessTextColor = Qt::white;
        }

        QString accessText = access ? tr("Yes") : tr("No");

		if (treeItem->text(m_columnAccessIndex) != accessText)
        {
			treeItem->setText(m_columnAccessIndex, accessText);
        }

		if (treeItem->backgroundColor(m_columnAccessIndex) != accessBackColor)
        {
			treeItem->setBackgroundColor(m_columnAccessIndex, accessBackColor);
        }

		if (treeItem->textColor(m_columnAccessIndex) != accessTextColor)
        {
			treeItem->setTextColor(m_columnAccessIndex, accessTextColor);
		}

    }

    // Status column


	if (treeItem->text(m_columnStatusIndex) != status)
	{
		treeItem->setText(m_columnStatusIndex, status);
	}

	QColor stateBackColor;
	QColor stateTextColor;

	if (valid == false)
	{
		stateBackColor = Qt::white;
		stateTextColor = Qt::darkGray;
	}
	else
	{
		if (controlIsEnabled == false)
		{
			stateBackColor = Qt::gray;
			stateTextColor = Qt::white;
		}
		else
		{
			if (errorCounter > 0)
			{
				stateBackColor = redColor;
				stateTextColor = Qt::white;
			}
			else
			{
				if (hasUnappliedParams == true)
				{
					stateBackColor = Qt::yellow;
					stateTextColor = Qt::black;
				}
				else
				{
					stateBackColor = Qt::white;
					stateTextColor = Qt::black;
				}
			}
		}
	}

	if (treeItem->backgroundColor(m_columnStatusIndex) != stateBackColor)
    {
		 treeItem->setBackgroundColor(m_columnStatusIndex, stateBackColor);
    }

	if (treeItem->textColor(m_columnStatusIndex) != stateTextColor)
    {
		treeItem->setTextColor(m_columnStatusIndex, stateTextColor);
    }
}

void TuningWorkspace::updateTreeItemCounters(QTreeWidgetItem* treeItem, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

	int childCount = filter->childFiltersCount();

	int counterIndex = 0;

	for (int i = 0; i < childCount; i++)
	{
		TuningFilter* childFilter = filter->childFilter(i).get();
		if (childFilter == nullptr)
		{
			Q_ASSERT(childFilter);
			return;
		}

		if (childFilter->isCounter() == false || childFilter->counterType() != TuningFilter::CounterType::FilterTree)
		{
			continue;
		}

		// Set column text and color

		if (counterIndex >= static_cast<int>(m_columnDiscreteCountIndexes.size()))
		{
			Q_ASSERT(false);
			return;
		}

		int columnIndex = m_columnDiscreteCountIndexes[counterIndex];

		TuningCounters tc = childFilter->counters();

		QColor backColor = tc.discreteCounter == 0 ? Qt::white : childFilter->backAlertedColor();
		QColor textColor = tc.discreteCounter == 0 ? Qt::black : childFilter->textAlertedColor();

		//QString text = QString("%1 %2").arg(childFilter->caption()) .arg(tc.discreteCounter);
		QString text = QString("%1").arg(tc.discreteCounter);

		if (treeItem->text(columnIndex) != text)
		{
			treeItem->setText(columnIndex, text);
		}

		if (treeItem->backgroundColor(columnIndex) != backColor)
		{
			treeItem->setBackgroundColor(columnIndex, backColor);
		}

		if (treeItem->textColor(columnIndex) != textColor)
		{
			treeItem->setTextColor(columnIndex, textColor);
		}

		//

		counterIndex++;
	}
}

void TuningWorkspace::activateControl(const QString& equipmentId, bool enable)
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	// Take Control

	QString action = enable ? tr("activate") : tr("deactivate");

	bool forceTakeControl = false;

	if (m_tuningTcpClient->singleLmControlMode() == true && m_tuningTcpClient->clientIsActive() == false)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!\n\nCurrent client is not selected as active now.\n\nAre you sure you want to take control and %1 the source %2?").arg(action).arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}

		forceTakeControl = true;
	}
	else
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Are you sure you want to %1 the source %2?").arg(action).arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	m_tuningTcpClient->activateTuningSourceControl(equipmentId, enable, forceTakeControl);
}

QTreeWidgetItem* TuningWorkspace::findFilterWidget(const QString& id, QTreeWidgetItem* treeItem)
{
	for (int i = 0; i < treeItem->childCount(); i++)
	{
		QTreeWidgetItem* childItem = treeItem->child(i);
		if (childItem == nullptr)
		{
			assert(childItem);
			return nullptr;
		}

		std::shared_ptr<TuningFilter> filter = childItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return nullptr;
		}

		if (filter->ID() == id)
		{
			return childItem;
		}

		// Recursive search

		QTreeWidgetItem* result = findFilterWidget(id, childItem);

		if (result != nullptr)
		{
			return result;
		}
	}

	return nullptr;
}

bool TuningWorkspace::eventFilter(QObject *object, QEvent *event)
{
	if (m_tab != nullptr && object == m_tab->tabBar() &&
			(event->type() == QEvent::MouseButtonPress ||
			 event->type() == QEvent::MouseButtonRelease ||
			 event->type() == QEvent::KeyPress))
	{
		if (askForSavePendingChanges() == false)
		{
			return true;
		}
	}

	if (m_filterTree != nullptr && (object == m_filterTree || object == m_filterTree->viewport()) &&
			(event->type() == QEvent::MouseButtonPress ||
			 event->type() == QEvent::MouseButtonRelease ||
			 event->type() == QEvent::KeyPress))
	{
		if (askForSavePendingChanges() == false)
		{
			return true;
		}
	}

	for (FilterButton* b : m_filterButtons)
	{
		if (object == b &&
				(event->type() == QEvent::MouseButtonPress ||
				 event->type() == QEvent::MouseButtonRelease ||
				 event->type() == QEvent::KeyPress))
		{
			if (askForSavePendingChanges() == false)
			{
				return true;
			}
		}
	}

	return QWidget::eventFilter(object, event);
}

void TuningWorkspace::slot_treeSelectionChanged()
{
	QList <QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
	if (selectedItems.size() != 1)
	{
		return;
	}

	QTreeWidgetItem* selected = selectedItems[0];
	if (selected == nullptr)
	{
		return;
	}

	m_treeFilter = selected->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

	emit treeFilterSelectionChanged(m_treeFilter);
}

void TuningWorkspace::slot_treeContextMenuRequested(const QPoint& pos)
{
	QTreeWidgetItem* item = m_filterTree->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (m_tuningTcpClient->singleLmControlMode() == false)
	{
		return;
	}

	if (filter->isEmpty() == true)
	{
		return;
	}

	if (filter->isSourceEquipment() == false)
	{
		return;
	}

	TuningSource ts;

	if (m_tuningTcpClient->tuningSourceInfo(::calcHash(filter->caption()), &ts) == false)
	{
		return;
	}

	QMenu menu(this);

	// EnableControl

	QAction* actionEnable = new QAction(tr("Activate Control"), &menu);

	auto fEnableControl = [this, filter]() -> void
	{
		activateControl(filter->caption(), true);
	};
	actionEnable->setEnabled(ts.state.controlisactive() == false);
	connect(actionEnable, &QAction::triggered, this, fEnableControl);

	menu.addAction(actionEnable);

	// Disable Control

	QAction* actionDisable = new QAction(tr("Deactivate Control"), &menu);

	auto fDisableControl = [this, filter]() -> void
	{
		activateControl(filter->caption(), false);
	};
	actionDisable->setEnabled(ts.state.controlisactive() == true);
	connect(actionDisable, &QAction::triggered, this, fDisableControl);

	menu.addAction(actionDisable);

	// Run the menu

	menu.exec(QCursor::pos());
}

void TuningWorkspace::slot_maskReturnPressed()
{
	slot_maskApply();
}

void TuningWorkspace::slot_maskApply()
{
	if (m_filterTree->topLevelItemCount() != 1)
	{
		return;
	}

	QTreeWidgetItem* rootItem = m_filterTree->topLevelItem(0);
	if (rootItem == nullptr)
	{
		assert(false);
		return;
	}

	std::shared_ptr<TuningFilter> rootFilter = rootItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (rootFilter == nullptr)
	{
		assert(rootFilter);
		return;
	}

	if (m_treeMask->text().isEmpty() == false)
	{
		m_treeMask->setStyleSheet("QLineEdit { color: red }");
		m_treeMaskApply->setStyleSheet("QPushButton { color: red }");
	}
	else
	{
		m_treeMask->setStyleSheet(QString());
		m_treeMaskApply->setStyleSheet(QString());
	}

	updateFiltersTree(rootFilter);
}

void TuningWorkspace::slot_parentTreeFilterChanged(std::shared_ptr<TuningFilter> filter)
{
	m_treeFilter = filter;
	emit treeFilterSelectionChanged(m_treeFilter);
}

void TuningWorkspace::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (m_currentbuttonFilter == nullptr)
	{
		assert(m_currentbuttonFilter);
		return;
	}

	// Remember the tab index for current button

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		int index = m_tab->currentIndex();

		m_activeTabPagesMap[m_currentbuttonFilter->ID()] = index;
	}

	// Set the new filter

	m_currentbuttonFilter = filter;

	// Update tab

	createTabPages();

	emit buttonFilterSelectionChanged(filter);
}
