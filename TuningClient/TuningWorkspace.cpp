#include "Stable.h"
#include "TuningWorkspace.h"
#include "Settings.h"
#include "ObjectFilter.h"

TuningWorkspace::TuningWorkspace(QWidget *parent)
	:QWidget(parent)
{
	QVBoxLayout* pLayout = new QVBoxLayout();
	setLayout(pLayout);

	// Fill the filter tree
	//

	std::vector<QTreeWidgetItem*> treeItems;

	int count = theFilters.topFilterCount();
	for (int i = 0; i < count; i++)
	{
		ObjectFilter* f = theFilters.topFilter(i).get();
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
		item->setData(0, Qt::UserRole, f->hash());

		treeItems.push_back(item);

		addChildTreeObjects(f, item);
	}

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

	count = theFilters.topFilterCount();
	for (int i = 0; i < count; i++)
	{
		ObjectFilter* f = theFilters.topFilter(i).get();
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


void TuningWorkspace::addChildTreeObjects(ObjectFilter* filter, QTreeWidgetItem* parent)
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
		ObjectFilter* f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, f->hash());
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

	Hash filterHash = item->data(0, Qt::UserRole).value<Hash>();

	emit filterSelectionChanged(filterHash);
}
