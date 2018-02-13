#include "TuningWorkspace.h"
#include "Settings.h"
#include "MainWindow.h"

#include <QButtonGroup>
#include <QTreeWidget>
#include <QTreeWidgetItem>

int TuningWorkspace::m_instanceCounter = 0;

TuningWorkspace::TuningWorkspace(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> workspaceFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent) :
	QWidget(parent),
	m_treeFilter(treeFilter),
	m_workspaceFilter(workspaceFilter),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient)
{
	//qDebug() << "TuningWorkspace::TuningWorkspace m_instanceCounter = " << m_instanceCounter;
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
	qDebug() << "TuningWorkspace::~TuningWorkspace m_instanceCounter = " << m_instanceCounter;

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

	if (m_singleTuningPage != nullptr)
	{
		if (m_singleTuningPage->hasPendingChanges() == true)
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

	if (m_singleTuningPage != nullptr)
	{
		if (m_singleTuningPage->askForSavePendingChanges() == false)
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

		connect(m_filterTree, &QTreeWidget::currentItemChanged, this, &TuningWorkspace::slot_currentTreeItemChanged);
		connect(m_filterTree, &QWidget::customContextMenuRequested, this, &TuningWorkspace::slot_treeContextMenuRequested);

		QStringList headerLabels;
		headerLabels << tr("Caption");
		headerLabels << tr("Discretes");
		headerLabels << tr("Status");
		if (theConfigSettings.showSOR == true)
		{
			headerLabels << tr("SOR");
		}
		headerLabels << tr("");

		m_filterTree->setColumnCount(headerLabels.size());
		m_filterTree->setHeaderLabels(headerLabels);

		m_filterTree->setColumnWidth(columnName, 200);
		m_filterTree->setColumnWidth(columnDiscreteCountIndex, 60);
		m_filterTree->setColumnWidth(columnStatusIndex, 60);
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

	m_filterTree->addTopLevelItem(rootItem);

	rootItem->setSelected(true);

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
			std::shared_ptr<TuningFilter> emptyFilter = std::make_shared<TuningFilter>();

			QUuid uid = QUuid::createUuid();

			emptyFilter->setID(uid.toString());

			QWidget* tp = createTuningPageOrWorkspace(emptyFilter);

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
			TuningWorkspace* tw = new TuningWorkspace(m_treeFilter, childWorkspaceFilter, m_tuningSignalManager, m_tuningTcpClient);

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
			TuningPage* tp = new TuningPage(m_treeFilter, childWorkspaceFilter, m_currentbuttonFilter, m_tuningSignalManager, m_tuningTcpClient);

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
	if (theConfigSettings.showDiscreteCounters == false)
	{
		return;
	}

	// Tab counters

	if (m_tab != nullptr)
	{
		int tabIndex = 0;

		assert(m_tab->count() == static_cast<int>(m_tabsFilters.size()));

		for (std::shared_ptr<TuningFilter> f : m_tabsFilters)
		{
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
				newCaption = QString("%1 [%2]").arg(f->caption()).arg(discreteCount);
			}

			if (m_tab->tabText(tabIndex) != newCaption)
			{
				m_tab->setTabText(tabIndex, newCaption);
			}

			tabIndex++;
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
			newCaption = QString("%1 [%2]").arg(button->caption()).arg(discreteCount);
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
		TuningFilterCounters counters = filter->counters();

		// Counters column

		if (theConfigSettings.showDiscreteCounters == true && filter->hasDiscreteCounter() == true)
		{
			treeItem->setText(columnDiscreteCountIndex, QString("%1").arg(counters.discreteCounter));
		}

		// Status column

		if (filter->isSourceEquipment() == true)
		{
			updateTuningSourceTreeItem(treeItem, filter.get());
		}
		else
		{
			if (counters.errorCounter == 0)
			{
				treeItem->setText(columnStatusIndex, QString());
				treeItem->setBackground(columnStatusIndex, QBrush(Qt::white));
				treeItem->setForeground(columnStatusIndex, QBrush(Qt::black));
			}
			else
			{
				treeItem->setText(columnStatusIndex, QString("%1 errors").arg(counters.errorCounter));
				treeItem->setBackground(columnStatusIndex, QBrush(Qt::red));
				treeItem->setForeground(columnStatusIndex, QBrush(Qt::white));
			}
		}

		// SOR Column

		if (theConfigSettings.showSOR == true)
		{
			treeItem->setText(columnSorIndex, QString("%1").arg(counters.sorCounter));

			if (counters.sorCounter == 0)
			{
				treeItem->setBackground(columnSorIndex, QBrush(Qt::white));
				treeItem->setForeground(columnSorIndex, QBrush(Qt::black));
			}
			else
			{
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

void TuningWorkspace::updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilter* filter)
{

	QString state;
	int errorsCount = 0;
	int sorCount = 0;

	Hash hash = treeItem->data(1, Qt::UserRole).value<Hash>();

	if (m_tuningTcpClient->tuningSourceStatus(hash, &errorsCount, &sorCount, &state) == false)
	{
		treeItem->setText(columnStatusIndex, tr("Unknown"));
		treeItem->setBackground(columnStatusIndex, QBrush(Qt::gray));
		treeItem->setForeground(columnStatusIndex, QBrush(Qt::white));

		return;
	}

	if (filter->counters().controlEnabledCounter == 0)
	{
		treeItem->setText(columnStatusIndex, tr("Disabled"));
		treeItem->setBackground(columnStatusIndex, QBrush(Qt::gray));
		treeItem->setForeground(columnStatusIndex, QBrush(Qt::white));
	}
	else
	{
		treeItem->setText(columnStatusIndex, state);

		if (errorsCount == 0)
		{
			treeItem->setBackground(columnStatusIndex, QBrush(Qt::white));
			treeItem->setForeground(columnStatusIndex, QBrush(Qt::black));
		}
		else
		{
			treeItem->setBackground(columnStatusIndex, QBrush(Qt::red));
			treeItem->setForeground(columnStatusIndex, QBrush(Qt::white));
		}
	}
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

void TuningWorkspace::slot_currentTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	Q_UNUSED(previous);

	if (current == nullptr)
	{
		m_treeFilter = nullptr;
		emit treeFilterSelectionChanged(nullptr);
	}
	else
	{
		std::shared_ptr<TuningFilter> filter = current->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		emit treeFilterSelectionChanged(filter);
	}
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
			if (QMessageBox::warning(this, qAppName(),
									 tr("Are you sure you want to activate the source %1?").arg(filter->caption()),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}

			m_tuningTcpClient->activateTuningSourceControl(filter->caption(), true);
	};
	actionEnable->setEnabled(ts.state.controlisactive() == false);
	connect(actionEnable, &QAction::triggered, this, fEnableControl);

	// Disable Control

	QAction* actionDisable = new QAction(tr("Deactivate Control"), &menu);

	auto fDisableControl = [this, filter]() -> void
	{
			if (QMessageBox::warning(this, qAppName(),
									 tr("Are you sure you want to deactivate the source %1?").arg(filter->caption()),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}

			m_tuningTcpClient->activateTuningSourceControl(filter->caption(), false);
	};
	actionDisable->setEnabled(ts.state.controlisactive() == true);
	connect(actionDisable, &QAction::triggered, this, fDisableControl);

	// Run the menu

	menu.addAction(actionEnable);
	menu.addAction(actionDisable);

	menu.exec(QCursor::pos());
}

void TuningWorkspace::slot_maskReturnPressed()
{
	updateFiltersTree();
}

void TuningWorkspace::slot_maskApply()
{
	updateFiltersTree();
}

void TuningWorkspace::slot_parentTreeFilterChanged(std::shared_ptr<TuningFilter> filter)
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

	if (m_tab == nullptr)
	{
		emit buttonFilterSelectionChanged(filter);
	}
}
