#include "DialogConnectionsEditor.h"
#include "ui_DialogConnectionsEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "Settings.h"
#include <QMessageBox>

//
//
// DialogConnectionsPropertyEditor
//
//

DialogConnectionsPropertyEditor::DialogConnectionsPropertyEditor(std::shared_ptr<PropertyObject> object, QWidget *parent, Hardware::ConnectionStorage *connections, bool readOnly)
	:PropertyEditorDialog(object, parent, readOnly)
{
	QString windowTitle = tr("Connection Properties");

	if (readOnly == true)
	{
		windowTitle += tr(" [Read only]");
	}
	setWindowTitle(windowTitle);

	m_connections = connections;

    setSplitterPosition(theSettings.m_connectionSplitterState);
    if (theSettings.m_connectionPropertiesWindowPos.x() != -1 && theSettings.m_connectionPropertiesWindowPos.y() != -1)
    {
        move(theSettings.m_connectionPropertiesWindowPos);
        restoreGeometry(theSettings.m_connectionPropertiesWindowGeometry);
    }
}

DialogConnectionsPropertyEditor::~DialogConnectionsPropertyEditor()
{
}

bool DialogConnectionsPropertyEditor::onPropertiesChanged(std::shared_ptr<PropertyObject> object)
{
    if (m_connections == nullptr)
    {
        assert(m_connections);
        return false;
    }

	Hardware::Connection* e = dynamic_cast<Hardware::Connection*>(object.get());
	if (e == nullptr)
    {
		assert(e);
        return false;
    }

	// Check if port 1 ID is not equal to port 2 ID
	//
	if (e->port1EquipmentID() == e->port2EquipmentID())
	{
		QString s = QString("Port 1 identifier is eqal to Port 2 identifier.\r\n\r\nAre you sure you want to continue?");
		if (QMessageBox::warning(this, "Connections Editor", s, QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
		{
			return true;
		}
		return false;
	}

	// Check if port IDs are not equal port IDs in other connections
	//

	QString duplicateConnectionName;
	QString duplicatePortName;

	QString port1ID = e->port1EquipmentID();
	QString port2ID = e->port2EquipmentID();

	bool duplicateFound = false;

	for (int i = 0; i < m_connections->count(); i++)
	{
		Hardware::Connection* c = m_connections->get(i).get();

		if (e->index() == c->index())
		{
			continue;
		}

		if (port1ID.length() > 0)
		{
			if (port1ID == c->port1EquipmentID())
			{
				duplicatePortName = port1ID;
				duplicateConnectionName = c->connectionID();
				duplicateFound = true;
			}
			if (port1ID == c->port2EquipmentID())
			{
				duplicatePortName = port1ID;
				duplicateConnectionName = c->connectionID();
				duplicateFound = true;
			}
		}

		if (e->mode() == Hardware::OptoPort::Mode::Optical)
		{
			if (port2ID.length() > 0)
			{
				if (port2ID == c->port1EquipmentID())
				{
					duplicatePortName = port2ID;
					duplicateConnectionName = c->connectionID();
					duplicateFound = true;
				}
				if (port2ID == c->port2EquipmentID())
				{
					duplicatePortName = port2ID;
					duplicateConnectionName = c->connectionID();
					duplicateFound = true;
				}
			}
		}
	}

	if (duplicateFound == true)
    {
		QString s = QString("Port '%1' is already in use in connection '%2'.\r\n\r\nAre you sure you want to continue?").arg(duplicatePortName).arg(duplicateConnectionName);
		if (QMessageBox::warning(this, "Connections Editor", s, QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
		{
			return true;
		}
        return false;
    }

    return true;
}

void DialogConnectionsPropertyEditor::closeEvent(QCloseEvent * e)
{
    Q_UNUSED(e);
    saveSettings();

}

void DialogConnectionsPropertyEditor::done(int r)
{
    saveSettings();
    PropertyEditorDialog::done(r);
}

void DialogConnectionsPropertyEditor::saveSettings()
{
    theSettings.m_connectionSplitterState = splitterPosition();
    theSettings.m_connectionPropertiesWindowPos = pos();
    theSettings.m_connectionPropertiesWindowGeometry = saveGeometry();
}


//
//
// DialogConnectionsEditor
//
//

DialogConnectionsEditor::DialogConnectionsEditor(DbController *pDbController, QWidget *parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogConnectionsEditor),
	m_dbController(pDbController)
{
    assert(db());

    ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Optical Connections Editor"));

    QStringList l;
	l << tr("№");
	l << tr("ConnectionID");
	l << tr("Port1 EquipmentID");
	l << tr("Port2 EquipmentID");
    ui->m_list->setColumnCount(l.size());
    ui->m_list->setHeaderLabels(l);
    int il = 0;
	ui->m_list->setColumnWidth(il++, 50);
    ui->m_list->setColumnWidth(il++, 100);
    ui->m_list->setColumnWidth(il++, 250);
    ui->m_list->setColumnWidth(il++, 250);
	ui->m_list->setSortingEnabled(true);

	ui->m_list->setSelectionMode(QAbstractItemView::SingleSelection);

    QString errorCode;

    if (connections.load(db(), errorCode) == false)
    {
        QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
        return;
    }

	m_checkedOut = connections.isCheckedOut(db());

	updateButtons();
    fillConnectionsList();

	for (int i = 0; i < ui->m_list->columnCount(); i++)
	{
		ui->m_list->resizeColumnToContents(i);
	}

	ui->m_list->sortByColumn(theSettings.m_connectionEditorSortColumn, theSettings.m_connectionEditorSortOrder);

	connect(ui->m_list->header(), &QHeaderView::sortIndicatorChanged, this, &DialogConnectionsEditor::sortIndicatorChanged);


	if (theSettings.m_connectionEditorWindowPos.x() != -1 && theSettings.m_connectionEditorWindowPos.y() != -1)
	{
		move(theSettings.m_connectionEditorWindowPos);
		restoreGeometry(theSettings.m_connectionEditorWindowGeometry);
	}
}

DialogConnectionsEditor::~DialogConnectionsEditor()
{
	theSettings.m_connectionEditorWindowPos = pos();
	theSettings.m_connectionEditorWindowGeometry = saveGeometry();

	theDialogConnectionsEditor = nullptr;
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

        if (connection->mode() != Hardware::OptoPort::Mode::Optical)
		{
			continue;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		setConnectionText(item, connection.get());

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

void DialogConnectionsEditor::updateButtons()
{
	int selectedCount = ui->m_list->selectedItems().size();

	bool editingEnabled = m_checkedOut == true && selectedCount > 0;

	ui->m_Add->setEnabled(m_checkedOut == true);

	ui->m_Edit->setText(m_checkedOut == true ? tr("Edit") : tr("View"));
	ui->m_Edit->setEnabled(selectedCount > 0);
	ui->m_Remove->setEnabled(editingEnabled == true);

	ui->m_checkIn->setEnabled(m_checkedOut == true);
	ui->m_checkOut->setEnabled(m_checkedOut == false);
	ui->m_Undo->setEnabled(m_checkedOut == true);

	ui->m_Apply->setEnabled(m_checkedOut == true);
	ui->m_OK->setEnabled(m_checkedOut == true);
}

bool DialogConnectionsEditor::continueWithDuplicateCaptions()
{
	bool duplicated = false;
	QString duplicatedCaption;

	for (int i = 0; i < connections.count(); i++)
	{
		Hardware::Connection* c = connections.get(i).get();
		if (c->mode() != Hardware::OptoPort::Mode::Optical)
		{
			continue;
		}

		for (int j = 0; j < connections.count(); j++)
		{
			Hardware::Connection* e = connections.get(j).get();

			if (i == j)
			{
				continue;
			}

			if (e->connectionID() == c->connectionID())
			{
				duplicated = true;
				duplicatedCaption = e->connectionID();
				break;
			}
		}
		if (duplicated == true)
		{
			break;
		}
	}

	if (duplicated == true)
	{
		QString s = QString("Connection with ID '%1' already exists.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
		if (QMessageBox::warning(this, "Connections Editor", s, QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
		{
			return false;
		}
	}
	return true;
}

void DialogConnectionsEditor::setConnectionText(QTreeWidgetItem* item, Hardware::Connection* connection)
{
	if (item == nullptr || connection == nullptr)
	{
		assert(item);
		assert(connection);
		return;
	}

	int c = 0;
	QString numString = QString::number(connection->index()).rightJustified(4, '0');
	item->setText(c++, numString);
	item->setText(c++, connection->connectionID());
	item->setText(c++, connection->port1EquipmentID());
	item->setText(c++, connection->port2EquipmentID());

}

void DialogConnectionsEditor::closeEvent(QCloseEvent* e)
{
	if (continueWithDuplicateCaptions() == false)
	{
		e->ignore();
		return;
	}

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
    std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }
	connection->setConnectionID("NewConnection");
	connection->setPort1EquipmentID("SYSTEMID_RACKID_CHID_MD00_PORT01");
	connection->setPort2EquipmentID("SYSTEMID_RACKID_CHID_MD00_PORT02");
	connection->setEnableSerial(true);

    connections.add(connection);

	QTreeWidgetItem* item = new QTreeWidgetItem();
	setConnectionText(item, connection.get());
	item->setData(0, Qt::UserRole, QVariant::fromValue(connection));

	ui->m_list->addTopLevelItem(item);

    // Select the created element
    //
	ui->m_list->clearSelection();
	ui->m_list->scrollToItem(item);
	item->setSelected(true);

    m_modified = true;
}

void DialogConnectionsEditor::on_m_Edit_clicked()
{
    QList<QTreeWidgetItem*> items = ui->m_list->selectedItems();
    if (items.size() != 1)
    {
        return;
    }

	bool readOnly = ui->m_checkIn->isEnabled() == false;

	QTreeWidgetItem* editItem = items[0];

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

	DialogConnectionsPropertyEditor* pd = new DialogConnectionsPropertyEditor(editConnection, this, &connections, readOnly);

	if (pd->exec() == QDialog::Accepted && readOnly == false)
    {
        // If properties were edited, update the object from its copy
        //
        *connection = *editConnection;

		setConnectionText(editItem, connection.get());

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


    std::shared_ptr<Hardware::Connection> connection = items[0]->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();
    if (connection == nullptr)
    {
        assert(connection);
        return;
    }

	if (QMessageBox::warning(this, tr("Delete"), tr("Are you sure you want to delete the connection '%1'?").arg(connection->connectionID()),
							 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

    connections.remove(connection);

	// Remove the tree item
	//

	int deleteIndex = ui->m_list->indexOfTopLevelItem(items[0]);

	QTreeWidgetItem* deleteItem = ui->m_list->takeTopLevelItem(deleteIndex);
	delete deleteItem;

	// Select the item above deleted
	//

	ui->m_list->clearSelection();

	int selectedIndex = deleteIndex - 1;
	if (ui->m_list->topLevelItemCount() > 0 && selectedIndex >= 0)
    {
		ui->m_list->selectionModel()->select(ui->m_list->model()->index (selectedIndex, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
		ui->m_list->scrollToItem(ui->m_list->topLevelItem(selectedIndex));
    }

	// Refresh all text, because index was changed
	//

	for (int i = 0; i < ui->m_list->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = ui->m_list->topLevelItem(i);

		std::shared_ptr<Hardware::Connection> connection = item->data(0, Qt::UserRole).value<std::shared_ptr<Hardware::Connection>>();

		setConnectionText(item, connection.get());

	}

    m_modified = true;
}

void DialogConnectionsEditor::on_m_Apply_clicked()
{
	if (continueWithDuplicateCaptions() == false)
	{
		return;
	}

	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}
}

void DialogConnectionsEditor::on_m_OK_clicked()
{
	if (continueWithDuplicateCaptions() == false)
	{
		return;
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

void DialogConnectionsEditor::on_m_Cancel_clicked()
{
	reject();
}

void DialogConnectionsEditor::reject()
{
	if (continueWithDuplicateCaptions() == false)
	{
		return;
	}

	if (askForSaveChanged() == true)
	{
		QDialog::reject();
	}

}


void DialogConnectionsEditor::on_m_list_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index);

	on_m_Edit_clicked();
}

void DialogConnectionsEditor::on_m_checkOut_clicked()
{
    if (connections.checkOut(db()) == false)
    {
        QMessageBox::critical(this, "Connections Editor", "Check out error!");
        return;
    }

	m_checkedOut = true;

	updateButtons();
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

	m_checkedOut = false;

	updateButtons();
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

	m_checkedOut = false;

	updateButtons();

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

void DialogConnectionsEditor::on_m_list_itemSelectionChanged()
{
	updateButtons();
}

void DialogConnectionsEditor::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_connectionEditorSortColumn = column;
	theSettings.m_connectionEditorSortOrder = order;
}

DialogConnectionsEditor* theDialogConnectionsEditor = nullptr;


