#include "DialogConnectionsEditor.h"
#include "ui_DialogConnectionsEditor.h"
#include "../include/PropertyEditorDialog.h"

//
//
// DialogConnectionsPropertyEditor
//
//

DialogConnectionsPropertyEditor::DialogConnectionsPropertyEditor(std::shared_ptr<PropertyObject> object, QWidget *parent, Hardware::ConnectionStorage *connections)
    :PropertyEditorDialog(object, parent)
{
	m_connections = connections;
}

bool DialogConnectionsPropertyEditor::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{
    if (m_connections == nullptr)
    {
        assert(m_connections);
        return false;
    }

    Hardware::Connection* c = dynamic_cast<Hardware::Connection*>(object.get());
    if (c == nullptr)
    {
        assert(c);
        return false;
    }

    bool uniqueConnections = m_connections->checkUniqueConnections(c);
    if (uniqueConnections == false)
    {
        QMessageBox::warning(this, "Connections Editor", "Duplicate string identifiers and port numbers found! Please correct the errors.");
        return false;
    }
    return true;
}


//
//
// DialogConnectionsEditor
//
//

DialogConnectionsEditor::DialogConnectionsEditor(DbController *pDbController, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogConnectionsEditor),
	m_dbController(pDbController)
{
    assert(db());

    ui->setupUi(this);

    setWindowTitle(tr("Optical Connections Editor"));

    QStringList l;
    l << tr("ID");
    l << tr("Caption");
    l << tr("Device1 StrID");
    l << tr("Device2 StrID");
    ui->m_list->setColumnCount(l.size());
    ui->m_list->setHeaderLabels(l);
    int il = 0;
    ui->m_list->setColumnWidth(il++, 50);
    ui->m_list->setColumnWidth(il++, 100);
    ui->m_list->setColumnWidth(il++, 250);
    ui->m_list->setColumnWidth(il++, 250);

    QString errorCode;

    if (connections.load(db(), errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
        return;
    }

    updateButtons(connections.isCheckedOut(db()));
    fillConnectionsList();
}

DialogConnectionsEditor::~DialogConnectionsEditor()
{
    delete ui;
}

void DialogConnectionsEditor::fillConnectionsList()
{
    ui->m_list->clear();

    for (int i = 0; i < connections.count(); i++)
    {
        std::shared_ptr<Hardware::Connection> connection = connections.get(i);
        if (connection == nullptr)
        {
            assert(connection);
            break;
        }

		if (connection->connectionType() != Hardware::Connection::ConnectionType::OpticalConnectionType)
		{
			continue;
		}

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(connection->index()) <<
                                                    connection->caption() <<
                                                    connection->device1StrID() <<
													connection->device2StrID());
        item->setData(0, Qt::UserRole, QVariant::fromValue(connection));
		ui->m_list->addTopLevelItem(item);
    }
}

bool DialogConnectionsEditor::askForSaveChanged()
{
    if (m_modified == false)
    {
        return true;
    }

    QMessageBox::StandardButton result = QMessageBox::warning(this, "Connections Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
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

bool DialogConnectionsEditor::saveChanges()
{
    // save to db
    //
    if (connections.save(db()) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't save connections."));
        return false;
    }

    m_modified = false;

    return true;
}

void DialogConnectionsEditor::updateButtons(bool checkOut)
{
    ui->m_Add->setEnabled(checkOut == true);
    ui->m_Edit->setEnabled(checkOut == true);
    ui->m_Remove->setEnabled(checkOut == true);
    ui->m_checkIn->setEnabled(checkOut == true);
    ui->m_checkOut->setEnabled(checkOut == false);
    ui->m_Undo->setEnabled(checkOut == true);
    ui->m_OK->setEnabled(checkOut == true);
}

void DialogConnectionsEditor::closeEvent(QCloseEvent* e)
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

DbController* DialogConnectionsEditor::db()
{
    return m_dbController;
}

void DialogConnectionsEditor::on_m_Add_clicked()
{
    int index = ui->m_list->topLevelItemCount();

    std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }
    connection->setCaption("New Connection");
    connection->setDevice1StrID("SYS_RACKID_CHID_MD_PORT1");
    connection->setDevice2StrID("SYS_RACKID_CHID_MD_PORT2");
    connection->setEnable(true);

    connections.add(connection);

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(connection->index()) <<
                                                connection->caption() <<
                                                connection->device1StrID() <<
												connection->device2StrID());
    item->setData(0, Qt::UserRole, QVariant::fromValue(connection));
    ui->m_list->insertTopLevelItem(index, item);

    // Select the created element
    //
    ui->m_list->clearSelection();
    ui->m_list->selectionModel()->select(ui->m_list->model()->index (index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);

    m_modified = true;
}

void DialogConnectionsEditor::on_m_Edit_clicked()
{
    QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
    if (items.size() != 1)
    {
        return;
    }

    // Remember index that was selected
    //
    int selectedIndex = ui->m_list->indexOfTopLevelItem(items[0]);

    std::shared_ptr<Hardware::Connection> connection = items[0]->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }

    // Create an object copy and pass it to the property editor
    //

    std::shared_ptr<Hardware::Connection> editConnection = std::make_shared<Hardware::Connection>();
    *editConnection = *connection;

    DialogConnectionsPropertyEditor* pd = new DialogConnectionsPropertyEditor(editConnection, this, &connections);

    if (pd->exec() == QDialog::Accepted)
    {
        // If properties were edited, update the object from its copy
        //
        *connection = *editConnection;

        fillConnectionsList();
        ui->m_list->selectionModel()->select(ui->m_list->model()->index (selectedIndex, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        m_modified = true;
    }
}

void DialogConnectionsEditor::on_m_Remove_clicked()
{
    QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
    if (items.size() != 1)
    {
        return;
    }

    // Remember index that was selected
    //
    int selectedIndex = ui->m_list->indexOfTopLevelItem(items[0]);
    if (selectedIndex > 0)
    {
        selectedIndex--;
    }

    std::shared_ptr<Hardware::Connection> connection = items[0]->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }
    connections.remove(connection);

    fillConnectionsList();
    if (ui->m_list->topLevelItemCount() > 0 && selectedIndex >= 0)
    {
        ui->m_list->selectionModel()->select(ui->m_list->model()->index (selectedIndex, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    m_modified = true;
}

void DialogConnectionsEditor::on_m_OK_clicked()
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

void DialogConnectionsEditor::on_m_Cancel_clicked()
{
    if (askForSaveChanged() == true)
    {
        reject();
    }
    return;
}


void DialogConnectionsEditor::on_m_list_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);
    if (ui->m_Edit->isEnabled() == true)
    {
        on_m_Edit_clicked();
    }
}

void DialogConnectionsEditor::on_m_checkOut_clicked()
{
    if (connections.checkOut(db()) == false)
    {
        QMessageBox::critical(this, "Connections Editor", "Check out error!");
        return;
    }

    updateButtons(true);
}


void DialogConnectionsEditor::on_m_checkIn_clicked()
{
    bool ok;
    QString comment = QInputDialog::getText(this, tr("Connections Editor"),
                                            tr("Please enter comment:"), QLineEdit::Normal,
                                            tr("comment"), &ok);

    if (ok == false)
    {
        return;
    }
    if (comment.isEmpty())
    {
        QMessageBox::warning(this, "Connections Editor", "No comment supplied!");
        return;
    }

    if (m_modified == true)
    {
        if (saveChanges() == false)
        {
            return;
        }
    }

    if (connections.checkIn(db(), comment) == false)
    {
        QMessageBox::critical(this, "Connections Editor", "Check in error!");
        return;
    }

    updateButtons(false);
}

void DialogConnectionsEditor::on_m_Undo_clicked()
{
    QMessageBox::StandardButton result = QMessageBox::warning(this, "Connections Editor", "Are you sure you want to undo the changes?", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::No)
    {
        return;
    }

    if (connections.undo(db()) == false)
    {
        QMessageBox::critical(this, "Connections Editor", "Undo error!");
        return;
    }

    m_modified = false;

    updateButtons(false);

    QString errorCode;

    // Load the unmodified file
    //
    if (connections.load(db(), errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
        return;
    }

    fillConnectionsList();
}
