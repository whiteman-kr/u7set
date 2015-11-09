#include "DialogConnectionsEditor.h"
#include "ui_DialogConnectionsEditor.h"
#include "../include/PropertyEditorDialog.h"

//
//
// DialogConnectionsEditor
//
//

DialogConnectionsEditor::DialogConnectionsEditor(DbController *pDbController, QWidget *parent) :
    m_dbController(pDbController),
    QDialog(parent),
    ui(new Ui::DialogConnectionsEditor)
{
    assert(db());

    ui->setupUi(this);

    setWindowTitle(tr("Connections Editor"));

    ui->m_list->setColumnCount(6);
    QStringList l;
    l << tr("ID");
    l << tr("Caption");
    l << tr("Device1 StrID");
    l << tr("Device1 Port");
    l << tr("Device2 StrID");
    l << tr("Device2 Port");
    l << tr("Mode");
    l << tr("Enabled");
    ui->m_list->setHeaderLabels(l);
    ui->m_list->setColumnWidth(0, 50);
    ui->m_list->setColumnWidth(1, 100);
    ui->m_list->setColumnWidth(2, 150);
    ui->m_list->setColumnWidth(3, 80);
    ui->m_list->setColumnWidth(4, 150);
    ui->m_list->setColumnWidth(5, 80);
    ui->m_list->setColumnWidth(6, 50);
    ui->m_list->setColumnWidth(7, 50);

    QString errorCode;

    if (m_connections.load(db(), errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
        return;
    }

    fillConnectionsList();
}

DialogConnectionsEditor::~DialogConnectionsEditor()
{
    delete ui;
}

void DialogConnectionsEditor::fillConnectionsList()
{
    ui->m_list->clear();

    for (int i = 0; i < m_connections.count(); i++)
    {
        std::shared_ptr<Hardware::Connection> connection = m_connections.get(i);
        if (connection == nullptr)
        {
            assert(connection);
            break;
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(connection->index()) <<
                                                    connection->caption() <<
                                                    connection->device1StrID() <<
                                                    QString::number(connection->device1Port()) <<
                                                    connection->device2StrID() <<
                                                    QString::number(connection->device2Port()) <<
                                                    (connection->connectionMode() == Hardware::Connection::ConnectionMode::ModeRS232 ? "RS-232" : "RS-485") <<
                                                    (connection->enable() ? "true" : "false"));
        item->setData(0, Qt::UserRole, QVariant::fromValue(connection));
        ui->m_list->insertTopLevelItem(i, item);
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
    bool uniqueConnections = m_connections.checkUniqueConnections();
    if (uniqueConnections == false)
    {
        QMessageBox::warning(this, "Connections Editor", "Duplicate string identifiers and port numbers found! Please correct the errors.");
        return false;
    }

    bool ok;
    QString comment = QInputDialog::getText(this, tr("Connections Editor"),
                                            tr("Please enter comment:"), QLineEdit::Normal,
                                            tr("comment"), &ok);

    if (ok == false)
    {
        return false;
    }
    if (comment.isEmpty())
    {
        QMessageBox::warning(this, "Connections Editor", "No comment supplied!");
        return false;
    }

    // save to db
    //
    if (m_connections.save(db(), comment) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't save connections."));
        return false;
    }

    m_modified = false;

    return true;
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
    m_connections.add(connection);

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << QString::number(connection->index()) <<
                                                connection->caption() <<
                                                connection->device1StrID() <<
                                                QString::number(connection->device1Port()) <<
                                                connection->device2StrID() <<
                                                QString::number(connection->device2Port()) <<
                                                (connection->connectionMode() == Hardware::Connection::ConnectionMode::ModeRS232 ? "RS-232" : "RS-485") <<
                                                (connection->enable() ? "true" : "false"));
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

    PropertyEditorDialog* pd = new PropertyEditorDialog(editConnection, this);

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
    m_connections.remove(connection);

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
    on_m_Edit_clicked();
}
