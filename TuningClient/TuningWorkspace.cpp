#include "Stable.h"
#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"
#include "TuningFilterEditor.h"

TuningWorkspace::TuningWorkspace(QWidget *parent)
	:QWidget(parent)
{
    m_objects = theObjectManager->objects();

    QVBoxLayout* pLayout = new QVBoxLayout();
	setLayout(pLayout);

    fillFiltersTree();

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

        TuningPage* tp = new TuningPage(tuningPageIndex++, f, &m_objects);

		connect(this, &TuningWorkspace::filterSelectionChanged, tp, &TuningPage::slot_filterTreeChanged);

		tuningPages.push_back(std::make_pair(tp, f->caption()));
	}

	if (tuningPages.empty() == true)
	{
		// No tab pages, create only one page
		//
        m_tuningPage = new TuningPage(tuningPageIndex, nullptr, &m_objects);

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

std::vector<TuningObject>* TuningWorkspace::objects()
{
    return &m_objects;
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

    if (rootFilter->childFiltersCount() != 0)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<rootFilter->caption());
        item->setData(0, Qt::UserRole, QVariant::fromValue(rootFilter));

        addChildTreeObjects(rootFilter, item);

        // Create tree control
        //
        if (m_filterTree == nullptr)
        {
            m_filterTree = new QTreeWidget();
            m_filterTree->setSortingEnabled(true);
            connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);

            QStringList headerLabels;
            headerLabels<<tr("Caption");

            m_filterTree->setColumnCount(headerLabels.size());
            m_filterTree->setHeaderLabels(headerLabels);

            // Create splitter control
            //
            m_hSplitter = new QSplitter();
            m_hSplitter->addWidget(m_filterTree);
        }
        else
        {
            m_filterTree->clear();
        }

        // Fill filters control
        //

        m_filterTree->blockSignals(true);

        m_filterTree->addTopLevelItem(item);

        item->setExpanded(true);

        item->setSelected(true);

        m_filterTree->sortItems(0, Qt::AscendingOrder);

        m_filterTree->blockSignals(false);
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


void TuningWorkspace::slot_runPresetEditor()
{
    TuningFilterStorage editStorage = theFilters;

    bool editAutomatic = false;

    TuningFilterEditor d(&editStorage, &m_objects, editAutomatic, this);

    connect(theMainWindow, &MainWindow::signalsUpdated, &d, &TuningFilterEditor::slot_signalsUpdated);

    if (d.exec() == QDialog::Accepted)
    {
        theFilters = editStorage;

        QString errorMsg;

        if (theFilters.save(theSettings.userFiltersFile(), &errorMsg) == false)
        {
            theLogFile->writeError(errorMsg);
            QMessageBox::critical(this, tr("Error"), errorMsg);
        }

        fillFiltersTree();
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
