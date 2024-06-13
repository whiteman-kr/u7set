#include "TuningWorkspace.h"
#include "Main.h"
#include "MainWindow.h"
#include "Settings.h"
#include <ClientLib/TuningConnection.h>

using namespace TuningFilters;
//
// FilterButton
//

FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, bool check, QWidget* parent)
	:QPushButton(filter->caption(), parent)
{
	Q_ASSERT(filter);

	m_filter = filter;

	setCheckable(true);

	if (check == true)
	{
		setChecked(true);
	}

	setMinimumSize(100, 25);

	update(0);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
{
	return m_filter;
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
		newCaption = m_filter->caption();
	}
	else
	{
		newCaption = QString(" %1 [%2] ").arg(m_filter->caption()).arg(discreteCounter);
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

	if (m_filter->useColors() == true)
	{
		if (counter() != 0 && m_filter->backAlertedColor() != m_filter->textAlertedColor())
		{
			// Alerted state

			backColor = m_filter->backAlertedColor();
			textColor = m_filter->textAlertedColor();

			backSelectedColor = backColor;
			textSelectedColor = textColor;
		}
		else
		{
			if (m_filter->backColor() != m_filter->textColor())
			{
				backColor = m_filter->backColor();
				textColor = m_filter->textColor();
			}

			if (m_filter->backSelectedColor() != m_filter->textSelectedColor())
			{
				backSelectedColor = m_filter->backSelectedColor();
				textSelectedColor = m_filter->textSelectedColor();
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
		emit filterButtonClicked(m_filter);
	}

}

//
// TuningWorkspace
//

int TuningWorkspace::m_instanceCounter = 0;

TuningWorkspace::TuningWorkspace(TuningConfigController& configController,
								 ClientLib::TuningSignalManager& tuningSignalManager,
								 TuningClientFilterStorage& tuningFilterStorage,
								 ClientLib::TuningUserManager& userManager,
								 ClientLib::TuningConnection& tuningConnection,
								 std::shared_ptr<TuningFilter> treeFilter,
								 std::shared_ptr<TuningFilter> workspaceFilter,
								 bool hasFilterTree,
								 QWidget* parent) :
	m_configController(configController),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningFilterStorage(tuningFilterStorage),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection),
	m_treeFilter(treeFilter),
	m_workspaceFilter(workspaceFilter),
	QWidget(parent)
{
	//qDebug() << "TuningWorkspace::TuningWorkspace m_instanceCounter = " << m_instanceCounter;
	m_instanceCounter++;

	//assert(m_treeFilter); // Can be nullptr
	assert(m_workspaceFilter);

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
		m_treeLayoutWidget = new TreeFilterWidget(m_configController,
												  m_tuningFilterStorage,
												  m_userManager,
												  m_tuningConnection,
												  this);
		m_treeLayoutWidget->fillFiltersTree(m_workspaceFilter);

		connect(m_treeLayoutWidget, &TreeFilterWidget::treeFilterSelectionChanged, [this](std::shared_ptr<TuningFilter> filter){
			m_treeFilter = filter;
			emit treeFilterChanged(filter);
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

	if (workspaceFilter->useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Window, workspaceFilter->backColor());
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
		m_treeLayoutWidget->fillFiltersTree(m_workspaceFilter);

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

		swp->createControls(m_workspaceFilter);
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
	if (m_workspaceFilter == nullptr)
	{
		assert(m_workspaceFilter);
		return;
	}

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

		FilterButton* button = new FilterButton(f, firstButton);
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
		QButtonGroup* filterButtonGroup = new QButtonGroup(this);

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
	if (m_workspaceFilter == nullptr)
	{
		assert(m_workspaceFilter);
		return;
	}

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

		for (const auto& t : tuningPages)
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
			std::shared_ptr<TuningFilter> singlePageFilter = nullptr;

			if (m_currentbuttonFilter != nullptr)
			{
				// If a button is pressed - set button filter as page filter

				singlePageFilter = m_currentbuttonFilter;
			}
			else
			{
				// Otherwise set workspace filter to page filter

				singlePageFilter = std::make_shared<TuningFilter>();

				singlePageFilter->setCaption(m_workspaceFilter->caption());

				// Copy signals' hashes from parent filter to single page's filter

				singlePageFilter->setSignalsHashes(m_workspaceFilter->signalsHashes());
			}

			QWidget* tp = createTuningPageOrWorkspace(singlePageFilter);

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
			TuningWorkspace* tw = new TuningWorkspace(m_configController,
													  m_tuningSignalManager,
													  m_tuningFilterStorage,
													  m_userManager,
													  m_tuningConnection,
													  m_treeFilter,
													  childWorkspaceFilter,
													  false/*hasFilterTree*/,
													  this/*parent*/);

			m_tuningWorkspacesMap[childWorkspaceFilterId] = tw;

			connect(this, &TuningWorkspace::treeFilterChanged, tw, &TuningWorkspace::slot_parentWorkspaceTreeFilterChanged);

			return tw;
		}
		else
		{
			return it->second;
		}
	}
	else
	{
		if (childWorkspaceFilter->isTab() && childWorkspaceFilter->tabType() == TuningFilter::TabType::FiltersSwitch )
		{
			// We have to create Presets Switch page
			//
			SwitchFiltersPage* swp = new SwitchFiltersPage(m_configController, m_tuningSignalManager, m_tuningFilterStorage, m_userManager, m_tuningConnection, childWorkspaceFilter, this);
			m_switchPresetPages.push_back(swp);
			return swp;
		}
		else
		{
			// We have to create tuning page
			//
			auto it = m_tuningPagesMap.find(childWorkspaceFilterId);
			if (it == m_tuningPagesMap.end())
			{
				TuningPage* tp = new TuningPage(m_configController, m_tuningSignalManager, m_tuningFilterStorage, m_userManager, m_tuningConnection, m_treeFilter, childWorkspaceFilter, this);

				m_tuningPagesMap[childWorkspaceFilterId] = tp;

				connect(this, &TuningWorkspace::treeFilterChanged, tp, &TuningPage::slot_treeFilterChanged);

				if (childWorkspaceFilter->isButton() == true)
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
}

void TuningWorkspace::updateTabsButtonsCounters()
{
	// Tab counters

	if (m_tab != nullptr && m_tab->isVisible() == true)
	{
		int tabFiltersCount = static_cast<int>(m_tabsFilters.size());

		if (m_tab->count() != tabFiltersCount)
		{
			//qDebug() << m_tab->count();
			//qDebug() << static_cast<int>(m_tabsFilters.size());
			assert(m_tab->count() == tabFiltersCount);
		}

		for (int ti = 0; ti < tabFiltersCount; ti++)
		{
			std::shared_ptr<TuningFilter> f = m_tabsFilters[ti];

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
				newCaption = QString(" %1 [%2] ").arg(f->caption()).arg(discreteCount);
			}

			if (m_tab->tabText(ti) != newCaption)
			{
				m_tab->setTabText(ti, newCaption);
			}

			// Tab text color

			if (f->useColors() == true)
			{
				QColor tabTextColor;

				if (discreteCount > 0)
				{
					tabTextColor = f->textAlertedColor();
				}
				else
				{
					tabTextColor = f->textColor();
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

void TuningWorkspace::slot_parentWorkspaceTreeFilterChanged(std::shared_ptr<TuningFilter> filter)
{
	// This slot is called only for nested workspaces!
	//
	m_treeFilter = filter;
	emit treeFilterChanged(m_treeFilter);
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

	emit buttonFilterSelectionChanged(filter);
}
