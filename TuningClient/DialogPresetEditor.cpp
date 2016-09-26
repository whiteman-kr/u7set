#include "DialogPresetEditor.h"
#include "DialogPresetProperties.h"
#include "ui_DialogPresetEditor.h"

DialogPresetEditor::DialogPresetEditor(ObjectFilterStorage *filters, QWidget *parent) :
	QDialog(parent),
	m_filters(filters),
	ui(new Ui::DialogPresetEditor)
{
	ui->setupUi(this);


	int count = m_filters->topFilterCount();
	for (int i = 0; i < count; i++)
	{
		ObjectFilter* f = m_filters->topFilter(i).get();
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
		item->setData(0, Qt::UserRole, f->hash());

		addChildTreeObjects(f, item);

		ui->m_presetsTree->addTopLevelItem(item);
	}
}

DialogPresetEditor::~DialogPresetEditor()
{
	delete ui;
}

void DialogPresetEditor::addChildTreeObjects(ObjectFilter* filter, QTreeWidgetItem* parent)
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
		ObjectFilter* f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<f->caption());
		item->setData(0, Qt::UserRole, f->hash());
		parent->addChild(item);

		addChildTreeObjects(f, item);
	}
}

void DialogPresetEditor::on_m_addPreset_clicked()
{
	QTreeWidgetItem* parentItem = ui->m_presetsTree->currentItem();

	Hash hash = 0;

	if (parentItem != nullptr)
	{
		hash = parentItem->data(0, Qt::UserRole).value<Hash>();
	}

	ObjectFilter* parentFilter = nullptr;

	if (hash != 0)
	{
		parentFilter = m_filters->filter(hash).get();
	}

	std::shared_ptr<ObjectFilter> newFilter = std::make_shared<ObjectFilter>(ObjectFilter::FilterType::Tree);

	QUuid uid = QUuid::createUuid();
	newFilter->setStrID(uid.toString());
	newFilter->setCaption("New Filter");

	bool result = false;

	if (parentFilter != nullptr)
	{
		// this is child filter
		result = m_filters->addFilter(parentFilter, newFilter);
	}
	else
	{
		result = m_filters->addTopFilter(newFilter);
	}


	if (result == false)
	{
		QMessageBox::critical(this, "Error", tr("Failed to add filter: hash %1 is not unique").arg(newFilter->hash()));
		return;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<newFilter->caption());
	item->setData(0, Qt::UserRole, newFilter->hash());

	if (parentItem == nullptr)

	{
		ui->m_presetsTree->addTopLevelItem(item);
	}
	else
	{
		parentItem->addChild(item);
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

	Hash hash = item->data(0, Qt::UserRole).value<Hash>();
	if (hash == 0)
	{
		return;
	}

	std::shared_ptr<ObjectFilter> filter = m_filters->filter(hash);
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

	Hash hash = item->data(0, Qt::UserRole).value<Hash>();
	if (hash == 0)
	{
		return;
	}

	if (m_filters->removeFilter(hash) == true)
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
