#include "TuningFilterEditor.h"
#include "../lib/PropertyEditorDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>

//
// DialogChooseTuningSignals
//

ChooseTuningSignalsWidget::ChooseTuningSignalsWidget(TuningSignalManager* signalStorage, bool requestValuesEnabled, QWidget* parent)
	:QWidget(parent),
		m_signalManager(signalStorage)
{

	setWindowTitle(tr("Filter Signals"));

	// Left part
	//

	QHBoxLayout* mainLayout = new QHBoxLayout();

	QVBoxLayout* leftLayout = new QVBoxLayout();

	m_baseSignalsTable = new QTableView();
	m_baseSignalsTable->verticalHeader()->hide();
	m_baseSignalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_baseSignalsTable->verticalHeader()->setDefaultSectionSize(16);
	m_baseSignalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_baseSignalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_baseSignalsTable->setSortingEnabled(true);
	connect(m_baseSignalsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &ChooseTuningSignalsWidget::baseSortIndicatorChanged);
	connect(m_baseSignalsTable, &QTableView::doubleClicked, this, &ChooseTuningSignalsWidget::on_m_baseSignalsTable_doubleClicked);
	leftLayout->addWidget(m_baseSignalsTable);

	m_baseSignalTypeCombo = new QComboBox();
	m_baseSignalTypeCombo->blockSignals(true);
	m_baseSignalTypeCombo->addItem(tr("All signals"), static_cast<int>(SignalType::All));
	m_baseSignalTypeCombo->addItem(tr("Analog signals"), static_cast<int>(SignalType::Analog));
	m_baseSignalTypeCombo->addItem(tr("Discrete signals"), static_cast<int>(SignalType::Discrete));
	m_baseSignalTypeCombo->setCurrentIndex(0);
	m_baseSignalTypeCombo->blockSignals(false);
	connect(m_baseSignalTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_m_baseSignalTypeCombo_currentIndexChanged(int)));
	leftLayout->addWidget(m_baseSignalTypeCombo);

	QHBoxLayout* leftFilterLayout = new QHBoxLayout();

	m_baseFilterTypeCombo = new QComboBox();
	m_baseFilterTypeCombo->blockSignals(true);
	m_baseFilterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterType::All));
	m_baseFilterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterType::AppSignalID));
	m_baseFilterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterType::CustomAppSignalID));
	m_baseFilterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterType::EquipmentID));
	m_baseFilterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterType::Caption));
	m_baseFilterTypeCombo->setCurrentIndex(0);
	m_baseFilterTypeCombo->blockSignals(false);
	connect(m_baseFilterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_m_baseFilterTypeCombo_currentIndexChanged(int)));
	leftFilterLayout->addWidget(m_baseFilterTypeCombo);

	m_baseFilterText = new QLineEdit();
	connect(m_baseFilterText, &QLineEdit::returnPressed, this, &ChooseTuningSignalsWidget::on_m_baseFilterText_returnPressed);
	leftFilterLayout->addWidget(m_baseFilterText);

	m_baseApplyFilter = new QPushButton(tr("Apply Filter"));
	connect(m_baseApplyFilter, &QPushButton::clicked, this, &ChooseTuningSignalsWidget::on_m_baseApplyFilter_clicked);
	leftFilterLayout->addWidget(m_baseApplyFilter);

	// Value filter controls
	//

	if (requestValuesEnabled == true)
	{
		leftFilterLayout->addSpacing(20);

		QLabel* l = new QLabel(tr("Value:"));
		leftFilterLayout->addWidget(l);

		m_baseFilterValueCombo = new QComboBox();
		m_baseFilterValueCombo->addItem(tr("Any Value"), static_cast<int>(FilterType::All));
		m_baseFilterValueCombo->addItem(tr("Discrete 0"), static_cast<int>(FilterType::Zero));
		m_baseFilterValueCombo->addItem(tr("Discrete 1"), static_cast<int>(FilterType::One));
		leftFilterLayout->addWidget(m_baseFilterValueCombo);

		connect(m_baseFilterValueCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_m_baseFilterValueCombo_currentIndexChanged(int)));

		m_baseFilterValueCombo->setCurrentIndex(0);
	}

	leftLayout->addLayout(leftFilterLayout);

	mainLayout->addLayout(leftLayout);

	// Middle part
	//

	QVBoxLayout* midLayout = new QVBoxLayout();

	midLayout->addStretch();

	m_addValue = new QPushButton(tr("Add"));
	connect(m_addValue, &QPushButton::clicked, this, &ChooseTuningSignalsWidget::on_m_add_clicked);
	midLayout->addWidget(m_addValue);

	m_removeValue = new QPushButton(tr("Remove"));
	connect(m_removeValue, &QPushButton::clicked, this, &ChooseTuningSignalsWidget::on_m_remove_clicked);
	midLayout->addWidget(m_removeValue);

	midLayout->addStretch();

	mainLayout->addLayout(midLayout);

	// Right part
	//

	QVBoxLayout* rightLayout = new QVBoxLayout();

	m_filterValuesTree = new QTreeWidget();
	m_filterValuesTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_filterValuesTree->setWordWrap(false);
	connect(m_filterValuesTree, &QTreeWidget::doubleClicked, this, &ChooseTuningSignalsWidget::on_m_filterValuesTree_doubleClicked);

	QStringList headers;
	headers << tr("CustomAppSignalID");
	headers << tr("AppSignalID");
	headers << tr("Type");
	headers << tr("Caption");
	headers << tr("Value");

	m_filterValuesTree->setColumnCount(headers.size());
	m_filterValuesTree->setHeaderLabels(headers);
	rightLayout->addWidget(m_filterValuesTree);

	QHBoxLayout* rightGridLayout = new QHBoxLayout();

	/*m_moveUp = new QPushButton(tr("Move Up"));
	connect(m_moveUp, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_moveUp_clicked);
	rightGridLayout->addWidget(m_moveUp);

	m_moveDown = new QPushButton(tr("Move Down"));
	connect(m_moveDown, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_moveDown_clicked);
	rightGridLayout->addWidget(m_moveDown);*/

	rightGridLayout->addStretch();

	m_setValue = new QPushButton(tr("Set Value"));
	connect(m_setValue, &QPushButton::clicked, this, &ChooseTuningSignalsWidget::on_m_setValue_clicked);
	rightGridLayout->addWidget(m_setValue);

	if (requestValuesEnabled == true)
	{
		m_setCurrent = new QPushButton(tr("Set Current"));
		connect(m_setCurrent, &QPushButton::clicked, this, &ChooseTuningSignalsWidget::on_m_setCurrent_clicked);
		rightGridLayout->addWidget(m_setCurrent);
	}

	m_moveUpAction = new QAction(tr("Move Up"), this);
	connect(m_moveUpAction, &QAction::triggered, this, &ChooseTuningSignalsWidget::on_m_moveUp_clicked);

	m_moveDownAction = new QAction(tr("Move Down"), this);
	connect(m_moveDownAction, &QAction::triggered, this, &ChooseTuningSignalsWidget::on_m_moveDown_clicked);

	m_setValueAction = new QAction(tr("Set Value"), this);
	connect(m_setValueAction, &QAction::triggered, this, &ChooseTuningSignalsWidget::on_m_setValue_clicked);

	m_setCurrentAction = new QAction(tr("Set Current"), this);
	connect(m_setCurrentAction, &QAction::triggered, this, &ChooseTuningSignalsWidget::on_m_setCurrent_clicked);

	rightLayout->addLayout(rightGridLayout);

	mainLayout->addLayout(rightLayout);

	//

	setLayout(mainLayout);

	// Objects and model
	//
	m_baseModel = new TuningModel(m_signalManager, std::vector<QString>(), this);

	m_baseModel->addColumn(TuningModelColumns::CustomAppSignalID);
	m_baseModel->addColumn(TuningModelColumns::AppSignalID);
	m_baseModel->addColumn(TuningModelColumns::Type);
	m_baseModel->addColumn(TuningModelColumns::Caption);
	m_baseModel->addColumn(TuningModelColumns::Default);

	m_baseSignalsTable->setModel(m_baseModel);

	fillBaseSignalsList();

	// Set column width
	//

	m_baseSignalsTable->resizeColumnsToContents();

}

void ChooseTuningSignalsWidget::setFilter(std::shared_ptr<TuningFilter> selectedFilter)
{
	m_filter = selectedFilter;

	if (m_filterValuesTree == nullptr)
	{
		Q_ASSERT(m_filterValuesTree);
		return;
	}

	m_filterValuesTree->clear();

	if (m_filter != nullptr)
	{
		std::vector <TuningFilterSignal> values = m_filter->getFilterSignals();

		for (const TuningFilterSignal& v: values)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			setFilterValueItemText(item, v);
			m_filterValuesTree->addTopLevelItem(item);
		}

		for (int i = 0; i < m_filterValuesTree->columnCount(); i++)
		{
			m_filterValuesTree->resizeColumnToContents(i);
		}

		m_filterValuesTree->setSortingEnabled(true);
		m_filterValuesTree->sortByColumn(0, Qt::AscendingOrder);
	}
}

void ChooseTuningSignalsWidget::fillBaseSignalsList()
{
	if (m_signalManager == nullptr)
	{
		assert(m_signalManager);
		return;
	}

	SignalType signalType = SignalType::All;
	QVariant data = m_baseSignalTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		signalType = static_cast<SignalType>(data.toInt());
	}

	FilterType filterType = FilterType::AppSignalID;
	data = m_baseFilterTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		filterType = static_cast<FilterType>(data.toInt());
	}

	FilterType filterValue = FilterType::All;
	if (m_baseFilterValueCombo != nullptr)
	{
		data = m_baseFilterValueCombo->currentData();
		if (data.isValid() == true)
		{
			filterValue = static_cast<FilterType>(data.toInt());
		}
	}

	QString filterText = m_baseFilterText->text().trimmed();

	std::vector<Hash> hashes = m_signalManager->signalHashes();

	std::vector<Hash> filteredHashes;
	filteredHashes.reserve(hashes.size());

	AppSignalParam asp;

	for (Hash hash : hashes)
	{
		if (m_signalManager->signalParam(hash, &asp) == false)
		{
			assert(false);
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

		if (filterValue != FilterType::All && asp.isDiscrete() == true)
		{
			bool ok = false;

			const TuningSignalState state = m_signalManager->state(hash, &ok);

			if (ok == true && state.valid() == true)
			{
				if (filterValue == FilterType::Zero && state.value().discreteValue() != 0)
				{
					continue;
				}
				if (filterValue == FilterType::One && state.value().discreteValue() != 1)
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
			case FilterType::All:
			{
				if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true
						||asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true
						||asp.equipmentId().contains(filterText, Qt::CaseInsensitive) == true
						||asp.caption().contains(filterText, Qt::CaseInsensitive) == true )
				{
					filterResult = true;
				}
			}
				break;
			case FilterType::AppSignalID:
			{
				if (asp.appSignalId().contains(filterText, Qt::CaseInsensitive) == true)
				{
					filterResult = true;
				}
			}
				break;
			case FilterType::CustomAppSignalID:
			{
				if (asp.customSignalId().contains(filterText, Qt::CaseInsensitive) == true)
				{
					filterResult = true;
				}
			}
				break;
			case FilterType::EquipmentID:
			{
				if (asp.equipmentId().contains(filterText, Qt::CaseInsensitive) == true)
				{
					filterResult = true;
				}
			}
				break;
			case FilterType::Caption:
			{
				if (asp.caption().contains(filterText, Qt::CaseInsensitive) == true)
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

	m_baseModel->setHashes(filteredHashes);
	m_baseSignalsTable->sortByColumn(m_sortColumn, m_sortOrder);
}

void ChooseTuningSignalsWidget::on_m_add_clicked()
{
	if (m_filter == nullptr)
	{
		return;
	}

	for (const QModelIndex& index : m_baseSignalsTable->selectionModel()->selectedRows())
	{
		Hash hash = m_baseModel->hashByIndex(index.row(), 0);

		bool ok = false;

		const AppSignalParam p = m_signalManager->signalParam(hash, &ok);
		if (ok == false)
		{
			Q_ASSERT(false);
			return;
		}

		const TuningSignalState s = m_signalManager->state(hash, &ok);

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

		TuningFilterSignal ofv;
		ofv.setAppSignalId(p.appSignalId());
		if (s.valid() == true)
		{
			ofv.setValue(s.value());
		}

		m_filter->addFilterSignal(ofv);

		// Add tree item

		QTreeWidgetItem* childItem = new QTreeWidgetItem();
		setFilterValueItemText(childItem, ofv);

		m_filterValuesTree->addTopLevelItem(childItem);

		// Adjust width if this is the first item

		if (m_filterValuesTree->topLevelItemCount() == 1)
		{
			for (int i = 0; i < m_filterValuesTree->columnCount(); i++)
			{
				m_filterValuesTree->resizeColumnToContents(i);
			}
		}
	}
}

void ChooseTuningSignalsWidget::on_m_remove_clicked()
{
	if (m_filter == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_filterValuesTree->selectedItems();

	for (auto selectedItem : selectedItems)
	{
		if (selectedItem == nullptr)
		{
			Q_ASSERT(selectedItem);
			return;
		}

		Hash hash = selectedItem->data(static_cast<int>(Columns::AppSignalID), Qt::UserRole).value<Hash>();

		if (m_filter == nullptr)
		{
			Q_ASSERT(m_filter);
			return;
		}

		if (m_filter->removeFilterSignal(hash) == false)
		{
			Q_ASSERT(false);
			return;
		}

		delete selectedItem;
	}
}

void ChooseTuningSignalsWidget::on_m_setValue_clicked()
{
	if (m_filter == nullptr)
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

	QList<QTreeWidgetItem*> selectedItems = m_filterValuesTree->selectedItems();

	for (auto selectedItem : selectedItems)
	{
		if (selectedItem == nullptr)
		{
			Q_ASSERT(selectedItem);
			return;
		}

		Hash hash = selectedItem->data(static_cast<int>(Columns::AppSignalID), Qt::UserRole).value<Hash>();

		if (m_filter == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		TuningFilterSignal fv;

		if (m_filter->filterSignalExists(hash) == true)
		{
			bool ok = m_filter->filterSignal(hash, fv);
			Q_UNUSED(ok);
		}

		if (m_signalManager->signalExists(hash) == false)
		{
			continue;
		}

		AppSignalParam asp;
		if (m_signalManager->signalParam(hash, &asp) == false)
		{
			assert(false);
			return;
		}

		if (first == true)
		{
			lowLimit = asp.tuningLowBound();
			highLimit = asp.tuningHighBound();
			precision = asp.precision();

			if (fv.useValue() == true)
			{
				value = fv.value();
			}
			value.setType(asp.toTuningType());

			defaultValue = asp.tuningDefaultValue();

			first = false;
		}
		else
		{
			if (asp.toTuningType() != value.type())
			{
				QMessageBox::warning(this, tr("Filter Editor"), tr("Please select signals of same type (analog or discrete)."));
				return;
			}

			if (asp.isAnalog() == true)
			{
				if (lowLimit != asp.tuningLowBound() || highLimit != asp.tuningHighBound())
				{
					QMessageBox::warning(this, tr("Filter Editor"), tr("Selected signals have different input range."));
					return;
				}
			}

			if (asp.tuningDefaultValue() != defaultValue)
			{
				QMessageBox::warning(this, tr("Filter Editor"), tr("Selected signals have different default value."));
				return;
			}

			if (fv.useValue() == true && fv.value() != value)
			{
				sameValue = false;
			}
		}
	}

	DialogInputTuningValue d(value, defaultValue, sameValue, lowLimit, highLimit, precision, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	for (auto selectedItem : selectedItems)
	{
		if (selectedItem == nullptr)
		{
			Q_ASSERT(selectedItem);
			return;
		}

		Hash hash = selectedItem->data(static_cast<int>(Columns::AppSignalID), Qt::UserRole).value<Hash>();

		AppSignalParam asp;
		if (m_signalManager->signalParam(hash, &asp) == false)
		{
			Q_ASSERT(false);
			return;
		}

		TuningFilterSignal fv;
		fv.setAppSignalId(asp.appSignalId());
		fv.setUseValue(true);
		fv.setValue(d.value());

		m_filter->addFilterSignal(fv);

		setFilterValueItemText(selectedItem, fv);
	}
}

void ChooseTuningSignalsWidget::on_m_setCurrent_clicked()
{
	if (m_filter == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_filterValuesTree->selectedItems();

	for (auto selectedItem : selectedItems)
	{
		if (selectedItem == nullptr)
		{
			Q_ASSERT(selectedItem);
			return;
		}

		Hash hash = selectedItem->data(static_cast<int>(Columns::AppSignalID), Qt::UserRole).value<Hash>();

		if (m_filter == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		AppSignalParam asp;
		if (m_signalManager->signalParam(hash, &asp) == false)
		{
			Q_ASSERT(false);
			return;
		}

		TuningValue currentValue;

		bool ok = false;
		emit getCurrentSignalValue(hash, &currentValue, &ok);

		if (ok == true)
		{
			TuningFilterSignal fv;
			fv.setAppSignalId(asp.appSignalId());
			fv.setUseValue(true);
			fv.setValue(currentValue);

			m_filter->addFilterSignal(fv);

			setFilterValueItemText(selectedItem, fv);
		}
		else
		{
			QMessageBox::warning(this, tr("Filter Editor"), tr("Can't get current value of signal %1!").arg(asp.customSignalId()));
		}
	}
}

void ChooseTuningSignalsWidget::on_m_baseApplyFilter_clicked()
{
	fillBaseSignalsList();
}

void ChooseTuningSignalsWidget::on_m_baseSignalsTable_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	on_m_add_clicked();
}

void ChooseTuningSignalsWidget::on_m_filterValuesTree_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	on_m_setValue_clicked();
}

void ChooseTuningSignalsWidget::baseSortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_baseModel->sort(column, order);
}

void ChooseTuningSignalsWidget::on_m_baseFilterTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillBaseSignalsList();
}

void ChooseTuningSignalsWidget::on_m_baseFilterValueCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillBaseSignalsList();
}

void ChooseTuningSignalsWidget::on_m_baseFilterText_returnPressed()
{
	fillBaseSignalsList();
}

void ChooseTuningSignalsWidget::on_m_moveUp_clicked()
{

}

void ChooseTuningSignalsWidget::on_m_moveDown_clicked()
{

}

void ChooseTuningSignalsWidget::on_m_baseSignalTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillBaseSignalsList();
}


void ChooseTuningSignalsWidget::setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterSignal& value)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setData(static_cast<int>(Columns::AppSignalID), Qt::UserRole, value.appSignalHash());

	if (m_signalManager->signalExists(value.appSignalHash()) == false)
	{
		QStringList l;
		l.push_back("?");
		l.push_back(value.appSignalId());
		l.push_back("?");
		l.push_back("?");
		l.push_back("?");

		int i = 0;
		for (auto s : l)
		{
			item->setText(i++, s);
		}

		return;
	}

	AppSignalParam asp;
	if (m_signalManager->signalParam(value.appSignalHash(), &asp) == false)
	{
		assert(false);
		return;
	}

	QStringList l;
	l.push_back(asp.customSignalId());
	l.push_back(value.appSignalId());
	l.push_back(asp.tuningDefaultValue().tuningValueTypeString());
	l.push_back(asp.caption());
	if (value.useValue() == true)
	{
		if (asp.isAnalog() == false)
		{
			l.push_back(value.value().toString());
		}
		else
		{
			l.push_back(value.value().toString(asp.precision()));
		}
	}
	else
	{
		l.push_back("<Default>");
	}

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

//
// TuningFilterEditor
//

TuningFilterEditor::TuningFilterEditor(TuningFilterStorage* filterStorage, TuningSignalManager* signalManager,
									   bool readOnly,
									   bool requestValuesEnabled,
									   bool typeTreeEnabled,
									   bool typeButtonEnabled,
									   bool typeTabEnabled,
									   bool typeCounterEnabled,
									   TuningFilter::Source source,
									   QByteArray mainSplitterState,
									   int propertyEditorSplitterPos):
	m_filterStorage(filterStorage),
	m_signalManager(signalManager),
	m_readOnly(readOnly),
	m_requestValuesEnabled(requestValuesEnabled),
	m_typeTreeEnabled(typeTreeEnabled),
	m_typeButtonEnabled(typeButtonEnabled),
	m_typeTabEnabled(typeTabEnabled),
	m_typeCounterEnabled(typeCounterEnabled),
	m_source(source)
{

	assert(filterStorage);
	assert(m_signalManager);

	initUserInterface(mainSplitterState, propertyEditorSplitterPos);


	// Add presets to tree
	//

	for (int i = 0; i < m_filterStorage->root()->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = m_filterStorage->root()->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			return;
		}

		if (f->source() != m_source)
        {
            continue;
        }

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		addChildTreeObjects(f, item);

		m_presetsTree->addTopLevelItem(item);
	}

	// Set column width

	for (int i = 0; i < m_presetsTree->columnCount(); i++)
	{
		m_presetsTree->resizeColumnToContents(i);

		if (m_presetsTree->columnWidth(i) < 200)
		{
			m_presetsTree->setColumnWidth(i, 200);
		}
	}

	//
}

TuningFilterEditor::~TuningFilterEditor()
{
}


void TuningFilterEditor::saveUserInterfaceSettings(QByteArray* mainSplitterState, int* propertyEditorSplitterPos)
{
	if (mainSplitterState == nullptr || propertyEditorSplitterPos == nullptr)
	{
		Q_ASSERT(mainSplitterState);
		Q_ASSERT(propertyEditorSplitterPos);
		return;
	}

	if (m_hSplitter != nullptr)
	{
		*mainSplitterState = m_hSplitter->saveState();
	}
	if (m_propertyEditor != nullptr)
	{
		*propertyEditorSplitterPos = m_propertyEditor->splitterPosition();
	}
}

bool TuningFilterEditor::eventFilter(QObject *obj, QEvent *event)
{
	// Filter Enter key press from PropertyEditor, it will close the dialog

	if (obj == m_propertyEditor && event->type() == QEvent::KeyPress)
	{
		return true;
	}
	else
	{
		return QWidget::eventFilter(obj, event);
	}
}

void TuningFilterEditor::on_m_addPreset_clicked()
{

	// Get the type of selected filter
	//
	std::shared_ptr<TuningFilter> selectedFilter = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	if (selectedItems.isEmpty() == false)
	{
		QTreeWidgetItem* item = selectedItems[0];

		selectedFilter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

		if (selectedFilter == nullptr)
		{
			assert(selectedFilter);
			return;
		}
	}

	// Allow items

	bool allowTree = (selectedFilter == nullptr || selectedFilter->interfaceType() == TuningFilter::InterfaceType::Tree);
	bool allowTabs = (selectedFilter == nullptr || selectedFilter->interfaceType() == TuningFilter::InterfaceType::Button);
	bool allowButtons = (selectedFilter == nullptr || selectedFilter->interfaceType() == TuningFilter::InterfaceType::Tab);
	bool allowCounters = (selectedFilter == nullptr);

	if (selectedFilter == nullptr)
	{
		// Buttons and Tabs can't be both added to top level

		for (int i = 0; i < m_presetsTree->topLevelItemCount(); i++)
		{
			 QTreeWidgetItem* item = m_presetsTree->topLevelItem(i);

			 std::shared_ptr<TuningFilter> f = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

			 if (f == nullptr)
			 {
				 assert(f);
				 return;
			 }

			 if (f->interfaceType() == TuningFilter::InterfaceType::Button)
			 {
				 allowTabs = false;
			 }

			 if (f->interfaceType() == TuningFilter::InterfaceType::Tab)
			 {
				 allowButtons = false;
			 }
		}
	}

	// Disable menu items for custom editor type

	allowTree &= m_typeTreeEnabled;
	allowTabs &= m_typeTabEnabled;
	allowButtons &= m_typeButtonEnabled;
	allowCounters &= m_typeCounterEnabled;

	if (m_typeTabEnabled == false && m_typeButtonEnabled == false && m_typeCounterEnabled == false && allowTree == true)
	{
		// This is made for TuningClient

		addPreset(TuningFilter::InterfaceType::Tree);
		return;
	}

	// Create menu

	QMenu menu(this);

	{
		// Tree
		QAction* action = new QAction(tr("Tree"), &menu);

		auto f = [this]() -> void
		{
				addPreset(TuningFilter::InterfaceType::Tree);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowTree);

		menu.addAction(action);
	}

	{
		// Tab
		QAction* action = new QAction(tr("Tab"), &menu);

		auto f = [this]() -> void
		{
				addPreset(TuningFilter::InterfaceType::Tab);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowTabs);

		menu.addAction(action);
	}

	{
		// Tab
		QAction* action = new QAction(tr("Button"), &menu);

		auto f = [this]() -> void
		{
				addPreset(TuningFilter::InterfaceType::Button);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowButtons);

		menu.addAction(action);
	}

	{
		// Counter
		QAction* action = new QAction(tr("Counter"), &menu);

		auto f = [this]() -> void
		{
				addPreset(TuningFilter::InterfaceType::Counter);
		};
		connect(action, &QAction::triggered, this, f);

		action->setEnabled(allowCounters);

		menu.addAction(action);
	}

	// Run the menu

	menu.exec(QCursor::pos());
}

void TuningFilterEditor::on_m_removePreset_clicked()
{
	if (QMessageBox::warning(this, tr("Remove Filter"),
							 tr("Are you sure you want to remove selected filters?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	while (true)
	{
		// Create the list of selected Presets
		//
		QList<QTreeWidgetItem*> selectedPresets;
		for (auto p : m_presetsTree->selectedItems())
		{
			selectedPresets.push_back(p);
		}

		if (selectedPresets.isEmpty() == true)
		{
			return;
		}

		// Delete the first selected preset
		//
		QTreeWidgetItem* item = selectedPresets[0];
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem == nullptr)
		{
			m_filterStorage->root()->removeChild(filter);

			QTreeWidgetItem* deleteItem = m_presetsTree->takeTopLevelItem(m_presetsTree->indexOfTopLevelItem(item));
			delete deleteItem;
		}
		else
		{
			TuningFilter* parentFilter = filter->parentFilter();
			if (parentFilter == nullptr)
			{
				assert(parentFilter);
				return;
			}
			parentFilter->removeChild(filter);

			QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
			delete deleteItem;
		}

		m_modified = true;
	}

}

void TuningFilterEditor::on_m_moveUpPreset_clicked()
{
	movePresets(-1);

}

void TuningFilterEditor::on_m_moveDownPreset_clicked()
{
	movePresets(1);
}

void TuningFilterEditor::on_m_copyPreset_clicked()
{
	std::vector<std::shared_ptr<TuningFilter>> filters;

	// Create the list of selected Presets
	//
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	for (auto p : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = p->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		filters.push_back(filter);
	}

	if (filters.empty() == true)
	{
		return;
	}

	m_filterStorage->copyToClipboard(filters);

}

void TuningFilterEditor::on_m_pastePreset_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	QTreeWidgetItem* parentItem = nullptr;

	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	if (selectedItems.isEmpty() == false)
	{
		QTreeWidgetItem* firstTreeItem = selectedItems.front();
		Q_ASSERT(firstTreeItem);

		std::shared_ptr<TuningFilter> filter = firstTreeItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			Q_ASSERT(filter);
			return;
		}

		parentFilter = filter;
		parentItem = firstTreeItem;
	}

	std::shared_ptr<TuningFilter> pastedRoot = m_filterStorage->pasteFromClipboard();

	if (pastedRoot == nullptr)
	{
		return;
	}

	int count = pastedRoot->childFiltersCount();
	if (count == 0)
	{
		return;
	}

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> newFilter = pastedRoot->childFilter(i);

		QUuid uid = QUuid::createUuid();
		newFilter->setID(uid.toString());

		QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
		setFilterItemText(newPresetItem, newFilter.get());
		newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));

		addChildTreeObjects(newFilter, newPresetItem);

		if (parentItem == nullptr || parentFilter == nullptr)
		{
			// no item was selected, add top level item
			//
			m_filterStorage->root()->addChild(newFilter);
			m_presetsTree->addTopLevelItem(newPresetItem);
		}
		else
		{
			// an item was selected, add child item
			//
			parentFilter->addChild(newFilter);

			parentItem->addChild(newPresetItem);
		}

	}

	m_modified = true;
}

void TuningFilterEditor::on_m_presetsTree_itemSelectionChanged()
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	int presetsCount = selectedItems.size();

	m_removePreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_removePresetAction->setEnabled(m_removePreset->isEnabled());

	m_copyPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_copyPresetAction->setEnabled(m_copyPreset->isEnabled());

	m_moveUpPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_moveUpPresetAction->setEnabled(m_moveUpPreset->isEnabled());

	m_moveDownPreset->setEnabled(m_readOnly == false && presetsCount > 0);
	m_moveDownPresetAction->setEnabled(m_moveDownPreset->isEnabled());

	//

	QList<std::shared_ptr<PropertyObject>> selectedFilters;

	std::shared_ptr<TuningFilter> firstFilter = nullptr;


	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		selectedFilters.push_back(filter);

		if (firstFilter == nullptr)
		{
			firstFilter = filter;
		}
	}

	if (selectedItems.size() == 1)
	{
		m_chooseTuningSignalsWidget->setFilter(firstFilter);
	}
	else
	{
		m_chooseTuningSignalsWidget->setFilter(nullptr);
	}

	m_propertyEditor->setObjects(selectedFilters);
	m_propertyEditor->setReadOnly(m_readOnly);

}

void TuningFilterEditor::on_m_presetsTree_contextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_presetsTreeContextMenu->exec(this->cursor().pos());
}

void TuningFilterEditor::presetPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	QList<std::shared_ptr<PropertyObject>> selectedFilters;

	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> selectedFilter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (selectedFilter == nullptr)
		{
			assert(selectedFilter);
			return;
		}

		selectedFilters.push_back(selectedFilter);

		for (std::shared_ptr<PropertyObject> modifiedFilter : objects)
		{
			TuningFilter* f = dynamic_cast<TuningFilter*>(modifiedFilter.get());

			if (f == nullptr)
			{
				assert(f);
				return;

			}

			f->updateOptionalProperties();

			if (selectedFilter->ID() == f->ID())
			{
				setFilterItemText(item, f);
				break;
			}
		}
	}
}

void TuningFilterEditor::slot_getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok)
{
	emit getCurrentSignalValue(appSignalHash, value, ok);
}

void TuningFilterEditor::initUserInterface(QByteArray mainSplitterState, int propertyEditorSplitterPos)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);


	// Create splitter control
	//
	m_hSplitter = new QSplitter();


	// Left part
	//

	QWidget* lw = new QWidget();
	QVBoxLayout* leftLayout = new QVBoxLayout(lw);
	leftLayout->setContentsMargins(0, 0, 0, 0);

	m_presetsTree = new QTreeWidget();
	m_presetsTree->setExpandsOnDoubleClick(false);

	QStringList headerLabels;
	headerLabels << tr("Caption");
	headerLabels << tr("Type");

	m_presetsTree->setColumnCount(headerLabels.size());
	m_presetsTree->setHeaderLabels(headerLabels);
	m_presetsTree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	connect(m_presetsTree, &QTreeWidget::itemSelectionChanged, this, &TuningFilterEditor::on_m_presetsTree_itemSelectionChanged);

	m_presetsTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_presetsTree, &QTreeWidget::customContextMenuRequested, this, &TuningFilterEditor::on_m_presetsTree_contextMenu);

	leftLayout->addWidget(m_presetsTree);

	QHBoxLayout* leftGridLayout = new QHBoxLayout();

	m_addPreset = new QPushButton(tr("Add Filter"));
	connect(m_addPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_addPreset_clicked);
	leftGridLayout->addWidget(m_addPreset);

	m_removePreset = new QPushButton(tr("Remove Filter"));
	m_removePreset->setEnabled(false);
	connect(m_removePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_removePreset_clicked);
	leftGridLayout->addWidget(m_removePreset);

	leftGridLayout->addStretch();

	m_moveUpPreset = new QPushButton(tr("Up"));
	connect(m_moveUpPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveUpPreset_clicked);
	m_moveUpPreset->setEnabled(false);
	leftGridLayout->addWidget(m_moveUpPreset);

	m_moveDownPreset = new QPushButton(tr("Down"));
	m_moveDownPreset->setEnabled(false);
	connect(m_moveDownPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveDownPreset_clicked);
	leftGridLayout->addWidget(m_moveDownPreset);

	leftGridLayout->addStretch();

	m_copyPreset = new QPushButton(tr("Copy"));
	m_copyPreset->setEnabled(false);
	connect(m_copyPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_copyPreset_clicked);
	leftGridLayout->addWidget(m_copyPreset);

	m_pastePreset = new QPushButton(tr("Paste"));
	connect(m_pastePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_pastePreset_clicked);
	leftGridLayout->addWidget(m_pastePreset);

	leftLayout->addLayout(leftGridLayout);

	m_addPresetAction = new QAction(tr("Add Filter"), this);
	connect(m_addPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_addPreset_clicked);

	m_removePresetAction = new QAction(tr("Remove Filter"), this);
	connect(m_removePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_removePreset_clicked);

	m_moveUpPresetAction = new QAction(tr("Move Up"), this);
	connect(m_moveUpPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_moveUpPreset_clicked);

	m_moveDownPresetAction = new QAction(tr("Move Down"), this);
	connect(m_moveDownPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_moveDownPreset_clicked);

	m_copyPresetAction = new QAction(tr("Copy"), this);
	connect(m_copyPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_copyPreset_clicked);

	m_pastePresetAction = new QAction(tr("Paste"), this);
	connect(m_pastePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_pastePreset_clicked);

	m_presetsTreeContextMenu = new QMenu(this);
	m_presetsTreeContextMenu->addAction(m_addPresetAction);
	m_presetsTreeContextMenu->addAction(m_removePresetAction);
	m_presetsTreeContextMenu->addSeparator();
	m_presetsTreeContextMenu->addAction(m_moveUpPresetAction);
	m_presetsTreeContextMenu->addAction(m_moveDownPresetAction);
	m_presetsTreeContextMenu->addSeparator();
	m_presetsTreeContextMenu->addAction(m_copyPresetAction);
	m_presetsTreeContextMenu->addAction(m_pastePresetAction);
	//

	m_hSplitter->addWidget(lw);

	// Right side

	QWidget* rw = new QWidget();
	QVBoxLayout* rightLayout = new QVBoxLayout(rw);
	rightLayout->setContentsMargins(0, 0, 0, 0);

	QTabWidget* tab = new QTabWidget();
	rightLayout->addWidget(tab);

	// Properties Tab

	QWidget* propertyTabWidget = new QWidget();

	QHBoxLayout* propertyEditorLayout = new QHBoxLayout(propertyTabWidget);

	m_propertyEditor = new ExtWidgets::PropertyEditor(propertyTabWidget);

	propertyEditorLayout->addWidget(m_propertyEditor);

	m_propertyEditor->installEventFilter(this);

	if (propertyEditorSplitterPos > 100)
	{
		m_propertyEditor->setSplitterPosition(propertyEditorSplitterPos);
	}
	else
	{
		m_propertyEditor->setSplitterPosition(100);
	}

	m_hSplitter->restoreState(mainSplitterState);

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &TuningFilterEditor::presetPropertiesChanged);

	tab->addTab(propertyTabWidget, tr("Properties"));

	// Signals Tab

	m_chooseTuningSignalsWidget = new ChooseTuningSignalsWidget(m_signalManager, m_requestValuesEnabled, this);

	connect(m_chooseTuningSignalsWidget, &ChooseTuningSignalsWidget::getCurrentSignalValue, this, &TuningFilterEditor::slot_getCurrentSignalValue, Qt::DirectConnection);

	tab->addTab(m_chooseTuningSignalsWidget, tr("Signals"));

	//

	m_hSplitter->addWidget(rw);

	//

	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(m_hSplitter);

	setLayout(mainLayout);
}

void TuningFilterEditor::addPreset(TuningFilter::InterfaceType interfaceType)
{
	std::shared_ptr<TuningFilter> newFilter = std::make_shared<TuningFilter>(interfaceType);

	QUuid uid = QUuid::createUuid();
	newFilter->setID(uid.toString());
	newFilter->setCaption(tr("New Filter"));
	newFilter->setSource(m_source);

	QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
	setFilterItemText(newPresetItem, newFilter.get());
	newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));

	QTreeWidgetItem* parentItem = nullptr;
	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	if (selectedItems.isEmpty() == false)
	{
		parentItem = selectedItems[0];

		parentFilter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (parentFilter == nullptr)
		{
			assert(parentFilter);
			return;
		}
	}

	if (parentItem == nullptr || parentFilter == nullptr)
	{
		// no item was selected, add top level item
		//
		m_filterStorage->root()->addChild(newFilter);

		m_presetsTree->addTopLevelItem(newPresetItem);

		newPresetItem->setSelected(true);
	}
	else
	{
		// an item was selected, add child item
		//
		parentFilter->addChild(newFilter);

		parentItem->addChild(newPresetItem);

		parentItem->setExpanded(true);

		parentItem->setSelected(false);

		newPresetItem->setSelected(true);
	}

	m_modified = true;
}


void TuningFilterEditor::addChildTreeObjects(const std::shared_ptr<TuningFilter>& filter, QTreeWidgetItem* parent)
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

	// Add child presets
	//
	for (int i = 0; i < filter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->source() != m_source)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		addChildTreeObjects(f, item);

		parent->addChild(item);
	}
}

void TuningFilterEditor::setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter)
{
	if (item == nullptr || filter == nullptr)
	{
		assert(item);
		assert(filter);
		return;
	}



	QStringList l;
	l << filter->caption();
	l << E::valueToString<TuningFilter::InterfaceType>(filter->interfaceType());

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}


void TuningFilterEditor::movePresets(int direction)
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	// Check if selected items have same parent
	//

	bool first = true;

	QTreeWidgetItem* parentItem = nullptr;

	for (auto selectedItem : selectedItems)
	{
		if (first == true)
		{
			first = false;
			parentItem = selectedItem->parent();
		}
		else
		{
			if (parentItem != selectedItem->parent())
			{
				QMessageBox::critical(this, tr("Filter Editor"), tr("To change presets order, select presets of the same parent!"));
				return;
			}
		}
	}

	// Remember indexes
	//
	std::vector<int> selectedIndexes;

	for (auto selectedItem : selectedItems)
	{
		if (parentItem == nullptr)
		{
			 selectedIndexes.push_back(m_presetsTree->indexOfTopLevelItem(selectedItem));
		}
		else
		{
			selectedIndexes.push_back(parentItem->indexOfChild(selectedItem));
		}
	}

	// Sort indexes
	//
	if (direction < 0)
	{
		std::sort(selectedIndexes.begin(), selectedIndexes.end(), std::less<int>());
	}
	else
	{
		std::sort(selectedIndexes.begin(), selectedIndexes.end(), std::greater<int>());
	}

	// Change order
	//
	std::shared_ptr<TuningFilter> parentFilter = nullptr;

	if (parentItem == nullptr)
	{
		parentFilter = m_filterStorage->root();
	}
	else
	{
		parentFilter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	}

	if (parentFilter == nullptr)
	{
		Q_ASSERT(parentFilter);
		return;
	}

	int maxIndex = parentItem == nullptr ? m_presetsTree->topLevelItemCount() : parentItem->childCount();

	for (int index : selectedIndexes)
	{
		int newIndex = index + direction;
		if (newIndex < 0 || newIndex >= maxIndex)
		{
			return;
		}

		// Swap filters

		std::shared_ptr<TuningFilter> childFilter = parentFilter->childFilter(index);
		if (childFilter == nullptr)
		{
			Q_ASSERT(childFilter);
			return;
		}

		parentFilter->removeChild(childFilter);
		parentFilter->insertChild(newIndex, childFilter);

		// Swap tree items

		if (parentItem == nullptr)
		{
			// These are top level items
			//

			QTreeWidgetItem* takeItem = m_presetsTree->takeTopLevelItem(index);
			m_presetsTree->insertTopLevelItem(newIndex, takeItem);
			takeItem->setSelected(true);
		}
		else
		{
			QTreeWidgetItem* takeItem = parentItem->takeChild(index);
			parentItem->insertChild(newIndex, takeItem);
			takeItem->setSelected(true);
		}
	}
}


