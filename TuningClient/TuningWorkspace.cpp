#include "Stable.h"
#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

TuningWorkspace::TuningWorkspace(TuningObjectManager* tuningObjectManager, const TuningObjectStorage* objects, QWidget *parent) :
	m_objects(*objects),
	QWidget(parent)
{

	assert(tuningObjectManager);
	assert(objects);

    QVBoxLayout* pLayout = new QVBoxLayout();
	setLayout(pLayout);

    // Fill tree
    //

    fillFiltersTree();

	// Fill tab pages
	//

	std::vector<std::pair<TuningPage*, QString>> tuningPages;

	int tuningPageIndex = 0;

	// Tabs by filters

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

		TuningPage* tp = new TuningPage(tuningPageIndex++, f, tuningObjectManager, &m_objects);

		connect(this, &TuningWorkspace::filterSelectionChanged, tp, &TuningPage::slot_filterTreeChanged);

		tuningPages.push_back(std::make_pair(tp, f->caption()));
	}



	if (tuningPages.empty() == true)
	{
		// No tab pages, create only one page
		//
		std::shared_ptr<TuningFilter> emptyTabFilter = nullptr;

		m_tuningPage = new TuningPage(tuningPageIndex, emptyTabFilter, tuningObjectManager, &m_objects);

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

        connect(m_tab, &QTabWidget::currentChanged, this, &TuningWorkspace::slot_currentTabChanged);


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

void TuningWorkspace::fillFiltersTree()
{
    // Fill the filter tree
    //
    std::shared_ptr<TuningFilter> rootFilter = theFilters.m_root;
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

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<rootFilter->caption());
    item->setData(0, Qt::UserRole, QVariant::fromValue(rootFilter));

    addChildTreeObjects(rootFilter, item, mask);

    if (item->childCount() == 0)
    {
        delete item;
        return;
    }

    // Create tree control
    //
    if (m_hSplitter == nullptr)
    {
        m_filterTree = new QTreeWidget();
        m_filterTree->setSortingEnabled(true);
        connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);

        QStringList headerLabels;
        headerLabels<<tr("Caption");

        m_filterTree->setColumnCount(headerLabels.size());
        m_filterTree->setHeaderLabels(headerLabels);

        m_treeMask = new QLineEdit();
        connect(m_treeMask, &QLineEdit::returnPressed, this, &TuningWorkspace::slot_maskReturnPressed);

        m_treeMaskApply = new QPushButton(tr("Filter"));
        connect(m_treeMaskApply, &QPushButton::clicked, this, &TuningWorkspace::slot_maskApply);

        QHBoxLayout* searchLayout = new QHBoxLayout();
        searchLayout->addWidget(m_treeMask);
        searchLayout->addWidget(m_treeMaskApply);

        QWidget* treeLayoutWidget = new QWidget();

        QVBoxLayout* treeLayout = new QVBoxLayout(treeLayoutWidget);

        treeLayout->addWidget(m_filterTree);
        treeLayout->addLayout(searchLayout);

        // Create splitter control
        //
        m_hSplitter = new QSplitter();
        m_hSplitter->addWidget(treeLayoutWidget);
    }
    else
    {
        m_filterTree->clear();
    }

    // Fill filters control
    //

    m_filterTree->blockSignals(true);

    m_filterTree->addTopLevelItem(item);

    item->setSelected(true);

    item->setExpanded(true);

    m_filterTree->sortItems(0, Qt::AscendingOrder);

    m_filterTree->blockSignals(false);

    if (mask.isEmpty() == false)
    {
        m_filterTree->expandAll();
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

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<caption);
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		parent->addChild(item);

        addChildTreeObjects(f, item, mask);
	}
}


void TuningWorkspace::slot_treeSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		emit filterSelectionChanged(nullptr);
	}
	else
	{
		std::shared_ptr<TuningFilter> filter = selectedItems[0]->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		emit filterSelectionChanged(filter);
	}
}

void TuningWorkspace::slot_maskReturnPressed()
{
    fillFiltersTree();
}

void TuningWorkspace::slot_maskApply()
{
    fillFiltersTree();
}


void TuningWorkspace::slot_currentTabChanged(int index)
{
    if (m_tab == nullptr)
    {
        assert(m_tab);
        return;
    }

    TuningPage* page = dynamic_cast<TuningPage*>(m_tab->widget(index));
    if (page == nullptr)
    {
        assert(page);
        return;
    }

    // Set the tab back color
    //

    QColor backColor = page->backColor();
    QColor textColor = page->textColor();

    if (backColor.isValid() && textColor.isValid() && backColor != textColor)
    {
        // See http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qtabwidget-and-qtabbar
        //

        QString s = QString("\
            QTabWidget::pane \
            {\
                background: solid %1;\
                border-top: 2px solid %1;\
                color: %2;\
            }\
            QTabWidget::tab-bar\
            {\
              alignment: left;\
            }\
            QTabBar::tab\
            {\
                border: 2px solid #C4C4C3;\
                border-bottom-color: #C2C7CB;\
                border-top-left-radius: 4px;\
                border-top-right-radius: 4px;\
                min-width: 8px;\
                padding: 4px;\
            }\
            QTabBar::tab:selected\
            {\
                background: solid %1;\
                border-color: #C2C7CB;\
                border-bottom-color: #C2C7CB;\
                color: %2;\
            }\
        ").arg(backColor.name()).arg(textColor.name());

        m_tab->setStyleSheet(s);
    }

}
