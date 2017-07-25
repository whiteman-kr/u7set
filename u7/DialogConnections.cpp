#include "DialogConnections.h"
#include "../lib/PropertyEditorDialog.h"
#include "Settings.h"
#include <QMessageBox>
#include <QFileDialog>

DialogConnections* theDialogConnections = nullptr;

DialogConnections::DialogConnections(DbController* db, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	  m_db(db),
	  m_connections(db)
{
	assert(m_db);

	setWindowTitle(tr("Connections Editor"));

	setAttribute(Qt::WA_DeleteOnClose);

	// Create user interface
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QHBoxLayout* maskLayout = new QHBoxLayout();

	m_mask = new QLineEdit();
	m_maskApply = new QPushButton(tr("Filter"));
	m_maskReset = new QPushButton(tr("Reset"));

	connect(m_mask, &QLineEdit::returnPressed, this, &DialogConnections::onMaskReturn);
	connect(m_maskApply, &QPushButton::clicked, this, &DialogConnections::onMaskApply);
	connect(m_maskReset, &QPushButton::clicked, this, &DialogConnections::onMaskReset);

	m_connectionsTree = new QTreeWidget();

	QStringList l;
	l << tr("ConnectionID");
	l << tr("Type");
	l << tr("State");
	l << tr("User");

	m_connectionsTree->setColumnCount(l.size());
	m_connectionsTree->setHeaderLabels(l);

	int il = 0;
	m_connectionsTree->setColumnWidth(il++, 140);
	m_connectionsTree->setColumnWidth(il++, 80);
	m_connectionsTree->setColumnWidth(il++, 40);
	m_connectionsTree->setColumnWidth(il++, 40);
	m_connectionsTree->setSortingEnabled(true);
	m_connectionsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_connectionsTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_connectionsTree->setRootIsDecorated(false);

	connect(m_connectionsTree, &QTreeWidget::itemSelectionChanged, this, &DialogConnections::onItemSelectionChanged);
	connect(m_connectionsTree, &QWidget::customContextMenuRequested, this, &DialogConnections::onCustomContextMenuRequested);

	m_connectionPropertyEditor = new ExtWidgets::PropertyEditor(this);

	connect(m_connectionPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogConnections::onPropertiesChanged);

	maskLayout->addWidget(m_mask);
	maskLayout->addWidget(m_maskApply);
	maskLayout->addWidget(m_maskReset);

	m_splitter = new QSplitter(Qt::Horizontal);

	m_splitter->addWidget(m_connectionsTree);
	m_splitter->addWidget(m_connectionPropertyEditor);

	m_splitter->setChildrenCollapsible(false);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();

	m_btnAdd = new QPushButton(tr("Add"));
	m_btnRemove = new QPushButton(tr("Remove"));
	m_btnCheckOut = new QPushButton(tr("Check Out"));
	m_btnCheckIn = new QPushButton(tr("Check In"));
	m_btnUndo = new QPushButton(tr("Undo"));
	m_btnRefresh = new QPushButton(tr("Refresh"));
	m_btnReport = new QPushButton(tr("Report"));
	m_btnClose = new QPushButton(tr("Close"));

	buttonsLayout->addWidget(m_btnAdd);
	buttonsLayout->addWidget(m_btnRemove);
	buttonsLayout->addWidget(m_btnCheckOut);
	buttonsLayout->addWidget(m_btnCheckIn);
	buttonsLayout->addWidget(m_btnUndo);
	buttonsLayout->addWidget(m_btnRefresh);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(m_btnReport);
	buttonsLayout->addWidget(m_btnClose);

	connect (m_btnAdd, &QPushButton::clicked, this, &DialogConnections::onAdd);
	connect (m_btnRemove, &QPushButton::clicked, this, &DialogConnections::onRemove);
	connect (m_btnCheckOut, &QPushButton::clicked, this, &DialogConnections::onCheckOut);
	connect (m_btnCheckIn, &QPushButton::clicked, this, &DialogConnections::onCheckIn);
	connect (m_btnUndo, &QPushButton::clicked, this, &DialogConnections::onUndo);
	connect (m_btnRefresh, &QPushButton::clicked, this, &DialogConnections::onRefresh);
	connect (m_btnReport, &QPushButton::clicked, this, &DialogConnections::onReport);
	connect (m_btnClose, &QPushButton::clicked, this, &DialogConnections::close);

	mainLayout->addLayout(maskLayout);
	mainLayout->addWidget(m_splitter);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);

	// Mask and completer
	//
	m_completer = new QCompleter(theSettings.m_connectionEditorMasks, this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_mask->setCompleter(m_completer);

	connect(m_mask, &QLineEdit::textEdited, [=](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_mask, &QLineEdit::setText);

	// Popup menu
	//
	m_addAction = new QAction(tr("Add"), this);
	m_removeAction = new QAction(tr("Remove"), this);
	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkInAction = new QAction(tr("Check In"), this);
	m_undoAction = new QAction(tr("Undo"), this);
	m_refreshAction = new QAction(tr("Refresh"), this);

	connect(m_addAction, &QAction::triggered, this, &DialogConnections::onAdd);
	connect(m_removeAction, &QAction::triggered, this, &DialogConnections::onRemove);
	connect(m_checkOutAction, &QAction::triggered, this, &DialogConnections::onCheckOut);
	connect(m_checkInAction, &QAction::triggered, this, &DialogConnections::onCheckIn);
	connect(m_undoAction, &QAction::triggered, this, &DialogConnections::onUndo);
	connect(m_refreshAction, &QAction::triggered, this, &DialogConnections::onRefresh);

	m_popupMenu = new QMenu(this);
	m_popupMenu->addAction(m_addAction);
	m_popupMenu->addAction(m_removeAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_checkOutAction);
	m_popupMenu->addAction(m_checkInAction);
	m_popupMenu->addAction(m_undoAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_refreshAction);

	// Load connections
	//
	QString errorMessage;

	bool ok = m_connections.loadFromConnectionsFolder(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(parent, qAppName(), errorMessage);
		return;
	}

	// Load deprecated connections
	//
	Hardware::ConnectionStorage xmlConnections(m_db);

	ok = xmlConnections.loadFromXmlDeprecated(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(parent, qAppName(), errorMessage);
		return;
	}

	if (xmlConnections.count() > 0)
	{
		QMessageBox::warning(parent, tr("Connections Editor"), tr("%1 connections have been imported from deprecated file Connections.xml.").arg(xmlConnections.count()));

		for (int i = 0; i < xmlConnections.count(); i++)
		{
			std::shared_ptr<Hardware::Connection> c = xmlConnections.get(i);

			m_connections.add(c->uuid(), c);
			m_connections.save(c->uuid(), &errorMessage);
		}

		ok = xmlConnections.deleteXmlDeprecated(&errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(parent, qAppName(), QString("Delete deprecated connection xml file error: ") + errorMessage);
		}
	}

	// fill data
	//
	fillConnectionsList();

	updateButtonsEnableState();

	// sort items
	//
	for (int i = 0; i < m_connectionsTree->columnCount(); i++)
	{
		m_connectionsTree->resizeColumnToContents(i);
	}

	m_connectionsTree->sortByColumn(theSettings.m_connectionEditorSortColumn, theSettings.m_connectionEditorSortOrder);

	connect(m_connectionsTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogConnections::onSortIndicatorChanged);

	// Restore settings
	//
	if (theSettings.m_connectionEditorWindowPos.x() != -1 && theSettings.m_connectionEditorWindowPos.y() != -1)
	{
		move(theSettings.m_connectionEditorWindowPos);
		restoreGeometry(theSettings.m_connectionEditorWindowGeometry);

		m_splitter->restoreState(theSettings.m_connectionEditorSplitterState);
		m_connectionPropertyEditor->setSplitterPosition(theSettings.m_connectionEditorPeSplitterPosition);
	}

	return;
}

DialogConnections::~DialogConnections()
{
	theSettings.m_connectionEditorWindowPos = pos();
	theSettings.m_connectionEditorWindowGeometry = saveGeometry();
	theSettings.m_connectionEditorSplitterState = m_splitter->saveState();
	theSettings.m_connectionEditorPeSplitterPosition = m_connectionPropertyEditor->splitterPosition();

	::theDialogConnections = nullptr;

	return;
}

void DialogConnections::setFilter(QString filter)
{
	assert(m_mask);

	m_mask->setText(filter);
	onMaskApply();

	return;
}

bool DialogConnections::addConnection(QString port1Id, QString port2Id)
{
	std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();

	connection->setConnectionID(tr("CONN_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')));
	connection->setPort1EquipmentID(port1Id);
	connection->setPort2EquipmentID(port2Id);

	if (port2Id.isEmpty() == true)
	{
		connection->setType(Hardware::Connection::Type::SinglePort);
	}
	else
	{
		connection->setType(Hardware::Connection::Type::PortToPort);
	}

	bool ok = addConnection(connection);
	return ok;
}

void DialogConnections::onMaskReturn()
{
	onMaskApply();
	return;
}

void DialogConnections::onMaskApply()
{
	// Get mask
	//
	QString maskText = m_mask->text();

	if (maskText.isEmpty() == false)
	{
		m_masks = maskText.split(';', QString::SkipEmptyParts);
		m_masks.removeDuplicates();

		for (QString& mask : m_masks)
		{
			mask = mask.trimmed();
		}

		for (auto mask : m_masks)
		{
			// Save filter history
			//
			if (theSettings.m_connectionEditorMasks.contains(mask) == false)
			{
				theSettings.m_connectionEditorMasks.append(mask);

				QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
				if (model == nullptr)
				{
					assert(model);
					return;
				}
				model->setStringList(theSettings.m_connectionEditorMasks);
			}
		}
	}
	else
	{
		m_masks.clear();
	}

	fillConnectionsList();

	return;
}

void DialogConnections::onMaskReset()
{
	m_mask->setText(QString());
	onMaskApply();
	return;
}

bool DialogConnections::addConnection(std::shared_ptr<Hardware::Connection> connection)
{
	if (connection == nullptr)
	{
		assert(connection);
		return false;
	}

	// Check if connection ports are not used
	//
	auto foundPort1 = m_connections.getPortConnection(connection->port1EquipmentID());
	auto foundPort2 = m_connections.getPortConnection(connection->port2EquipmentID());

	if (foundPort1 != nullptr)
	{
		QMessageBox::critical(this, tr("Connections Editor"), tr("Connection with port %1 already exists.").arg(connection->port1EquipmentID()));
		return false;
	}

	if (foundPort2 != nullptr)
	{
		QMessageBox::critical(this, tr("Connections Editor"), tr("Connection with port %1 already exists.").arg(connection->port2EquipmentID()));
		return false;
	}

	// Add connection, update UI
	//
	QString errorMessage;

	m_connections.add(connection->uuid(), connection);

	bool ok = m_connections.save(connection->uuid(), &errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save connection %1: %2").arg(connection->connectionID()).arg(errorMessage));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(0, Qt::UserRole, connection->uuid());

	m_connectionsTree->addTopLevelItem(item);

	updateTreeItemText(item);
	updateButtonsEnableState();

	m_connectionsTree->clearSelection();
	item->setSelected(true);

	return true;
}

void DialogConnections::fillConnectionsList()
{
	m_connectionsTree->clear();

	int count = m_connections.count();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<Hardware::Connection> connection = m_connections.get(i);
		if (connection == nullptr)
		{
			assert(connection);
			break;
		}

		if (m_masks.empty() == false)
		{
			bool maskResult = false;

			for (const QString& mask : m_masks)
			{
				if (connection->connectionID().contains(mask, Qt::CaseInsensitive) == true ||
					connection->port1EquipmentID().contains(mask, Qt::CaseInsensitive) == true ||
					connection->port2EquipmentID().contains(mask, Qt::CaseInsensitive) == true)
				{
					maskResult = true;
					break;
				}
			}

			if (maskResult == false)
			{
				continue;
			}
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setData(0, Qt::UserRole, connection->uuid());

		m_connectionsTree->addTopLevelItem(item);

		updateTreeItemText(item);
	}

	return;
}

void DialogConnections::setPropertyEditorObjects()
{
	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		m_connectionPropertyEditor->clear();

		updateButtonsEnableState();

		return;
	}

	bool readOnly = false;

	QList<std::shared_ptr<PropertyObject>> objects;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::Connection> connection = m_connections.get(uuid);
		if (connection == nullptr)
		{
			assert(connection);
			return;
		}

		if (m_connections.fileInfo(connection->uuid()).state() != VcsState::CheckedOut)
		{
			readOnly = true;
		}

		objects.push_back(connection);
	}

	m_connectionPropertyEditor->setExpertMode(theSettings.isExpertMode());
	m_connectionPropertyEditor->setReadOnly(readOnly);
	m_connectionPropertyEditor->setObjects(objects);

	return;
}

bool DialogConnections::continueWithDuplicateCaptions()
{
	bool duplicated = false;
	QString duplicatedCaption;

	for (int i = 0; i < m_connections.count(); i++)
	{
		Hardware::Connection* c = m_connections.get(i).get();

		for (int j = 0; j < m_connections.count(); j++)
		{
			Hardware::Connection* e = m_connections.get(j).get();
			assert(e);

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
		QString s = tr("Connection with ID '%1' already exists.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
		auto mbResult = QMessageBox::warning(this, tr("Connections Editor"), s, QMessageBox::Yes | QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}

void DialogConnections::onSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_connectionEditorSortColumn = column;
	theSettings.m_connectionEditorSortOrder = order;

	return;
}

void DialogConnections::onItemSelectionChanged()
{
	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogConnections::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	// Save modified objects
	//
	for (auto object : objects)
	{
		Hardware::Connection* c = dynamic_cast<Hardware::Connection*>(object.get());
		if (c == nullptr)
		{
			assert(c);
			continue;
		}

		QString errorMessage;
		bool ok = m_connections.save(c->uuid(), &errorMessage);

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save connection %1: %2").arg(c->connectionID()).arg(errorMessage));
			continue;
		}
	}

	// update tree items
	//
	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();
	for (auto item : selectedItems)
	{
		updateTreeItemText(item);
	}

	return;
}

void DialogConnections::onAdd()
{
	std::shared_ptr<Hardware::Connection> connection = std::make_shared<Hardware::Connection>();

	connection->setConnectionID(tr("CONN_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')));
	connection->setPort1EquipmentID("SYSTEMID_RACKID_CHID_MD00_PORT01");
	connection->setPort2EquipmentID("SYSTEMID_RACKID_CHID_MD00_PORT02");

	addConnection(connection);
	return;
}

void DialogConnections::onRemove()
{
	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, tr("Connections Editor"), tr("Are you sure you want to remove selected connections?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;

		bool ok = m_connections.removeFile(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_connections.remove(uuid);

			int index = m_connectionsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_connectionsTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				assert(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			// File was marked as deleted
			//
			updateTreeItemText(item);
		}
	}

	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogConnections::onCheckOut()
{
	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool ok = m_connections.checkOut(uuid, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		updateTreeItemText(item);
	}

	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogConnections::onCheckIn()
{
	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, tr("Connections Editor"),
											tr("Please enter the comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return;
	}

	if (comment.isEmpty())
	{
		QMessageBox::warning(this, tr("Connections Editor"), tr("No comment supplied!"));
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileWasRemoved = false;
		QString errorMessage;

		ok = m_connections.checkIn(uuid, comment, &fileWasRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileWasRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_connections.remove(uuid);

			int index = m_connectionsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_connectionsTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				assert(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			updateTreeItemText(item);
		}
	}

	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogConnections::onUndo()
{
	QList <QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, tr("Connections Editor"), tr("Are you sure you want to undo changes on selected connections?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;
		QString errorMessage;

		bool ok = m_connections.undo(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_connections.remove(uuid);

			int index = m_connectionsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_connectionsTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				assert(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			// read previous data from file

			std::shared_ptr<DbFile> file = nullptr;

			DbFileInfo fi = m_connections.fileInfo(uuid);

			bool ok = m_db->getLatestVersion(fi, &file, this);
			if (ok == true && file != nullptr)
			{
				QByteArray data;
				file->swapData(data);

				std::shared_ptr<Hardware::Connection> connection = m_connections.get(uuid);

				if (connection != nullptr && connection->Load(data) == true)
				{
					updateTreeItemText(item);
				}
			}
		}
	}

	updateButtonsEnableState();

	setPropertyEditorObjects();
}

void DialogConnections::onRefresh()
{
	m_connections.clear();

	QString errorMessage;

	bool ok = m_connections.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMessage);
		return;
	}

	fillConnectionsList();
	updateButtonsEnableState();

	return;
}

void DialogConnections::closeEvent(QCloseEvent* e)
{
	if (continueWithDuplicateCaptions() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
}

void DialogConnections::reject()
{
	if (continueWithDuplicateCaptions() == true)
	{
		QDialog::reject();
	}
}

void DialogConnections::onReport()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"),
													"./",
													tr("Text files (*.txt);; All files (*.*)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	QFile file(fileName);

	if (file.open(QFile::WriteOnly) == false)
	{
		QMessageBox::critical(this, tr("Connections Editor"), tr("Failed to create file %1!").arg(fileName));
		return;
	}

	QTextStream textStream(&file);

	textStream << tr("Radiy Platform Configuration Tool\r\n");
	textStream << qApp->applicationName() << tr(" v") << qApp->applicationVersion() << "\r\n";

	textStream << "\r\n";

	textStream << tr("Project Name:\t") << m_db->currentProject().projectName() << "\r\n";
	textStream << tr("User Name:\t") << m_db->currentUser().username() << "\r\n";

	textStream << "\r\n";



	textStream << tr("Generated at:\t") << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") << "\r\n";

	textStream << "\r\n";

	int count = m_connectionsTree->topLevelItemCount();

	textStream << tr("Connections count: ") << count << "\r\n\r\n";

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_connectionsTree->topLevelItem(i);

		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::Connection> connection = m_connections.get(uuid);
		if (connection == nullptr)
		{
			assert(connection);
			return;
		}

		textStream << tr("------------ Connection ") << i + 1 << tr(" ------------\r\n\r\n");
		textStream << tr("ConnectionID: ") << connection->connectionID() << "\r\n\r\n";
		textStream << tr("Port1 EquipmentID: ") << connection->port1EquipmentID() << "\r\n";
		textStream << tr("Port2 EquipmentID: ") << connection->port2EquipmentID() << "\r\n";
		textStream << tr(connection->manualSettings() ? "\r\nMode: manual\r\n" : "\r\nMode: automatic\r\n");

		textStream << tr("Port type: ") << connection->typeStr() << "\r\n";

		if (connection->port1EnableSerial() == true)
		{
			textStream << tr("Port 1 Serial mode: ") << connection->serialModeStr(connection->port1SerialMode()) <<"\r\n";
			textStream << tr("Port 2 Enable duplex: ") << (connection->port1EnableDuplex() ? "Yes" : "No") <<"\r\n";
		}

		if (connection->port2EnableSerial() == true)
		{
			textStream << tr("Port 2 Serial mode: ") << connection->serialModeStr(connection->port2SerialMode()) <<"\r\n";
			textStream << tr("Port 2 Enable duplex: ") << (connection->port2EnableDuplex() ? "Yes" : "No") <<"\r\n";
		}

		if (connection->manualSettings() == true)
		{
			textStream << tr("\r\nManual settings:\r\n");
			textStream << tr("Port1 start address: ") << connection->port1ManualTxStartAddress()<<"\r\n";
			textStream << tr("Port1 TX words quantity: ") << connection->port1ManualTxWordsQuantity()<<"\r\n";
			textStream << tr("Port1 RX words quantity: ") << connection->port1ManualRxWordsQuantity()<<"\r\n";

			textStream << tr("Port2 start address: ") << connection->port2ManualTxStartAddress()<<"\r\n";
			textStream << tr("Port2 TX words quantity: ") << connection->port2ManualTxWordsQuantity()<<"\r\n";
			textStream << tr("Port2 RX words quantity: ") << connection->port2ManualRxWordsQuantity()<<"\r\n";
		}

		textStream<<"\r\n";
	}

	textStream.flush();

	file.close();
}

void DialogConnections::onCustomContextMenuRequested(const QPoint &pos)
{
	Q_UNUSED(pos);

	m_popupMenu->exec(this->cursor().pos());
}


void DialogConnections::updateTreeItemText(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();

	std::shared_ptr<Hardware::Connection> connection = m_connections.get(uuid);
	if (connection == nullptr)
	{
		assert(connection);
		return;
	}

	int c = 0;
	item->setText(c++, connection->connectionID());
	item->setText(c++, connection->manualSettings() ? tr("Manual") : tr("Auto"));

	DbFileInfo fi = m_connections.fileInfo(connection->uuid());

	if (fi.state() == VcsState::CheckedOut)
	{
		item->setText(c++, fi.action().text());

		int userId = fi.userId();
		item->setText(c++, m_db->username(userId));
	}
	else
	{
		item->setText(c++, "");
		item->setText(c++, "");
	}

	return;
}

void DialogConnections::updateButtonsEnableState()
{
	int selectedCount = 0;
	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<QTreeWidgetItem*> selectedItems = m_connectionsTree->selectedItems();

	selectedCount = selectedItems.size();

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::Connection> connection = m_connections.get(uuid);
		if (connection == nullptr)
		{
			assert(connection);
			return;
		}

		if (m_connections.fileInfo(connection->uuid()).state() == VcsState::CheckedOut)
		{
			checkedOutCount++;
		}
		else
		{
			checkedInCount++;
		}
	}

	m_btnRemove->setEnabled(selectedCount > 0);
	m_removeAction->setEnabled(selectedCount > 0);

	m_btnCheckOut->setEnabled(selectedCount > 0 && checkedInCount > 0);
	m_checkOutAction->setEnabled(selectedCount > 0 && checkedInCount > 0);

	m_btnCheckIn->setEnabled(selectedCount > 0 && checkedOutCount > 0);
	m_checkInAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);

	m_btnUndo->setEnabled(selectedCount > 0 && checkedOutCount > 0);
	m_undoAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);

	return;
}



