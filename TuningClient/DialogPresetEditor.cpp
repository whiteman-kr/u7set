#include "DialogPresetEditor.h"
#include "DialogPresetProperties.h"
#include "ui_DialogPresetEditor.h"

#include "MainWindow.h"
#include "DialogInputValue.h"

//
//
//

DialogPresetEditor::DialogPresetEditor(TuningFilterStorage *filterStorage, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_filterStorage(filterStorage),
	ui(new Ui::DialogPresetEditor)
{
	ui->setupUi(this);

	// Fill Presets Tree
	//
	ui->m_presetsTree->setExpandsOnDoubleClick(false);

	QStringList headerLabels;
	headerLabels<<"Caption";
	headerLabels<<"Type";
	headerLabels<<"AppSignalID";
	headerLabels<<"AppSignalCaption";
	headerLabels<<"Value";

	ui->m_presetsTree->setColumnCount(headerLabels.size());
	ui->m_presetsTree->setHeaderLabels(headerLabels);
	ui->m_presetsTree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

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

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		item->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));
		addChildTreeObjects(f, item);

		ui->m_presetsTree->addTopLevelItem(item);
	}

	ui->m_presetsTree->expandAll();

	for (int i = 0; i < ui->m_presetsTree->columnCount(); i++)
	{
		ui->m_presetsTree->resizeColumnToContents(i);
	}

	// Objects Filters
	//
	ui->m_signalTypeCombo->blockSignals(true);
	ui->m_signalTypeCombo->addItem("All signals", static_cast<int>(SignalType::All));
	ui->m_signalTypeCombo->addItem("Analog signals", static_cast<int>(SignalType::Analog));
	ui->m_signalTypeCombo->addItem("Discrete signals", static_cast<int>(SignalType::Discrete));
	ui->m_signalTypeCombo->setCurrentIndex(0);
	ui->m_signalTypeCombo->blockSignals(false);

	// Objects Masks
	//
	ui->m_maskTypeCombo->blockSignals(true);
	ui->m_maskTypeCombo->addItem("AppSignalID", static_cast<int>(MaskType::AppSignalID));
	ui->m_maskTypeCombo->addItem("CustomAppSignalID", static_cast<int>(MaskType::CustomAppSignalID));
	ui->m_maskTypeCombo->addItem("EquipmentID", static_cast<int>(MaskType::EquipmentID));
	ui->m_maskTypeCombo->setCurrentIndex(0);
	ui->m_maskTypeCombo->blockSignals(false);

	// Objects and model
	//
	m_model = new TuningItemModel(this);
	m_model->addColumn(TuningItemModel::Columns::CustomAppSignalID);
	m_model->addColumn(TuningItemModel::Columns::AppSignalID);
	m_model->addColumn(TuningItemModel::Columns::EquipmentID);
	m_model->addColumn(TuningItemModel::Columns::Caption);

	ui->m_signalsTable->setModel(m_model);

	ui->m_signalsTable->verticalHeader()->hide();
	ui->m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	ui->m_signalsTable->verticalHeader()->setDefaultSectionSize(16);
	ui->m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->m_signalsTable->setSortingEnabled(true);

	connect(ui->m_signalsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &DialogPresetEditor::sortIndicatorChanged);


	fillObjectsList();

	ui->m_signalsTable->resizeColumnsToContents();

	connect(theMainWindow, &MainWindow::signalsUpdated, this, &DialogPresetEditor::slot_signalsUpdated);

}

DialogPresetEditor::~DialogPresetEditor()
{
	delete ui;
}

void DialogPresetEditor::fillObjectsList()
{

	SignalType signalType = SignalType::All;
	QVariant data = ui->m_signalTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		signalType = static_cast<SignalType>(data.toInt());
	}

	MaskType maskType = MaskType::AppSignalID;
	data = ui->m_maskTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		maskType = static_cast<MaskType>(data.toInt());
	}

	QString mask = ui->m_mask->text().trimmed();
	QRegExp rx(mask);
	rx.setPatternSyntax(QRegExp::Wildcard);

	std::vector<TuningObject> objects = theObjects.objects();

	std::vector<int> objectsIndexes;

	for (int i = 0; i < objects.size(); i++)
	{
		const TuningObject& o = objects[i];

		if (signalType == SignalType::Analog && o.analog() == false)
		{
			continue;
		}

		if (signalType == SignalType::Discrete && o.analog() == true)
		{
			continue;
		}

		if (mask.isEmpty() == false)
		{
			switch (maskType)
			{
			case MaskType::AppSignalID:
				{
					if (rx.exactMatch(o.appSignalID()) == false)
					{
						continue;
					}
				}
				break;
			case MaskType::CustomAppSignalID:
				{
					if (rx.exactMatch(o.customAppSignalID()) == false)
					{
						continue;
					}
				}
				break;
			case MaskType::EquipmentID:
				{
					if (rx.exactMatch(o.equipmentID()) == false)
					{
						continue;
					}
				}
				break;
			}
		}

		objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(objects, objectsIndexes);
	ui->m_signalsTable->sortByColumn(m_sortColumn, m_sortOrder);
}

std::shared_ptr<TuningFilter> DialogPresetEditor::selectedFilter(QTreeWidgetItem** item)
{
	if (item == nullptr)
	{
		assert(item);
		return nullptr;
	}

	*item = nullptr;

	QList<QTreeWidgetItem*> selectedItems = ui->m_presetsTree->selectedItems();
	if (selectedItems.empty() == true)
	{
		return nullptr;
	}

	QTreeWidgetItem* selectedItem = selectedItems[0];

	while (selectedItem != nullptr && isFilter(selectedItem) == false)
	{
		selectedItem = selectedItem->parent();
	}

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

void DialogPresetEditor::getSelectedCount(int& selectedPresets, int& selectedSignals)
{
	selectedPresets = 0;
	selectedSignals = 0;

	QList<QTreeWidgetItem*> selectedItems = ui->m_presetsTree->selectedItems();
	for (auto item : selectedItems)
	{
		if (isFilter(item) == true)
		{
			selectedPresets++;
		}
		if (isSignal(item) == true)
		{
			selectedSignals++;
		}
	}
}

bool DialogPresetEditor::isFilter(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}
	TreeItemType type = static_cast<TreeItemType>(item->data(1, Qt::UserRole).toInt());
	return (type == TreeItemType::Filter);
}

bool DialogPresetEditor::isSignal(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}
	TreeItemType type = static_cast<TreeItemType>(item->data(1, Qt::UserRole).toInt());
	return (type == TreeItemType::Signal);
}

void DialogPresetEditor::addChildTreeObjects(const std::shared_ptr<TuningFilter>& filter, QTreeWidgetItem* parent)
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

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		item->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

		addChildTreeObjects(f, item);

		parent->addChild(item);
	}

	//Add values
	//
	std::vector<TuningFilterValue> values = filter->signalValues();

	QList<QTreeWidgetItem*> children;
	for (const TuningFilterValue& ofv : values)
	{

		QTreeWidgetItem* childItem = new QTreeWidgetItem();
		setSignalItemText(childItem, ofv);
		childItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Signal));
		childItem->setData(2, Qt::UserRole, QVariant::fromValue(ofv));

		children.push_back(childItem);
	}

	parent->addChildren(children);
}

void DialogPresetEditor::setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter)
{
	if (item == nullptr || filter == nullptr)
	{
		assert(item);
		assert(filter);
		return;
	}

	QStringList l;
	l << filter->caption();
	l.append(tr("Preset"));

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

void DialogPresetEditor::setSignalItemText(QTreeWidgetItem* item, const TuningFilterValue& value)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QStringList l;
	l.push_back("-");
	l.push_back("Signal");
	l.push_back(value.appSignalId());
	l.push_back(value.caption());
	if (value.useValue() == true)
	{
		if (value.analog() == false)
		{
			l.push_back(value.value() == 0 ? tr("No") : tr("Yes"));
		}
		else
		{
			l.push_back(QString::number(value.value(), 'f', value.decimalPlaces()));
		}
	}

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

void DialogPresetEditor::on_m_addPreset_clicked()
{
	std::shared_ptr<TuningFilter> newFilter = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);

	QUuid uid = QUuid::createUuid();
	newFilter->setStrID(uid.toString());
	newFilter->setCaption("New Filter");

	QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
	setFilterItemText(newPresetItem, newFilter.get());
	newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));
	newPresetItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

	QTreeWidgetItem* parentItem = nullptr;
	std::shared_ptr<TuningFilter> parentFilter = selectedFilter(&parentItem);

	if (parentItem == nullptr || parentFilter == nullptr)
	{
		// no item was selected, add top level item
		//
		m_filterStorage->m_root->addChild(newFilter);
		ui->m_presetsTree->addTopLevelItem(newPresetItem);
	}
	else
	{
		// an item was selected, add child item
		//
		parentFilter->addChild(newFilter);

		parentItem->addChild(newPresetItem);
		parentItem->setExpanded(true);
	}

	m_modified = true;
}

void DialogPresetEditor::on_m_editPreset_clicked()
{
	QTreeWidgetItem* editItem = nullptr;
	std::shared_ptr<TuningFilter> editFilter = selectedFilter(&editItem);

	if (editItem == nullptr || editFilter == nullptr)
	{
		return;
	}

	DialogPresetProperties d(editFilter, this);
	if (d.exec() == QDialog::Accepted)
	{
		setFilterItemText(editItem, editFilter.get());

		m_modified = true;
	}
}

void DialogPresetEditor::on_m_removePreset_clicked()
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
		for (auto p : ui->m_presetsTree->selectedItems())
		{
			if (isFilter(p) == true)
			{
				selectedPresets.push_back(p);
			}
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

			QTreeWidgetItem* deleteItem = ui->m_presetsTree->takeTopLevelItem(ui->m_presetsTree->indexOfTopLevelItem(item));
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

void DialogPresetEditor::on_m_moveUp_clicked()
{

}

void DialogPresetEditor::on_m_moveDown_clicked()
{

}

void DialogPresetEditor::on_m_add_clicked()
{
	QList<QTreeWidgetItem*> selectedPresets;
	for (auto p : ui->m_presetsTree->selectedItems())
	{
		if (isFilter(p) == true)
		{
			selectedPresets.push_back(p);
		}
	}

	if (selectedPresets.size() != 1)
	{
		QMessageBox::critical(this, "Error", "Select one preset to add signals!");
		return;
	}

	QTreeWidgetItem* presetItem = selectedPresets[0];

	if (isFilter(presetItem) == false)
	{
		return;
	}

	std::shared_ptr<TuningFilter> filter = presetItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	QList<QTreeWidgetItem*> children;

	for (const QModelIndex& i : ui->m_signalsTable->selectionModel()->selectedRows())
	{
		TuningObject o = m_model->object(i.row());

		if (filter->valueExists(o.appSignalHash()) == true)
		{
			continue;
		}

		TuningFilterValue ofv;
		ofv.setAppSignalId(o.appSignalID());
		ofv.setCaption(o.caption());
		ofv.setAnalog(o.analog());
		if (o.analog() == true)
		{
			ofv.setDecimalPlaces(o.decimalPlaces());
		}
		if (o.valid() == true)
		{
			ofv.setValue(o.value());
		}

		QTreeWidgetItem* childItem = new QTreeWidgetItem();
		setSignalItemText(childItem, ofv);
		childItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Signal));
		childItem->setData(2, Qt::UserRole, QVariant::fromValue(ofv));

		children.push_back(childItem);

		filter->addValue(ofv);
	}

	presetItem->addChildren(children);

}

void DialogPresetEditor::on_m_remove_clicked()
{
	for (QTreeWidgetItem* item : ui->m_presetsTree->selectedItems())
	{
		if (item == nullptr)
		{
			assert(item);
			return;
		}

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

		TuningFilterValue ofv = item->data(2, Qt::UserRole).value<TuningFilterValue>();
		filter->removeValue(ofv.hash());

		QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
		delete deleteItem;
	}
}

void DialogPresetEditor::on_m_presetsTree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	int presetsCount = 0;
	int signalsCount = 0;

	getSelectedCount(presetsCount, signalsCount);

	if (presetsCount == 1 && signalsCount == 0)
	{
		on_m_editPreset_clicked();
	}

	if (presetsCount == 0 && signalsCount > 0)
	{
		on_m_setValue_clicked();
	}
}

void DialogPresetEditor::on_m_signalTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}

void DialogPresetEditor::on_m_presetsTree_itemSelectionChanged()
{
	int presetsCount = 0;
	int signalsCount = 0;

	getSelectedCount(presetsCount, signalsCount);

	ui->m_editPreset->setEnabled(presetsCount == 1 && signalsCount == 0);
	ui->m_removePreset->setEnabled(presetsCount > 0 && signalsCount == 0);

	ui->m_add->setEnabled(presetsCount == 1 && signalsCount == 0);
	ui->m_remove->setEnabled(presetsCount == 0 && signalsCount > 0);

	ui->m_setValue->setEnabled(presetsCount == 0 && signalsCount > 0);
}

void DialogPresetEditor::on_m_setValue_clicked()
{
	bool first = true;
	TuningFilterValue firstValue;

	bool sameValue = true;

	QList<QTreeWidgetItem*> selectedItems = ui->m_presetsTree->selectedItems();
	for (auto item : selectedItems)
	{
		if (isSignal(item) == false)
		{
			continue;
		}

		TuningFilterValue ov = item->data(2, Qt::UserRole).value<TuningFilterValue>();

		if (first == true)
		{
			firstValue = ov;
			first = false;
		}
		else
		{
			if (ov.analog() != firstValue.analog())
			{
				QMessageBox::warning(this, "Preset Editor", "Please select signals of same type (analog or discrete).");
				return;
			}

			if (ov.value() != firstValue.value())
			{
				sameValue = false;
			}
		}
	}

	DialogInputValue d(firstValue.analog(), firstValue.value(), sameValue, firstValue.decimalPlaces());
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

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
		ov.setUseValue(true);
		ov.setValue(d.value());

		item->setData(2, Qt::UserRole, QVariant::fromValue(ov));
		setSignalItemText(item, ov);

		filter->setValue(ov.hash(), ov.value());
	}

	m_modified = true;

}

void DialogPresetEditor::on_m_applyMask_clicked()
{
	fillObjectsList();
}

void DialogPresetEditor::on_m_signalsTable_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	int presetsCount = 0;
	int signalsCount = 0;

	getSelectedCount(presetsCount, signalsCount);

	if (presetsCount == 1 && signalsCount == 0)
	{
		on_m_add_clicked();
	}
}

void DialogPresetEditor::slot_signalsUpdated()
{
	fillObjectsList();
}

void DialogPresetEditor::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}
