#include "SwitchFiltersPage.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QSplitter>
#include <QPushButton>
#include <QMessageBox>

#include "SwitchFiltersPageOptions.h"
#include "MainWindow.h"
#include "Settings.h"

FilterPushButton::FilterPushButton(const QString& caption, std::shared_ptr<TuningFilter> filter, QWidget* parent):
	QPushButton(caption, parent),
	m_filter(filter)
{
}

std::shared_ptr<TuningFilter> FilterPushButton::filter()
{
	return m_filter;
}

void FilterPushButton::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);

	emit clicked(m_filter);

	return;
}

//
//
//

QString SwitchFiltersPage::tag_FilterButton = "FilterButtons";
QString SwitchFiltersPage::tag_FilterSwitch = "FilterSwitches";

SwitchFiltersPage::SwitchFiltersPage(std::shared_ptr<TuningFilter> workspaceFilter,
									 TuningSignalManager* tuningSignalManager,
									 TuningClientTcpClient* tuningTcpClient, TuningFilterStorage* tuningFilterStorage,
									 QWidget* parent) :
	QWidget(parent),
	m_workspaceFilter(workspaceFilter),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient),
	m_tuningFilterStorage(tuningFilterStorage)
{
	m_mainLayout = new QVBoxLayout(this);

	if (m_workspaceFilter == nullptr)
	{
		Q_ASSERT(m_workspaceFilter);
		return;
	}

	updateFilters(m_tuningFilterStorage->root());

	connect(theMainWindow, &MainWindow::timerTick500, this, &SwitchFiltersPage::slot_timerTick500);

	// Determine button and list colors from tags
	// Tags format: ...;AlertBackColor=#f00000;AlertTextColor=#c00000;GrayedBackColor=#d0d000;GrayedTextColor=#000000;...

	QStringList tags = workspaceFilter->tagsList();
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

	if (workspaceFilter->isTab() == true && workspaceFilter->useColors() == true)
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Background, workspaceFilter->backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
	}
	else
	{
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
	}

	setLayout(m_mainLayout);
}


SwitchFiltersPage::~SwitchFiltersPage()
{
}

void SwitchFiltersPage::updateFilters(std::shared_ptr<TuningFilter> root)
{
	m_buttonFilters.clear();
	m_listFilters.clear();

	createFiltersList(root);

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

	// Create new controls
	//

	// Apply Button
	//
	if (theConfigSettings.autoApply == false &&
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

		if (m_prevButton != nullptr)
		{
			m_prevButton->setVisible(m_buttonFilters.size() > theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount);
		}
		if (m_nextButton != nullptr)
		{
			m_nextButton->setVisible(m_buttonFilters.size() > theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount);
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
		m_vSplitter	= new QSplitter(Qt::Vertical);
		m_vSplitter->addWidget(m_filterButtonsWidget);
		m_vSplitter->addWidget(m_filterTableWidget);
		connect(m_vSplitter, &QSplitter::splitterMoved, [this](int pos, int index)
		{
			Q_UNUSED(pos);
			Q_UNUSED(index);
			theSettings.m_switchPresetsPageSplitterPosition = m_vSplitter->saveState();
		});
		if (theSettings.m_switchPresetsPageSplitterPosition.isEmpty() == false)
		{
			m_vSplitter->restoreState(theSettings.m_switchPresetsPageSplitterPosition);
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

void SwitchFiltersPage::createFiltersList(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

	if (m_workspaceFilter->tagsList().isEmpty() == true ||
			m_workspaceFilter->hasAnyTag(filter->tagsList()) == true)
	{
		if (filter->isEmpty() == false &&
				filter->tagsList().contains(tag_FilterButton))
		{
			filter->setHasDiscreteCounter(true);
			m_buttonFilters.push_back(filter);
		}

		if (filter->isEmpty() == false &&
				filter->tagsList().contains(tag_FilterSwitch))
		{
			filter->setHasDiscreteCounter(true);
			m_listFilters.push_back(filter);
		}
	}

	int childCount = filter->childFiltersCount();
	for (int i = 0; i < childCount; i++)
	{
		std::shared_ptr<TuningFilter> childFilter = filter->childFilter(i);
		if (childFilter == nullptr)
		{
			Q_ASSERT(childFilter);
			return;
		}

		createFiltersList(childFilter);
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
		if (item == nullptr || item->widget() == nullptr)
		{
			Q_ASSERT(item);
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
		auto f = m_buttonFilters[i];
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		FilterPushButton* b = new FilterPushButton(f->caption(), f, this);

		m_filterButtons.push_back(b);

		connect(b, &FilterPushButton::clicked, this, &SwitchFiltersPage::slot_filterButtonClicked);

		b->setFixedSize(theSettings.m_switchPresetsPageButtonsWidth, theSettings.m_switchPresetsPageButtonsHeight);

		m_buttonsLayout->addWidget(b, row, col);

		if (col++ >= theSettings.m_switchPresetsPageColCount - 1)
		{
			col = 0;

			if (row++ >= theSettings.m_switchPresetsPageRowCount - 1)
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
		auto f = m_listFilters[i];

		for (int c = 0; c < static_cast<int>(Columns::ColumnCount); c++)
		{
			QTableWidgetItem* item = new QTableWidgetItem();
			if (item == nullptr)
			{
				Q_ASSERT(item);
				return;
			}

			m_filterTable->setItem(i, c, item);

			FilterCheckBox* check = new FilterCheckBox(tr("OFF"));
			connect(check, &FilterCheckBox::pressed, this, &SwitchFiltersPage::onFilterTablePressed);

			check->setStyleSheet("FilterCheckBox::indicator{width:25px;height:25px;}");

			switch (c)
			{
			case static_cast<int>(Columns::State):
				m_filterTable->setCellWidget(i, static_cast<int>(Columns::State), check);
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				break;
			case static_cast<int>(Columns::Caption):
				item->setText(f->caption());
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				break;
			case static_cast<int>(Columns::Counter):
				item->setText("0/0");
				item->setFlags(0/*Qt::ItemIsEnabled/* | Qt::ItemIsSelectable*/);
				break;
			}
		}
	}

	connect(m_filterTable, &FilterTableWidget::spacePressed, this, &SwitchFiltersPage::onFilterTablePressed);
}

bool SwitchFiltersPage::changeFilterSignals(std::shared_ptr<TuningFilter> filter)
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return false;
	}

	if (m_tuningTcpClient->takeClientControl(this) == false)
	{
		return false;
	}

	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return false;
	}

	TuningCounters tc = filter->counters();

	int discreteCount = countDiscretes(filter.get());
	int writingEnabledCount = countWritingEnabled(filter.get());

	if (discreteCount == 0 || discreteCount != writingEnabledCount)
	{
		return false;
	}

	int newValue = 0;

	if (tc.discreteCounter == 0)
	{
		if (QMessageBox::warning(this, qAppName(), tr("Are you sure you want to switch ON  signals of the filter '%1'?").arg(filter->caption()),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}

		newValue = 1;
	}
	else
	{
		if (tc.discreteCounter == discreteCount)
		{
			if (QMessageBox::warning(this, qAppName(), tr("Are you sure you want to switch OFF signals of the filter '%1'?").arg(filter->caption()),
									 QMessageBox::Yes | QMessageBox::No,
									 QMessageBox::No) != QMessageBox::Yes)
			{
				return false;
			}

			newValue = 0;
		}
		else
		{
			int result = QMessageBox::warning(this, qAppName(), tr("Signals of the filter '%1' have different values. Please select the following action:").arg(filter->caption()),
											  tr("Set All to 0"), tr("Set All to 1"), tr("Cancel"), 2);
			if (result == 0)
			{
				newValue = 0;
			}
			else
			{
				if (result == 1)
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

	std::vector <TuningFilterSignal> filterSignals = filter->getFilterSignals();

	for (auto f : filterSignals)
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(f.appSignalHash(), &ok);
		if (ok == false)
		{
			continue;
		}

		if (asp.toTuningType() != TuningValueType::Discrete)
		{
			continue;
		}

		TuningValue tv;
		tv.setType(TuningValueType::Discrete);
		tv.setDiscreteValue(newValue);

		m_tuningTcpClient->writeTuningSignal(f.appSignalId(), tv);
	}

	m_tuningTcpClient->writeLogSignalChange(tr("'%1' filter is toggled.").arg(filter->caption()));

	return true;
}

void SwitchFiltersPage::apply()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	if (m_tuningTcpClient->takeClientControl(this) == false)
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

	TuningCounters rootCounters = m_tuningFilterStorage->root()->counters();

	if (rootCounters.sorCounter > 0)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!!!\n\nSOR Signal(s) are set in logic modules!\n\nIf you apply these changes, module can run into RUN SAFE STATE.\n\nAre you sure you STILL WANT TO APPLY the changes?"),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	m_tuningTcpClient->applyTuningSignals();

	return;
}

int SwitchFiltersPage::countDiscretes(TuningFilter* filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return 0;
	}

	int result = 0;

	std::vector <TuningFilterSignal> filterSignals = filter->getFilterSignals();

	for (auto tfs : filterSignals)
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(tfs.appSignalHash(), &ok);
		if (ok == false)
		{
			continue;
		}

		if (asp.toTuningType() == TuningValueType::Discrete)
		{
			result++;
		}
	}

	return result;
}

int SwitchFiltersPage::countWritingEnabled(TuningFilter* filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return 0;
	}

	int result = 0;

	std::vector <TuningFilterSignal> filterSignals = filter->getFilterSignals();

	for (auto tfs : filterSignals)
	{
		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(tfs.appSignalHash(), &ok);
		if (ok == false)
		{
			continue;
		}

		if (asp.toTuningType() != TuningValueType::Discrete)
		{
			continue;
		}

		const TuningSignalState state = m_tuningSignalManager->state(tfs.appSignalHash(), &ok);

		if (ok == true)
		{
			if (m_tuningTcpClient->writingIsEnabled(state) == true)
			{
				result++;
			}
		}
	}

	return result;
}

void SwitchFiltersPage::showEvent(QShowEvent *ev)
{
	Q_UNUSED(ev);

	slot_timerTick500();
}

void SwitchFiltersPage::onOptions()
{
	SwitchFiltersPageOptions d(this,
							   theSettings.m_switchPresetsPageColCount,
							   theSettings.m_switchPresetsPageRowCount,
							   theSettings.m_switchPresetsPageButtonsWidth,
							   theSettings.m_switchPresetsPageButtonsHeight);
	if (d.exec() == QDialog::Accepted)
	{
		theSettings.m_switchPresetsPageColCount = d.buttonsColCount();
		theSettings.m_switchPresetsPageRowCount = d.buttonsRowCount();
		theSettings.m_switchPresetsPageButtonsWidth = d.buttonsWidth();
		theSettings.m_switchPresetsPageButtonsHeight = d.buttonsHeight();

		m_buttonStartIndex = 0;

		createButtons();

		m_prevButton->setVisible(m_buttonFilters.size() > theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount);
		m_nextButton->setVisible(m_buttonFilters.size() > theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount);

		slot_timerTick500();
	}
}

void SwitchFiltersPage::onPrev()
{
	if (m_buttonStartIndex >= theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount)
	{
		m_buttonStartIndex -= theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount;

		createButtons();

		slot_timerTick500();
	}
}
void SwitchFiltersPage::onNext()
{
	if (m_buttonStartIndex < m_buttonFilters.size() - theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount)
	{
		m_buttonStartIndex += theSettings.m_switchPresetsPageColCount * theSettings.m_switchPresetsPageRowCount;

		createButtons();

		slot_timerTick500();
	}
}

void SwitchFiltersPage::onApply()
{
	apply();
}

void SwitchFiltersPage::slot_timerTick500()
{
	if  (isVisible() == false)
	{
		return;
	}

	// Buttons

	int count = static_cast<int>(m_filterButtons.size());
	for (int i = 0; i < count; i++)
	{
		auto b = m_filterButtons[i];
		if (b == nullptr)
		{
			Q_ASSERT(b);
			return;
		}

		auto f = b->filter();
		if (f == nullptr)
		{
			Q_ASSERT(f);
			return;
		}

		int discreteCount = countDiscretes(f.get());
		int writingEnabledCount = countWritingEnabled(f.get());

		TuningCounters tc = f->counters();

		QString text = tr("%1\n\n%2 / %3").arg(f->caption()).arg(tc.discreteCounter).arg(discreteCount);

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

		if (tc.discreteCounter == 0)
		{
			b->setStyleSheet(QString());
			b->setDown(false);
		}
		else
		{
			if (tc.discreteCounter == discreteCount)
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
		auto f = m_listFilters[i];

		int discreteCount = countDiscretes(f.get());
		int writingEnabledCount = countWritingEnabled(f.get());

		TuningCounters tc = f->counters();

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
		QString counterText = tr("%1 / %2").arg(tc.discreteCounter).arg(discreteCount);

		QColor backColor;
		QColor textColor;
		Qt::CheckState checkState;

		if (tc.discreteCounter == 0)
		{
			backColor = Qt::white;
			textColor = Qt::black;
			checkState = Qt::Unchecked;
			checkText = tr("OFF");
		}
		else
		{
			if (tc.discreteCounter == discreteCount)
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

		if (itemCaption->backgroundColor() != backColor)
		{
			itemCaption->setBackgroundColor(backColor);
		}

		if (itemCaption->textColor() != textColor)
		{
			itemCaption->setTextColor(textColor);
		}

		if (itemCounter->backgroundColor() != backColor)
		{
			itemCounter->setBackgroundColor(backColor);
		}

		if (itemCounter->textColor() != textColor)
		{
			itemCounter->setTextColor(textColor);
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

void SwitchFiltersPage::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

	changeFilterSignals(filter);
}

void SwitchFiltersPage::onFilterTablePressed()
{
	int row = m_filterTable->currentRow();
	if (row < 0 || row >= m_listFilters.size())
	{
		Q_ASSERT(false);
		return;
	}

	std::shared_ptr<TuningFilter> filter = m_listFilters[row];
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

	changeFilterSignals(filter);
}
