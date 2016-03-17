#include "Rs232SignalListEditor.h"
#include <QTableWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include "DialogConnectionsEditor.h"
#include <QCloseEvent>

Rs232SignalListEditor::Rs232SignalListEditor(DbController* pDbController, QWidget *parent) :
	QDialog(parent),
	m_rs232Connections(new QTableWidget(this)),
	m_signalList(new QTableWidget(this)),
	m_db(pDbController)
{
	assert(m_db);

	setWindowTitle(tr("Serial port signal list editor"));
	resize(800, 600);

	m_rs232Connections->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_rs232Connections->fontMetrics().height() * 1.4));

	QStringList l;
	l << tr("Caption")
        << tr("Port StrID")
		<< tr("Mode")
        << tr("Enabled")
        << tr("Duplex");
    m_rs232Connections->setColumnCount(l.size());
    m_rs232Connections->setHorizontalHeaderLabels(l);
	m_rs232Connections->setColumnWidth(0, 150);
	m_rs232Connections->setColumnWidth(1, 300);
	m_rs232Connections->setColumnWidth(2, 70);
    m_rs232Connections->setColumnWidth(3, 70);
    m_rs232Connections->setColumnWidth(4, 70);

	m_rs232Connections->setSelectionMode(QAbstractItemView::SingleSelection);
	m_rs232Connections->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

	m_signalList->verticalHeader()->setDefaultSectionSize(static_cast<int>(m_signalList->fontMetrics().height() * 1.4));

	m_signalList->setColumnCount(1);
	m_signalList->setHorizontalHeaderLabels(QStringList() << "Signal Str ID");
	m_signalList->setColumnWidth(0, 300);

	m_signalList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_signalList->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

	hl = new QHBoxLayout;
	hl->addWidget(m_checkOut);
	hl->addWidget(m_checkIn);
	hl->addWidget(m_undo);
	hl->addStretch();

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

void Rs232SignalListEditor::closeEvent(QCloseEvent *e)
{
	if (!m_connections.isCheckedOut(m_db) || saveChanges())
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
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

	if (!ok)
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
    connection->setPort1StrID("SYSTEMID_RACKID_CHID_MD00_PORT01");
    connection->setSerialMode(Hardware::OptoPort::SerialMode::RS232);
    connection->setMode(Hardware::OptoPort::Mode::Serial);
	connection->setEnable(true);

	m_connections.add(connection);

	m_rs232Connections->insertRow(index);
	m_rs232Connections->setItem(index, 0, new QTableWidgetItem(connection->caption()));
	m_rs232Connections->item(index, 0)->setData(Qt::UserRole, m_connections.count() - 1);
    m_rs232Connections->setItem(index, 1, new QTableWidgetItem(connection->port1StrID()));
	m_rs232Connections->item(index, 1)->setData(Qt::UserRole, m_connections.count() - 1);
    m_rs232Connections->setItem(index, 2, new QTableWidgetItem(connection->serialMode() == Hardware::OptoPort::SerialMode::RS232 ? "RS-232" : "RS-485"));
	m_rs232Connections->item(index, 2)->setData(Qt::UserRole, m_connections.count() - 1);
	m_rs232Connections->setItem(index, 3, new QTableWidgetItem(connection->enable() ? "true" : "false"));
	m_rs232Connections->item(index, 3)->setData(Qt::UserRole, m_connections.count() - 1);
    m_rs232Connections->setItem(index, 4, new QTableWidgetItem(connection->enableDuplex() ? "true" : "false"));
    m_rs232Connections->item(index, 4)->setData(Qt::UserRole, m_connections.count() - 1);

	// Select the created element
	//
	m_rs232Connections->clearSelection();
	m_rs232Connections->selectionModel()->select(m_rs232Connections->model()->index(index, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);

	m_modified = true;
}

void Rs232SignalListEditor::editConnection()
{
	QList<QTableWidgetItem*> items = m_rs232Connections->selectedItems();
	if (items.size() == 0)
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
	pd->resize(640, 300);
	pd->setWindowTitle(tr("Connection properties"));

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
	if (items.size() == 0)
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
	if (ok && !id.isEmpty())
	{
		QStringList&& signalList = connection->signalList();
		if (signalList.contains(id))
		{
			QMessageBox::information(this, windowTitle(), tr("This signal ID already exists"));
			return;
		}
		items = m_signalList->selectedItems();
		if (items.size() == 0)
		{
			connection->setSignalList(signalList << id);
			fillSignalList(true);
			m_signalList->selectionModel()->select(m_signalList->model()->index(m_signalList->rowCount() - 1, 0), QItemSelectionModel::SelectCurrent);
		}
		else
		{
			int row = m_signalList->row(items[0]);
			signalList.insert(row, id);
			connection->setSignalList(signalList);
			fillSignalList(true);
			m_signalList->selectionModel()->select(m_signalList->model()->index(row, 0), QItemSelectionModel::SelectCurrent);
		}
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
	if (items.size() == 0)
	{
		return;
	}

	QStringList&& signalList = connection->signalList();
	bool ok;
	int row = m_signalList->row(items[0]);
	QString id = QInputDialog::getText(this, windowTitle(),
											tr("Please enter new id:"), QLineEdit::Normal,
											signalList[row], &ok);

	if (ok && !id.isEmpty())
	{
		if (signalList.contains(id))
		{
			QMessageBox::information(this, windowTitle(), tr("This signal ID already exists"));
			return;
		}
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
	if (items.size() == 0)
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

        if (connection->mode() != Hardware::OptoPort::Mode::Serial)
		{
			continue;
		}

		m_rs232Connections->insertRow(rowCount);
		m_rs232Connections->setItem(rowCount, 0, new QTableWidgetItem(connection->caption()));
		m_rs232Connections->item(rowCount, 0)->setData(Qt::UserRole, i);
        m_rs232Connections->setItem(rowCount, 1, new QTableWidgetItem(connection->port1StrID()));
		m_rs232Connections->item(rowCount, 1)->setData(Qt::UserRole, i);
        m_rs232Connections->setItem(rowCount, 2, new QTableWidgetItem(connection->serialMode() == Hardware::OptoPort::SerialMode::RS232 ? "RS-232" : "RS-485"));
		m_rs232Connections->item(rowCount, 2)->setData(Qt::UserRole, i);
		m_rs232Connections->setItem(rowCount, 3, new QTableWidgetItem(connection->enable() ? "true" : "false"));
		m_rs232Connections->item(rowCount, 3)->setData(Qt::UserRole, i);
        m_rs232Connections->setItem(rowCount, 4, new QTableWidgetItem(connection->enableDuplex() ? "true" : "false"));
        m_rs232Connections->item(rowCount, 4)->setData(Qt::UserRole, i);
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

bool Rs232SignalListEditor::saveChanges()
{
	if (!m_connections.save(m_db))
	{
		QMessageBox::StandardButton result = QMessageBox::critical(this, windowTitle(), tr("Can't save connections."), QMessageBox::Ignore | QMessageBox::Retry);
		if (result == QMessageBox::Ignore)
		{
			return true;
		}
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
}

