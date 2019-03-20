#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

#include <QButtonGroup>
#include <QTreeWidget>
#include <QTreeWidgetItem>

//
// FilterButton
//

FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, bool check, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filter = filter;
	m_caption = caption;

	setCheckable(true);

	if (check == true)
	{
		setChecked(true);
	}

	setMinimumSize(100, 25);

	QColor backColor = Qt::lightGray;
	QColor textColor = Qt::white;

	QColor backSelectedColor = Qt::darkGray;
	QColor textSelectedColor = Qt::white;

	if (filter->backColor().isValid() && filter->textColor().isValid() && filter->backColor() != filter->textColor())
	{
		backColor = filter->backColor();
		textColor = filter->textColor();
	}

	if (filter->backSelectedColor().isValid() && filter->textSelectedColor().isValid() && filter->backSelectedColor() != filter->textSelectedColor())
	{
		backSelectedColor = filter->backSelectedColor();
		textSelectedColor = filter->textSelectedColor();
	}

	setStyleSheet(tr("\
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
					 .arg(textSelectedColor.name()));

	update();


	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
{
	return m_filter;
}

QString FilterButton::caption() const
{
	return m_caption;
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

TuningWorkspace::TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, TuningFilterStorage* tuningFilterStorage, QWidget* parent) :
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

	if (workspaceFilter->backColor().isValid() && workspaceFilter->textColor().isValid() && workspaceFilter->backColor() != workspaceFilter->textColor())
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

		if (columnNameIndex != -1)
		{
			int width = m_filterTree->columnWidth(columnNameIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnIndex", width);

		}
		if (columnDiscreteCountIndex != -1)
		{
			int width = m_filterTree->columnWidth(columnDiscreteCountIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnDiscreteCount", width);
		}
		if (columnStatusIndex != -1)
		{
			int width = m_filterTree->columnWidth(columnStatusIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnStatus", width);
		}
		if (columnSorIndex != -1)
		{
			int width = m_filterTree->columnWidth(columnSorIndex);
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
	updateCounters();
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

		m_filterTree->viewport()->installEventFilter(this);
		m_filterTree->installEventFilter(this);

		m_filterTree->setContextMenuPolicy(Qt::CustomContextMenu);

		connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);
		connect(m_filterTree, &QWidget::customContextMenuRequested, this, &TuningWorkspace::slot_treeContextMenuRequested);

		int columnIndex = columnNameIndex;

		QStringList headerLabels;

		headerLabels << tr("Caption");
		columnIndex++;

		if (theConfigSettings.showDiscreteCounters == true)
		{
			headerLabels << tr("Discretes");
			columnDiscreteCountIndex = columnIndex;
			columnIndex++;
		}

		headerLabels << tr("Status");
		columnStatusIndex = columnIndex;
		columnIndex++;

		if (theConfigSettings.showSOR == true)
		{
			headerLabels << tr("SOR");
			columnSorIndex = columnIndex;
			columnIndex++;
		}

		headerLabels << tr("");

		m_filterTree->setColumnCount(headerLabels.size());
		m_filterTree->setHeaderLabels(headerLabels);

		// Set column width

		const int columnMaxWidth = 500;

		QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

		if (columnNameIndex != -1)
		{
			const int defaultWidth = 200;

			int width = settings.value("TuningWorkspace/FilterTreeColumnIndex", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(columnNameIndex, width);
		}
		if (columnDiscreteCountIndex != -1)
		{
			const int defaultWidth = 80;

			int width = settings.value("TuningWorkspace/FilterTreeColumnDiscreteCount", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(columnDiscreteCountIndex, width);
		}
		if (columnStatusIndex != -1)
		{
			const int defaultWidth = 150;

			int width = settings.value("TuningWorkspace/FilterTreeColumnStatus", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(columnStatusIndex, width);
		}
		if (columnSorIndex != -1)
		{
			const int defaultWidth = 60;

			int width = settings.value("TuningWorkspace/FilterTreeColumnSor", defaultWidth).toInt();
			if (width < defaultWidth || width > columnMaxWidth)
			{
				width = defaultWidth;
			}

			m_filterTree->setColumnWidth(columnSorIndex, width);
		}

		//

		m_treeMask = new QLineEdit();
		connect(m_treeMask, &QLineEdit::returnPressed, this, &TuningWorkspace::slot_maskReturnPressed);

		QPushButton* m_treeMaskApply = new QPushButton(tr("Filter"));
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

		FilterButton* button = new FilterButton(f, f->caption(), firstButton);
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

		static QString equipmentString = tr("Equipment");
		static QString schemasString = tr("Schemas");
		Q_UNUSED(equipmentString);
		Q_UNUSED(schemasString);

		QStringList l;
		l << tr(caption.toUtf8().data());	// Try to translate filter name!

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		if (f->isSourceEquipment() == true)
		{
			item->setData(1, Qt::UserRole, ::calcHash(f->caption()));
		}

		parent->addChild(item);

		addChildTreeObjects(f, item, mask);
	}
}

void TuningWorkspace::updateCounters()
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

		QString newCaption;
		if (discreteCount == 0)
		{
			newCaption = button->caption();
		}
		else
		{
			newCaption = QString(" %1 [%2] ").arg(button->caption()).arg(discreteCount);
		}

		if (button->text() != newCaption)
		{
			button->setText(newCaption);
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
		TuningCounters counters = filter->counters();

		// Counters column

		if (columnDiscreteCountIndex != -1 && filter->hasDiscreteCounter() == true)
		{
			treeItem->setText(columnDiscreteCountIndex, QString("%1").arg(counters.discreteCounter));
		}

		// Status column

		if (filter->isSourceEquipment() == true)
		{
			updateTuningSourceTreeItem(treeItem);
		}
		else
		{
			assert(columnStatusIndex != -1);

			if (counters.errorCounter == 0)
			{
				treeItem->setText(columnStatusIndex, QString());
				treeItem->setBackground(columnStatusIndex, QBrush(Qt::white));
				treeItem->setForeground(columnStatusIndex, QBrush(Qt::black));
			}
			else
			{
				treeItem->setText(columnStatusIndex, QString("E: %1").arg(counters.errorCounter));
				treeItem->setBackground(columnStatusIndex, QBrush(Qt::red));
				treeItem->setForeground(columnStatusIndex, QBrush(Qt::white));
			}
		}

		// SOR Column

		if (columnSorIndex != -1 && theConfigSettings.showSOR == true)
		{
			if (counters.sorActive == false)
			{
				treeItem->setText(columnSorIndex, QString());	// Inactive
				treeItem->setBackground(columnSorIndex, QBrush(Qt::white));
				treeItem->setForeground(columnSorIndex, QBrush(Qt::black));
			}
			else
			{
				if (counters.sorValid == false)
				{
					treeItem->setText(columnSorIndex, "?");
					treeItem->setBackground(columnSorIndex, QBrush(Qt::red));
					treeItem->setForeground(columnSorIndex, QBrush(Qt::white));
				}
				else
				{
					if (counters.sorCounter == 0)
					{
						treeItem->setText(columnSorIndex, QString());	// Sor NO
						treeItem->setBackground(columnSorIndex, QBrush(Qt::white));
						treeItem->setForeground(columnSorIndex, QBrush(Qt::black));
					}
					else
					{
						if (counters.sorCounter == 1)
						{
							treeItem->setText(columnSorIndex, tr("SOR"));
						}
						else
						{
							treeItem->setText(columnSorIndex, QString("SOR [%1]").arg(counters.sorCounter));
						}
						treeItem->setBackground(columnSorIndex, QBrush(Qt::red));
						treeItem->setForeground(columnSorIndex, QBrush(Qt::white));
					}
				}
			}
		}
	}

	int count = treeItem->childCount();
	for (int i = 0; i < count; i++)
	{
		updateTreeItemsStatus(treeItem->child(i));
	}
}

void TuningWorkspace::updateTuningSourceTreeItem(QTreeWidgetItem* treeItem)
{
	Hash hash = treeItem->data(1, Qt::UserRole).value<Hash>();

	assert(columnStatusIndex != -1);

	int errorCounter = m_tuningTcpClient->sourceErrorCount(hash);

	TuningSource ts;

	QString status;
	bool valid = false;
	bool controlIsEnabled = false;
	bool hasUnappliedParams = false;

	if (m_tuningTcpClient->tuningSourceInfo(hash, &ts) == false)
	{
		valid = false;
		status = tr("Unknown");
	}
	else
	{
		valid = ts.valid();
		controlIsEnabled = ts.state.controlisactive();
		hasUnappliedParams = ts.state.hasunappliedparams();

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

	if (treeItem->text(columnStatusIndex) != status)
	{
		treeItem->setText(columnStatusIndex, status);
	}

	QBrush backColor;
	QBrush textColor;

	if (valid == false)
	{
		backColor = QBrush(Qt::white);
		textColor = QBrush(Qt::darkGray);
	}
	else
	{
		if (controlIsEnabled == false)
		{
			backColor = QBrush(Qt::gray);
			textColor = QBrush(Qt::white);
		}
		else
		{
			if (errorCounter > 0)
			{
				backColor = QBrush(Qt::red);
				textColor = QBrush(Qt::white);
			}
			else
			{
				if (hasUnappliedParams == true)
				{
					backColor = QBrush(Qt::yellow);
					textColor = QBrush(Qt::black);
				}
				else
				{
					backColor = QBrush(Qt::white);
					textColor = QBrush(Qt::black);
				}
			}
		}
	}

	treeItem->setBackground(columnStatusIndex, backColor);
	treeItem->setForeground(columnStatusIndex, textColor);
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
