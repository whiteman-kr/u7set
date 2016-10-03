#include "Stable.h"
#include "TuningWorkspace.h"
#include "Settings.h"
#include "TuningFilter.h"

TuningWorkspace::TuningWorkspace(QWidget *parent)
	:QWidget(parent)
{
	QVBoxLayout* pLayout = new QVBoxLayout();
	setLayout(pLayout);

	// Fill the filter tree
	//

	std::vector<QTreeWidgetItem*> treeItems;

	fillFilters(treeItems, theFilters);
	fillFilters(treeItems, theUserFilters);

	if (treeItems.empty() == false)
	{
		// Create tree control
		//
		m_filterTree = new QTreeWidget();

		for (auto item : treeItems)
		{
			m_filterTree->addTopLevelItem(item);
		}

		connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);

		// Create splitter control
		//
		m_hSplitter = new QSplitter();
		m_hSplitter->addWidget(m_filterTree);
	}

	// Fill tab pages
	//

	std::vector<std::pair<TuningPage*, QString>> tuningPages;

	int tuningPageIndex = 0;

	int count = theFilters.m_root->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = theFilters.m_root->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTab() == false)
		{
			continue;
		}

		TuningPage* tp = new TuningPage(tuningPageIndex++, f);


		connect(this, &TuningWorkspace::filterSelectionChanged, tp, &TuningPage::slot_filterTreeChanged);

		tuningPages.push_back(std::make_pair(tp, f->caption()));
	}

	if (tuningPages.empty() == true)
	{
		// No tab pages, create only one page
		//
		m_tuningPage = new TuningPage(tuningPageIndex, nullptr);

		connect(this, &TuningWorkspace::filterSelectionChanged, m_tuningPage, &TuningPage::slot_filterTreeChanged);

		if (m_hSplitter != nullptr)
		{
			m_hSplitter->addWidget(m_tuningPage);
		}
		else
		{
			pLayout->addWidget(m_tuningPage);
		}
	}
	else
	{
		// Create tab control and add pages
		//
		m_tab = new QTabWidget();

		if (m_hSplitter != nullptr)
		{
			m_hSplitter->addWidget(m_tab);
		}
		else
		{
			pLayout->addWidget(m_tab);
		}

		for (auto t : tuningPages)
		{
			TuningPage* tp = t.first;
			QString tabName = t.second;

			m_tab->addTab(tp, tabName);
		}
	}

	// Restore splitter size
	//
	if (m_hSplitter != nullptr)
	{
		pLayout->addWidget(m_hSplitter);
		m_hSplitter->restoreState(theSettings.m_mainWindowSplitterState);
	}
}

TuningWorkspace::~TuningWorkspace()
{
	if (m_hSplitter != nullptr)
	{
		theSettings.m_mainWindowSplitterState = m_hSplitter->saveState();
	}
}



void TuningWorkspace::fillFilters(std::vector<QTreeWidgetItem*>& treeItems, TuningFilterStorage& filterStorage)
{
	for (int i = 0; i < filterStorage.m_root->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = filterStorage.m_root->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			return;
		}

		if (f->isTree() == false)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		treeItems.push_back(item);

		addChildTreeObjects(f, item);
	}

}

void TuningWorkspace::addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent)
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

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		parent->addChild(item);

		addChildTreeObjects(f, item);
	}
}

void TuningWorkspace::slot_treeSelectionChanged()
{
	QTreeWidgetItem* item = m_filterTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

	emit filterSelectionChanged(filter);
}
