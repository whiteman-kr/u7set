#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

#include <QButtonGroup>

int TuningWorkspace::m_instanceCounter = 0;

TuningWorkspace::TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent) :
	QWidget(parent),
	m_treeFilter(treeFilter),
	m_workspaceFilter(workspaceFilter),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient)
{
	qDebug() << "TuningWorkspace::TuningWorkspace m_instanceCounter = " << m_instanceCounter;
	m_instanceCounter++;

	//assert(m_treeFilter); // Can be nullptr
	assert(m_workspaceFilter);
	assert(m_tuningSignalManager);
	assert(m_tuningTcpClient);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* rightWidget = new QWidget();

	m_rightLayout = new QVBoxLayout(rightWidget);

	//

	updateFiltersTree();

	//

	createButtons();

	if (m_buttonsLayout != nullptr)
	{
		m_rightLayout->addLayout(m_buttonsLayout);
	}

	//

	updateTabControl();

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
	qDebug() << "TuningWorkspace::~TuningWorkspace m_instanceCounter = " << m_instanceCounter;

	if (m_hSplitter != nullptr)
	{
		theSettings.m_tuningWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

void TuningWorkspace::onTimer()
{
	updateTreeItemsStatus();
}

void TuningWorkspace::updateFiltersTree()
{
	// Fill the filter tree
	//
	std::shared_ptr<TuningFilter> rootFilter = m_workspaceFilter;
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
	if (m_filterTree == nullptr)
	{
		m_filterTree = new QTreeWidget();
		m_filterTree->setSortingEnabled(true);
		connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TuningWorkspace::slot_treeSelectionChanged);

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

	m_treeFilter = nullptr;

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

void TuningWorkspace::createButtons()
{
	std::vector<FilterButton*> buttons;

	// Buttons
	//

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
		buttons.push_back(button);

		if (firstButton)
		{
			firstButton = false;
		}

		connect(button, &FilterButton::filterButtonClicked, this, &TuningWorkspace::slot_filterButtonClicked);

	}

	if (buttons.empty() == false)
	{
		QButtonGroup* filterButtonGroup = new QButtonGroup();

		filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: buttons)
		{
			filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();

		m_buttonFilter = buttons[0]->filter();
	}
}

void TuningWorkspace::updateTabControl()
{
	// Fill tab pages
	//

	std::vector<std::pair<QWidget*, std::shared_ptr<TuningFilter>>> tuningPages;

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

		QWidget* tp = createTuningPage(f);

		tuningPages.push_back(std::make_pair(tp, f));
	}

	// Buttons level tabs

	if (m_buttonFilter != nullptr)
	{
		for (int i = 0; i < m_buttonFilter->childFiltersCount(); i++)
		{
			std::shared_ptr<TuningFilter> f = m_buttonFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			if (f->isTab() == false)
			{
				continue;
			}

			QWidget* tp = createTuningPage(f);

			tuningPages.push_back(std::make_pair(tp, f));
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

			m_tab->setVisible(false);
		}
		else
		{
			m_tab->setVisible(false);

			m_tab->clear();
		}

		if (m_tuningPage != nullptr)
		{
			m_tuningPage->setVisible(false);
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

		if (m_buttonFilter == nullptr)
		{
			assert(m_buttonFilter);
			return;
		}

		auto it = m_activeTabPagesMap.find(m_buttonFilter->ID());
		if (it != m_activeTabPagesMap.end())
		{
			int index = m_activeTabPagesMap[m_buttonFilter->ID()];
			m_tab->setCurrentIndex(index);
		}
	}
	else
	{
		// No tab pages, create only one page
		//
		if (m_tuningPage == nullptr)
		{

			std::shared_ptr<TuningFilter> emptyFilter = std::make_shared<TuningFilter>();

			QUuid uid = QUuid::createUuid();

			emptyFilter->setID(uid.toString());

			QWidget* tp = createTuningPage(emptyFilter);

			m_rightLayout->addWidget(tp);

			m_tuningPage = (TuningPage*)tp;
		}

		if (m_tab != nullptr)
		{
			m_tab->setVisible(false);
		}
		m_tuningPage->setVisible(true);
	}
}

QWidget* TuningWorkspace::createTuningPage(std::shared_ptr<TuningFilter> childWorkspaceFilter)
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
		auto it = m_tuningWorkspacesMap.find(childWorkspaceFilterId);
		if (it == m_tuningWorkspacesMap.end())
		{
			TuningWorkspace* tw = new TuningWorkspace(m_treeFilter, childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient);

			m_tuningWorkspacesMap[childWorkspaceFilterId] = tw;

			connect(this, &TuningWorkspace::treeFilterSelectionChanged, tw, &TuningWorkspace::slot_treeFilterChanged);

			return tw;
		}
		else
		{
			return it->second;
		}
	}
	else
	{

		auto it = m_tuningPagesMap.find(childWorkspaceFilterId);
		if (it == m_tuningPagesMap.end())
		{
			TuningPage* tp = new TuningPage(m_treeFilter, childWorkspaceFilter, m_buttonFilter, m_tuningSignalManager, m_tuningTcpClient);

			m_tuningPagesMap[childWorkspaceFilterId] = tp;

			connect(this, &TuningWorkspace::treeFilterSelectionChanged, tp, &TuningPage::slot_treeFilterSelectionChanged);

			connect(this, &TuningWorkspace::buttonFilterSelectionChanged, tp, &TuningPage::slot_buttonFilterSelectionChanged);

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

		int lmErrorsCount = m_tuningTcpClient->getLMErrorsCount(equipmentHashes);
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
			int sorCount = m_tuningTcpClient->getSORCount(equipmentHashes);
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


void TuningWorkspace::slot_treeSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		m_treeFilter = nullptr;
		emit treeFilterSelectionChanged(nullptr);
	}
	else
	{
		std::shared_ptr<TuningFilter> filter = selectedItems[0]->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		m_treeFilter = filter;
		emit treeFilterSelectionChanged(filter);
	}
}

void TuningWorkspace::slot_maskReturnPressed()
{
	updateFiltersTree();
}

void TuningWorkspace::slot_maskApply()
{
	updateFiltersTree();
}

void TuningWorkspace::slot_treeFilterChanged(std::shared_ptr<TuningFilter> filter)
{
	m_treeFilter = filter;
	emit treeFilterSelectionChanged(filter);
}

void TuningWorkspace::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	// Remember the tab index for current button

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		if (m_buttonFilter == nullptr)
		{
			assert(m_buttonFilter);
			return;
		}

		int index = m_tab->currentIndex();

		m_activeTabPagesMap[m_buttonFilter->ID()] = index;
	}

	// Set the new filter

	m_buttonFilter = filter;

	// Update tab

	updateTabControl();

	if (m_tab == nullptr)
	{
		emit buttonFilterSelectionChanged(filter);
	}
}
