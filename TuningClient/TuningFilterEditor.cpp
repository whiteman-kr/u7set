#include "Settings.h"
#include "TuningFilterEditor.h"
#include "DialogProperties.h"
#include "DialogInputValue.h"

//
//
//

TuningFilterEditor::TuningFilterEditor(TuningFilterStorage *filterStorage, const TuningObjectStorage *objects, bool showAutomatic, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_showAutomatic(showAutomatic),
	m_filterStorage(filterStorage),
    m_objects(objects)
{

    assert(filterStorage);
    assert(m_objects);

    initUserInterface();

    // Objects and model
    //
    m_model = new TuningItemModel(this);
    m_model->addColumn(TuningItemModel::Columns::CustomAppSignalID);
    m_model->addColumn(TuningItemModel::Columns::AppSignalID);
    m_model->addColumn(TuningItemModel::Columns::Type);
    m_model->addColumn(TuningItemModel::Columns::Caption);

    m_signalsTable->setModel(m_model);

    fillObjectsList();

    m_signalsTable->resizeColumnsToContents();

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

        if (f->automatic() && m_showAutomatic == false)
        {
            continue;
        }

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setFilterItemText(item, f.get());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		item->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

        addChildTreeObjects(f, item);

        m_presetsTree->addTopLevelItem(item);
	}

    for (int i = 0; i < m_presetsTree->columnCount(); i++)
	{
        m_presetsTree->resizeColumnToContents(i);
	}

    //

    if (theSettings.m_presetEditorPos.x() != -1 && theSettings.m_presetEditorPos.y() != -1)
    {
        move(theSettings.m_presetEditorPos);
        restoreGeometry(theSettings.m_presetEditorGeometry);
    }
    else
    {
        resize(1024, 768);
    }

}

TuningFilterEditor::~TuningFilterEditor()
{
    theSettings.m_presetEditorPos = pos();
    theSettings.m_presetEditorGeometry = saveGeometry();

}

void TuningFilterEditor::initUserInterface()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* mainHorzLayout = new QHBoxLayout();

    // Left part
    //
    QVBoxLayout* leftLayout = new QVBoxLayout();

    m_signalsTable = new QTableView();
    m_signalsTable->verticalHeader()->hide();
    m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    m_signalsTable->verticalHeader()->setDefaultSectionSize(16);
    m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_signalsTable->setSortingEnabled(true);
    connect(m_signalsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningFilterEditor::sortIndicatorChanged);
    connect(m_signalsTable, &QTableView::doubleClicked, this, &TuningFilterEditor::on_m_signalsTable_doubleClicked);
    leftLayout->addWidget(m_signalsTable);

    m_signalTypeCombo = new QComboBox();
    m_signalTypeCombo->blockSignals(true);
    m_signalTypeCombo->addItem(tr("All signals"), static_cast<int>(SignalType::All));
    m_signalTypeCombo->addItem(tr("Analog signals"), static_cast<int>(SignalType::Analog));
    m_signalTypeCombo->addItem(tr("Discrete signals"), static_cast<int>(SignalType::Discrete));
    m_signalTypeCombo->setCurrentIndex(0);
    m_signalTypeCombo->blockSignals(false);
    connect(m_signalTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_m_signalTypeCombo_currentIndexChanged(int)));
    leftLayout->addWidget(m_signalTypeCombo);

    QHBoxLayout* leftFilterLayout = new QHBoxLayout();

    m_filterTypeCombo = new QComboBox();
    m_filterTypeCombo->blockSignals(true);
    m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterType::All));
    m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterType::AppSignalID));
    m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterType::CustomAppSignalID));
    m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterType::EquipmentID));
    m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterType::Caption));
    m_filterTypeCombo->setCurrentIndex(0);
    m_filterTypeCombo->blockSignals(false);
    connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(on_m_filterTypeCombo_currentIndexChanged(int)));
    leftFilterLayout->addWidget(m_filterTypeCombo);

    m_filterText = new QLineEdit();
    connect(m_filterText, &QLineEdit::returnPressed, this, &TuningFilterEditor::on_m_filterText_returnPressed);
    leftFilterLayout->addWidget(m_filterText);

    m_applyFilter = new QPushButton(tr("Apply Filter"));
    connect(m_applyFilter, &QPushButton::clicked, this, &TuningFilterEditor::on_m_applyFilter_clicked);
    leftFilterLayout->addWidget(m_applyFilter);

    leftLayout->addLayout(leftFilterLayout);

    mainHorzLayout->addLayout(leftLayout);

    // Middle part
    //

    QVBoxLayout* midLayout = new QVBoxLayout();

    midLayout->addStretch();

    m_add = new QPushButton(tr("Add"));
    m_add->setEnabled(false);
    connect(m_add, &QPushButton::clicked, this, &TuningFilterEditor::on_m_add_clicked);
    midLayout->addWidget(m_add);

    m_remove = new QPushButton(tr("Remove"));
    m_remove->setEnabled(false);
    connect(m_remove, &QPushButton::clicked, this, &TuningFilterEditor::on_m_remove_clicked);
    midLayout->addWidget(m_remove);

    midLayout->addStretch();

    mainHorzLayout->addLayout(midLayout);

    // Right part
    //

    QVBoxLayout* rightLayout = new QVBoxLayout();

    m_presetsTree = new QTreeWidget();
    m_presetsTree->setExpandsOnDoubleClick(false);

    QStringList headerLabels;
    headerLabels<<tr("Caption");
    headerLabels<<tr("Type");
    headerLabels<<tr("CustomAppSignalID");
    headerLabels<<tr("AppSignalID");
    headerLabels<<tr("Caption");
    headerLabels<<tr("Value");

    m_presetsTree->setColumnCount(headerLabels.size());
    m_presetsTree->setHeaderLabels(headerLabels);
    m_presetsTree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    connect(m_presetsTree, &QTreeWidget::itemSelectionChanged, this, &TuningFilterEditor::on_m_presetsTree_itemSelectionChanged);
    connect(m_presetsTree, &QTreeWidget::doubleClicked, this, &TuningFilterEditor::on_m_presetsTree_doubleClicked);

    rightLayout->addWidget(m_presetsTree);

    QGridLayout* rightGridLayout = new QGridLayout();

    int col = 0;

    m_addPreset = new QPushButton(tr("Add Preset"));
    connect(m_addPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_addPreset_clicked);
    rightGridLayout->addWidget(m_addPreset, 0, col);

    m_editPreset = new QPushButton(tr("Edit Preset"));
    m_editPreset->setEnabled(false);
    connect(m_editPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_editPreset_clicked);
    rightGridLayout->addWidget(m_editPreset, 1, col);

    m_removePreset = new QPushButton(tr("Remove Preset"));
    m_removePreset->setEnabled(false);
    connect(m_removePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_removePreset_clicked);
    rightGridLayout->addWidget(m_removePreset, 2, col);

    col++;

    m_copyPreset = new QPushButton(tr("Copy"));
    m_copyPreset->setEnabled(false);
    connect(m_copyPreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_copyPreset_clicked);
    rightGridLayout->addWidget(m_copyPreset, 0, col);

    m_pastePreset = new QPushButton(tr("Paste"));
    connect(m_pastePreset, &QPushButton::clicked, this, &TuningFilterEditor::on_m_pastePreset_clicked);
    rightGridLayout->addWidget(m_pastePreset, 1, col);

    col++;

    m_moveUp = new QPushButton(tr("Move Up"));
    connect(m_moveUp, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveUp_clicked);
    rightGridLayout->addWidget(m_moveUp, 0, col);

    m_moveDown = new QPushButton(tr("Move Down"));
    connect(m_moveDown, &QPushButton::clicked, this, &TuningFilterEditor::on_m_moveDown_clicked);
    rightGridLayout->addWidget(m_moveDown, 1, col);

    col++;

    rightGridLayout->setColumnStretch(col, 1);

    col++;

    m_setValue = new QPushButton(tr("Set Value"));
    m_setValue->setEnabled(false);
    connect(m_setValue, &QPushButton::clicked, this, &TuningFilterEditor::on_m_setValue_clicked);
    rightGridLayout->addWidget(m_setValue, 0, col);

    m_setCurrent = new QPushButton(tr("Set Current"));
    m_setCurrent->setEnabled(false);
    //connect(m_setCurrent, &QPushButton::clicked, this, &TuningFilterEditor::on_m_s);
    rightGridLayout->addWidget(m_setCurrent, 1, col);

    rightLayout->addLayout(rightGridLayout);

    mainHorzLayout->addLayout(rightLayout);

    //

    m_okCancelButtonBox = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel);
    connect(m_okCancelButtonBox, &QDialogButtonBox::accepted, this, &TuningFilterEditor::accept);
    connect(m_okCancelButtonBox, &QDialogButtonBox::rejected, this, &TuningFilterEditor::reject);

    mainLayout->addLayout(mainHorzLayout);
    mainLayout->addWidget(m_okCancelButtonBox);

    setLayout(mainLayout);
}

void TuningFilterEditor::fillObjectsList()
{
    if (m_objects == nullptr)
    {
        assert(m_objects);
        return;
    }

	SignalType signalType = SignalType::All;
    QVariant data = m_signalTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
		signalType = static_cast<SignalType>(data.toInt());
	}

    FilterType filterType = FilterType::AppSignalID;
    data = m_filterTypeCombo->currentData();
	if (data.isNull() == false && data.isValid() == true)
	{
        filterType = static_cast<FilterType>(data.toInt());
	}

    QString filterText = m_filterText->text().trimmed();

    std::vector<TuningObject> objects;

    for (int i = 0; i < m_objects->objectCount(); i++)
	{
        const TuningObject* o = m_objects->objectPtr(i);

        if (signalType == SignalType::Analog && o->analog() == false)
		{
			continue;
		}

        if (signalType == SignalType::Discrete && o->analog() == true)
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
                if (o->appSignalID().contains(filterText, Qt::CaseInsensitive) == true
                        ||o->customAppSignalID().contains(filterText, Qt::CaseInsensitive) == true
                        ||o->equipmentID().contains(filterText, Qt::CaseInsensitive) == true
                        ||o->caption().contains(filterText, Qt::CaseInsensitive) == true )
                {
                    filterResult = true;
                }
            }
                break;
            case FilterType::AppSignalID:
				{
                    if (o->appSignalID().contains(filterText, Qt::CaseInsensitive) == true)
                    {
                        filterResult = true;
					}
				}
				break;
            case FilterType::CustomAppSignalID:
				{
                    if (o->customAppSignalID().contains(filterText, Qt::CaseInsensitive) == true)
                    {
                        filterResult = true;
                    }
				}
				break;
            case FilterType::EquipmentID:
				{
                    if (o->equipmentID().contains(filterText, Qt::CaseInsensitive) == true)
                    {
                        filterResult = true;
                    }
				}
				break;
            case FilterType::Caption:
                {
                    if (o->caption().contains(filterText, Qt::CaseInsensitive) == true)
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

        objects.push_back(*o);
	}

    m_model->setObjects(objects);
    m_signalsTable->sortByColumn(m_sortColumn, m_sortOrder);
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

void TuningFilterEditor::getSelectedCount(int& selectedPresets, int& selectedSignals)
{
	selectedPresets = 0;
	selectedSignals = 0;

    QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
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

bool TuningFilterEditor::isFilter(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}
	TreeItemType type = static_cast<TreeItemType>(item->data(1, Qt::UserRole).toInt());
	return (type == TreeItemType::Filter);
}

bool TuningFilterEditor::isSignal(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return false;
	}
	TreeItemType type = static_cast<TreeItemType>(item->data(1, Qt::UserRole).toInt());
	return (type == TreeItemType::Signal);
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

        if (f->automatic() && m_showAutomatic == false)
        {
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

void TuningFilterEditor::setFilterItemText(QTreeWidgetItem* item, TuningFilter* filter)
{
	if (item == nullptr || filter == nullptr)
	{
		assert(item);
		assert(filter);
		return;
	}

	QStringList l;
    if (filter->automatic() == true)
    {
        l << filter->caption() + tr(" <AUTO>");
    }
    else
    {
        l << filter->caption();
    }
	l.append(tr("Preset"));

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

void TuningFilterEditor::setSignalItemText(QTreeWidgetItem* item, const TuningFilterValue& value)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

    TuningObject* object = m_objects->objectPtrByHash(value.appSignalHash());
    if (object == nullptr)
    {
        assert(object);
        return;
    }

	QStringList l;
	l.push_back("-");
    l.push_back(tr("Signal"));
    l.push_back(object->customAppSignalID());
    l.push_back(value.appSignalId());
    l.push_back(object->caption());
    if (value.useValue() == true)
	{
        if (object->analog() == false)
		{
            l.push_back(value.value() == 0 ? tr("0") : tr("1"));
		}
		else
		{
            l.push_back(QString::number(value.value(), 'f', object->decimalPlaces()));
		}
    }

	int i = 0;
	for (auto s : l)
	{
		item->setText(i++, s);
	}
}

void TuningFilterEditor::on_m_addPreset_clicked()
{
	std::shared_ptr<TuningFilter> newFilter = std::make_shared<TuningFilter>(TuningFilter::FilterType::Tree);

	QUuid uid = QUuid::createUuid();
	newFilter->setStrID(uid.toString());
    newFilter->setCaption(tr("New Filter"));

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
        m_presetsTree->addTopLevelItem(newPresetItem);
	}
	else
	{
		// an item was selected, add child item
		//
		parentFilter->addChild(newFilter);

		parentItem->addChild(newPresetItem);
        //parentItem->setExpanded(true);
	}

	m_modified = true;
}

void TuningFilterEditor::on_m_editPreset_clicked()
{
	QTreeWidgetItem* editItem = nullptr;
	std::shared_ptr<TuningFilter> editFilter = selectedFilter(&editItem);

	if (editItem == nullptr || editFilter == nullptr)
	{
		return;
	}

    bool readOnly = editFilter->automatic();

    DialogProperties d(editFilter, this, readOnly);
	if (d.exec() == QDialog::Accepted)
	{
		setFilterItemText(editItem, editFilter.get());

		m_modified = true;
	}
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
        if (isFilter(p) == true)
        {
            std::shared_ptr<TuningFilter> filter = p->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
            if (filter == nullptr)
            {
                assert(filter);
                return;
            }

            filters.push_back(filter);
        }
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
        if (isFilter(p) == true)
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
        newFilter->setStrID(uid.toString());

        QTreeWidgetItem* newPresetItem = new QTreeWidgetItem();
        setFilterItemText(newPresetItem, newFilter.get());
        newPresetItem->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));
        newPresetItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Filter));

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


void TuningFilterEditor::on_m_moveUp_clicked()
{

}

void TuningFilterEditor::on_m_moveDown_clicked()
{

}

void TuningFilterEditor::on_m_add_clicked()
{
	QList<QTreeWidgetItem*> selectedPresets;
    for (auto p : m_presetsTree->selectedItems())
	{
		if (isFilter(p) == true)
		{
			selectedPresets.push_back(p);
		}
	}

	if (selectedPresets.size() != 1)
	{
        QMessageBox::critical(this, tr("Error"), tr("Select one preset to add signals!"));
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

    for (const QModelIndex& i : m_signalsTable->selectionModel()->selectedRows())
	{
        TuningObject* o = m_model->object(i.row());

        if (o == nullptr)
        {
            assert(o);
            continue;
        }

        if (filter->valueExists(o->appSignalHash()) == true)
		{
			continue;
		}

		TuningFilterValue ofv;
        ofv.setAppSignalId(o->appSignalID());
        if (o->valid() == true)
		{
            ofv.setValue(o->value());
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

void TuningFilterEditor::on_m_remove_clicked()
{
    for (QTreeWidgetItem* item : m_presetsTree->selectedItems())
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
        filter->removeValue(ofv.appSignalHash());

		QTreeWidgetItem* deleteItem = parentItem->takeChild(parentItem->indexOfChild(item));
		delete deleteItem;
	}
}

void TuningFilterEditor::on_m_presetsTree_doubleClicked(const QModelIndex &index)
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

void TuningFilterEditor::on_m_signalTypeCombo_currentIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}

void TuningFilterEditor::on_m_presetsTree_itemSelectionChanged()
{
	int presetsCount = 0;
	int signalsCount = 0;

	getSelectedCount(presetsCount, signalsCount);

    m_editPreset->setEnabled(presetsCount == 1 && signalsCount == 0);
    m_removePreset->setEnabled(presetsCount > 0 && signalsCount == 0);

    m_copyPreset->setEnabled(presetsCount > 0 && signalsCount == 0);

    m_add->setEnabled(presetsCount == 1 && signalsCount == 0);
    m_remove->setEnabled(presetsCount == 0 && signalsCount > 0);

    m_setValue->setEnabled(presetsCount == 0 && signalsCount > 0);
}

void TuningFilterEditor::on_m_setValue_clicked()
{
    bool first = true;
    bool analog = false;
    float lowLimit = 0;
    float highLimit = 0;
    int decimalPlaces = 0;
    float firstValue = 0;

	bool sameValue = true;

    QList<QTreeWidgetItem*> selectedItems = m_presetsTree->selectedItems();
	for (auto item : selectedItems)
	{
		if (isSignal(item) == false)
		{
			continue;
		}

		TuningFilterValue ov = item->data(2, Qt::UserRole).value<TuningFilterValue>();

        TuningObject* object = m_objects->objectPtrByHash(ov.appSignalHash());
        if (object == nullptr)
        {
            assert(object);
            return;
        }

        if (first == true)
		{
            analog = object->analog();
            lowLimit = object->lowLimit();
            highLimit = object->highLimit();
            decimalPlaces = object->decimalPlaces();
            firstValue = ov.value();
            first = false;
		}
		else
		{
            if (analog != object->analog())
			{
                QMessageBox::warning(this, tr("Preset Editor"), tr("Please select signals of same type (analog or discrete)."));
				return;
			}

            if (analog == true)
            {
                if (lowLimit != object->lowLimit() || highLimit != object->highLimit())
                {
                    QMessageBox::warning(this, tr("Preset Editor"), tr("Selected signals have different input range."));
                    return;
                }
            }

            if (ov.value() != firstValue)
			{
				sameValue = false;
			}
		}
	}

    DialogInputValue d(analog, firstValue, sameValue, lowLimit, highLimit, decimalPlaces);
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

        filter->setValue(ov.appSignalHash(), ov.value());
	}

    m_modified = true;

}

void TuningFilterEditor::on_m_applyFilter_clicked()
{
	fillObjectsList();
}

void TuningFilterEditor::on_m_signalsTable_doubleClicked(const QModelIndex &index)
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

void TuningFilterEditor::slot_signalsUpdated()
{
	fillObjectsList();
}

void TuningFilterEditor::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}

void TuningFilterEditor::on_m_filterTypeCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    fillObjectsList();
}

void TuningFilterEditor::on_m_filterText_returnPressed()
{
    fillObjectsList();
}
