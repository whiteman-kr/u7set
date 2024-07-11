#include "SwitchFiltersPage.h"

#include <ClientLib/TuningConnection.h>

#include "Main.h"
#include "MainWindow.h"
#include "Settings.h"
#include "SwitchFiltersPageOptions.h"

FilterPushButton::FilterPushButton(const QString& filterId, const QString& caption, QWidget* parent):
	QPushButton(caption, parent),
	m_filterId(filterId)
{
}

QString FilterPushButton::filterId() const 
{
	return m_filterId;
}

void FilterPushButton::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	emit clicked(m_filterId);
}

//
//
//

QString SwitchFiltersPage::tag_FilterButton = "FilterButtons";
QString SwitchFiltersPage::tag_FilterSwitch = "FilterSwitches";

SwitchFiltersPage::SwitchFiltersPage(TuningConfigController& configController,
					  ClientLib::TuningSignalManager& tuningSignalManager,
					  AppSignalLists::AppSignalListSet& appSignalLists,
					  ClientLib::TuningUserManager& userManager,
					  ClientLib::TuningConnection& tuningConnection,
					  const TuningLib::TuningUiItem& uiItem,
					  const TuningCountersManager& tuningCounters,
					  QWidget* parent) :
	QWidget(parent),
	m_configController(configController),
    m_tuningSignalManager(tuningSignalManager),
	m_appSignalLists(appSignalLists),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection),
    m_workspaceUi(uiItem),
	m_tuningCounters(tuningCounters)
{
	m_mainLayout = new QVBoxLayout(this);

	// Determine button and list colors from tags
	// Tags format: ...;AlertBackColor=#f00000;AlertTextColor=#c00000;GrayedBackColor=#d0d000;GrayedTextColor=#000000;...

	QStringList tags = m_workspaceUi.tagsList();
	for (const QString& tag : tags)
	{
		QStringList pair = tag.split('=');
		if (pair.size() != 2)
		{
			continue;
		}

		QColor color = QColor(pair[1]);
		if (color.isValid() == false)
		{
			continue;
		}

		if (pair[0].startsWith("AlertBackColor"))
		{
			m_alertBackColor = color;
			continue;
		}
		if (pair[0].startsWith("AlertTextColor"))
		{
			m_alertTextColor = color;
			continue;
		}
		if (pair[0].startsWith("GrayedBackColor"))
		{
			m_partialBackColor = color;
			continue;
		}
		if (pair[0].startsWith("GrayedTextColor"))
		{
			m_partialTextColor = color;
			continue;
		}
	}

	// Background Color

	Q_ASSERT(m_workspaceUi.isTab() == true);	// This must be tab!

	if (m_workspaceUi.useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Window, m_workspaceUi.backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
	}
	else
	{
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
	}

	createControls();

	connect(theApp.mainWindow(), &MainWindow::timerTick500, this, &SwitchFiltersPage::onTimer);

	setLayout(m_mainLayout);
}

void SwitchFiltersPage::createControls()
{
	m_buttonFilters.clear();
	m_listFilters.clear();

	// Delete all controls
	//

	m_filterButtons.clear();

	if (m_filterButtonsWidget != nullptr)
	{
		delete m_filterButtonsWidget;
		m_filterButtonsWidget = nullptr;
	}

	if (m_filterTableWidget != nullptr)
	{
		delete m_filterTableWidget;
		m_filterTableWidget = nullptr;
	}

	if (m_vSplitter != nullptr)
	{
		delete m_vSplitter;
		m_vSplitter = nullptr;
	}

	if (m_promptLabel != nullptr)
	{
		delete m_promptLabel;
		m_promptLabel = nullptr;
	}

	// Controls in this form only used in Multiple-LM control mode
	//
	for (const SoftwareEndpoint::TuningService& tsc : m_configController.configuration().clientSettings.tuningServices)
	{
		if (tsc.singleLmControl == true)
		{
			m_promptLabel = new QLabel(tr("This tab should be used only with Multiple LM Control Mode of TuningServices. Disable \"SingleLMControl\" mode for all services."));
			m_promptLabel->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
			m_mainLayout->addWidget(m_promptLabel);
			return;
		}
	}

	// Fill filters
	//
	createFiltersList();

	// Apply Button
	//
	if (m_configController.configuration().clientSettings.applyMode == TuningClientSettings::ApplyMode::Manual &&
		(m_buttonFilters.empty() == false || m_listFilters.empty() == false))
	{
		m_applyButton = new QPushButton(tr("Apply"), this);
		connect(m_applyButton, &QPushButton::clicked, this, &SwitchFiltersPage::onApply);
	}

	// Buttons
	//
	if (m_buttonFilters.empty() == false)
	{
		m_filterButtonsWidget = new QWidget(this);

		// Control Layout

		QHBoxLayout* scrollControlsLayout = new QHBoxLayout();

		const int controlHeight = 25;

		QPushButton* b = new QPushButton(this);
		QPixmap pixmap(":/Images/Images/ButtonSettings.png");
		b->setIcon(QIcon(pixmap));
		b->setIconSize(QSize(static_cast<int>(controlHeight * 0.8), static_cast<int>(controlHeight * 0.8)));
		b->setFixedSize(controlHeight, controlHeight);
		connect(b, &QPushButton::clicked, this, &SwitchFiltersPage::onOptions);
		scrollControlsLayout->addWidget(b);

		scrollControlsLayout->addStretch();

		m_prevButton = new QPushButton("<", this);
		scrollControlsLayout->addWidget(m_prevButton);
		connect(m_prevButton, &QPushButton::clicked, this, &SwitchFiltersPage::onPrev);
		m_prevButton->setFixedHeight(controlHeight);

		m_nextButton = new QPushButton(">", this);
		scrollControlsLayout->addWidget(m_nextButton);
		connect(m_nextButton, &QPushButton::clicked, this, &SwitchFiltersPage::onNext);
		m_nextButton->setFixedHeight(controlHeight);

		scrollControlsLayout->addStretch();

		scrollControlsLayout->addSpacerItem(new QSpacerItem(controlHeight, 0));


		// Buttons layout

		m_buttonsLayout = new QGridLayout();

		m_buttonStartIndex = 0;

		createButtons();

		const auto& s = TuningClientAppSettings::instance().user();

		if (m_prevButton != nullptr)
		{
			m_prevButton->setVisible(m_buttonFilters.size() > s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount);
		}
		if (m_nextButton != nullptr)
		{
			m_nextButton->setVisible(m_buttonFilters.size() > s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount);
		}

		// Main layout

		QVBoxLayout* topLayout = new QVBoxLayout(m_filterButtonsWidget);
		topLayout->setContentsMargins(0, 0, 0, 15);
		topLayout->addStretch();
		topLayout->addLayout(m_buttonsLayout);
		topLayout->addStretch();
		topLayout->addLayout(scrollControlsLayout);

		// Apply button at the bottom

		if (m_applyButton != nullptr && m_listFilters.empty() == true)
		{
			scrollControlsLayout->addWidget(m_applyButton);
		}
	}

	// Table
	//

	if (m_listFilters.empty() == false)
	{
		m_filterTableWidget = new QWidget(this);

		QVBoxLayout* bottomLayout = new QVBoxLayout(m_filterTableWidget);
		bottomLayout->setContentsMargins(0, 15, 0, 0);

		m_filterTable = new FilterTableWidget(this);

		bottomLayout->addWidget(m_filterTable);

		// Apply button at the bottom

		if (m_applyButton != nullptr)
		{
			QHBoxLayout* al = new QHBoxLayout();
			al->addStretch();
			al->addWidget(m_applyButton);
			bottomLayout->addLayout(al);
		}

		createListItems();
	}

	// Splitter
	//
	if (m_filterButtonsWidget != nullptr && m_filterTableWidget != nullptr)
	{
		auto& s = TuningClientAppSettings::instance().user();

		m_vSplitter	= new QSplitter(Qt::Vertical);
		m_vSplitter->addWidget(m_filterButtonsWidget);
		m_vSplitter->addWidget(m_filterTableWidget);
		connect(m_vSplitter, &QSplitter::splitterMoved, [this, &s](int pos, int index)
		{
			Q_UNUSED(pos);
			Q_UNUSED(index);
			s.m_switchPresetsPageSplitterPosition = m_vSplitter->saveState();
		});
		if (s.m_switchPresetsPageSplitterPosition.isEmpty() == false)
		{
			m_vSplitter->restoreState(s.m_switchPresetsPageSplitterPosition);
		}
		m_mainLayout->addWidget(m_vSplitter);
	}
	else
	{
		if (m_filterButtonsWidget != nullptr)
		{
			m_mainLayout->addWidget(m_filterButtonsWidget);
		}
		else
		{
			if (m_filterTableWidget != nullptr)
			{
				m_mainLayout->addWidget(m_filterTableWidget);
			}
			else
			{
				m_promptLabel = new QLabel(tr("No filters to display.\nCreate filters that contain one of the following tags: '%1' or '%2'.").arg(tag_FilterButton).arg(tag_FilterSwitch));
				m_promptLabel->setAlignment(Qt::AlignHCenter | Qt::AlignCenter);

				m_mainLayout->addWidget(m_promptLabel);
			}
		}
	}
}

void SwitchFiltersPage::createFiltersList()
{
	for (int i = 0; i < m_appSignalLists.count(); i++)
	{
		AppSignalLists::AppSignalList* list = m_appSignalLists.get(i).get();

		auto hasAnyTag = [](const QStringList& container, const QStringList& tags) -> bool
		{
			for (const QString& t : tags)
			{
				if (container.contains(t) == true)
				{
					return true;
				}
			}
			return false;
		};

		if (m_workspaceUi.tagsList().isEmpty() == true || hasAnyTag(m_workspaceUi.tagsList(), list->userTagsList()) == true)
		{
			if (list->userTagsList().contains(tag_FilterButton))
			{
				m_buttonFilters.push_back(list);
			}

			if (list->userTagsList().contains(tag_FilterSwitch))
			{
				m_listFilters.push_back(list);
			}
		}
	}
}

void SwitchFiltersPage::createButtons()
{
	if (m_buttonsLayout == nullptr)
	{
		Q_ASSERT(m_buttonsLayout);
		return;
	}

	// Take all buttons
	//
	int count = m_buttonsLayout->count();
	for (int i = 0; i < count; i++)
	{
		QLayoutItem* item = m_buttonsLayout->takeAt(0);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}
		if (item->widget() == nullptr)
		{
			Q_ASSERT(item->widget());
			return;
		}

		delete item->widget();
		delete item;
	}

	// Create new buttons
	//

	m_filterButtons.clear();

	int row = 0;
	int col = 0;

	int buttonsCount = static_cast<int>(m_buttonFilters.size());
	for (int i = m_buttonStartIndex; i < buttonsCount; i++)
	{
		AppSignalLists::AppSignalList* list = m_buttonFilters[i];
		if (list == nullptr)
		{
			Q_ASSERT(list);
			return;
		}

		FilterPushButton* b = new FilterPushButton(list->id(), list->caption(), this);

		m_filterButtons.push_back(b);

		connect(b, &FilterPushButton::clicked, this, &SwitchFiltersPage::onFilterButtonClicked);

		const auto& s = TuningClientAppSettings::instance().user();

		b->setFixedSize(s.m_switchPresetsPageButtonsWidth, s.m_switchPresetsPageButtonsHeight);

		m_buttonsLayout->addWidget(b, row, col);

		if (col++ >= TuningClientAppSettings::instance().user().m_switchPresetsPageColCount - 1)
		{
			col = 0;

			if (row++ >= TuningClientAppSettings::instance().user().m_switchPresetsPageRowCount - 1)
			{
				break;
			}
		}
	}
}

void SwitchFiltersPage::createListItems()
{
	if (m_filterTable == nullptr)
	{
		Q_ASSERT(m_filterTable);
		return;
	}

	m_filterTable->setColumnCount(static_cast<int>(Columns::ColumnCount));

	QStringList labels;
	labels << tr("State");
	labels << tr("Caption");
	labels << tr("Counter");
	m_filterTable->setHorizontalHeaderLabels(labels);

	m_filterTable->setRowCount(static_cast<int>(m_listFilters.size()));

	m_filterTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_filterTable->setSelectionMode(QAbstractItemView::SingleSelection);

	m_filterTable->verticalHeader()->setVisible(false);

	m_filterTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_filterTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_filterTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);

	for (int i = 0; i < static_cast<int>(m_listFilters.size()); i++)
	{
		AppSignalLists::AppSignalList* list = m_listFilters[i];

		for (int c = 0; c < static_cast<int>(Columns::ColumnCount); c++)
		{
			QTableWidgetItem* item = new QTableWidgetItem();
			if (item == nullptr)
			{
				Q_ASSERT(item);
				return;
			}

			m_filterTable->setItem(i, c, item);

			FilterCheckBox* check = nullptr;

			switch (c)
			{
			case static_cast<int>(Columns::State):
				{
					check = new FilterCheckBox(tr("OFF"), this);
					connect(check, &FilterCheckBox::pressed, this, &SwitchFiltersPage::onFilterTablePressed);
					check->setStyleSheet("FilterCheckBox::indicator{width:25px;height:25px;}");
					m_filterTable->setCellWidget(i, static_cast<int>(Columns::State), check);
					item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				}
				break;
			case static_cast<int>(Columns::Caption):
				{
					item->setText(list->caption());
					item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				}
				break;
			case static_cast<int>(Columns::Counter):
				{
					item->setText("0/0");
					item->setFlags(Qt::NoItemFlags);
				}
				break;
			}
		}
	}

	connect(m_filterTable, &FilterTableWidget::spacePressed, this, &SwitchFiltersPage::onFilterTablePressed);
}

bool SwitchFiltersPage::changeFilterSignals(const QString& filterId)
{
	if (m_userManager.login(this) == false)
	{
		return false;
	}

	// Check if all signals are enabled to write
	//
	int discreteCount = 0;
	int writingEnabledCount = 0;
	int alertedCount = 0;

	auto buttonList = m_appSignalLists.get(filterId);
	if (buttonList == nullptr) 
	{
		Q_ASSERT(buttonList);
		return false;
	}
	countDiscretes(*buttonList, discreteCount, writingEnabledCount, alertedCount);

	/*if (discreteCount == 0 || discreteCount != writingEnabledCount)
	{
		return false;
	}*/

	// Output warning message
	//
	int newValue = 0;

	if (alertedCount == 0)
	{
		if (QMessageBox::warning(this, qAppName(), tr("Are you sure you want to switch ON  signals of the filter '%1'?").arg(buttonList->caption()),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}

		newValue = 1;
	}
	else
	{
		if (alertedCount == discreteCount)
		{
			if (QMessageBox::warning(this, qAppName(), tr("Are you sure you want to switch OFF signals of the filter '%1'?").arg(buttonList->caption()),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) != QMessageBox::Yes)
			{
				return false;
			}

			newValue = 0;
		}
		else
		{
			QMessageBox msgBox{this};
			msgBox.setText(tr("Signals of the filter '%1' have different values. Please select the following action:").arg(buttonList->caption()));
			QPushButton* saveTo0Button = msgBox.addButton(tr("Set All to 0"), QMessageBox::ActionRole);
			QPushButton* saveTo1Button = msgBox.addButton(tr("Set All to 1"), QMessageBox::ActionRole);
			/*QPushButton* saveTo2Button = */msgBox.addButton(tr("Set All to 2"), QMessageBox::ActionRole);

			if (msgBox.clickedButton() == saveTo0Button)
			{
				newValue = 0;
			}
			else
			{
				if (msgBox.clickedButton() == saveTo1Button)
				{
					newValue = 1;
				}
				else
				{
					return false;
				}
			}
		}
	}

	// Get filter signals and their hashes
	//
	std::set<Hash> signalsHashes = buttonList->tuningListHashesCache();

	// Write new values
	//
	std::vector<ClientLib::TuningWriteCommand> commands;

	for (Hash hash : signalsHashes)
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
		if (ok == false)
		{
			continue;
		}

		if (asp.tuningType() != TuningValueType::Discrete)
		{
			continue;
		}

		TuningValue tv;
		tv.setType(TuningValueType::Discrete);
		tv.setDiscreteValue(newValue);

		ClientLib::TuningWriteCommand c(hash, tv);
		commands.push_back(c);
	}

	if (commands.empty() == false)
	{
		m_tuningConnection.writeTuningSignals(commands);
	}
	
	return true;
}

void SwitchFiltersPage::apply()
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want apply the changes?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	// Get SOR counters

	if (m_tuningCounters.totalCounters().sorCounter > 0)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!!!\n\nSOR Signal(s) are set in logic modules!\n\nIf you apply these changes, module can run into RUN SAFE STATE.\n\nAre you sure you STILL WANT TO APPLY the changes?"),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	m_tuningConnection.applyTuningSignals();
	return;
}

void SwitchFiltersPage::countDiscretes(const AppSignalLists::AppSignalList& list, int& total, int& writingEnabled, int& alerted)
{
	total = 0;
	writingEnabled = 0;
	alerted = 0;

	for (const auto& hash : list.tuningListHashesCache())
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager.signalParam(hash, &ok);
		if (ok == false)
		{
			continue;
		}

		if (asp.tuningType() != TuningValueType::Discrete)
		{
			continue;
		}
		
		total++;

		const TuningSignalState state = m_tuningSignalManager.state(hash, &ok);
		if (ok == true && state.valid() == true)
		{
			if (state.writingIsEnabled() == true)
			{
				writingEnabled++;
			}

			if (state.value().discreteValue() != 0) 
			{
				alerted++;
			}
		}
	}
}

void SwitchFiltersPage::showEvent(QShowEvent *ev)
{
	Q_UNUSED(ev);

	onTimer();
}

void SwitchFiltersPage::onOptions()
{
	auto& s = TuningClientAppSettings::instance().user();

	SwitchFiltersPageOptions d(this,
							   s.m_switchPresetsPageColCount,
							   s.m_switchPresetsPageRowCount,
							   s.m_switchPresetsPageButtonsWidth,
							   s.m_switchPresetsPageButtonsHeight);
	if (d.exec() == QDialog::Accepted)
	{
		s.m_switchPresetsPageColCount = d.buttonsColCount();
		s.m_switchPresetsPageRowCount = d.buttonsRowCount();
		s.m_switchPresetsPageButtonsWidth = d.buttonsWidth();
		s.m_switchPresetsPageButtonsHeight = d.buttonsHeight();

		m_buttonStartIndex = 0;

		createButtons();

		m_prevButton->setVisible(m_buttonFilters.size() > s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount);
		m_nextButton->setVisible(m_buttonFilters.size() > s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount);

		onTimer();
	}
}

void SwitchFiltersPage::onPrev()
{
	auto& s = TuningClientAppSettings::instance().user();

	if (m_buttonStartIndex >= s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount)
	{
		m_buttonStartIndex -= s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount;

		createButtons();

		onTimer();
	}
}
void SwitchFiltersPage::onNext()
{
	auto& s = TuningClientAppSettings::instance().user();

	if (m_buttonStartIndex < m_buttonFilters.size() - s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount)
	{
		m_buttonStartIndex += s.m_switchPresetsPageColCount * s.m_switchPresetsPageRowCount;

		createButtons();

		onTimer();
	}
}

void SwitchFiltersPage::onApply()
{
	apply();
}

void SwitchFiltersPage::onTimer()
{
	if  (isVisible() == false)
	{
		return;
	}

	// Buttons

	int count = static_cast<int>(m_filterButtons.size());
	for (int i = 0; i < count; i++)
	{
		FilterPushButton* b = m_filterButtons[i];
		if (b == nullptr)
		{
			Q_ASSERT(b);
			return;
		}

		AppSignalLists::AppSignalList* buttonList = m_appSignalLists.get(b->filterId()).get();
		if (buttonList == nullptr)
		{
			Q_ASSERT(buttonList);
			return;
		}

		int discreteCount = 0;
		int writingEnabledCount = 0;
		int alertedCount = 0;
		countDiscretes(*buttonList, discreteCount, writingEnabledCount, alertedCount);

		QString text = tr("%1\n\n%2 / %3").arg(buttonList->caption()).arg(alertedCount).arg(discreteCount);

		if (b->text() != text)
		{
			b->setText(text);
		}

		// Enable/Disable

		bool buttonEnabled = discreteCount != 0 && writingEnabledCount == discreteCount;
		if (b->isEnabled() != buttonEnabled)
		{
			b->setEnabled(buttonEnabled);
		}

		// Color

		if (alertedCount == 0)
		{
			b->setStyleSheet(QString());
			b->setDown(false);
		}
		else
		{
			if (alertedCount == discreteCount)
			{
				QColor textColor = b->isEnabled() ? m_alertTextColor : QColor(Qt::lightGray);

				QString s = tr("QPushButton { background-color: %1; color: %2 }").arg(m_alertBackColor.name()).arg(textColor.name());
				if (b->styleSheet() != s)
				{
					b->setStyleSheet(s);
				}
				b->setDown(true);
			}
			else
			{
				QColor textColor = b->isEnabled() ? m_partialTextColor : QColor(Qt::lightGray);

				QString s = tr("QPushButton { background-color: %1; color: %2 }").arg(m_partialBackColor.name()).arg(textColor.name());
				if (b->styleSheet() != s)
				{
					b->setStyleSheet(s);
				}
				b->setDown(false);
			}
		}

	}

	// List

	count = static_cast<int>(m_listFilters.size());
	for (int i = 0; i < count; i++)
	{
		Q_ASSERT(m_filterTable);

		int discreteCount = 0;
		int writingEnabledCount = 0;
		int alertedCount = 0;
		countDiscretes(*m_listFilters[i], discreteCount, writingEnabledCount, alertedCount);

		QTableWidgetItem* itemCheck = m_filterTable->item(i, static_cast<int>(Columns::State));
		if (itemCheck == nullptr)
		{
			Q_ASSERT(itemCheck);
			return;
		}

		QTableWidgetItem* itemCaption = m_filterTable->item(i, static_cast<int>(Columns::Caption));
		if (itemCaption == nullptr)
		{
			Q_ASSERT(itemCaption);
			return;
		}

		QTableWidgetItem* itemCounter = m_filterTable->item(i, static_cast<int>(Columns::Counter));
		if (itemCounter == nullptr)
		{
			Q_ASSERT(itemCounter);
			return;
		}

		//

		QString checkText;
		QString counterText = tr("%1 / %2").arg(alertedCount).arg(discreteCount);

		QColor backColor;
		QColor textColor;
		Qt::CheckState checkState;

		if (alertedCount == 0)
		{
			backColor = Qt::white;
			textColor = Qt::black;
			checkState = Qt::Unchecked;
			checkText = tr("OFF");
		}
		else
		{
			if (alertedCount == discreteCount)
			{
				backColor = m_alertBackColor;
				textColor = m_alertTextColor;
				checkState = Qt::Checked;
				checkText = tr("ON");
			}
			else
			{
				backColor = m_partialBackColor;
				textColor = m_partialTextColor;
				checkState = Qt::PartiallyChecked;
				checkText = tr("PARTIAL");
			}
		}

		if (itemCounter->text() != counterText)
		{
			itemCounter->setText(counterText);
		}

		if (itemCaption->background() != backColor)
		{
			itemCaption->setBackground(backColor);
		}

		if (itemCaption->foreground() != textColor)
		{
			itemCaption->setForeground(textColor);
		}

		if (itemCounter->background() != backColor)
		{
			itemCounter->setBackground(backColor);
		}

		if (itemCounter->foreground() != textColor)
		{
			itemCounter->setForeground(textColor);
		}

		FilterCheckBox* checkBox = dynamic_cast<FilterCheckBox*>(m_filterTable->cellWidget(i, static_cast<int>(Columns::State)));
		if (checkBox == nullptr)
		{
			Q_ASSERT(checkBox);
			return;
		}

		if (checkBox->checkState() != checkState)
		{
			checkBox->setCheckState(checkState);
		}

		if (checkBox->text() != checkText)
		{
			checkBox->setText(checkText);
		}

		bool buttonEnabled = discreteCount != 0 && writingEnabledCount == discreteCount;
		if (checkBox->isEnabled() != buttonEnabled)
		{
			checkBox->setEnabled(buttonEnabled);
		}
	}
}

void SwitchFiltersPage::onFilterButtonClicked(const QString& filterId)
{
	changeFilterSignals(filterId);
}

void SwitchFiltersPage::onFilterTablePressed()
{
	int row = m_filterTable->currentRow();
	if (row < 0 || row >= m_listFilters.size())
	{
		Q_ASSERT(false);
		return;
	}

	AppSignalLists::AppSignalList* list = m_listFilters[row];
	if (list == nullptr)
	{
		Q_ASSERT(list);
		return;
	}

	changeFilterSignals(list->id());
}
