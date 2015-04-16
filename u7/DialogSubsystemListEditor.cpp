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
	l << tr("StrID");
	l << tr("Caption");
	ui->m_list->setHeaderLabels(l);
	ui->m_list->setColumnWidth(0, 50);
	ui->m_list->setColumnWidth(1, 100);
	ui->m_list->setColumnWidth(2, 150);

	EditorDelegate *d = new EditorDelegate(this);
	ui->m_list->setItemDelegate(d);

	// Get file from the project databse
	//

	std::shared_ptr<DbFile> file = openFile(m_fileName);
	if (file == nullptr)
	{
		return;
	}

	QByteArray data;
	file->swapData(data);

	QString errorCode;

	Hardware::SubsystemStorage subsystems;

	if (subsystems.load(data, errorCode) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load the subsystems list file ") + m_fileName + "!\r\n" + errorCode);
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

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(subsystem->index()) << subsystem->strId() << subsystem->caption());
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setData(0, Qt::UserRole, i);
		ui->m_list->insertTopLevelItem(i, item);
	}
}

DialogSubsystemListEditor::~DialogSubsystemListEditor()
{
	delete ui;
}

std::shared_ptr<DbFile> DialogSubsystemListEditor::openFile(const QString& fileName)
{
	std::vector<DbFileInfo> fileList;

	bool ok = db()->getFileList(&fileList, db()->mcFileId(), fileName, nullptr);

	if (ok == false || fileList.size() != 1)
	{
		return nullptr;
	}

	std::shared_ptr<DbFile> result = nullptr;

	ok = db()->getLatestVersion(fileList[0], &result, nullptr);

	if (ok == false || result == nullptr)
	{
		return nullptr;
	}

	return result;
}

std::shared_ptr<DbFile> DialogSubsystemListEditor::createFile(const QString& fileName)
{
	std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
	pf->setFileName(fileName);

	if (db()->addFile(pf, db()->mcFileId(), this) == false)
	{
		QMessageBox::critical(this, "Error", "Error adding file " + m_fileName + " to the database!");
		return nullptr;
	}

	return pf;
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

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << "0" << "StrID" << "Caption");
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
		QString strId = item->text(1);
		QString caption = item->text(2);

		std::shared_ptr<Hardware::Subsystem> subsystem = std::make_shared<Hardware::Subsystem>(index, strId, caption);
		subsystems.add(subsystem);
	}

	// save to db
	//
	QByteArray data;
	if (subsystems.save(data) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save subsystems to a QByteArray."));
		return;
	}

	std::shared_ptr<DbFile> file = openFile(m_fileName);
	if (file == nullptr)
	{
		// try to create a file
		//
		file = createFile(m_fileName);

		if (file == nullptr)
		{
			QMessageBox::critical(this, QString("Error"), tr("Can't create a file ") + m_fileName + "!");
			return;
		}
	}

	file->swapData(data);

	if (db()->setWorkcopy(file, this) == false)
	{
		QMessageBox::critical(this, "Error", "Set work copy error!");
		return;
	}
}

DbController* DialogSubsystemListEditor::db()
{
	return m_dbController;
}
