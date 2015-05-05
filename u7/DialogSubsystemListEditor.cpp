#include "DialogSubsystemListEditor.h"
#include "ui_DialogSubsystemListEditor.h"
#include <QMessageBox>

//
//
// EditorDelegate
//
//

EditorDelegate::EditorDelegate(QObject *parent):QItemDelegate(parent)
{
}

QWidget* EditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.column() > 0)
	{
		return QItemDelegate::createEditor(parent, option, index);
	}
	return nullptr;
}

//
//
// DialogSubsystemListEditor
//
//

DialogSubsystemListEditor::DialogSubsystemListEditor(DbController *pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSubsystemListEditor),
	m_dbController(pDbController)
{
	assert(db());

	ui->setupUi(this);

	setWindowTitle(tr("Subsystems List Editor"));

	ui->m_list->setColumnCount(3);
	QStringList l;
	l << tr("Index");
	l << tr("Key");
	l << tr("StrID");
	l << tr("Caption");
	ui->m_list->setHeaderLabels(l);
	ui->m_list->setColumnWidth(0, 50);
	ui->m_list->setColumnWidth(1, 50);
	ui->m_list->setColumnWidth(2, 100);
	ui->m_list->setColumnWidth(3, 150);

	EditorDelegate *d = new EditorDelegate(this);
	ui->m_list->setItemDelegate(d);

	QString errorCode;

	Hardware::SubsystemStorage subsystems;

	if (subsystems.load(db(), errorCode) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load subsystems!"));
		return;
	}

	for (int i = 0; i < subsystems.count(); i++)
	{
		std::shared_ptr<Hardware::Subsystem> subsystem = subsystems.get(i);
		if (subsystem == nullptr)
		{
			assert(subsystem);
			break;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(subsystem->index()) <<
													QString::number(subsystem->key()) <<
													subsystem->strId() <<
													subsystem->caption());
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setData(0, Qt::UserRole, i);
		ui->m_list->insertTopLevelItem(i, item);
	}
}

DialogSubsystemListEditor::~DialogSubsystemListEditor()
{
	delete ui;
}

void DialogSubsystemListEditor::on_m_add_clicked()
{
	int index = -1;

	QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
	if (items.size() != 1)
	{
		index = ui->m_list->topLevelItemCount();
	}
	else
	{
		index = items[0]->data(0, Qt::UserRole).toInt() + 1;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << "0" << "0" << "StrID" << "Caption");
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	ui->m_list->insertTopLevelItem(index, item);

	// Renumber indexes
	//
	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		item->setText(0, QString::number(i));
		item->setData(0, Qt::UserRole, i);
	}

	// Select the created element
	//
	ui->m_list->clearSelection();
	ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);

}

void DialogSubsystemListEditor::on_m_remove_clicked()
{
	int index = -1;

	QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
	if (items.size() != 1)
	{
		return;
	}
	else
	{
		index = items[0]->data(0, Qt::UserRole).toInt();
	}

	ui->m_list->takeTopLevelItem(index);

	// Renumber indexes
	//
	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		item->setText(0, QString::number(i));
		item->setData(0, Qt::UserRole, i);
	}

	ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void DialogSubsystemListEditor::on_DialogSubsystemListEditor_accepted()
{
	Hardware::SubsystemStorage subsystems;

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int index = item->data(0, Qt::UserRole).toInt();
		int key = item->text(1).toInt();
		QString strId = item->text(2);
		QString caption = item->text(3);

		std::shared_ptr<Hardware::Subsystem> subsystem = std::make_shared<Hardware::Subsystem>(index, key, strId, caption);
		subsystems.add(subsystem);
	}

	// save to db
	//
	if (subsystems.save(db()) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save subsystems."));
		return;
	}
}

DbController* DialogSubsystemListEditor::db()
{
	return m_dbController;
}
