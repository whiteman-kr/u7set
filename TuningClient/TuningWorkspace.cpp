#include "TuningWorkspace.h"
#include "Main.h"
#include "MainWindow.h"
#include "Settings.h"
#include <ClientLib/TuningConnection.h>

//
// FilterButton
//

FilterButton::FilterButton(const TuningLib::TuningUiItem& tuningUiItem, bool check, QWidget* parent)
	:QPushButton(tuningUiItem.caption(), parent),
	m_tuningUiItem(tuningUiItem)
{
	setCheckable(true);

	if (check == true)
	{
		setChecked(true);
	}

	setMinimumSize(100, 25);

	update(0);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

bool FilterButton::hasDiscreteCounter() const 
{
	return m_tuningUiItem.hasDiscreteCounter();
}

QString FilterButton::filters() const 
{
	return m_tuningUiItem.filters();
}

int FilterButton::counter() const
{
	return m_discreteCounter;
}

void FilterButton::update(int discreteCounter)
{
	// Text

	QString newCaption;

	if (discreteCounter == 0)
	{
		newCaption = m_tuningUiItem.caption();
	}
	else
	{
		newCaption = QString(" %1 [%2] ").arg(m_tuningUiItem.caption()).arg(discreteCounter);
	}

	m_discreteCounter = discreteCounter;

	if (text() != newCaption)
	{
		setText(newCaption);
	}

	// Color

	QColor backColor = Qt::lightGray;
	QColor textColor = Qt::white;

	QColor backSelectedColor = Qt::darkGray;
	QColor textSelectedColor = Qt::white;

	if (m_tuningUiItem.useColors() == true)
	{
		if (counter() != 0 && m_tuningUiItem.backAlertedColor() != m_tuningUiItem.textAlertedColor())
		{
			// Alerted state

			backColor = m_tuningUiItem.backAlertedColor();
			textColor = m_tuningUiItem.textAlertedColor();

			backSelectedColor = backColor;
			textSelectedColor = textColor;
		}
		else
		{
			if (m_tuningUiItem.backColor() != m_tuningUiItem.textColor())
			{
				backColor = m_tuningUiItem.backColor();
				textColor = m_tuningUiItem.textColor();
			}

			if (m_tuningUiItem.backSelectedColor() != m_tuningUiItem.textSelectedColor())
			{
				backSelectedColor = m_tuningUiItem.backSelectedColor();
				textSelectedColor = m_tuningUiItem.textSelectedColor();
			}
		}
	}

	QString style = tr("\
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
					   .arg(textSelectedColor.name());


					if (styleSheet() != style)
	{
					setStyleSheet(style);
}
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_tuningUiItem.uuid());
	}
}

//
// TuningWorkspace
//

int TuningWorkspace::m_instanceCounter = 0;

TuningWorkspace::TuningWorkspace(TuningConfigController& configController,
					ClientLib::TuningSignalManager& tuningSignalManager,
					TuningLib::TuningUiStorage& tuningUi,
					AppSignalLists::AppSignalListSet& appSignalLists,
					ClientLib::TuningUserManager& userManager,
					ClientLib::TuningConnection& tuningConnection,
					const TuningLib::TuningUiItem& workspaceUi, // Ui item specifies this workspace
					TuningCountersManager& tuningCounters,
					const QUuid& treeListUuid,                  // List selected in list tree
					bool hasFilterTree,
					QWidget* parent) :
	m_configController(configController),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningUi(tuningUi),
	m_appSignalLists(appSignalLists),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection),
	m_workspaceUi(workspaceUi),
	m_tuningCounters(tuningCounters),
	m_treeListUuid(treeListUuid),
	QWidget(parent)
{
	//qDebug() << "TuningWorkspace::TuningWorkspace m_instanceCounter = " << m_instanceCounter;
	m_instanceCounter++;

	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* rightWidget = new QWidget();

	m_rightLayout = new QVBoxLayout(rightWidget);

	//

	createButtons();

	if (m_buttonsLayout != nullptr)
	{
		m_rightLayout->addLayout(m_buttonsLayout);
	}

	//

	createTabPages();

	// Create filters tree
	//
	if (hasFilterTree == true)
	{
		m_treeLayoutWidget =
			new TreeFilterWidget(m_configController, m_tuningUi, m_appSignalLists, m_userManager, m_tuningConnection, m_tuningCounters, this);

		connect(m_treeLayoutWidget,
				&TreeFilterWidget::treeFilterSelectionChanged,
				[this](const QUuid& filterUuid)
				{
					emit treeFilterChanged(filterUuid);
				});

		// Create splitter control
		//
		m_hSplitter = new QSplitter();
		m_hSplitter->addWidget(m_treeLayoutWidget);
		m_hSplitter->addWidget(rightWidget);
		mainLayout->addWidget(m_hSplitter);

		// Restore splitter size
		//
		m_hSplitter->restoreState(TuningClientAppSettings::instance().user().m_tuningWorkspaceSplitterState);

		// Show/hide filter tree
		//
		m_treeLayoutWidget->setVisible(m_treeLayoutWidget->isEmpty() == false);
	}
	else
	{
		mainLayout->addWidget(rightWidget);
	}

	// Color

	if (workspaceUi.useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Window, workspaceUi.backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
		show();
	}

	connect(theApp.mainWindow(), &MainWindow::timerTick500, this, &TuningWorkspace::onTimer);
}

TuningWorkspace::~TuningWorkspace()
{
	m_instanceCounter--;
	//qDebug() << "TuningWorkspace::~TuningWorkspace m_instanceCounter = " << m_instanceCounter;

	if (m_hSplitter != nullptr)
	{
		TuningClientAppSettings::instance().user().m_tuningWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

bool TuningWorkspace::hasPendingChanges()
{
	for (auto& it : m_tuningPagesMap)
	{
		TuningPage* tp = it.second;

		if (tp->hasPendingChanges() == true)
		{
			return true;
		}
	}

	for (auto& it : m_tuningWorkspacesMap)
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
	for (auto& it : m_tuningPagesMap)
	{
		TuningPage* tp = it.second;

		if (tp->askForSavePendingChanges() == false)
		{
			return false;
		}
	}

	for (auto& it : m_tuningWorkspacesMap)
	{
		TuningWorkspace* tw = it.second;

		if (tw->askForSavePendingChanges() == false)
		{
			return false;
		}
	}

	return true;
}

void TuningWorkspace::updateFilters()
{
	if (m_treeLayoutWidget != nullptr)
	{
		m_treeLayoutWidget->fillFiltersTree();

		// Show/hide filter tree
		//
		m_treeLayoutWidget->setVisible(m_treeLayoutWidget->isEmpty() == false);
	}

	for (auto swp : m_switchPresetPages)
	{
		if (swp == nullptr)
		{
			Q_ASSERT(swp);
			return;
		}

		swp->createControls();
	}
}

void TuningWorkspace::onTimer()
{
	updateTabsButtonsCounters();

	if (m_treeLayoutWidget != nullptr)
	{
		m_treeLayoutWidget->updateFiltersTree();
	}
}

void TuningWorkspace::createButtons()
{
	// Create buttons
	//
	m_filterButtons.clear();
	m_currentButtonUi = nullptr;

	for (int i = 0; i < m_workspaceUi.childCount(); i++)
	{
		TuningLib::TuningUiItem* uiItem = m_workspaceUi.child(i).get();
		if (uiItem == nullptr)
		{
			assert(uiItem);
			continue;
		}

		if (uiItem->isButton() == true)
		{
			FilterButton* button = new FilterButton(*uiItem, m_filterButtons.empty() == true /*first button*/);
			button->installEventFilter(this);
			connect(button, &FilterButton::filterButtonClicked, this, &TuningWorkspace::slot_filterButtonClicked);

			if (m_filterButtons.empty() == true /*first button*/)
			{
				m_currentButtonUi = uiItem;
			}

			m_filterButtons.push_back(button);
		}
	}

	// Place buttons to layout
	//
	if (m_filterButtons.empty() == false)
	{
		QButtonGroup* filterButtonGroup = new QButtonGroup(this);
		filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: m_filterButtons)
		{
			filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();
	}
}

void TuningWorkspace::createTabPages()
{
	// Fill tab pages
	//
	std::vector<std::pair<QWidget*, const TuningLib::TuningUiItem*>> tuningPages;
	m_tabsUiItems.clear();

	// Workspace level tabs

	for (int i = 0; i < m_workspaceUi.childCount(); i++)
	{
		const TuningLib::TuningUiItem* uiItem = m_workspaceUi.child(i).get();
		if (uiItem == nullptr)
		{
			assert(uiItem);
			continue;
		}

		if (uiItem->isTab() == true)
		{
			QWidget* tp = createTuningPageOrWorkspace(*uiItem);
			tuningPages.push_back(std::make_pair(tp, uiItem));
			m_tabsUiItems.push_back(uiItem);
		}
	}

	// Buttons level tabs

	if (m_currentButtonUi != nullptr)
	{
		for (int i = 0; i < m_currentButtonUi->childCount(); i++)
		{
			TuningLib::TuningUiItem* uiItem = m_currentButtonUi->child(i).get();
			if (uiItem == nullptr)
			{
				assert(uiItem);
				continue;
			}

			if (uiItem->isTab() == false)
			{
				continue;
			}

			QWidget* tp = createTuningPageOrWorkspace(*uiItem);
			tuningPages.push_back(std::make_pair(tp, uiItem));
			m_tabsUiItems.push_back(uiItem);
		}
	}

	if (tuningPages.empty() == false)
	{
		// Create tab control and add pages
		//
		if (m_tab == nullptr)
		{
			m_tab = new QTabWidget();
			m_tab->setObjectName("TuningTabWidget");

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

		for (const auto& [tabPage, uiItem] : tuningPages)
		{
			QWidget* w = new QWidget();

			QHBoxLayout* l = new QHBoxLayout(w);

			l->addWidget(tabPage);

			m_tab->addTab(w, uiItem->caption());
		}

		m_tab->setVisible(true);

		// set the active tab

		if (m_currentButtonUi != nullptr)
		{
			auto it = m_activeTabPagesMap.find(m_currentButtonUi->uuid());
			if (it != m_activeTabPagesMap.end())
			{
				int index = m_activeTabPagesMap[m_currentButtonUi->uuid()];
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
			const TuningLib::TuningUiItem* singlePageUi = nullptr;

			if (m_currentButtonUi != nullptr)
			{
				// If a button is pressed - set button filter as page filter

				singlePageUi = m_currentButtonUi;
			}
			else
			{
				// Otherwise set workspace filter to page filter

				singlePageUi = &m_workspaceUi;
			}

			QWidget* tp = createTuningPageOrWorkspace(*singlePageUi);

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

QWidget* TuningWorkspace::createTuningPageOrWorkspace(const TuningLib::TuningUiItem& childWorkspaceUi)
{
	bool hasTabsAndButtons = false;

	for (int c = 0; c < childWorkspaceUi.childCount(); c++)
	{
		const TuningLib::TuningUiItem* uiItem = childWorkspaceUi.child(c).get();
		if (uiItem == nullptr)
		{
			assert(uiItem);
			continue;
		}

		if (uiItem->isTab() == true || uiItem->isButton() == true)
		{
			hasTabsAndButtons = true;
			break;
		}
	}

	if (hasTabsAndButtons == true)
	{
		return createChildWorkspace(childWorkspaceUi);
	}
	else
	{
		return createTuningPage(childWorkspaceUi);
	}
}

QWidget* TuningWorkspace::createChildWorkspace(const TuningLib::TuningUiItem& childWorkspaceUi) 
{
	// We have to create nested workspace
	//
	auto it = m_tuningWorkspacesMap.find(childWorkspaceUi.uuid());
	if (it == m_tuningWorkspacesMap.end())
	{
		TuningWorkspace* tw = new TuningWorkspace(m_configController,
												  m_tuningSignalManager,
												  m_tuningUi,
												  m_appSignalLists,
												  m_userManager,
												  m_tuningConnection,
												  childWorkspaceUi,
												  m_tuningCounters,
												  m_treeListUuid,
												  false /*hasFilterTree*/,
												  this /*parent*/);

		m_tuningWorkspacesMap[childWorkspaceUi.uuid()] = tw;

		connect(this, &TuningWorkspace::treeFilterChanged, tw, &TuningWorkspace::slot_parentWorkspaceTreeFilterChanged);

		return tw;
	}
	else
	{
		return it->second;
	}
}

QWidget* TuningWorkspace::createTuningPage(const TuningLib::TuningUiItem& childWorkspaceUi)
{
	if (childWorkspaceUi.isTab() && childWorkspaceUi.tabType() == TuningLib::TuningUiItem::TabType::FiltersSwitch)
	{
		// We have to create Presets Switch page
		//
		SwitchFiltersPage* swp = new SwitchFiltersPage(m_configController,
													   m_tuningSignalManager,
													   m_appSignalLists,
													   m_userManager,
													   m_tuningConnection,
													   childWorkspaceUi,
													   m_tuningCounters,
													   this);
		m_switchPresetPages.push_back(swp);
		return swp;
	}
	else
	{
		// We have to create tuning page
		//
		auto it = m_tuningPagesMap.find(childWorkspaceUi.uuid());
		if (it == m_tuningPagesMap.end())
		{
			TuningPage* tp = new TuningPage(m_configController,
											m_tuningSignalManager,
											m_tuningUi,
											m_appSignalLists,
											m_userManager,
											m_tuningConnection,
											m_treeListUuid,
											childWorkspaceUi,
											m_tuningCounters,
											this);

			m_tuningPagesMap[childWorkspaceUi.uuid()] = tp;

			connect(this, &TuningWorkspace::treeFilterChanged, tp, &TuningPage::slot_treeFilterChanged);

			if (childWorkspaceUi.isButton() == true)
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


void TuningWorkspace::updateTabsButtonsCounters()
{
	// Tab counters

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		int tabFiltersCount = static_cast<int>(m_tabsUiItems.size());

		if (m_tab->count() != tabFiltersCount)
		{
			assert(m_tab->count() == tabFiltersCount);
			return;
		}

		for (int ti = 0; ti < tabFiltersCount; ti++)
		{
			const auto& tabUi = m_tabsUiItems[ti];
			Q_ASSERT (tabUi);
			Q_ASSERT (tabUi->isTab());

			if (tabUi->hasDiscreteCounter() == false)
			{
				continue;
			}

			int discreteCount = m_tuningCounters.counters(tabUi->filters()).discreteCounter;

			QString newCaption;
			if (discreteCount == 0)
			{
				newCaption = tabUi->caption();
			}
			else
			{
				newCaption = QString(" %1 [%2] ").arg(tabUi->caption()).arg(discreteCount);
			}

			if (m_tab->tabText(ti) != newCaption)
			{
				m_tab->setTabText(ti, newCaption);
			}

			// Tab text color

			if (tabUi->useColors() == true)
			{
				QColor tabTextColor;

				if (discreteCount > 0)
				{
					tabTextColor = tabUi->textAlertedColor();
				}
				else
				{
					tabTextColor = tabUi->textColor();
				}

				if (m_tab->tabBar()->tabTextColor(ti) != tabTextColor)
				{
					m_tab->tabBar()->setTabTextColor(ti, tabTextColor);
				}
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

		if (button->hasDiscreteCounter() == false) 
		{
			continue;
		}

		int discreteCount = m_tuningCounters.counters(button->filters()).discreteCounter;
		if (discreteCount != button->counter())
		{
			button->update(discreteCount);
		}
	}
}

bool TuningWorkspace::eventFilter(QObject *object, QEvent *event)
{
	bool navigationKey = false;

	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Up ||
				keyEvent->key() == Qt::Key_Down ||
				keyEvent->key() == Qt::Key_Left ||
				keyEvent->key() == Qt::Key_Right ||
				keyEvent->key() == Qt::Key_PageUp ||
				keyEvent->key() == Qt::Key_PageDown ||
				keyEvent->key() == Qt::Key_Home ||
				keyEvent->key() == Qt::Key_End ||
				keyEvent->key() == Qt::Key_Space ||
				keyEvent->key() == Qt::Key_Enter ||
				keyEvent->key() == Qt::Key_Return)
			navigationKey = true;
	}

	if (m_tab != nullptr && object == m_tab->tabBar() &&
		(event->type() == QEvent::MouseButtonPress ||
		 event->type() == QEvent::MouseButtonRelease ||
		 event->type() == QEvent::MouseButtonDblClick ||
		 navigationKey == true))
	{
		if (askForSavePendingChanges() == false)
		{
			return true;
		}
	}

	if (m_treeLayoutWidget != nullptr &&
			(object == m_treeLayoutWidget->treeWidget() || object == m_treeLayoutWidget->treeWidget()->viewport()) &&
		(event->type() == QEvent::MouseButtonPress ||
		 event->type() == QEvent::MouseButtonRelease ||
		 event->type() == QEvent::MouseButtonDblClick ||
		 navigationKey == true))
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
			 event->type() == QEvent::MouseButtonDblClick ||
			 navigationKey == true))
		{
			if (askForSavePendingChanges() == false)
			{
				return true;
			}
		}
	}

	return QWidget::eventFilter(object, event);
}

void TuningWorkspace::slot_parentWorkspaceTreeFilterChanged(const QUuid& filterUuid)
{
	// This slot is called only for nested workspaces!
	//
	emit treeFilterChanged(filterUuid);
}

void TuningWorkspace::slot_filterButtonClicked(const QUuid& uiItemUuid)
{
	// Remember the tab index for current button

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		int index = m_tab->currentIndex();
		m_activeTabPagesMap[uiItemUuid] = index;
	}

	// Set the new filter
	m_currentButtonUi = m_tuningUi.get(uiItemUuid);
	Q_ASSERT(m_currentButtonUi);

	// Update tab

	createTabPages();

	emit buttonFilterSelectionChanged(uiItemUuid);
}
