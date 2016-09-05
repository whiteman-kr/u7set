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

	for (auto f : theFilters.filters)
	{
		if (f->isTree() == false)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());

		treeItems.push_back(item);

		addChildTreeObjects(f.get(), item);
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

		// Create splitter control
		//
		m_hSplitter = new QSplitter();
		m_hSplitter->addWidget(m_filterTree);
	}

	// Fill tab pages
	//

	std::vector<std::pair<TuningPage*, QString>> tuningPages;

	for (auto f : theFilters.filters)
	{
		if (f->isTab() == false)
		{
			continue;
		}

		TuningPage* tp = new TuningPage(f);

		tuningPages.push_back(std::make_pair(tp, f->caption()));
	}

	if (tuningPages.empty() == true)
	{
		// No tab pages, create only one page
		//
		m_tuningPage = new TuningPage();
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

	for (auto f : filter->childFilters)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());

		parent->addChild(item);

		addChildTreeObjects(f.get(), item);
	}
}
