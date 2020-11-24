#include "DialogTagsEditor.h"
#include "ui_DialogTagsEditor.h"

DialogTagsEditor::DialogTagsEditor(DbController* pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTagsEditor),
	m_dbController(pDbController)
{
	assert(db());

	ui->setupUi(this);

	setWindowTitle(tr("Tags Editor"));

	ui->m_list->setColumnCount(1);
	QStringList l;
	l << tr("Tag");
	ui->m_list->setHeaderLabels(l);

	QStringList tags;

	if (db()->getTags(&tags) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load tags!"));
		return;
	}

	for (int i = 0; i < static_cast<int>(tags.size()); i++)
	{
		const QString& tag = tags[i];

		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << tag);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setData(0, Qt::UserRole, i);
		ui->m_list->insertTopLevelItem(ui->m_list->topLevelItemCount(), item);
	}
}

DialogTagsEditor::~DialogTagsEditor()
{
	delete ui;
}

void DialogTagsEditor::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.20),
		   static_cast<int>(screen.height() * 0.20));
	move(screen.center() - rect().center());

	return;
}

QStringList DialogTagsEditor::getTags() const
{
	QStringList tags;

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return QStringList();
		}

		QString tag = item->text(0);

		tags.push_back(tag);
	}

	return tags;
}

bool DialogTagsEditor::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, qAppName(), "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

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

bool DialogTagsEditor::saveChanges()
{
	bool ok = false;
	QString comment = QInputDialog::getText(this, qAppName(),
											tr("Please enter comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return false;
	}
	if (comment.isEmpty())
	{
		QMessageBox::warning(this, qAppName(), "No comment supplied!");
		return false;
	}

	// save to db
	//
	QStringList tags = getTags();

	if (db()->writeTags(tags, comment) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save tags."));
		return false;
	}

	m_modified = false;

	return true;
}

void DialogTagsEditor::closeEvent(QCloseEvent* e)
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

void DialogTagsEditor::on_m_add_clicked()
{
	// Get existing tags

	QStringList tags = getTags();

	QString tag;

	while(true)
	{
		bool ok = false;
		tag = QInputDialog::getText(this, qAppName(),
												tr("Please enter tag:"), QLineEdit::Normal,
												tr("tag"), &ok);

		if (ok == false)
		{
			return;
		}

		if (std::find(tags.begin(), tags.end(), tag) == tags.end())
		{
			break;
		}

		QMessageBox::warning(this, qAppName(), "This tag already exists, please enter another one.");
	}


	// --
	//
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

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << tag);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	ui->m_list->insertTopLevelItem(index, item);

	// Renumber indexes
	//
	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* ti = ui->m_list->topLevelItem(i);
		ti->setData(0, Qt::UserRole, i);
	}

	// Select the created element
	//
	ui->m_list->clearSelection();
	ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_modified = true;

}

void DialogTagsEditor::on_m_remove_clicked()
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

	QTreeWidgetItem* deletedItem = ui->m_list->takeTopLevelItem(index);
	delete deletedItem;

	// Renumber indexes
	//
	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		item->setData(0, Qt::UserRole, i);
	}

	if (ui->m_list->topLevelItemCount() > 0 && index != -1)
	{
		ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	m_modified = true;
}


DbController* DialogTagsEditor::db()
{
	return m_dbController;
}


void DialogTagsEditor::on_buttonOk_clicked()
{
	bool hasDuplicates = false;

	QStringList tags = getTags();

	for (const QString& tag : tags)
	{
		if (tags.count(tag) > 1)
		{
			hasDuplicates = true;
			break;
		}
	}

	if (hasDuplicates == true)
	{
		auto mb = QMessageBox::warning(
					  this,
					  qAppName(),
					  tr("Warning!\n\nThere are duplicated tags in the list.\n\nAre you sure you want to continue?"),
					  QMessageBox::Yes | QMessageBox::No,
					  QMessageBox::No);

		if (mb == QMessageBox::No)
		{
			return;
		}
	}

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

void DialogTagsEditor::on_buttonCancel_clicked()
{
	if (askForSaveChanged() == true)
	{
		reject();
	}
	return;
}

void DialogTagsEditor::on_m_list_itemChanged(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);
	m_modified = true;
}
