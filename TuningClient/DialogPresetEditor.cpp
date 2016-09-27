#include "DialogPresetEditor.h"
#include "DialogPresetProperties.h"
#include "ui_DialogPresetEditor.h"

DialogPresetEditor::DialogPresetEditor(ObjectFilterStorage *filterStorage, QWidget *parent) :
	QDialog(parent),
	m_filterStorage(filterStorage),
	ui(new Ui::DialogPresetEditor)
{
	ui->setupUi(this);

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

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		addChildTreeObjects(f, item);

		ui->m_presetsTree->addTopLevelItem(item);
	}
}

DialogPresetEditor::~DialogPresetEditor()
{
	delete ui;
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

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));
		parent->addChild(item);

		addChildTreeObjects(f, item);
	}
}

void DialogPresetEditor::on_m_addPreset_clicked()
{
	std::shared_ptr<ObjectFilter> newFilter = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);

	QUuid uid = QUuid::createUuid();
	newFilter->setStrID(uid.toString());
	newFilter->setCaption("New Filter");

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<newFilter->caption());
	item->setData(0, Qt::UserRole, QVariant::fromValue(newFilter));

	QTreeWidgetItem* parentItem = ui->m_presetsTree->currentItem();

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

	std::shared_ptr<ObjectFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<ObjectFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	DialogPresetProperties d(filter, this);
	if (d.exec() == QDialog::Accepted)
	{
		item->setText(0, filter->caption());

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
			parent->takeChild(parent->indexOfChild(item));
		}
		else
		{
			ui->m_presetsTree->takeTopLevelItem(ui->m_presetsTree->indexOfTopLevelItem(item));
		}
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

}


void DialogPresetEditor::on_m_presetsTree_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_m_editPreset_clicked();
}
