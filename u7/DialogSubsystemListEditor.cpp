#include "DialogSubsystemListEditor.h"
#include "ui_DialogSubsystemListEditor.h"
#include <QMessageBox>
#include <QInputDialog>
#include <set>

//
//
// SubsystemListEditorDelegate
//
//

SubsystemListEditorDelegate::SubsystemListEditorDelegate(QObject *parent):QItemDelegate(parent)
{
}

QWidget* SubsystemListEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() == DialogSubsystemListEditor::Key)
    {
        QLineEdit* edit = new QLineEdit(parent);

        QRegExp rx("[\\d]{1,2}");
        edit->setValidator(new QRegExpValidator(rx, edit));

        return edit;
    }

    if(index.column() == DialogSubsystemListEditor::SubsystemID)
    {
        QLineEdit* edit = new QLineEdit(parent);

        QRegExp rx("^[A-Za-z][A-Za-z\\d]*$");
        edit->setValidator(new QRegExpValidator(rx, edit));

        return edit;
    }

    if(index.column() == DialogSubsystemListEditor::Caption)
	{
		return QItemDelegate::createEditor(parent, option, index);
	}

    return nullptr;
}

void SubsystemListEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == DialogSubsystemListEditor::Key || index.column() == DialogSubsystemListEditor::SubsystemID)
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

void SubsystemListEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == DialogSubsystemListEditor::Key || index.column() == DialogSubsystemListEditor::SubsystemID)
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
	l << tr("Key [0-%1]").arg(Hardware::Subsystem::MaxKeyValue);
	l << tr("SubsystemID");
	l << tr("Caption");
	ui->m_list->setHeaderLabels(l);
	ui->m_list->setColumnWidth(0, 50);
    ui->m_list->setColumnWidth(1, 70);
	ui->m_list->setColumnWidth(2, 100);
    ui->m_list->setColumnWidth(3, 130);

	m_editorDelegate = new SubsystemListEditorDelegate(this);
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

	return;
}

DialogSubsystemListEditor::~DialogSubsystemListEditor()
{
	delete ui;
}

void DialogSubsystemListEditor::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(screen.width() * 0.30, screen.height() * 0.60);
	move(screen.center() - rect().center());

	// --
	//
	assert(ui->m_list);
	assert(ui->m_list->columnCount() == 4);

	ui->m_list->setColumnWidth(0, ui->m_list->width() * 0.15);
	ui->m_list->setColumnWidth(1, ui->m_list->width() * 0.15);
	ui->m_list->setColumnWidth(2, ui->m_list->width() * 0.30);
	ui->m_list->setColumnWidth(3, ui->m_list->width() * 0.30);

	return;
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
        int key = item->text(Key).toInt();
        QString strId = item->text(SubsystemID);
        QString caption = item->text(Caption);

		if (key < 0 || key > Hardware::Subsystem::MaxKeyValue)
        {
			QMessageBox::warning(this, "Subsystem List Editor", tr("Wrong key value '%1' for SubsystemID '%2'!\n\nKey value must be in range 0-%3.").arg(key).arg(strId).arg(Hardware::Subsystem::MaxKeyValue));
            return false;
        }

        // Add here other names in future
        //
        if (strId == "Reports" ||
                strId == "LogicSchemas" ||
                strId == "build.log" ||
                strId == "build.xml"
                )
        {
            QMessageBox::warning(this, "Subsystem List Editor", tr("Subsystem ID '%2' is a reserved word!").arg(strId));
            return false;
        }

        int count = subsystems.count();
        for (int s = 0; s < count; s++)
        {
            std::shared_ptr<Hardware::Subsystem> subsystemPtr = subsystems.get(s);

            if (subsystemPtr->subsystemId() == strId)
            {
                QMessageBox::warning(this, "Subsystem List Editor", tr("Subsystem ID '%2' already exists!").arg(strId));
                return false;
            }

            if (subsystemPtr->key() == key)
            {
                QMessageBox::warning(this, "Subsystem List Editor", tr("Key value '%1' already exists!").arg(key));
                return false;
            }
        }

		std::shared_ptr<Hardware::Subsystem> subsystem = std::make_shared<Hardware::Subsystem>(index, key, strId, caption);
		subsystems.add(subsystem);
	}


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
	// Get default ssKey
	//
	std::set<int> usedSsKeys;

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);
		assert(item);

		int key = item->text(Key).toInt();
		usedSsKeys.insert(key);
	}

	int defaultKey = -1;

	if (usedSsKeys.empty() == true)
	{
		defaultKey = 0;
	}
	else
	{
		for (int i = 0; i <= Hardware::Subsystem::MaxKeyValue; i++)
		{
			if (usedSsKeys.count(i) == 0)
			{
				defaultKey = i;
				break;
			}
		}
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

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << "0" << QString::number(defaultKey) << QString("SUBSYSTEM%1ID").arg(defaultKey) << "Subsystem Caption");
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
