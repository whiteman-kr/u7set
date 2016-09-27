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

	ui->m_presetsTree->setExpandsOnDoubleClick(false);

	QStringList headerLabels;
	headerLabels<<"Caption";
	headerLabels<<"Type";
	headerLabels<<"AppSignalID";
	headerLabels<<"AppSignalCaption";
	headerLabels<<"Value";

	ui->m_presetsTree->setColumnCount(headerLabels.size());
	ui->m_presetsTree->setHeaderLabels(headerLabels);

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



	// Objects

/*	m_model = new TuningItemModel(m_tuningPageIndex, this);

	ui->m_signalsTable->setModel(m_model);

	ui->m_signalsTable->verticalHeader()->hide();
	ui->m_signalsTable->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	ui->m_signalsTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->m_signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	fillObjectsList();*/
}

DialogPresetEditor::~DialogPresetEditor()
{
	delete ui;
}
/*
void TuningPage::fillObjectsList()
{
	m_objectsIndexes.clear();

	for (int i = 0; i < theObjects.objectsCount(); i++)
	{
		TuningObject o = theObjects.object(i);

		if (m_treeFilter != nullptr)
		{
			if (m_treeFilter->folder() == true)
			{
				continue;
			}

			bool result = true;

			ObjectFilter* treeFilter = m_treeFilter.get();
			while (treeFilter != nullptr)
			{
				if (treeFilter->match(o) == false)
				{
					result = false;
					break;
				}

				treeFilter = treeFilter->parentFilter();
			}
			if (result == false)
			{
				continue;
			}
		}

		if (m_tabFilter != nullptr)
		{
			if (m_tabFilter->match(o) == false)
			{
				continue;
			}
		}

		if (m_buttonFilter != nullptr)
		{
			if (m_buttonFilter->match(o) == false)
			{
				continue;
			}
		}

		m_objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(m_objectsIndexes);
}*/

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

	// remove all signal items
	//
	int childCount = item->childCount();
	for (int i = childCount - 1; i >= 0; i--)
	{
		QTreeWidgetItem* childItem = item->child(i);
		if (childItem == nullptr)
		{
			assert(childItem);
			return;
		}

		if (isSignal(childItem) == false)
		{
			continue;
		}

		item->takeChild(i);
	}

	//add signal items
	//
	QStringList apsList = filter->appSignalIdsList();

	QList<QTreeWidgetItem*> children;
	for (auto aps : apsList)
	{
		QStringList l;
		l.push_back("-");
		l.push_back("Signal");
		l.push_back(aps);
		l.push_back("Caption");
		l.push_back(tr("0/Yes"));

		QTreeWidgetItem* childItem = new QTreeWidgetItem(l);
		childItem->setData(1, Qt::UserRole, static_cast<int>(TreeItemType::Signal));

		children.push_back(childItem);
	}

	item->addChildren(children);
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

	if (QMessageBox::warning(this, tr("Remove Preset"),
							 tr("Are you sure you want to remove preset %1?")
							 .arg(filter->caption()),
							 QMessageBox::StandardButton::Yes,
							 QMessageBox::StandardButton::No) != QMessageBox::StandardButton::Yes)
	{
		return;
	}

	if (m_filterStorage->removeFilter(filter) == true)
	{
		QTreeWidgetItem* parent = item->parent();
		if (parent != nullptr)
		{
			parent->takeChild(parent->indexOfChild(item));
		}
		else
		{
			ui->m_presetsTree->takeTopLevelItem(ui->m_presetsTree->indexOfTopLevelItem(item));
		}
	}

	m_modified = true;
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

}


void DialogPresetEditor::on_m_presetsTree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_m_editPreset_clicked();
}
