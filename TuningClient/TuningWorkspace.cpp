#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

TuningWorkspace::TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent) :
	QWidget(parent),
	m_treeFilter(treeFilter),
	m_workspaceFilter(workspaceFilter),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient)
{

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
}

TuningWorkspace::~TuningWorkspace()
{
	if (m_hSplitter != nullptr)
	{
		theSettings.m_tuningWorkspaceSplitterState = m_hSplitter->saveState();
	}
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

		m_filterTree->setColumnCount(headerLabels.size());
		m_filterTree->setHeaderLabels(headerLabels);

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

		FilterButton* button = new FilterButton(f, f->caption());
		buttons.push_back(button);

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

		// Set the first button checked
		//
		buttons[0]->blockSignals(true);
		buttons[0]->setChecked(true);
		m_buttonFilter = buttons[0]->filter();
		buttons[0]->blockSignals(false);
	}
}

void TuningWorkspace::updateTabControl()
{
	// Fill tab pages
	//

	std::vector<std::pair<QWidget*, QString>> tuningPages;

	int tuningPageIndex = 0;

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

		QWidget* tp = createTuningPage(tuningPageIndex, f);

		tuningPageIndex++;

		tuningPages.push_back(std::make_pair(tp, f->caption()));
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

			QWidget* tp = createTuningPage(tuningPageIndex, f);

			tuningPageIndex++;

			tuningPages.push_back(std::make_pair(tp, f->caption()));
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

			connect(m_tab, &QTabWidget::currentChanged, this, &TuningWorkspace::slot_currentTabChanged);
		}
		else
		{
			m_tab->clear();
		}

		if (m_tuningPage != nullptr)
		{
			m_tuningPage->setVisible(false);
		}
		m_tab->setVisible(true);

		for (auto t : tuningPages)
		{
			QWidget* w = new QWidget();

			QHBoxLayout* l = new QHBoxLayout(w);

			QWidget* tp = t.first;

			QString tabName = t.second;

			l->addWidget(tp);

			m_tab->addTab(w, tabName);
		}
	}
	else
	{
		// No tab pages, create only one page
		//
		if (m_tuningPage == nullptr)
		{
			QWidget* tp = createTuningPage(0, nullptr);

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

QWidget* TuningWorkspace::createTuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> childWorkspaceFilter)
{
	bool createChildWorkspace = false;

	if (childWorkspaceFilter != nullptr)
	{
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
	}

	if (createChildWorkspace == true)
	{
		TuningWorkspace* tw = new TuningWorkspace(m_treeFilter, childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient, this);

		connect(this, &TuningWorkspace::treeFilterSelectionChanged, tw, &TuningWorkspace::slot_treeFilterChanged);

		return tw;
	}
	else
	{
		TuningPage* tp = new TuningPage(tuningPageIndex, m_treeFilter, childWorkspaceFilter, m_buttonFilter, m_tuningSignalManager, m_tuningTcpClient);

		connect(this, &TuningWorkspace::treeFilterSelectionChanged, tp, &TuningPage::slot_treeFilterSelectionChanged);

		connect(this, &TuningWorkspace::buttonFilterSelectionChanged, tp, &TuningPage::slot_buttonFilterSelectionChanged);

		return tp;
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


void TuningWorkspace::slot_currentTabChanged(int index)
{
	/*if (m_tab == nullptr)
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
*/
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

	m_buttonFilter = filter;

	//if (m_tab == nullptr)
	//else
	{
		updateTabControl();
	}

	if (m_tab == nullptr || m_tab->count() == 0)
	{
		emit buttonFilterSelectionChanged(filter);
	}
}
