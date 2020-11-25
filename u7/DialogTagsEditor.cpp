#include "DialogTagsEditor.h"
#include "ui_DialogTagsEditor.h"


//
//
// DialogTagsEditorDelegate
//
//
DialogTagsEditorDelegate::DialogTagsEditorDelegate(QObject *parent):QItemDelegate(parent)
{
}

QWidget* DialogTagsEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.column() == 0)
	{
		QLineEdit* edit = new QLineEdit(parent);

		QRegExp rx("^[A-Za-z][A-Za-z_\\d]*$");
		edit->setValidator(new QRegExpValidator(rx, edit));

		return edit;
	}
	else
	{
		return QItemDelegate::createEditor(parent, option, index);
	}
}

void DialogTagsEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (index.column() == 0)
	{
		QString s = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
		edit->setText(s);
	}
	else
	{
		QItemDelegate::setEditorData(editor, index);
	}
}

void DialogTagsEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if (index.column() == 0)
	{
		QLineEdit* edit = qobject_cast<QLineEdit*>(editor);
		model->setData(index, edit->text(), Qt::EditRole);
	}
	else
	{
		QItemDelegate::setModelData(editor, model, index);
	}
}

//
// DialogTagsEditor
//

DialogTagsEditor::DialogTagsEditor(DbController* pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTagsEditor),
	m_dbController(pDbController)
{
	assert(db());

	ui->setupUi(this);

	setWindowTitle(tr("Tags Editor"));

	QStringList l;
	l << tr("Tag");
	l << tr("Description");
	ui->m_list->setHeaderLabels(l);
	ui->m_list->setColumnCount(l.size());

	std::vector<DbTag> tags;

	if (db()->getTags(&tags) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load tags!"));
		return;
	}

	for (int i = 0; i < static_cast<int>(tags.size()); i++)
	{
		const DbTag& tag = tags[i];

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, tag.tag);
		item->setText(1, tag.description);
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		ui->m_list->insertTopLevelItem(ui->m_list->topLevelItemCount(), item);
	}

	ui->m_list->setItemDelegate(new DialogTagsEditorDelegate(this));

	// Delete shortcut

	QShortcut* removeShortcut = new QShortcut(QKeySequence(QKeySequence::Delete), this);
	connect(removeShortcut, &QShortcut::activated, this, &DialogTagsEditor::on_remove_shortcut);

	return;
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

	resize(static_cast<int>(screen.width() * 0.25),
		   static_cast<int>(screen.height() * 0.25));
	move(screen.center() - rect().center());

	return;
}

std::vector<DbTag> DialogTagsEditor::getTags() const
{
	std::vector<DbTag> tags;

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return {};
		}

		tags.push_back({item->text(0), item->text(1)});
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
	std::vector<DbTag> tags = getTags();

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

	std::vector<DbTag> tags = getTags();

	QString newTag;

	while(true)
	{
		bool ok = false;
		newTag = QInputDialog::getText(this, qAppName(),
												tr("Please enter tag:"), QLineEdit::Normal,
												tr("tag"), &ok);

		if (ok == false)
		{
			return;
		}

		bool hasDuplicates = false;

		for (const DbTag& dbTag : tags)
		{
			if (dbTag.tag == newTag)
			{
				hasDuplicates = true;
				break;
			}
		}

		if (hasDuplicates == true)
		{
			QMessageBox::warning(this, qAppName(), "This tag already exists, please enter another one.");
			continue;
		}

		break;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(0, newTag);
	//item->setText(1, "<Description>");
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	ui->m_list->addTopLevelItem(item);

	// Select the created element
	//
	ui->m_list->clearSelection();
	item->setSelected(true);

	m_modified = true;

}

void DialogTagsEditor::on_m_remove_clicked()
{
	QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	if (QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected tag?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
	{
		return;
	}

	int index = ui->m_list->indexOfTopLevelItem(items[0]);

	// Delete item

	QTreeWidgetItem* deletedItem = ui->m_list->takeTopLevelItem(index);
	if (deletedItem == nullptr)
	{
		Q_ASSERT(deletedItem);
		return;
	}
	delete deletedItem;

	// Select next item

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

	std::vector<DbTag> dbTags = getTags();

	QStringList allTags;

	for (const DbTag& dbTag : dbTags)
	{
		if (allTags.count(dbTag.tag) != 0)
		{
			hasDuplicates = true;
			break;
		}
		allTags.push_back(dbTag.tag);
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

void DialogTagsEditor::on_remove_shortcut()
{
	if (ui->m_list->hasFocus() == false)
	{
		return;
	}

	on_m_remove_clicked();
}

void DialogTagsEditor::on_m_up_clicked()
{
	QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	int index = ui->m_list->indexOfTopLevelItem(items[0]);
	if (index == 0)
	{
		return;
	}

	QTreeWidgetItem* item = ui->m_list->takeTopLevelItem(index);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	ui->m_list->insertTopLevelItem(index - 1, item);
	ui->m_list->clearSelection();

	item->setSelected(true);

	m_modified = true;

	return;
}

void DialogTagsEditor::on_m_down_clicked()
{
	QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	int index = ui->m_list->indexOfTopLevelItem(items[0]);
	if (index == ui->m_list->topLevelItemCount() - 1)
	{
		return;
	}

	QTreeWidgetItem* item = ui->m_list->takeTopLevelItem(index);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	ui->m_list->insertTopLevelItem(index + 1, item);
	ui->m_list->clearSelection();

	item->setSelected(true);

	m_modified = true;

	return;
}
