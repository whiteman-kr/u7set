#include "TuningFilterEditor.h"
#include "../lib/PropertyEditorDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>

//
// DialogChooseTuningSignals
//

DialogChooseTuningSignals::DialogChooseTuningSignals(TuningSignalManager* signalStorage, std::shared_ptr<TuningFilter> filter, bool setCurrentEnabled, QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
		m_signalManager(signalStorage),
		m_filter(filter)
{

	m_filterValues = m_filter->getValues();

	// Left part
	//

	QHBoxLayout* mainHorzLayout = new QHBoxLayout();

	QVBoxLayout* leftLayout = new QVBoxLayout();

	m_baseSignalsTable = new QTableView();
	m_baseSignalsTable->verticalHeader()->hide();
	m_baseSignalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_baseSignalsTable->verticalHeader()->setDefaultSectionSize(16);
	m_baseSignalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_baseSignalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_baseSignalsTable->setSortingEnabled(true);
	connect(m_baseSignalsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogChooseTuningSignals::baseSortIndicatorChanged);
	connect(m_baseSignalsTable, &QTableView::doubleClicked, this, &DialogChooseTuningSignals::on_m_baseSignalsTable_doubleClicked);
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
	connect(m_baseFilterText, &QLineEdit::returnPressed, this, &DialogChooseTuningSignals::on_m_baseFilterText_returnPressed);
	leftFilterLayout->addWidget(m_baseFilterText);

	m_baseApplyFilter = new QPushButton(tr("Apply Filter"));
	connect(m_baseApplyFilter, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_baseApplyFilter_clicked);
	leftFilterLayout->addWidget(m_baseApplyFilter);

	leftLayout->addLayout(leftFilterLayout);

	mainHorzLayout->addLayout(leftLayout);

	// Middle part
	//

	QVBoxLayout* midLayout = new QVBoxLayout();

	midLayout->addStretch();

	m_addValue = new QPushButton(tr("Add"));
	connect(m_addValue, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_add_clicked);
	midLayout->addWidget(m_addValue);

	m_removeValue = new QPushButton(tr("Remove"));
	connect(m_removeValue, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_remove_clicked);
	midLayout->addWidget(m_removeValue);

	midLayout->addStretch();

	mainHorzLayout->addLayout(midLayout);

	// Right part
	//

	QVBoxLayout* rightLayout = new QVBoxLayout();

	m_filterValuesTree = new QTreeWidget();
	m_filterValuesTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_filterValuesTree, &QTreeWidget::doubleClicked, this, &DialogChooseTuningSignals::on_m_filterValuesTree_doubleClicked);

	QStringList l;
	l << tr("Custom AppSignal Id");
	l << tr("App Signal Id");
	l << tr("Type");
	l << tr("Caption");
	l << tr("Value");

	m_filterValuesTree->setColumnCount(l.size());
	m_filterValuesTree->setHeaderLabels(l);
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
	connect(m_setValue, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_setValue_clicked);
	rightGridLayout->addWidget(m_setValue);

	if (setCurrentEnabled == true)
	{
		m_setCurrent = new QPushButton(tr("Set Current"));
		m_setCurrent->setEnabled(false);
		connect(m_setCurrent, &QPushButton::clicked, this, &DialogChooseTuningSignals::on_m_setCurrent_clicked);
		rightGridLayout->addWidget(m_setCurrent);
	}

	m_moveUpAction = new QAction(tr("Move Up"), this);
	connect(m_moveUpAction, &QAction::triggered, this, &DialogChooseTuningSignals::on_m_moveUp_clicked);

	m_moveDownAction = new QAction(tr("Move Down"), this);
	connect(m_moveDownAction, &QAction::triggered, this, &DialogChooseTuningSignals::on_m_moveDown_clicked);

	m_setValueAction = new QAction(tr("Set Value"), this);
	connect(m_setValueAction, &QAction::triggered, this, &DialogChooseTuningSignals::on_m_setValue_clicked);

	m_setCurrentAction = new QAction(tr("Set Current"), this);
	connect(m_setCurrentAction, &QAction::triggered, this, &DialogChooseTuningSignals::on_m_setCurrent_clicked);

	rightLayout->addLayout(rightGridLayout);

	mainHorzLayout->addLayout(rightLayout);

	//

	QHBoxLayout* m_okCancelLayout = new QHBoxLayout();

	m_okCancelLayout->addStretch();

	m_buttonOk = new QPushButton(tr("OK"));
	connect(m_buttonOk, &QPushButton::clicked, this, &DialogChooseTuningSignals::accept);
	m_okCancelLayout->addWidget(m_buttonOk);

	m_buttonCancel = new QPushButton(tr("Cancel"));
	connect(m_buttonCancel, &QPushButton::clicked, this, &DialogChooseTuningSignals::reject);
	m_okCancelLayout->addWidget(m_buttonCancel);

	//

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(mainHorzLayout);
	mainLayout->addLayout(m_okCancelLayout);

	setLayout(mainLayout);

	// Objects and model
	//
	m_baseModel = new TuningModel(m_signalManager, this);

	m_baseModel->addColumn(TuningModel::Columns::CustomAppSignalID);
	m_baseModel->addColumn(TuningModel::Columns::AppSignalID);
	m_baseModel->addColumn(TuningModel::Columns::Type);
	m_baseModel->addColumn(TuningModel::Columns::Caption);
	m_baseModel->addColumn(TuningModel::Columns::Default);

	m_baseSignalsTable->setModel(m_baseModel);

	fillBaseSignalsList();

	fillFilterValuesList();

	// Set column width
	//

	m_baseSignalsTable->resizeColumnsToContents();

}

void DialogChooseTuningSignals::accept()
{
	m_filter->setValues(m_filterValues);

	QDialog::accept();
}


void DialogChooseTuningSignals::fillBaseSignalsList()
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

void DialogChooseTuningSignals::fillFilterValuesList()
{
	m_filterValuesTree->clear();

	for (const TuningFilterValue& v: m_filterValues)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterValueItemText(item, v);
		m_filterValuesTree->addTopLevelItem(item);
	}
}

void DialogChooseTuningSignals::on_m_add_clicked()
{
	std::vector<Hash> baseHashes  = m_baseModel->hashes();

	for (const QModelIndex& index : m_baseSignalsTable->selectionModel()->selectedRows())
	{
		Hash hash = baseHashes[index.row()];

		bool ok = false;

		const AppSignalParam p = m_signalManager->signalParam(hash, &ok);
		const TuningSignalState s = m_signalManager->state(hash, &ok);

		bool alreadyExists = false;

		for (const TuningFilterValue& v : m_filterValues)
		{
			if (v.appSignalHash() == p.hash())
			{
				alreadyExists = true;
				break;
			}

		}

		if (alreadyExists == true)
		{
			continue;
		}

		// Create value

		TuningFilterValue ofv;
		ofv.setAppSignalId(p.appSignalId());
		if (s.valid() == true)
		{
			ofv.setValue(s.value());
		}

		m_filterValues.push_back(ofv);

		// Add tree item

		QTreeWidgetItem* childItem = new QTreeWidgetItem();

		setFilterValueItemText(childItem, ofv);

		m_filterValuesTree->addTopLevelItem(childItem);
	}
}

void DialogChooseTuningSignals::on_m_remove_clicked()
{

	QModelIndexList selectedRows = m_filterValuesTree->selectionModel()->selectedRows();

	for (int i = selectedRows.size() - 1; i >= 0; i--)
	{
		int index = selectedRows[i].row();

		m_filterValues.erase(m_filterValues.begin() + index);

		QTreeWidgetItem* deleteItem = m_filterValuesTree->takeTopLevelItem(index);
		delete deleteItem;
	}
}

void DialogChooseTuningSignals::on_m_setValue_clicked()
{
	bool first = true;
	bool analog = false;
	float lowLimit = 0.0;
	float highLimit = 0.0;
	int precision = 0;
	TuningValue value;
	TuningValue defaultValue;

	bool sameValue = true;

	QModelIndexList selectedRows = m_filterValuesTree->selectionModel()->selectedRows();

	for (int i = 0; i < selectedRows.size(); i++)
	{
		int index = selectedRows[i].row();

		const TuningFilterValue& fv = m_filterValues[index];

		if (m_signalManager->signalExists(fv.appSignalHash()) == false)
		{
			continue;
		}

		AppSignalParam asp;
		if (m_signalManager->signalParam(fv.appSignalHash(), &asp) == false)
		{
			assert(false);
			return;
		}

		if (first == true)
		{
			analog = asp.isAnalog();
			lowLimit = asp.lowEngineeringUnits();
			highLimit = asp.highEngineeringUnits();
			precision = asp.precision();

			value = fv.value();
			value.type = asp.toTuningType();

			defaultValue = TuningValue(asp.tuningDefaultValue(), asp.toTuningType());

			first = false;
		}
		else
		{
			if (analog != asp.isAnalog())
			{
				QMessageBox::warning(this, tr("Preset Editor"), tr("Please select signals of same type (analog or discrete)."));
				return;
			}

			if (analog == true)
			{
				if (lowLimit != asp.lowEngineeringUnits() || highLimit != asp.highEngineeringUnits())
				{
					QMessageBox::warning(this, tr("Preset Editor"), tr("Selected signals have different input range."));
					return;
				}
			}

			TuningValue checkDefaultValue(asp.tuningDefaultValue(), asp.toTuningType());

			if (checkDefaultValue.type != defaultValue.type || checkDefaultValue != defaultValue)
			{
				QMessageBox::warning(this, tr("Preset Editor"), tr("Selected signals have different default value."));
				return;
			}

			if (fv.value() != value)
			{
				sameValue = false;
			}
		}
	}

	DialogInputTuningValue d(analog, value, defaultValue, sameValue, lowLimit, highLimit, precision, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	for (int i = 0; i < selectedRows.size(); i++)
	{
		int index = selectedRows[i].row();

		TuningFilterValue& fv = m_filterValues[index];

		fv.setUseValue(true);
		fv.setValue(d.value());

		QTreeWidgetItem* item = m_filterValuesTree->topLevelItem(index);
		setFilterValueItemText(item, fv);
	}
}

void DialogChooseTuningSignals::on_m_setCurrent_clicked()
{
	/*QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	for (auto item : selectedItems)
	{
		if (isSignal(item) == false)
		{
			continue;
		}

		QTreeWidgetItem* parentItem = item->parent();
		if (parentItem == nullptr)
		{
			assert(parentItem);
			return;
		}

		if (isFilter(parentItem) == false)
		{
			assert(false);
			return;
		}

		std::shared_ptr<TuningFilter> filter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		TuningFilterValue ov = item->data(2, Qt::UserRole).value<TuningFilterValue>();

		bool ok = false;
		float newValue = 0;

		emit getCurrentSignalValue(ov.appSignalHash(), &newValue, &ok);

		if (ok == true)
		{
			ov.setUseValue(true);
			ov.setValue(newValue);
		}
		else
		{
			ov.setUseValue(false);
		}

		item->setData(2, Qt::UserRole, QVariant::fromValue(ov));
		setSignalItemText(item, ov);

		filter->setValue(ov);
	}

	m_modified = true;*/

}

void DialogChooseTuningSignals::on_m_baseApplyFilter_clicked()
{
	fillBaseSignalsList();
}

void DialogChooseTuningSignals::on_m_baseSignalsTable_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	on_m_add_clicked();
}

void DialogChooseTuningSignals::on_m_filterValuesTree_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	on_m_setValue_clicked();
}

void DialogChooseTuningSignals::baseSortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_baseModel->sort(column, order);
}

void DialogChooseTuningSignals::on_m_baseFilterTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillBaseSignalsList();
}

void DialogChooseTuningSignals::on_m_baseFilterText_returnPressed()
{
	fillBaseSignalsList();
}



void DialogChooseTuningSignals::on_m_moveUp_clicked()
{

}

void DialogChooseTuningSignals::on_m_moveDown_clicked()
{

}

void DialogChooseTuningSignals::on_m_baseSignalTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillBaseSignalsList();
}


void DialogChooseTuningSignals::setFilterValueItemText(QTreeWidgetItem* item, const TuningFilterValue& value)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

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
	l.push_back(asp.isAnalog() ? tr("A") : tr("D"));
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
									   bool setCurrentEnabled, TuningFilter::Source source,
									   int propertyEditorSplitterPos,
									   const QByteArray& dialogChooseSignalGeometry):
	m_filterStorage(filterStorage),
	m_signalManager(signalManager),
	m_readOnly(readOnly),
	m_setCurrentEnabled(setCurrentEnabled),
	m_source(source),
	m_propertyEditorSplitterPos(propertyEditorSplitterPos),
	m_dialogChooseSignalGeometry(dialogChooseSignalGeometry)
{

	assert(filterStorage);
	assert(m_signalManager);

	initUserInterface();


	// Add presets to tree
	//

	for (int i = 0; i < m_filterStorage->m_root->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = m_filterStorage->m_root->childFilter(i);
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


void TuningFilterEditor::saveUserInterfaceSettings(int* propertyEditorSplitterPos, QByteArray* dialogChooseSignalGeometry)
{
	*propertyEditorSplitterPos = m_propertyEditor->splitterPosition();
	*dialogChooseSignalGeometry = m_dialogChooseSignalGeometry;
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
	std::shared_ptr<TuningFilter> parentFilter = selectedFilter(&parentItem);

	if (parentItem == nullptr || parentFilter == nullptr)
	{
		// no item was selected, add top level item
		//
		m_filterStorage->m_root->addChild(newFilter);

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

void TuningFilterEditor::initUserInterface()
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QHBoxLayout* mainHorzLayout = new QHBoxLayout();


	// Left part
	//

	QVBoxLayout* leftLayout = new QVBoxLayout();

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

	m_addPreset = new QPushButton(tr("Add Preset"));
	connect(m_addPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_addPreset_clicked);
	leftGridLayout->addWidget(m_addPreset);

	m_removePreset = new QPushButton(tr("Remove Preset"));
	m_removePreset->setEnabled(false);
	connect(m_removePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_removePreset_clicked);
	leftGridLayout->addWidget(m_removePreset);

	leftGridLayout->addStretch();

	m_copyPreset = new QPushButton(tr("Copy"));
	m_copyPreset->setEnabled(false);
	connect(m_copyPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_copyPreset_clicked);
	leftGridLayout->addWidget(m_copyPreset);

	m_pastePreset = new QPushButton(tr("Paste"));
	connect(m_pastePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_pastePreset_clicked);
	leftGridLayout->addWidget(m_pastePreset);

	leftLayout->addLayout(leftGridLayout);

	m_addPresetAction = new QAction(tr("Add Preset"), this);
	connect(m_addPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_addPreset_clicked);

	m_removePresetAction = new QAction(tr("Remove Preset"), this);
	connect(m_removePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_removePreset_clicked);

	m_copyPresetAction = new QAction(tr("Copy"), this);
	connect(m_copyPresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_copyPreset_clicked);

	m_pastePresetAction = new QAction(tr("Paste"), this);
	connect(m_pastePresetAction, &QAction::triggered, this, &TuningFilterEditor::on_m_pastePreset_clicked);

	m_presetsTreeContextMenu = new QMenu(this);
	m_presetsTreeContextMenu->addAction(m_addPresetAction);
	m_presetsTreeContextMenu->addAction(m_removePresetAction);
	m_presetsTreeContextMenu->addSeparator();
	m_presetsTreeContextMenu->addAction(m_copyPresetAction);
	m_presetsTreeContextMenu->addAction(m_pastePresetAction);
	//

	mainHorzLayout->addLayout(leftLayout);

	// Right side

	QVBoxLayout* rightLayout = new QVBoxLayout();

	m_propertyEditor = new ExtWidgets::PropertyEditor(this);

	if (m_propertyEditorSplitterPos > 100)
	{
		m_propertyEditor->setSplitterPosition(m_propertyEditorSplitterPos);
	}
	else
	{
		m_propertyEditor->setSplitterPosition(100);
	}

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &TuningFilterEditor::presetPropertiesChanged);

	rightLayout->addWidget(m_propertyEditor);

	QHBoxLayout* rightGridLayout = new QHBoxLayout();

	rightGridLayout->addStretch();

	m_presetSignals = new QPushButton(tr("Signals..."));
	m_presetSignals->setEnabled(false);
	connect(m_presetSignals, &QPushButton::clicked, this, &TuningFilterEditor::on_m_presetsSignals_clicked);
	rightGridLayout->addWidget(m_presetSignals);

	rightLayout->addLayout(rightGridLayout);

	mainHorzLayout->addLayout(rightLayout);

	//

	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addLayout(mainHorzLayout);

	setLayout(mainLayout);
}



std::shared_ptr<TuningFilter> TuningFilterEditor::selectedFilter(QTreeWidgetItem** item)
{
	if (item == nullptr)
	{
		assert(item);
		return nullptr;
	}

	*item = nullptr;

	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	if (selectedItems.empty() == true)
	{
		return nullptr;
	}

	QTreeWidgetItem* selectedItem = selectedItems[0];

	if (selectedItem == nullptr)
	{
		return nullptr;
	}

	std::shared_ptr<TuningFilter> filter = selectedItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return nullptr;
	}

	*item = selectedItem;

	return filter;

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

	// Run the menu

	menu.exec(QCursor::pos());
}

void TuningFilterEditor::on_m_removePreset_clicked()
{
	if (QMessageBox::warning(this, tr("Remove Preset"),
							 tr("Are you sure you want to remove selected presets?"),
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
			m_filterStorage->m_root->removeChild(filter);

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

	for (auto p : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = p->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		parentFilter = filter;

		parentItem = p;

		break;
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
			m_filterStorage->m_root->addChild(newFilter);
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

	m_presetSignals->setEnabled(presetsCount == 1);

	QList<std::shared_ptr<PropertyObject>> selectedFilters;

	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		selectedFilters.push_back(filter);
	}

	m_propertyEditor->setObjects(selectedFilters);
	m_propertyEditor->setReadOnly(m_readOnly);

}

void TuningFilterEditor::presetPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		std::shared_ptr<TuningFilter> selectedFilter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (selectedFilter == nullptr)
		{
			assert(selectedFilter);
			return;
		}

		for (std::shared_ptr<PropertyObject> modifiedFilter : objects)
		{
			TuningFilter* f = dynamic_cast<TuningFilter*>(modifiedFilter.get());

			if (f == nullptr)
			{
				assert(f);
				return;

			}

			if (selectedFilter->ID() == f->ID())
			{
				setFilterItemText(item, f);
				break;
			}
		}
	}
}

void TuningFilterEditor::on_m_presetsSignals_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();

	if (selectedItems.size() != 1)
	{
		return;
	}

	std::shared_ptr<TuningFilter> selectedFilter = selectedItems[0]->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();

	if (selectedFilter == nullptr)
	{
		assert(selectedFilter);
		return;
	}

	DialogChooseTuningSignals d(m_signalManager, selectedFilter, m_setCurrentEnabled, this);

	if (m_dialogChooseSignalGeometry.isEmpty() == false)
	{
		d.restoreGeometry(m_dialogChooseSignalGeometry);
	}

	if (d.exec() == QDialog::Accepted)
	{

	}

	m_dialogChooseSignalGeometry = d.saveGeometry();

}


void TuningFilterEditor::on_m_presetsTree_contextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_presetsTreeContextMenu->exec(this->cursor().pos());
}
