#include "Rs232SignalListEditor.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include "DialogConnectionsEditor.h"

Rs232SignalListEditor::Rs232SignalListEditor(DbController* pDbController, QWidget *parent) :
	QDialog(parent),
	m_rs232Connections(new QTableWidget(this)),
	m_signalList(new QTableWidget(this)),
	m_db(pDbController)
{
	assert(m_db);

	setWindowTitle(tr("Serial port signal list editor"));

	m_rs232Connections->setColumnCount(4);
	QStringList l;
	l << tr("Caption")
		<< tr("OSM StrID")
		<< tr("Mode")
		<< tr("Enabled");
	m_rs232Connections->setHorizontalHeaderLabels(l);
	m_rs232Connections->setColumnWidth(0, 100);
	m_rs232Connections->setColumnWidth(1, 150);
	m_rs232Connections->setColumnWidth(2, 50);
	m_rs232Connections->setColumnWidth(4, 100);

	m_rs232Connections->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_rs232Connections, &QTableWidget::doubleClicked, this, &Rs232SignalListEditor::editConnection);
	connect(m_rs232Connections->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Rs232SignalListEditor::onConnectionChanged);

	m_addConnection = new QPushButton("Add", this);
	connect(m_addConnection, &QPushButton::pressed, this, &Rs232SignalListEditor::addConnection);
	m_editConnection = new QPushButton("Edit", this);
	connect(m_editConnection, &QPushButton::pressed, this, &Rs232SignalListEditor::editConnection);
	m_removeConnection = new QPushButton("Remove", this);
	connect(m_removeConnection, &QPushButton::pressed, this, &Rs232SignalListEditor::removeConnection);

	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(m_rs232Connections);

	QVBoxLayout* buttonLayout = new QVBoxLayout;
	buttonLayout->addWidget(m_addConnection);
	buttonLayout->addWidget(m_editConnection);
	buttonLayout->addWidget(m_removeConnection);
	buttonLayout->addStretch();
	hl->addLayout(buttonLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hl);

	m_signalList->setColumnCount(1);
	m_signalList->setHorizontalHeaderLabels(QStringList() << "Signal Str ID");
	m_signalList->setColumnWidth(0, 100);

	m_signalList->setSelectionMode(QAbstractItemView::SingleSelection);

	connect(m_signalList, &QTableWidget::doubleClicked, this, &Rs232SignalListEditor::editSignal);

	m_addSignal = new QPushButton("Add", this);
	connect(m_addSignal, &QPushButton::pressed, this, &Rs232SignalListEditor::addSignal);
	m_editSignal = new QPushButton("Edit", this);
	connect(m_editSignal, &QPushButton::pressed, this, &Rs232SignalListEditor::editSignal);
	m_removeSignal = new QPushButton("Remove", this);
	connect(m_removeSignal, &QPushButton::pressed, this, &Rs232SignalListEditor::removeSignal);

	hl = new QHBoxLayout;
	hl->addWidget(m_signalList);

	buttonLayout = new QVBoxLayout;
	buttonLayout->addWidget(m_addSignal);
	buttonLayout->addWidget(m_editSignal);
	buttonLayout->addWidget(m_removeSignal);
	buttonLayout->addStretch();
	hl->addLayout(buttonLayout);

	mainLayout->addLayout(hl);

	m_checkIn = new QPushButton("Check In", this);
	connect(m_checkIn, &QPushButton::pressed, this, &Rs232SignalListEditor::checkIn);
	m_checkOut = new QPushButton("Check Out", this);
	connect(m_checkOut, &QPushButton::pressed, this, &Rs232SignalListEditor::checkOut);
	m_undo = new QPushButton("Undo", this);
	connect(m_undo, &QPushButton::pressed, this, &Rs232SignalListEditor::undo);

	m_ok = new QPushButton("Ok", this);
	connect(m_ok, &QPushButton::pressed, this, &Rs232SignalListEditor::onOk);
	m_cancel = new QPushButton("Cancel", this);
	connect(m_cancel, &QPushButton::pressed, this, &Rs232SignalListEditor::onCancel);

	hl = new QHBoxLayout;
	hl->addWidget(m_checkOut);
	hl->addWidget(m_checkIn);
	hl->addWidget(m_undo);
	hl->addStretch();
	hl->addWidget(m_ok);
	hl->addWidget(m_cancel);

	mainLayout->addLayout(hl);
	setLayout(mainLayout);

	QString errorCode;

	if (!m_connections.load(m_db, errorCode))
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
		return;
	}

	updateButtons(m_connections.isCheckedOut(m_db));
	fillConnectionsList();
}

void Rs232SignalListEditor::checkOut()
{
	if (!m_connections.checkOut(m_db))
	{
		QMessageBox::critical(this, windowTitle(), "Check out error!");
		return;
	}

	updateButtons(true);
}

void Rs232SignalListEditor::checkIn()
{
	bool ok;
	QString comment = QInputDialog::getText(this, windowTitle(),
											tr("Please enter comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok)
	{
		return;
	}
	if (comment.isEmpty())
	{
		QMessageBox::warning(this, windowTitle(), "No comment supplied!");
		return;
	}

	if (m_modified)
	{
		if (!saveChanges())
		{
			return;
		}
	}

	if (!m_connections.checkIn(m_db, comment))
	{
		QMessageBox::critical(this, windowTitle(), "Check in error!");
		return;
	}

	updateButtons(false);
}

void Rs232SignalListEditor::undo()
{
	QMessageBox::StandardButton result = QMessageBox::warning(this, windowTitle(), "Are you sure you want to undo the changes?", QMessageBox::Yes | QMessageBox::No);
	if (result == QMessageBox::No)
	{
		return;
	}

	if (!m_connections.undo(m_db))
	{
		QMessageBox::critical(this, windowTitle(), "Undo error!");
		return;
	}

	m_modified = false;

	updateButtons(false);

	QString errorCode;

	// Load the unmodified file
	//
	if (!m_connections.load(m_db, errorCode))
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load connections!"));
		return;
	}

	fillConnectionsList();
}

void Rs232SignalListEditor::onOk()
{
	if (m_modified)
	{
		if (!saveChanges())
		{
			return;
		}
	}

	accept();
}

void Rs232SignalListEditor::onCancel()
{
	if (askForSaveChanged())
	{
		reject();
	}
}

void Rs232SignalListEditor::addConnection()
{
	int index = m_rs232Connections->rowCount();

	std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();
	if (connection == nullptr)
	{
		assert(connection);
		return;
	}
	connection->setCaption("New Connection");
	connection->setOsmStrID("OSM_ID");
	connection->setConnectionMode(Hardware::Connection::ConnectionMode::ModeRS232);
	connection->setConnectionType(Hardware::Connection::ConnectionType::SerialPortSignalListType);
	connection->setEnable(true);

	m_connections.add(connection);

	m_rs232Connections->insertRow(index);
	m_rs232Connections->setItem(index, 0, new QTableWidgetItem(connection->caption()));
	m_rs232Connections->item(index, 0)->setData(Qt::UserRole, m_connections.count() - 1);
	m_rs232Connections->setItem(index, 1, new QTableWidgetItem(connection->osmStrID()));
	m_rs232Connections->item(index, 1)->setData(Qt::UserRole, m_connections.count() - 1);
	m_rs232Connections->setItem(index, 2, new QTableWidgetItem(connection->connectionMode() == Hardware::Connection::ConnectionMode::ModeRS232 ? "RS-232" : "RS-485"));
	m_rs232Connections->item(index, 2)->setData(Qt::UserRole, m_connections.count() - 1);
	m_rs232Connections->setItem(index, 3, new QTableWidgetItem(connection->enable() ? "true" : "false"));
	m_rs232Connections->item(index, 3)->setData(Qt::UserRole, m_connections.count() - 1);

	// Select the created element
	//
	m_rs232Connections->clearSelection();
	m_rs232Connections->selectionModel()->select(m_rs232Connections->model()->index(index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_modified = true;
}

void Rs232SignalListEditor::editConnection()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	// Remember index that was selected
	//
	int selectedIndex = m_rs232Connections->row(items[0]);

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(items[0]->data(Qt::UserRole).toInt());
	if (connection == nullptr)
	{
		assert(connection);
		return;
	}

	// Create an object copy and pass it to the property editor
	//

	std::shared_ptr<Hardware::Connection> editConnection = std::make_shared<Hardware::Connection>();
	*editConnection = *connection;

	DialogConnectionsPropertyEditor* pd = new DialogConnectionsPropertyEditor(editConnection, this, &m_connections);

	if (pd->exec() == QDialog::Accepted)
	{
		// If properties were edited, update the object from its copy
		//
		*connection = *editConnection;

		fillConnectionsList();
		m_rs232Connections->selectionModel()->select(m_rs232Connections->model()->index(selectedIndex, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
		m_modified = true;
	}
}

void Rs232SignalListEditor::removeConnection()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	// Remember index that was selected
	//
	int selectedIndex = m_rs232Connections->row(items[0]);
	if (selectedIndex > 0)
	{
		selectedIndex--;
	}

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(items[0]->data(Qt::UserRole).toInt());
	if (connection == nullptr)
	{
		assert(connection);
		return;
	}
	m_connections.remove(connection);

	fillConnectionsList();
	if (m_rs232Connections->rowCount() > 0 && selectedIndex >= 0)
	{
		m_rs232Connections->selectionModel()->select(m_rs232Connections->model()->index (selectedIndex, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	m_modified = true;
}

void Rs232SignalListEditor::addSignal()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() == 0)
	{
		return;
	}

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(items[0]->data(Qt::UserRole).toInt());

	bool ok;
	QString&& id = QInputDialog::getText(this, windowTitle(),
										tr("Please enter new id:"), QLineEdit::Normal,
										"#NEW_SIGNAL_ID", &ok);
	if (ok)
	{
		connection->setSignalList(connection->signalList() << id);
		fillSignalList(true);
		m_signalList->selectionModel()->select(m_signalList->model()->index(m_signalList->rowCount() - 1, 0), QItemSelectionModel::SelectCurrent);
	}
}

void Rs232SignalListEditor::editSignal()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() == 0)
	{
		return;
	}

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(items[0]->data(Qt::UserRole).toInt());

	items = m_signalList->selectedItems();
	if (items.size() != 1)
	{
		return;
	}

	QStringList&& signalList = connection->signalList();
	bool ok;
	int row = m_signalList->row(items[0]);
	QString id = QInputDialog::getText(this, windowTitle(),
											tr("Please enter new id:"), QLineEdit::Normal,
											signalList[row], &ok);

	if (ok)
	{
		signalList[row] = id;
		connection->setSignalList(signalList);
		fillSignalList(true);
		m_signalList->selectionModel()->select(m_signalList->model()->index(row, 0), QItemSelectionModel::SelectCurrent);
	}
}

void Rs232SignalListEditor::removeSignal()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() == 0)
	{
		return;
	}

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(items[0]->data(Qt::UserRole).toInt());

	items = m_signalList->selectedItems();
	if (items.size() != 1)
	{
		return;
	}
	int row = m_signalList->row(items[0]);
	QStringList signalList = connection->signalList();
	signalList.removeAt(row);
	if (row != 0)
	{
		row--;
	}
	connection->setSignalList(signalList);
	fillSignalList(true);
	m_signalList->selectionModel()->select(m_signalList->model()->index(row, 0), QItemSelectionModel::SelectCurrent);
}

void Rs232SignalListEditor::onConnectionChanged()
{
	if (m_rs232Connections->selectedItems().size() == 0 ||
			!m_addConnection->isEnabled())
	{
		m_editConnection->setEnabled(false);
		m_removeConnection->setEnabled(false);
		m_addSignal->setEnabled(false);
	}
	else
	{
		m_editConnection->setEnabled(true);
		m_removeConnection->setEnabled(true);
		m_addSignal->setEnabled(true);
	}
	fillSignalList();
}

void Rs232SignalListEditor::fillConnectionsList()
{
	m_rs232Connections->setRowCount(0);

	int rowCount = 0;
	for (int i = 0; i < m_connections.count(); i++)
	{
		std::shared_ptr<Hardware::Connection> connection = m_connections.get(i);
		if (connection == nullptr)
		{
			assert(connection);
			break;
		}

		if (connection->connectionType() != Hardware::Connection::ConnectionType::SerialPortSignalListType)
		{
			continue;
		}

		m_rs232Connections->insertRow(rowCount);
		m_rs232Connections->setItem(rowCount, 0, new QTableWidgetItem(connection->caption()));
		m_rs232Connections->item(rowCount, 0)->setData(Qt::UserRole, i);
		m_rs232Connections->setItem(rowCount, 1, new QTableWidgetItem(connection->osmStrID()));
		m_rs232Connections->item(rowCount, 1)->setData(Qt::UserRole, i);
		m_rs232Connections->setItem(rowCount, 2, new QTableWidgetItem(connection->connectionMode() == Hardware::Connection::ConnectionMode::ModeRS232 ? "RS-232" : "RS-485"));
		m_rs232Connections->item(rowCount, 2)->setData(Qt::UserRole, i);
		m_rs232Connections->setItem(rowCount, 3, new QTableWidgetItem(connection->enable() ? "true" : "false"));
		m_rs232Connections->item(rowCount, 3)->setData(Qt::UserRole, i);
		rowCount++;
	}
}

void Rs232SignalListEditor::fillSignalList(bool forceUpdate)
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() == 0)
	{
		m_signalList->setRowCount(0);
		return;
	}
	int selectedConnection = items[0]->data(Qt::UserRole).toInt();
	if (!forceUpdate && selectedConnection == m_currentConnection)
	{
		return;
	}
	m_currentConnection = selectedConnection;
	m_signalList->setRowCount(0);
	QStringList&& list = m_connections.get(selectedConnection)->signalList();
	for (int i = 0; i < list.count(); i++)
	{
		m_signalList->insertRow(i);
		m_signalList->setItem(i, 0, new QTableWidgetItem(list[i]));
	}
}

bool Rs232SignalListEditor::askForSaveChanged()
{
	if (!m_modified)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, windowTitle(), "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (result == QMessageBox::Yes)
	{
		if (!saveChanges())
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

bool Rs232SignalListEditor::saveChanges()
{
	if (!m_connections.save(m_db))
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save connections."));
		return false;
	}

	m_modified = false;

	return true;
}

void Rs232SignalListEditor::updateButtons(bool checkOut)
{
	m_addConnection->setEnabled(checkOut);
	onConnectionChanged();

	m_addSignal->setEnabled(checkOut);
	m_editSignal->setEnabled(checkOut);
	m_removeSignal->setEnabled(checkOut);

	m_checkIn->setEnabled(checkOut);
	m_checkOut->setEnabled(!checkOut);
	m_undo->setEnabled(checkOut);
	m_ok->setEnabled(checkOut);
}

