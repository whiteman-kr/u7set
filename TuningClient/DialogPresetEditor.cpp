#include "DialogPresetEditor.h"
#include "DialogPresetProperties.h"
#include "ui_DialogPresetEditor.h"


//
//
//

DialogPresetEditor::DialogPresetEditor(ObjectFilterStorage *filterStorage, QWidget *parent) :
	QDialog(parent),
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

	int count = m_filterStorage->topFilterCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<ObjectFilter> f = m_filterStorage->topFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTree() == false)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setTreeItemText(item, f.get());
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

	// Objects
	//

	m_model = new TuningItemModel(this);
	m_model->addColumn(TuningItemModel::TuningPageColumns::CustomAppSignalID);
	m_model->addColumn(TuningItemModel::TuningPageColumns::AppSignalID);
	m_model->addColumn(TuningItemModel::TuningPageColumns::EquipmentID);
	m_model->addColumn(TuningItemModel::TuningPageColumns::Caption);
	//m_model->addColumn(TuningItemModel::TuningPageColumns::Value);

	ui->m_signalsTable->setModel(m_model);

	ui->m_signalsTable->verticalHeader()->hide();
	ui->m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	ui->m_signalsTable->verticalHeader()->setDefaultSectionSize(16);
	ui->m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	fillObjectsList();

	ui->m_signalsTable->resizeColumnsToContents();
}

DialogPresetEditor::~DialogPresetEditor()
{
	delete ui;
}

void DialogPresetEditor::fillObjectsList()
{
	m_objectsIndexes.clear();

	SignalType signalType = SignalType::All;
	QVariant data = ui->m_signalTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		signalType = static_cast<SignalType>(data.toInt());
	}

	for (int i = 0; i < theObjects.objectsCount(); i++)
	{
		TuningObject o = theObjects.object(i);

		if (signalType == SignalType::Analog && o.analog() == false)
		{
			continue;
		}

		if (signalType == SignalType::Discrete && o.analog() == true)
		{
			continue;
		}

		m_objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(m_objectsIndexes);
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

void DialogPresetEditor::addChildTreeObjects(const std::shared_ptr<ObjectFilter>& filter, QTreeWidgetItem* parent)
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
		std::shared_ptr<ObjectFilter> f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setTreeItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		item->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

		addChildTreeObjects(f, item);

		parent->addChild(item);
	}

	//add signal items
	//
	QStringList apsList = filter->appSignalIdsList();

	QList<QTreeWidgetItem*> children;
	for (auto aps : apsList)
	{
		ObjectFilterValue ofv;

		QStringList l;
		l.push_back("-");
		l.push_back("Signal");
		l.push_back(aps);
		l.push_back("Caption");
		l.push_back(tr("0/Yes"));

		QTreeWidgetItem* childItem = new QTreeWidgetItem(l);
		childItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Signal));
		childItem->setData(2, Qt::UserRole, QVariant::fromValue(ofv));

		children.push_back(childItem);
	}

	parent->addChildren(children);
}

void DialogPresetEditor::setTreeItemText(QTreeWidgetItem* item, ObjectFilter* filter)
{
	if (item == nullptr || filter == nullptr)
	{
		assert(item);
		assert(filter);
		return;
	}

	QStringList l;
	l << filter->caption();
	l.append(filter->folder() ? tr("Folder") : tr("Preset"));

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

void DialogPresetEditor::on_m_addPreset_clicked()
{
	QTreeWidgetItem* parentItem = ui->m_presetsTree->currentItem();
	if (parentItem != nullptr && isFilter(parentItem) == false)
	{
		return;
	}

	std::shared_ptr<ObjectFilter> newFilter = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);

	QUuid uid = QUuid::createUuid();
	newFilter->setStrID(uid.toString());
	newFilter->setCaption("New Filter");

	QTreeWidgetItem* item = new QTreeWidgetItem();
	setTreeItemText(item, newFilter.get());
	item->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));
	item->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

	if (parentItem != nullptr)
	{
		std::shared_ptr<ObjectFilter> parentFilter = parentItem->data(0, Qt::UserRole).value<std::shared_ptr<ObjectFilter>>();
		parentFilter->addChild(newFilter);

		parentItem->addChild(item);
		parentItem->setExpanded(true);
	}
	else
	{
		m_filterStorage->addTopFilter(newFilter);
		ui->m_presetsTree->addTopLevelItem(item);
	}

	m_modified = true;
}

void DialogPresetEditor::on_m_editPreset_clicked()
{
	QTreeWidgetItem* item = ui->m_presetsTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	if (isFilter(item) == false)
	{
		return;
	}

	std::shared_ptr<ObjectFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<ObjectFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	DialogPresetProperties d(filter, this);
	if (d.exec() == QDialog::Accepted)
	{
		setTreeItemText(item, filter.get());

		m_modified = true;
	}
}

void DialogPresetEditor::on_m_removePreset_clicked()
{
	if (QMessageBox::warning(this, tr("Remove Preset"),
							 tr("Are you sure you want to remove selected presets?"),
							 QMessageBox::StandardButton::Yes,
							 QMessageBox::StandardButton::No) != QMessageBox::StandardButton::Yes)
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

		std::shared_ptr<ObjectFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<ObjectFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		if (m_filterStorage->removeFilter(filter) == true)
		{
			QTreeWidgetItem* parent = item->parent();
			if (parent != nullptr)
			{
				QTreeWidgetItem* deleteItem = parent->takeChild(parent->indexOfChild(item));
				delete deleteItem;
			}
			else
			{
				QTreeWidgetItem* deleteItem = ui->m_presetsTree->takeTopLevelItem(ui->m_presetsTree->indexOfTopLevelItem(item));
				delete deleteItem;
			}
		}
		else
		{
			assert(false);
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

		QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
		delete deleteItem;
	}
}

void DialogPresetEditor::on_m_presetsTree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_m_editPreset_clicked();
}

void DialogPresetEditor::on_m_signalTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}
