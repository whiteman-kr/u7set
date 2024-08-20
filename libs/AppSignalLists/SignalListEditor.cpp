#include "../../AppSignalLib/ISignalManager.h"
#include "../../AppSignalLib/ITuningSignalManager.h"
#include "SignalListEditorPrivate.h"
#include "TextResource.h"
#include <AppSignalLists/SignalListEditor.h>

namespace AppSignalLists
{
	//
	// AppSignalListWidget
	//
	AppSignalListWidget::AppSignalListWidget(ISignalManager& appSignalManager, ITuningSignalManager* tuningSignalManager, QWidget* parent) :
		QWidget(parent),
		m_appSignalManager(appSignalManager),
		m_tuningSignalManager(tuningSignalManager),
		m_signalHashes(std::move(m_appSignalManager.signalHashes())),
		m_signalsModel(std::make_unique<SignalsModel>(m_appSignalManager)),
		m_itemsModel(std::make_unique<AppSignalListModel>(m_appSignalManager, m_tuningSignalManager != nullptr))
	{
		// Left part
		//
		QWidget* leftWidget = new QWidget();
		QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
		leftLayout->setContentsMargins(0, 0, 0, 0);

		QWidget* rightWidget = new QWidget();
		QHBoxLayout* rightLayout = new QHBoxLayout(rightWidget);
		rightLayout->setContentsMargins(0, 0, 0, 0);

		// Signals table
		//
		m_signalsTable = new QTableView();
		m_signalsTable->verticalHeader()->hide();
		m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_signalsTable->setSortingEnabled(true);
		m_signalsTable->horizontalHeader()->setHighlightSections(false);
		m_signalsTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		m_signalsTable->setModel(m_signalsModel.get());

		connect(m_signalsTable->selectionModel(),
				&QItemSelectionModel::selectionChanged,
				this,
				&AppSignalListWidget::onSignalsTableSelectionChanged);
		connect(m_signalsTable->horizontalHeader(),
				&QHeaderView::sortIndicatorChanged,
				this,
				&AppSignalListWidget::onSignalsSortIndicatorChanged);
		connect(m_signalsTable, &QTableView::doubleClicked, this, &AppSignalListWidget::onSignalsTableDoubleClicked);
		connect(m_signalsTable->horizontalHeader(),
				&QWidget::customContextMenuRequested,
				this,
				&AppSignalListWidget::onSignalsHeaderColumnContextMenuRequested);
		leftLayout->addWidget(m_signalsTable);

		// Signals Filters
		//
		m_signalTypeCombo = new QComboBox();
		m_signalTypeCombo->blockSignals(true);
		m_signalTypeCombo->addItem(tr("All signals"), static_cast<int>(SignalType::All));
		m_signalTypeCombo->addItem(tr("Analog signals"), static_cast<int>(SignalType::Analog));
		m_signalTypeCombo->addItem(tr("Discrete signals"), static_cast<int>(SignalType::Discrete));
		m_signalTypeCombo->setCurrentIndex(0);
		m_signalTypeCombo->blockSignals(false);
		connect(m_signalTypeCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&AppSignalListWidget::onSignalsTypeComboCurrentIndexChanged);
		leftLayout->addWidget(m_signalTypeCombo);

		QHBoxLayout* leftFilterLayout = new QHBoxLayout();

		m_filterTextTypeCombo = new QComboBox();
		m_filterTextTypeCombo->blockSignals(true);
		m_filterTextTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterTextType::All));
		m_filterTextTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterTextType::AppSignalID));
		m_filterTextTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterTextType::CustomAppSignalID));
		m_filterTextTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterTextType::EquipmentID));
		m_filterTextTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterTextType::Caption));
		m_filterTextTypeCombo->addItem(tr("Tag"), static_cast<int>(FilterTextType::Tag));
		m_filterTextTypeCombo->setCurrentIndex(0);
		m_filterTextTypeCombo->blockSignals(false);
		connect(m_filterTextTypeCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&AppSignalListWidget::onSignalsFilterTypeComboCurrentIndexChanged);
		leftFilterLayout->addWidget(m_filterTextTypeCombo, 1);

		m_filterTextEdit = new QLineEdit();
		connect(m_filterTextEdit, &QLineEdit::returnPressed, this, &AppSignalListWidget::onSignalsFilterTextChanged);
		connect(m_filterTextEdit,
				&QLineEdit::textChanged,
				this,
				[this](const QString& str)
				{
					if (str.isEmpty() == true)
					{
						onSignalsFilterTextChanged(); // Process mask if text was cleared
					}
				});
		m_filterTextEdit->setClearButtonEnabled(true);
		leftFilterLayout->addWidget(m_filterTextEdit, 3);

		m_applyFilterButton = new QPushButton(tr("Apply Filter"));
		connect(m_applyFilterButton, &QPushButton::clicked, this, &AppSignalListWidget::onSignalsApplyFilterClicked);
		leftFilterLayout->addWidget(m_applyFilterButton, 1);

		// Value filter controls
		//

		bool requestValuesEnabled = tuningSignalManager != nullptr;

		if (requestValuesEnabled == true)
		{
			leftFilterLayout->addSpacing(20);

			QLabel* l = new QLabel(tr("Value:"));
			leftFilterLayout->addWidget(l);

			m_filterValueCombo = new QComboBox();
			m_filterValueCombo->addItem(tr("Any Value"), static_cast<int>(FilterValueType::All));
			m_filterValueCombo->addItem(tr("Discrete 0"), static_cast<int>(FilterValueType::Zero));
			m_filterValueCombo->addItem(tr("Discrete 1"), static_cast<int>(FilterValueType::One));
			leftFilterLayout->addWidget(m_filterValueCombo);
			connect(m_filterValueCombo,
					static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					this,
					&AppSignalListWidget::onSignalsFilterValueComboCurrentIndexChanged);
			m_filterValueCombo->setCurrentIndex(0);
		}

		leftLayout->addLayout(leftFilterLayout);

		// Middle part
		//

		QVBoxLayout* addRemoveLayout = new QVBoxLayout();

		addRemoveLayout->addStretch();

		m_addValueButton = new QPushButton(tr("Add"));
		connect(m_addValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onAddClicked);
		addRemoveLayout->addWidget(m_addValueButton);
		m_addValueButton->setEnabled(false);

		m_removeValueButton = new QPushButton(tr("Remove"));
		connect(m_removeValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onRemoveClicked);
		addRemoveLayout->addWidget(m_removeValueButton);
		m_removeValueButton->setEnabled(false);

		addRemoveLayout->addStretch();

		rightLayout->addLayout(addRemoveLayout);

		// Right part
		//

		QVBoxLayout* itemsLayout = new QVBoxLayout();

		m_itemsTable = new QTableView();
		m_itemsTable->verticalHeader()->hide();
		m_itemsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
		m_itemsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		m_itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		m_itemsTable->setSortingEnabled(true);
		m_itemsTable->horizontalHeader()->setHighlightSections(false);
		m_itemsTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
		m_itemsTable->setModel(m_itemsModel.get());

		connect(m_itemsTable->selectionModel(),
				&QItemSelectionModel::selectionChanged,
				this,
				&AppSignalListWidget::onItemsTreeSelectionChanged);
		connect(m_itemsTable->horizontalHeader(),
				&QHeaderView::sortIndicatorChanged,
				this,
				&AppSignalListWidget::onItemsSortIndicatorChanged);
		connect(m_itemsTable, &QTableView::doubleClicked, this, &AppSignalListWidget::onItemsTreeDoubleClicked);
		connect(m_itemsTable->horizontalHeader(),
				&QWidget::customContextMenuRequested,
				this,
				&AppSignalListWidget::onItemsHeaderColumnContextMenuRequested);

		itemsLayout->addWidget(m_itemsTable);

		QHBoxLayout* rightGridLayout = new QHBoxLayout();

		rightGridLayout->addStretch();

		if (requestValuesEnabled == true)
		{
			m_setValueButton = new QPushButton(tr("Set Value"));
			connect(m_setValueButton, &QPushButton::clicked, this, &AppSignalListWidget::onSetValueClicked);
			rightGridLayout->addWidget(m_setValueButton);
			m_setValueButton->setEnabled(false);

			m_setCurrentButton = new QPushButton(tr("Set Current"));
			connect(m_setCurrentButton, &QPushButton::clicked, this, &AppSignalListWidget::onSetCurrentClicked);
			rightGridLayout->addWidget(m_setCurrentButton);
			m_setCurrentButton->setEnabled(false);
		}

		m_exportValuesButton = new QPushButton(tr("Export..."));
		connect(m_exportValuesButton, &QPushButton::clicked, this, &AppSignalListWidget::onExportValuesClicked);
		rightGridLayout->addWidget(m_exportValuesButton);

		m_importValuesButton = new QPushButton(tr("Import..."));
		connect(m_importValuesButton, &QPushButton::clicked, this, &AppSignalListWidget::onImportValuesClicked);
		rightGridLayout->addWidget(m_importValuesButton);
		m_importValuesButton->setEnabled(false);

		itemsLayout->addLayout(rightGridLayout);

		rightLayout->addLayout(itemsLayout);

		// Setup splitter
		//
		m_splitter = new QSplitter(Qt::Horizontal);
		m_splitter->addWidget(leftWidget);
		m_splitter->addWidget(rightWidget);
		m_splitter->setChildrenCollapsible(false);

		// Set main layout
		//
		{
			QHBoxLayout* mainLayout = new QHBoxLayout();
			mainLayout->setContentsMargins(0, 0, 0, 0);
			mainLayout->addWidget(m_splitter);
			setLayout(mainLayout);
		}

		fillSignalsList();

		// Restore signals list settings
		{
			int count = QSettings().value("AppSignalListWidget/signalsTableHeaderCount").toInt();
			QByteArray ba = QSettings().value("AppSignalListWidget/signalsTableHeader").toByteArray();
			if (ba.isEmpty() == true || count != m_signalsModel->columnCount())
			{
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::CustomAppSignalID));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::EquipmentID));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::Type));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::LowLimit));
				m_signalsTable->hideColumn(static_cast<int>(SignalsModel::Columns::HighLimit));
				m_signalsTable->resizeColumnsToContents();
			}
			else
			{
				m_signalsTable->horizontalHeader()->restoreState(ba);
			}
		}

		// Restore items list settings
		{
			int count = QSettings().value("AppSignalListWidget/itemsTreeHeaderCount").toInt();
			QByteArray ba = QSettings().value("AppSignalListWidget/itemsTreeHeader").toByteArray();
			if (ba.isEmpty() == true || count != m_itemsModel->columnCount())
			{
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::CustomAppSignalID));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::EquipmentID));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::Type));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::LowLimit));
				m_itemsTable->hideColumn(static_cast<int>(AppSignalListModel::Columns::HighLimit));
			}
			else
			{
				m_itemsTable->horizontalHeader()->restoreState(ba);
			}
		}

		QByteArray ba = QSettings().value("AppSignalListWidget/splitterState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_splitter->restoreState(ba);
		}
	}

	AppSignalListWidget::~AppSignalListWidget()
	{
		QSettings().setValue("AppSignalListWidget/signalsTableHeaderCount", m_signalsModel->columnCount());
		QSettings().setValue("AppSignalListWidget/signalsTableHeader", m_signalsTable->horizontalHeader()->saveState());

		QSettings().setValue("AppSignalListWidget/itemsTreeHeaderCount", m_itemsModel->columnCount());
		QSettings().setValue("AppSignalListWidget/itemsTreeHeader", m_itemsTable->horizontalHeader()->saveState());

		QSettings().setValue("AppSignalListWidget/splitterState", m_splitter->saveState());
	}

	bool AppSignalListWidget::readOnly() const
	{
		return m_readOnly;
	}

	void AppSignalListWidget::setReadOnly(bool value)
	{
		m_readOnly = value;
		enableSignalsListControls();
		enableItemsListControls();
		return;
	}

	AppSignalList* AppSignalListWidget::list()
	{
		return m_appSignalList;
	}

	const AppSignalList* AppSignalListWidget::list() const
	{
		return m_appSignalList;
	}

	void AppSignalListWidget::setList(AppSignalList* list)
	{
		m_appSignalList = list;
		fillItemsList();

		enableSignalsListControls();
		enableItemsListControls();
	}

	void AppSignalListWidget::fillSignalsList()
	{
		SignalType signalType = SignalType::All;
		QVariant data = m_signalTypeCombo->currentData();
		if (data.isNull() == false && data.isValid() == true)
		{
			signalType = static_cast<SignalType>(data.toInt());
		}

		FilterTextType filterType = FilterTextType::AppSignalID;
		data = m_filterTextTypeCombo->currentData();
		if (data.isNull() == false && data.isValid() == true)
		{
			filterType = static_cast<FilterTextType>(data.toInt());
		}

		FilterValueType filterValue = FilterValueType::All;
		if (m_filterValueCombo != nullptr)
		{
			data = m_filterValueCombo->currentData();
			if (data.isValid() == true)
			{
				filterValue = static_cast<FilterValueType>(data.toInt());
			}
		}

		QString filterText = m_filterTextEdit->text().trimmed();

		std::vector<Hash> filteredHashes;
		filteredHashes.reserve(m_signalHashes.size());

		if (filterText.isEmpty() == true && signalType == SignalType::All && filterValue == FilterValueType::All)
		{
			// Filter is not set - skip all filtering, just copy hashes array
			//
			filteredHashes = m_signalHashes;
		}
		else
		{
			// Some filters are set
			//
			for (Hash hash : m_signalHashes)
			{
				bool ok = false;
				const AppSignalParam& asp = m_appSignalManager.signalParam(hash, &ok);

				if (ok == false)
				{
					continue;
				}

				if (signalType == SignalType::Analog && asp.isAnalog() == false)
				{
					continue;
				}

				if (signalType == SignalType::Discrete && asp.isAnalog() == true)
				{
					continue;
				}

				// Value filter
				//
				if (filterValue != FilterValueType::All && m_tuningSignalManager != nullptr)
				{
					if (asp.isDiscrete() == false)
					{
						continue;
					}

					ok = false;
					const TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

					if (ok == true)
					{
						if (state.valid() == false)
						{
							continue;
						}
						if (filterValue == FilterValueType::Zero && state.value().discreteValue() != 0)
						{
							continue;
						}
						if (filterValue == FilterValueType::One && state.value().discreteValue() != 1)
						{
							continue;
						}
					}
				}

				// Text filter
				//
				if (filterText.isEmpty() == false)
				{
					bool filterResult = false;

					switch (filterType)
					{
					case FilterTextType::All:
						{
							if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true ||
								asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true ||
								asp.lmEquipmentId().contains(filterText, Qt::CaseInsensitive) == true ||
								asp.caption().contains(filterText, Qt::CaseInsensitive) == true || asp.tags().contains(filterText) == true)
							{
								filterResult = true;
							}
						}
						break;
					case FilterTextType::AppSignalID:
						{
							if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true)
							{
								filterResult = true;
							}
						}
						break;
					case FilterTextType::CustomAppSignalID:
						{
							if (asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true)
							{
								filterResult = true;
							}
						}
						break;
					case FilterTextType::EquipmentID:
						{
							if (asp.lmEquipmentId().contains(filterText, Qt::CaseInsensitive) == true)
							{
								filterResult = true;
							}
						}
						break;
					case FilterTextType::Caption:
						{
							if (asp.caption().contains(filterText, Qt::CaseInsensitive) == true)
							{
								filterResult = true;
							}
						}
						break;
					case FilterTextType::Tag:
						{
							if (asp.tags().contains(filterText) == true)
							{
								filterResult = true;
							}
						}
						break;
					}

					if (filterResult == false)
					{
						continue;
					}
				}

				filteredHashes.push_back(hash);
			}
		}

		m_signalsModel->setHashes(filteredHashes);
		m_signalsTable->sortByColumn(m_signalsSortColumn, m_signalsSortOrder);
	}


	void AppSignalListWidget::fillItemsList()
	{
		m_itemsModel->setList(m_appSignalList);
		m_itemsTable->sortByColumn(m_itemsSortColumn, m_itemsSortOrder);
	}

	void AppSignalListWidget::enableSignalsListControls()
	{
		const QModelIndexList& selection = m_signalsTable->selectionModel()->selectedRows();

		m_addValueButton->setEnabled(readOnly() == false && selection.size() > 0 && m_appSignalList != nullptr);
	}

	void AppSignalListWidget::enableItemsListControls()
	{
		const QModelIndexList& selection = m_itemsTable->selectionModel()->selectedRows();

		m_removeValueButton->setEnabled(readOnly() == false && selection.size() > 0);

		// Check if only tunable signals are selected
		//
		bool tunableSelected = true;
		for (const QModelIndex& index : selection)
		{
			Hash hash = m_itemsModel->itemHash(index.row());

			const AppSignalListItem& item = m_appSignalList->itemByHash(hash);

			bool ok = false;
			AppSignalParam asp = m_appSignalManager.signalParam(item.appSignalHash(), &ok);
			if (ok == false)
			{
				continue;
			}
			if (asp.enableTuning() == false)
			{
				tunableSelected = false;
			}
		}

		// Enable control buttons
		//
		if (m_setValueButton != nullptr)
		{
			m_setValueButton->setEnabled(readOnly() == false && selection.size() > 0 && tunableSelected == true);
		}

		if (m_setCurrentButton != nullptr)
		{
			m_setCurrentButton->setEnabled(readOnly() == false && selection.size() > 0 && tunableSelected == true);
		}

		m_importValuesButton->setEnabled(readOnly() == false);
	}

	void AppSignalListWidget::onSignalsSortIndicatorChanged(int column, Qt::SortOrder order)
	{
		m_signalsSortColumn = column;
		m_signalsSortOrder = order;

		m_signalsModel->sort(column, order);
	}

	void AppSignalListWidget::onSignalsTableSelectionChanged(const QItemSelection&, const QItemSelection&)
	{
		enableSignalsListControls();
	}

	void AppSignalListWidget::onSignalsApplyFilterClicked()
	{
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterTypeComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterValueComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsFilterTextChanged()
	{
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsTypeComboCurrentIndexChanged(int index)
	{
		Q_UNUSED(index);
		fillSignalsList();
	}

	void AppSignalListWidget::onSignalsTableDoubleClicked(const QModelIndex& index)
	{
		Q_UNUSED(index);

		if (readOnly() == true)
		{
			return;
		}

		onAddClicked();
	}

	void AppSignalListWidget::onSignalsHeaderColumnContextMenuRequested(const QPoint& pos)
	{
		QMenu menu(this);

		QList<QAction*> actions;

		std::vector<std::pair<SignalsModel::Columns, QString>> actionsData;
		actionsData.reserve(static_cast<int>(SignalsModel::Columns::Count));

		for (int i = 0; i < static_cast<int>(SignalsModel::Columns::Count); i++)
		{
			actionsData.emplace_back(static_cast<SignalsModel::Columns>(i), m_signalsModel->columnText(i));
		}

		for (std::pair<SignalsModel::Columns, QString> ad : actionsData)
		{
			QAction* action = new QAction(ad.second, this);
			action->setData(QVariant::fromValue(ad.first));
			action->setCheckable(true);
			action->setChecked(!m_signalsTable->horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

			if (m_signalsTable->horizontalHeader()->count() - m_signalsTable->horizontalHeader()->hiddenSectionCount() == 1 &&
				action->isChecked() == true)
			{
				action->setEnabled(false); // Impossible to uncheck the last column
			}

			connect(action, &QAction::toggled, this, &AppSignalListWidget::onSignalsHeaderColumnToggled);
			actions << action;
		}

		menu.exec(actions, mapToGlobal(pos), 0, this);
		return;
	}

	void AppSignalListWidget::onSignalsHeaderColumnToggled(bool checked)
	{
		QAction* action = dynamic_cast<QAction*>(sender());

		if (action == nullptr)
		{
			Q_ASSERT(action);
			return;
		}

		int column = action->data().value<int>();

		if (column >= static_cast<int>(SignalsModel::Columns::Count))
		{
			Q_ASSERT(column < static_cast<int>(SignalsModel::Columns::Count));
			return;
		}

		if (checked == true)
		{
			m_signalsTable->showColumn(column);
		}
		else
		{
			m_signalsTable->hideColumn(column);
		}

		return;
	}

	void AppSignalListWidget::onItemsSortIndicatorChanged(int column, Qt::SortOrder order)
	{
		m_itemsSortColumn = column;
		m_itemsSortOrder = order;

		m_itemsModel->sort(column, order);
	}

	void AppSignalListWidget::onItemsTreeSelectionChanged()
	{
		enableItemsListControls();
	}

	void AppSignalListWidget::onItemsTreeDoubleClicked(const QModelIndex& index)
	{
		Q_UNUSED(index);

		if (readOnly() == true)
		{
			return;
		}

		// Determine if user clicked on Value column of Tunable signal. If so, show Value dialog, otherwise remove the item
		//
		if (m_setValueButton != nullptr && m_setValueButton->isEnabled() == true &&
			index.column() == static_cast<int>(AppSignalListModel::Columns::Value))
		{
			// Determine if signal is tunable
			//
			Hash hash = m_itemsModel->itemHash(index.row());
			if (m_appSignalManager.signalExists(hash) == false)
			{
				return;
			}

			const AppSignalListItem& item = m_appSignalList->itemByHash(hash);

			bool ok = false;
			AppSignalParam asp = m_appSignalManager.signalParam(item.appSignalHash(), &ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				return;
			}
			if (asp.enableTuning() == true)
			{
				onSetValueClicked();
				return;
			}
		}

		onRemoveClicked();
	}

	void AppSignalListWidget::onItemsHeaderColumnContextMenuRequested(const QPoint& /*pos*/)
	{
		QMenu menu(this);

		QList<QAction*> actions;

		std::vector<std::pair<int, QString>> actionsData;
		actionsData.reserve(m_itemsModel->columnCount());

		for (int i = 0; i < m_itemsModel->columnCount(); i++)
		{
			actionsData.emplace_back(static_cast<int>(m_itemsModel->column(i)), m_itemsModel->columnText(i));
		}

		for (const auto& [col, text] : actionsData)
		{
			QAction* action = new QAction(text, this);
			action->setData(QVariant::fromValue(col));
			action->setCheckable(true);
			action->setChecked(!m_itemsTable->horizontalHeader()->isSectionHidden(col));

			if (m_itemsTable->horizontalHeader()->count() - m_itemsTable->horizontalHeader()->hiddenSectionCount() == 1 &&
				action->isChecked() == true)
			{
				action->setEnabled(false); // Impossible to uncheck the last column
			}

			connect(action, &QAction::toggled, this, &AppSignalListWidget::onItemsHeaderColumnToggled);
			actions << action;
		}

		menu.exec(actions, QCursor::pos(), 0, this);
		return;
	}

	void AppSignalListWidget::onItemsHeaderColumnToggled(bool checked)
	{
		QAction* action = dynamic_cast<QAction*>(sender());

		if (action == nullptr)
		{
			Q_ASSERT(action);
			return;
		}

		int column = action->data().value<int>();

		if (column >= m_itemsModel->columnCount())
		{
			Q_ASSERT(column < m_itemsModel->columnCount());
			return;
		}

		if (checked == true)
		{
			m_itemsTable->showColumn(column);
		}
		else
		{
			m_itemsTable->hideColumn(column);
		}

		return;
	}

	void AppSignalListWidget::onAddClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignalList == nullptr)
		{
			return;
		}

		for (const QModelIndex& index : m_signalsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_signalsModel->hash(index.row());

			if (m_itemsModel->itemExists(hash) == true)
			{
				continue;
			}

			bool ok = false;

			const AppSignalParam p = m_appSignalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				return;
			}
			/*
			const TuningSignalState s = m_signalManager.queuedState(hash, &ok);

			if (m_filter == nullptr)
			{
				Q_ASSERT(m_filter);
				return;
			}

			if (m_filter->filterSignalExists(p.hash()) == true)
			{
				continue;
			}

			// Create value
			*/
			AppSignalListItem item(p.appSignalId());
			/*if (s.valid() == true)
			{
				ofv.setValue(s.value());
			}*/

			m_appSignalList->add(item);

			if (m_itemsModel->add(item) == false)
			{
				Q_ASSERT(false);
				continue;
			}
		}

		emit signalsChanged();
	}

	void AppSignalListWidget::onRemoveClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignalList == nullptr)
		{
			return;
		}

		auto reply = QMessageBox::question(this, "Confirmation", QString("Are you sure you want to remove selected signals?"));
		if (reply == QMessageBox::No)
		{
			return;
		}

		// Build list of selected items hashes
		//
		std::vector<Hash> hashesToDelete;
		auto selection = m_itemsTable->selectionModel()->selectedRows();
		for (const QModelIndex& index : selection)
		{
			Hash hash = m_itemsModel->itemHash(index.row());
			hashesToDelete.push_back(hash);
		}

		for (Hash hash : hashesToDelete)
		{
			// Delete item from model
			//
			if (m_itemsModel->remove(hash) == false)
			{
				Q_ASSERT(false);
			}

			// Delete item from list
			//
			m_appSignalList->remove(hash);
		}

		emit signalsChanged();
	}

	void AppSignalListWidget::onSetValueClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignalList == nullptr)
		{
			return;
		}

		bool first = true;
		TuningValue lowLimit;
		TuningValue highLimit;
		int precision = 0;
		TuningValue value;
		TuningValue defaultValue;

		bool sameValue = true;
		bool sameDefaultValue = true;

		for (const QModelIndex& index : m_itemsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel->itemHash(index.row());

			if (m_appSignalManager.signalExists(hash) == false)
			{
				continue;
			}

			const AppSignalListItem& item = m_appSignalList->itemByHash(hash);

			bool ok = false;
			AppSignalParam asp = m_appSignalManager.signalParam(item.appSignalHash(), &ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				return;
			}

			if (first == true)
			{
				lowLimit = asp.tuningLowBound();
				highLimit = asp.tuningHighBound();
				precision = asp.precision();

				if (item.hasValue() == true)
				{
					value = item.value();
				}
				value.setType(asp.tuningType());

				defaultValue = asp.tuningDefaultValue();

				first = false;
			}
			else
			{
				if (asp.tuningType() != value.type())
				{
					QMessageBox::warning(this, qAppName(), tr("Please select signals of same type (analog or discrete)."));
					return;
				}

				if (asp.isAnalog() == true)
				{
					if (lowLimit != asp.tuningLowBound() || highLimit != asp.tuningHighBound())
					{
						QMessageBox::warning(this, qAppName(), tr("Selected signals have different input range."));
						return;
					}
				}

				if (item.hasValue() == true)
				{
					if (item.value() != value)
					{
						sameValue = false;
					}
				}
				if (defaultValue != asp.tuningDefaultValue())
				{
					sameDefaultValue = false;
				}
			}
		}

		DialogAppSignalListValue
			d(value, defaultValue, sameValue, sameDefaultValue, lowLimit, highLimit, E::AnalogFormat::g_9_or_9e, precision, this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		for (const QModelIndex& index : m_itemsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel->itemHash(index.row());

			if (m_appSignalManager.signalExists(hash) == false)
			{
				continue;
			}

			AppSignalListItem& item = m_appSignalList->itemByHash(hash);
			item.setValue(d.value());
			m_itemsTable->update(index);
		}
		emit signalsChanged();
	}

	void AppSignalListWidget::onSetCurrentClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignalList == nullptr)
		{
			return;
		}

		for (const QModelIndex& index : m_itemsTable->selectionModel()->selectedRows())
		{
			Hash hash = m_itemsModel->itemHash(index.row());

			if (m_appSignalManager.signalExists(hash) == false)
			{
				continue;
			}

			AppSignalListItem& item = m_appSignalList->itemByHash(hash);

			bool ok = false;
			auto state = m_tuningSignalManager->state(item.appSignalHash(), &ok);

			if (ok == true && state.valid() == true)
			{
				item.setValue(state.value());
				m_itemsTable->update(m_itemsModel->index(index.row(), static_cast<int>(AppSignalListModel::Columns::Value)));
			}
			else
			{
				QMessageBox::warning(this, qAppName(), tr("Can't get current value of signal %1!").arg(item.appSignalId()));
			}
		}

		emit signalsChanged();
	}

	void AppSignalListWidget::onExportValuesClicked()
	{
		if (m_appSignalList == nullptr)
		{
			return;
		}

		int columnCount = m_itemsModel->columnCount();
		int rowCount = m_itemsModel->rowCount();

		static QString path{"."};
		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Export to CSV"),
														path + QDir::separator() + m_appSignalList->id() + ".csv",
														tr("CSV (*.csv)"));

		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		QFile file(fileName);
		if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
			return;
		}

		QTextStream out(&file);
		out.setEncoding(QStringConverter::Utf8);

		QString csvHeader;
		for (int c = 0; c < columnCount; c++)
		{
			csvHeader += m_itemsModel->columnName(c) + ';';
		}
		out << csvHeader << "\r\n";

		for (int r = 0; r < rowCount; r++)
		{
			QString csvRow;
			for (int c = 0; c < columnCount; c++)
			{
				csvRow += m_itemsModel->cellText(c, r) + ';';
			}
			out << csvRow << "\r\n";
		}

		QMessageBox::information(this, qAppName(), tr("Export complete."));
	}

	void AppSignalListWidget::onImportValuesClicked()
	{
		if (readOnly() == true)
		{
			return;
		}

		if (m_appSignalList == nullptr)
		{
			return;
		}

		static QString path{"."};
		QString fileName = QFileDialog::getOpenFileName(this, tr("Import from CSV"), path, tr("CSV (*.csv)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		QFile file(fileName);
		if (file.open(QFile::ReadOnly) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Error writing file %1!").arg(fileName));
			return;
		}

		QTextStream in(&file);
		in.setEncoding(QStringConverter::Utf8);

		bool headerString = true;

		int columnAppSignalIdIndex = -1;
		int columnValueIndex = -1;

		int signalsAdded = 0;
		int signalsUpdated = 0;
		QStringList notFoundSignals;

		while (!in.atEnd())
		{
			QString str = in.readLine().trimmed();
			if (str.isEmpty() == true)
			{
				break;
			}

			QStringList strings = str.split(';', Qt::KeepEmptyParts);

			// Load header string and find columns with appSignalId and value
			//
			if (headerString == true)
			{
				headerString = false;

				for (int i = 0; i < strings.size(); i++)
				{
					const QString& s = strings[i];
					if (s == AppSignalLists::col_AppSignalId)
					{
						columnAppSignalIdIndex = i;
						continue;
					}
					if (s == AppSignalLists::col_Value)
					{
						columnValueIndex = i;
						continue;
					}
				}
				if (columnValueIndex == -1 || columnAppSignalIdIndex == -1)
				{
					QMessageBox::critical(this,
										  qAppName(),
										  tr("Error: '%1' and '%2' columns are absent in CSV file. Import is impossible.")
											  .arg(col_AppSignalId)
											  .arg(col_Value));
					return;
				}

				continue;
			}

			// Load signal list item

			const QString& appSignalId = strings[columnAppSignalIdIndex];
			const QString& valueStr = strings[columnValueIndex];

			Hash hash = ::calcHash(appSignalId);

			// Get signal parameters from database

			bool ok = false;
			const AppSignalParam asp = m_appSignalManager.signalParam(hash, &ok);
			if (ok == false)
			{
				notFoundSignals.push_back(appSignalId);
				continue;
			}

			// Read value

			std::optional<TuningValue> tv;
			if (valueStr.isEmpty() == false)
			{
				TuningValue v(asp.tuningType());
				bool valueOk = false;
				v.fromString(valueStr, &valueOk);

				if (valueOk == true)
				{
					tv = v;
				}
			}

			// Add item to the list

			bool signalExists = m_appSignalList->itemExists(hash);
			if (signalExists == false)
			{
				AppSignalListItem item(asp.appSignalId());
				if (tv.has_value() == true)
				{
					item.setValue(tv.value());
				}
				m_appSignalList->add(item);
				signalsAdded++;
			}
			else
			{
				AppSignalListItem& item = m_appSignalList->itemByHash(hash);
				if (tv.has_value() == true)
				{
					item.setValue(tv.value());
				}
				else
				{
					item.removeValue();
				}
				signalsUpdated++;
			}
		}

		fillItemsList();

		if (notFoundSignals.isEmpty() == true)
		{
			QMessageBox::information(
				this,
				qAppName(),
				tr("Import complete.\n\nAdded: %1 signal(s)\nUpdated: %2 signal(s)").arg(signalsAdded).arg(signalsUpdated));
		}
		else
		{
			int notFoundCount = notFoundSignals.size();
			bool notFoundAbove10 = notFoundSignals.size() > 10;

			// Leave only first ten signals
			//
			while (notFoundSignals.size() > 10)
			{
				notFoundSignals.removeLast();
			}

			QString message = tr("Import complete.\n\nAdded: %1 signal(s)\nUpdated: %2 signal(s)\n\n%3 signal(s) were not found:\n%4")
								  .arg(signalsAdded)
								  .arg(signalsUpdated)
								  .arg(notFoundCount)
								  .arg(notFoundSignals.join('\n'));
			if (notFoundAbove10 == true)
			{
				message += tr(" and more.");
			}

			QMessageBox::warning(this, qAppName(), message);
		}

		emit signalsChanged();

		return;
	}
} // namespace AppSignalLists