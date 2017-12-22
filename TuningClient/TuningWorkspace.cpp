#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

TuningWorkspace::TuningWorkspace(TuningSignalManager* tuningSignalManager, TuningFilterStorage* filterStorage, const TuningSignalStorage* objects, QWidget* parent) :
	QWidget(parent),
	m_objects(*objects),
	m_tuningSignalManager(tuningSignalManager),
	m_filterStorage(filterStorage)
{

	assert(tuningSignalManager);
	assert(m_filterStorage),
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

	int count = m_filterStorage->m_root->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = m_filterStorage->m_root->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTab() == false)
		{
			continue;
		}

		TuningPage* tp = new TuningPage(tuningPageIndex++, f, tuningSignalManager, filterStorage, &m_objects);

		connect(this, &TuningWorkspace::filterSelectionChanged, tp, &TuningPage::slot_filterTreeChanged);

		tuningPages.push_back(std::make_pair(tp, f->caption()));
	}



	if (tuningPages.empty() == true)
	{
		// No tab pages, create only one page
		//
		std::shared_ptr<TuningFilter> emptyTabFilter = nullptr;

		m_tuningPage = new TuningPage(tuningPageIndex, emptyTabFilter, tuningSignalManager, filterStorage, &m_objects);

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
		m_hSplitter->restoreState(theSettings.m_tuningWorkspaceSplitterState);
	}
}

TuningWorkspace::~TuningWorkspace()
{
	if (m_hSplitter != nullptr)
	{
		theSettings.m_tuningWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

void TuningWorkspace::onTimer()
{
	updateTreeItemsStatus();
}

void TuningWorkspace::fillFiltersTree()
{
	// Fill the filter tree
	//
	std::shared_ptr<TuningFilter> rootFilter = m_filterStorage->m_root;
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

	QTreeWidgetItem* item = new QTreeWidgetItem(l);
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
		connect(m_filterTree, &QTreeWidget::currentItemChanged, this, &TuningWorkspace::slot_currentItemChanged);

		QStringList headerLabels;
		headerLabels << tr("Caption");
		headerLabels << tr("Status");
		if (theConfigSettings.showSOR == true)
		{
			headerLabels << tr("SOR");
		}
		headerLabels << tr("");

		m_filterTree->setColumnCount(headerLabels.size());
		m_filterTree->setHeaderLabels(headerLabels);

		m_filterTree->setColumnWidth(columnName, 200);
		m_filterTree->setColumnWidth(columnErrorIndex, 60);
		if (theConfigSettings.showSOR == true)
		{
			m_filterTree->setColumnWidth(columnSorIndex, 60);
		}

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

		QStringList l;
		l << caption;

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		parent->addChild(item);

		addChildTreeObjects(f, item, mask);
	}
}

void TuningWorkspace::updateTreeItemsStatus(QTreeWidgetItem* treeItem)
{
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

	const std::vector<QString>& equipmentHashes = filter->equipmentHashes();

	if (equipmentHashes.empty() == false)
	{
		// Print the data in status and SOR columns
		//

		int lmErrorsCount = m_tuningSignalManager->getLMErrorsCount(equipmentHashes);
		if (lmErrorsCount == 0)
		{
			treeItem->setText(columnErrorIndex, QString());
			treeItem->setBackground(columnErrorIndex, QBrush(Qt::white));
			treeItem->setForeground(columnErrorIndex, QBrush(Qt::black));
		}
		else
		{
			treeItem->setText(columnErrorIndex, QString("ERR (%1)").arg(lmErrorsCount));
			treeItem->setBackground(columnErrorIndex, QBrush(Qt::red));
			treeItem->setForeground(columnErrorIndex, QBrush(Qt::white));
		}

		if (theConfigSettings.showSOR == true)
		{
			int sorCount = m_tuningSignalManager->getSORCount(equipmentHashes);
			if (sorCount == 0)
			{
				treeItem->setText(columnSorIndex, QString());
				treeItem->setBackground(columnSorIndex, QBrush(Qt::white));
				treeItem->setForeground(columnSorIndex, QBrush(Qt::black));
			}
			else
			{
				treeItem->setText(columnSorIndex, QString("SOR (%1)").arg(sorCount));
				treeItem->setBackground(columnSorIndex, QBrush(Qt::red));
				treeItem->setForeground(columnSorIndex, QBrush(Qt::white));
			}
		}
	}

	int count = treeItem->childCount();
	for (int i = 0; i < count; i++)
	{
		updateTreeItemsStatus(treeItem->child(i));
	}


}


void TuningWorkspace::slot_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{

	if (m_tuningPage != nullptr && m_tuningPage->askForSavePendingChanges() == false)
	{
		m_treeItemToSelect = previous;
		QTimer::singleShot(10, this, &TuningWorkspace::slot_selectPreviousTreeItem);
		return;
	}

	if (current == nullptr)
	{
		emit filterSelectionChanged(nullptr);
	}
	else
	{
		std::shared_ptr<TuningFilter> filter = current->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
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

void TuningWorkspace::slot_selectPreviousTreeItem()
{
	if (m_filterTree != nullptr && m_treeItemToSelect != nullptr)
	{
		m_filterTree->blockSignals(true);
		m_filterTree->setCurrentItem(m_treeItemToSelect);
		m_filterTree->blockSignals(false);
	}
}
