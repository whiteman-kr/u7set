#include "DialogSubsystemListEditor.h"
#include "ui_DialogSubsystemListEditor.h"
#include <QMessageBox>
#include <QInputDialog>

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
	l << tr("SubsystemID");
	l << tr("Caption");
	ui->m_list->setHeaderLabels(l);
	ui->m_list->setColumnWidth(0, 50);
	ui->m_list->setColumnWidth(1, 50);
	ui->m_list->setColumnWidth(2, 100);
	ui->m_list->setColumnWidth(3, 150);

	m_editorDelegate = new EditorDelegate(this);
	ui->m_list->setItemDelegate(m_editorDelegate);

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
													subsystem->subsystemId() <<
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

bool DialogSubsystemListEditor::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, "Subsystem List Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

	if (result == QMessageBox::Yes)
	{
		if (saveChanges() == false)
		{
			return false;
		}
		return true;
	}

	if (result == QMessageBox::No)
	{
		return true;
	}

	return false;
}

bool DialogSubsystemListEditor::saveChanges()
{
	bool ok;
	QString comment = QInputDialog::getText(this, tr("Subsystem List Editor"),
											tr("Please enter comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return false;
	}
	if (comment.isEmpty())
	{
		QMessageBox::warning(this, "Subsystem List Editor", "No comment supplied!");
		return false;
	}

	Hardware::SubsystemStorage subsystems;

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return false;
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
	if (subsystems.save(db(), comment) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save subsystems."));
		return false;
	}

	m_modified = false;

	return true;
}

void DialogSubsystemListEditor::closeEvent(QCloseEvent* e)
{
	if (askForSaveChanged() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
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

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << "0" << "5" << "SubsystemID" << "Caption");
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

	m_modified = true;

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

    if (ui->m_list->topLevelItemCount() > 0 && index != -1)
    {
        ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

	m_modified = true;
}


DbController* DialogSubsystemListEditor::db()
{
	return m_dbController;
}


void DialogSubsystemListEditor::on_buttonOk_clicked()
{
	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	accept();
	return;
}

void DialogSubsystemListEditor::on_buttonCancel_clicked()
{
	if (askForSaveChanged() == true)
	{
		reject();
	}
	return;
}

void DialogSubsystemListEditor::on_m_list_itemChanged(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);
	m_modified = true;
}
