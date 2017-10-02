#include "DialogBusEditor.h"

#include "Settings.h"

#include <QTreeWidget>
#include <QMenu>
#include <QGridLayout>
#include <QInputDialog>
#include "../lib/PropertyEditorDialog.h"

//
// DialogBusEditor
//

DialogBusEditor* theDialogBusEditor = nullptr;

DialogBusEditor::DialogBusEditor(DbController* db, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	m_busses(db),
	m_db(db)
{
	assert(m_db);
	setWindowTitle(tr("Bus Types Editor"));

	setAttribute(Qt::WA_DeleteOnClose);

	setMinimumSize(900, 400);

	// Create user interface
	//
	m_propEditorDialog = new PropertyEditorDialog(this);

	// Add signal context menu
	//
	m_addSignalMenu = new QMenu(this);

	m_analogAction = new QAction("Analog signal", this);
	m_discreteAction = new QAction("Discrete signal", this);

	connect(m_analogAction, &QAction::triggered, this, [this](){emit onSignalCreate(E::SignalType::Analog);});
	connect(m_discreteAction, &QAction::triggered, this, [this](){emit onSignalCreate(E::SignalType::Discrete);});

	m_addSignalMenu->addAction(m_analogAction);
	m_addSignalMenu->addAction(m_discreteAction);

	// m_busTree
	//
	m_busTree = new QTreeWidget();

	QStringList l;
	l << tr("BusTypeID");
	l << tr("State");
	l << tr("User");

	m_busTree->setColumnCount(l.size());
	m_busTree->setHeaderLabels(l);

	int il = 0;
	m_busTree->setColumnWidth(il++, 140);
	m_busTree->setColumnWidth(il++, 40);
	m_busTree->setColumnWidth(il++, 40);

	m_busTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_busTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_busTree->setRootIsDecorated(false);

	connect(m_busTree, &QTreeWidget::itemSelectionChanged, this, &DialogBusEditor::onBusSelectionChanged);
	connect(m_busTree, &QWidget::customContextMenuRequested, this, &DialogBusEditor::onBusCustomContextMenuRequested);

	// leftButtonsLayout
	//
	QHBoxLayout* leftButtonsLayout = new QHBoxLayout();

	m_buttonAdd = new QPushButton(tr("Add"));
	m_buttonCheckOut = new QPushButton(tr("Check Out"));
	m_buttonCheckIn = new QPushButton(tr("Check In"));
	m_buttonUndo = new QPushButton(tr("Undo"));

	leftButtonsLayout->addWidget(m_buttonAdd);
	leftButtonsLayout->addWidget(m_buttonCheckOut);
	leftButtonsLayout->addWidget(m_buttonCheckIn);
	leftButtonsLayout->addWidget(m_buttonUndo);
	leftButtonsLayout->addStretch();

	connect(m_buttonAdd, &QPushButton::clicked, this, &DialogBusEditor::onAdd);
	connect(m_buttonCheckOut, &QPushButton::clicked, this, &DialogBusEditor::onCheckOut);
	connect(m_buttonCheckIn, &QPushButton::clicked, this, &DialogBusEditor::onCheckIn);
	connect(m_buttonUndo, &QPushButton::clicked, this, &DialogBusEditor::onUndo);

	// m_busPropertyEditor

	m_busPropertyEditor = new ExtWidgets::PropertyEditor(this);
	m_busPropertyEditor->setExpertMode(theSettings.isExpertMode());

	connect(m_busPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogBusEditor::onBusPropertiesChanged);

	// m_signalsTree
	//
	m_signalsTree = new QTreeWidget();

	l.clear();
	l << tr("SignalID");
	l << tr("Type");
	l << tr("Signal Format");

	m_signalsTree->setColumnCount(l.size());
	m_signalsTree->setHeaderLabels(l);

	il = 0;
	m_signalsTree->setColumnWidth(il++, 150);
	m_signalsTree->setColumnWidth(il++, 60);
	m_signalsTree->setColumnWidth(il++, 50);

	m_signalsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_signalsTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_signalsTree->setRootIsDecorated(false);

	connect(m_signalsTree, &QTreeWidget::itemSelectionChanged, this, &DialogBusEditor::onSignalItemSelectionChanged);
	connect(m_signalsTree, &QWidget::customContextMenuRequested, this, &DialogBusEditor::onSignalCustomContextMenuRequested);
	connect(m_signalsTree, &QTreeWidget::itemDoubleClicked, this, &DialogBusEditor::onSignalItemDoubleClicked);

	// Signals buttons

	QVBoxLayout* signalsButtonsLayout = new QVBoxLayout();

	m_btnSignalAdd = new QPushButton(tr("Add"));
	m_btnSignalEdit = new QPushButton(tr("Edit"));
	m_btnSignalRemove = new QPushButton(tr("Remove"));
	m_btnSignalUp = new QPushButton(tr("Move Up"));
	m_btnSignalDown = new QPushButton(tr("Move Down"));

	connect (m_btnSignalAdd, &QPushButton::clicked, this, &DialogBusEditor::onSignalAdd);
	connect (m_btnSignalEdit, &QPushButton::clicked, this, &DialogBusEditor::onSignalEdit);
	connect (m_btnSignalRemove, &QPushButton::clicked, this, &DialogBusEditor::onSignalRemove);
	connect (m_btnSignalUp, &QPushButton::clicked, this, &DialogBusEditor::onSignalUp);
	connect (m_btnSignalDown, &QPushButton::clicked, this, &DialogBusEditor::onSignalDown);

	signalsButtonsLayout->addWidget(m_btnSignalAdd);
	signalsButtonsLayout->addWidget(m_btnSignalEdit);
	signalsButtonsLayout->addWidget(m_btnSignalRemove);
	signalsButtonsLayout->addWidget(m_btnSignalUp);
	signalsButtonsLayout->addWidget(m_btnSignalDown);
	signalsButtonsLayout->addStretch();

	// Signals layout

	QHBoxLayout* signalsEditLayout = new QHBoxLayout();
	signalsEditLayout->setContentsMargins(0, 0, 0, 0);

	signalsEditLayout->addWidget(m_signalsTree);
	signalsEditLayout->addLayout(signalsButtonsLayout);

	// Right buttons layout
	//
	QHBoxLayout* rightButtonsLayout = new QHBoxLayout();

	m_btnClose = new QPushButton(tr("Close"));

	rightButtonsLayout->addStretch();
	rightButtonsLayout->addWidget(m_btnClose);

	connect (m_btnClose, &QPushButton::clicked, this, &DialogBusEditor::close);

	// Left side
	//

	QVBoxLayout* leftLayout = new QVBoxLayout();
	leftLayout->setContentsMargins(0, 0, 0, 0);

	leftLayout->addWidget(m_busTree);
	leftLayout->addLayout(leftButtonsLayout);

	QWidget* lw = new QWidget();
	lw->setLayout(leftLayout);

	// Right side
	//
	QWidget* sw = new QWidget();
	sw->setLayout(signalsEditLayout);

	m_rightSplitter = new QSplitter(Qt::Vertical);
	m_rightSplitter->setChildrenCollapsible(false);
	m_rightSplitter->addWidget(m_busPropertyEditor);
	m_rightSplitter->addWidget(sw);

	QVBoxLayout* rightLayout = new QVBoxLayout();
	rightLayout->setContentsMargins(0, 0, 0, 0);

	rightLayout->addWidget(m_rightSplitter);
	rightLayout->addLayout(rightButtonsLayout);

	QWidget* rw = new QWidget();
	rw->setLayout(rightLayout);

	// Main splitter

	m_mainSplitter = new QSplitter(Qt::Horizontal);
	m_mainSplitter->setChildrenCollapsible(false);

	m_mainSplitter->addWidget(lw);
	m_mainSplitter->addWidget(rw);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_mainSplitter);

	setLayout(mainLayout);

	// Bus Popup menu
	//
	m_addAction = new QAction(tr("Add"), this);
	m_removeAction = new QAction(tr("Remove"), this);
	m_cloneAction = new QAction(tr("Clone"), this);
	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkInAction = new QAction(tr("Check In"), this);
	m_undoAction = new QAction(tr("Undo"), this);
	m_refreshAction = new QAction(tr("Refresh"), this);

	connect(m_addAction, &QAction::triggered, this, &DialogBusEditor::onAdd);
	connect(m_removeAction, &QAction::triggered, this, &DialogBusEditor::onRemove);
	connect(m_cloneAction, &QAction::triggered, this, &DialogBusEditor::onClone);
	connect(m_checkOutAction, &QAction::triggered, this, &DialogBusEditor::onCheckOut);
	connect(m_checkInAction, &QAction::triggered, this, &DialogBusEditor::onCheckIn);
	connect(m_undoAction, &QAction::triggered, this, &DialogBusEditor::onUndo);
	connect(m_refreshAction, &QAction::triggered, this, &DialogBusEditor::onRefresh);

	m_popupMenu = new QMenu(this);
	m_popupMenu->addAction(m_addAction);
	m_popupMenu->addAction(m_removeAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_cloneAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_checkOutAction);
	m_popupMenu->addAction(m_checkInAction);
	m_popupMenu->addAction(m_undoAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_refreshAction);

	// Signal Popup menu
	//
	m_signalAddAction = new QAction("Add", this);
	m_signalEditAction = new QAction("Edit", this);
	m_signalRemoveAction = new QAction("Remove", this);
	m_signalUpAction = new QAction("Move Up", this);
	m_signalDownAction = new QAction("Move Down", this);

	connect(m_signalAddAction, &QAction::triggered, this, &DialogBusEditor::onSignalAdd);
	connect(m_signalEditAction, &QAction::triggered, this, &DialogBusEditor::onSignalEdit);
	connect(m_signalRemoveAction, &QAction::triggered, this, &DialogBusEditor::onSignalRemove);
	connect(m_signalUpAction, &QAction::triggered, this, &DialogBusEditor::onSignalUp);
	connect(m_signalDownAction, &QAction::triggered, this, &DialogBusEditor::onSignalDown);

	m_signalPopupMenu = new QMenu(this);
	m_signalAddSubmenuAction = m_signalPopupMenu->addMenu(m_addSignalMenu);
	m_signalAddSubmenuAction->setText("Add");
	m_signalPopupMenu->addAction(m_signalEditAction);
	m_signalPopupMenu->addAction(m_signalRemoveAction);
	m_signalPopupMenu->addSeparator();
	m_signalPopupMenu->addAction(m_signalUpAction);
	m_signalPopupMenu->addAction(m_signalDownAction);

	// Load buses
	//
	QString errorMessage;

	if (m_busses.load(&errorMessage) == false)
	{
		QMessageBox::critical(parent, qAppName(), tr("Bus loading error: %1").arg(errorMessage));
		return;
	}

	// Fill data
	//
	fillBusList();

	updateButtonsEnableState();

	// Sort items
	//
	for (int i = 0; i < m_busTree->columnCount(); i++)
	{
		m_busTree->resizeColumnToContents(i);
	}

	m_busTree->sortByColumn(theSettings.m_busEditorSortColumn, theSettings.m_busEditorSortOrder);
	m_busTree->setSortingEnabled(true);

	connect(m_busTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogBusEditor::onBusSortIndicatorChanged);

	// Restore settings
	//
	if (theSettings.m_busEditorWindowPos.x() != -1 && theSettings.m_busEditorWindowPos.y() != -1)
	{
		move(theSettings.m_busEditorWindowPos);
		restoreGeometry(theSettings.m_busEditorWindowGeometry);

		m_mainSplitter->restoreState(theSettings.m_busEditorMainSplitterState);
		m_rightSplitter->restoreState(theSettings.m_busEditorRightSplitterState);
		m_busPropertyEditor->setSplitterPosition(theSettings.m_busEditorPropertySplitterPosition);

		m_propEditorDialog->resize(theSettings.m_busEditorPeWindowSize);
		m_propEditorDialog->setSplitterPosition(theSettings.m_busEditorPeSplitterPosition);
	}
	else
	{
		int w = 300;
		int h = 400;

		m_propEditorDialog->resize(w, h);
	}

	return;
}

DialogBusEditor::~DialogBusEditor()
{
	theSettings.m_busEditorWindowPos = pos();
	theSettings.m_busEditorWindowGeometry = saveGeometry();
	theSettings.m_busEditorMainSplitterState = m_mainSplitter->saveState();
	theSettings.m_busEditorRightSplitterState = m_rightSplitter->saveState();
	theSettings.m_busEditorPropertySplitterPosition = m_busPropertyEditor->splitterPosition();

	theSettings.m_busEditorPeWindowSize = m_propEditorDialog->size();

	theSettings.m_busEditorPeSplitterPosition = m_propEditorDialog->splitterPosition();

	::theDialogBusEditor = nullptr;

	return;
}

void DialogBusEditor::onAdd()
{
	std::shared_ptr<VFrame30::Bus> bus = std::make_shared<VFrame30::Bus>();

	bool ok = false;

	QString busTypeId = QInputDialog::getText(this, tr("Add Bus"),
										 tr("Enter bus type ID:"), QLineEdit::Normal,
										 tr("BUSTYPEID_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')), &ok);

	if (ok == false || busTypeId.isEmpty() == true)
	{
		return;
	}

	int count = m_busses.count();
	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<VFrame30::Bus> existingBus = m_busses.get(i);

		if (existingBus->busTypeId() == busTypeId)
		{
			QMessageBox::critical(this, qAppName(), tr("A bus with specified type ID already exists!"));
			return;
		}
	}

	bus->setUuid(QUuid::createUuid());
	bus->setBusTypeId(busTypeId);

	addBus(bus);

	return;
}

void DialogBusEditor::onRemove()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected busses?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;
		QString errorMessage;

		bool ok = m_busses.removeFile(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses.remove(uuid);

			int index = m_busTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_busTree->takeTopLevelItem(index);
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

			updateBusTreeItemText(item);
		}
	}

	updateButtonsEnableState();
	fillBusProperties();
	fillBusSignals();

	return;
}

void DialogBusEditor::onClone()
{
	VFrame30::Bus* bus = getCurrentBus();
	if (bus == nullptr)
	{
		return;
	}

	std::shared_ptr<VFrame30::Bus> cloneBus = std::make_shared<VFrame30::Bus>(*bus);

	bool ok = false;

	QString busTypeId = QInputDialog::getText(this, tr("Clone Bus"),
										 tr("Enter bus type ID:"), QLineEdit::Normal,
										 cloneBus->busTypeId() + " (clone)", &ok);

	if (ok == false || busTypeId.isEmpty() == true)
	{
		return;
	}

	int count = m_busses.count();
	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<VFrame30::Bus> existingBus = m_busses.get(i);
		if (existingBus->busTypeId() == busTypeId)
		{
			QMessageBox::critical(this, qAppName(), tr("A bus with specified type ID already exists!"));
			return;
		}
	}

	cloneBus->setUuid(QUuid::createUuid());
	cloneBus->setBusTypeId(busTypeId);

	addBus(cloneBus);

}

void DialogBusEditor::onCheckOut()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool ok = m_busses.checkOut(uuid, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		updateBusTreeItemText(item);
	}

	updateButtonsEnableState();
	fillBusProperties();

	return;
}

void DialogBusEditor::onCheckIn()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, qAppName(),
											tr("Please enter the comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return;
	}

	if (comment.isEmpty())
	{
		QMessageBox::warning(this, qAppName(), tr("No comment supplied!"));
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileWasRemoved = false;
		QString errorMessage;

		bool ok = m_busses.checkIn(uuid, comment, &fileWasRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileWasRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses.remove(uuid);

			int index = m_busTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_busTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				assert(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			updateBusTreeItemText(item);
		}
	}

	updateButtonsEnableState();
	fillBusProperties();

	return;
}

void DialogBusEditor::onUndo()
{
	QList <QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to undo changes on selected busses?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;
		QString errorMessage;

		if (m_busses.undo(uuid, &fileRemoved, &errorMessage) == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses.remove(uuid);

			int index = m_busTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_busTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				assert(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			// Read previous data from file
			//

			bool ok = reloadBus(uuid);
			if (ok == true)
			{
				updateBusTreeItemText(item);

				fillBusProperties();

				fillBusSignals();
			}
		}
	}

	updateButtonsEnableState();
	fillBusProperties();

	return;
}

void DialogBusEditor::onRefresh()
{
	m_busses.clear();

	QString errorMessage;

	if (m_busses.load(&errorMessage) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Bus loading error: %1").arg(errorMessage));
		return;
	}

	fillBusList();
	updateButtonsEnableState();

	return;
}

void DialogBusEditor::onSignalAdd()
{
	m_addSignalMenu->exec(this->cursor().pos());
}

void DialogBusEditor::onSignalCreate(E::SignalType type)
{
	QUuid uuid;

	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	VFrame30::BusSignal bs(type);

	bool ok = false;
	QString defaultSignalId = QString("sid_%1").arg(bus->busSignals().size());

	QString signalId = QInputDialog::getText(this, tr("Add Signal"),
												  tr("Enter SignalID:"), QLineEdit::Normal,
												  defaultSignalId, &ok);
	signalId = signalId.trimmed();

	if (ok == false || signalId.isEmpty() == true)
	{
		return;
	}

	bs.setSignalId(signalId);
	bs.setCaption(signalId);

	const std::vector<VFrame30::BusSignal>& busSignals = bus->busSignals();
	for (const VFrame30::BusSignal& checkBs : busSignals)
	{
		if (checkBs.signalId() == bs.signalId())
		{
			QMessageBox::critical(this, qAppName(), tr("A signal with SignalID '%1' already exists!").arg(bs.signalId()));
			return;
		}
	}

	bus->addSignal(bs);

	QTreeWidgetItem* item = new QTreeWidgetItem();

	updateSignalsTreeItemText(item, bs);

	item->setData(0, Qt::UserRole, m_signalsTree->topLevelItemCount());

	m_signalsTree->addTopLevelItem(item);

	saveBus(uuid);

	return;
}

void DialogBusEditor::onSignalEdit()
{
	QUuid uuid;
	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_signalsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	// Create a vector with pointers to objects
	//
	std::vector<VFrame30::BusSignal> busSignals = bus->busSignals();

	QList<std::shared_ptr<PropertyObject>> editSignalsPointers;

	std::vector<int> editIndexes;

	for (QTreeWidgetItem* item : selectedItems)
	{
		int index = item->data(0, Qt::UserRole).toInt();

		if (index < 0 || index >= static_cast<int>(busSignals.size()))
		{
			assert(false);
			return;
		}

		editIndexes.push_back(index);

		std::shared_ptr<VFrame30::BusSignal> bs = std::make_shared<VFrame30::BusSignal>(busSignals[index].type());

		*bs = busSignals[index];

		editSignalsPointers.push_back(bs);
	}

	bool readOnly = m_busses.fileInfo(uuid).state() != VcsState::CheckedOut;

	m_propEditorDialog->setReadOnly(readOnly);
	m_propEditorDialog->setObjects(editSignalsPointers);

	// Run property editor
	//
	if (m_propEditorDialog->exec() == QDialog::Accepted && readOnly == false)
	{
		// Save data back to bus
		//
		if (editIndexes.size() != editSignalsPointers.size())
		{
			assert(false);
			return;
		}

		for (int i = 0; i < editIndexes.size(); i++)
		{
			int editIndex = editIndexes[i];

			VFrame30::BusSignal* editSignal = (dynamic_cast<VFrame30::BusSignal*>(editSignalsPointers[i].get()));
			if (editSignal == nullptr)
			{
				assert(editSignal);
				return;
			}

			// Check for duplicate SignalIDs
			//

			bool duplicateSignalIds = false;

			for (int j = 0; j < busSignals.size(); j++)
			{
				if (editIndex != j && busSignals[j].signalId() == editSignal->signalId())
				{
					QMessageBox::critical(this, qAppName(), tr("A signal with SignalID '%1' already exists!").arg(busSignals[j].signalId()));
					duplicateSignalIds = true;
					break;
				}
			}

			if (duplicateSignalIds == true)
			{
				continue;
			}

			// Update the signal
			//

			busSignals[editIndex] = *editSignal;

			updateSignalsTreeItemText(selectedItems[i], busSignals[editIndex]);
		}

		bus->setBusSignals(busSignals);

		saveBus(uuid);
	}

	return;
}

void DialogBusEditor::onSignalRemove()
{
	QUuid uuid;
	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_signalsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected signals?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	std::vector<int> indexesToDelete;

	for (QTreeWidgetItem* item : selectedItems)
	{
		indexesToDelete.push_back(item->data(0, Qt::UserRole).toInt());
	}

	std::sort(indexesToDelete.begin(), indexesToDelete.end());

	for (int i = static_cast<int>(indexesToDelete.size() - 1); i >= 0; i--)
	{
		bus->removeSignalAt(indexesToDelete[i]);
	}

	fillBusSignals();

	saveBus(uuid);

	return;
}

void DialogBusEditor::onSignalUp()
{
	QUuid uuid;
	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_signalsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	std::vector<int> selectedIndexes;

	for (QTreeWidgetItem* item : selectedItems)
	{
		selectedIndexes.push_back(item->data(0, Qt::UserRole).toInt());
	}

	std::vector<VFrame30::BusSignal> busSignals = bus->busSignals();

	std::sort(selectedIndexes.begin(), selectedIndexes.end());

	for (int i = 0; i < static_cast<int>(selectedIndexes.size()); i++)
	{
		int index = selectedIndexes[i];
		if (index == 0)
		{
			break;
		}

		VFrame30::BusSignal temp = busSignals[index];
		busSignals[index] = busSignals[index - 1];
		busSignals[index - 1] = temp;

		QTreeWidgetItem* item1 = m_signalsTree->topLevelItem(index - 1);
		QTreeWidgetItem* item2 = m_signalsTree->topLevelItem(index);

		if (item1 == nullptr || item2 == nullptr)
		{
			assert(false);
			return;
		}

		updateSignalsTreeItemText(item1, busSignals[index - 1]);
		updateSignalsTreeItemText(item2, busSignals[index]);

		item1->setSelected(true);
		item2->setSelected(false);
	}

	bus->setBusSignals(busSignals);

	saveBus(uuid);

	return;
}

void DialogBusEditor::onSignalDown()
{
	QUuid uuid;
	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = m_signalsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	std::vector<int> selectedIndexes;

	for (QTreeWidgetItem* item : selectedItems)
	{
		selectedIndexes.push_back(item->data(0, Qt::UserRole).toInt());
	}

	std::vector<VFrame30::BusSignal> busSignals = bus->busSignals();

	std::sort(selectedIndexes.begin(), selectedIndexes.end());

	for (int i = static_cast<int>(selectedIndexes.size() - 1); i >= 0; i--)
	{
		int index = selectedIndexes[i];
		if (index >= static_cast<int>(busSignals.size() - 1))
		{
			break;
		}

		VFrame30::BusSignal temp = busSignals[index];
		busSignals[index] = busSignals[index + 1];
		busSignals[index + 1] = temp;

		QTreeWidgetItem* item1 = m_signalsTree->topLevelItem(index);
		QTreeWidgetItem* item2 = m_signalsTree->topLevelItem(index + 1);

		if (item1 == nullptr || item2 == nullptr)
		{
			assert(false);
			return;
		}

		updateSignalsTreeItemText(item1, busSignals[index]);
		updateSignalsTreeItemText(item2, busSignals[index + 1]);

		item1->setSelected(false);
		item2->setSelected(true);
	}

	bus->setBusSignals(busSignals);

	saveBus(uuid);

	return;
}

void DialogBusEditor::onSignalItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	Q_UNUSED(column);

	if (item != nullptr)
	{
		onSignalEdit();
	}
}

void DialogBusEditor::keyPressEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
	{
		return;
	}
	QDialog::keyPressEvent(evt);
}

void DialogBusEditor::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void DialogBusEditor::reject()
{
	QDialog::reject();
}

void DialogBusEditor::onBusPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	bool refillPropeties = false;

	for (const std::shared_ptr<PropertyObject>& object : objects)
	{
		const VFrame30::Bus* editBus = dynamic_cast<VFrame30::Bus*>(object.get());
		if (editBus == nullptr)
		{
			assert(editBus);
			return;
		}

		// Check if bus ID already exists

		bool alreadyExists = false;

		int count = m_busses.count();
		for (int i = 0; i < count; i++)
		{
			const std::shared_ptr<VFrame30::Bus> bus = m_busses.get(i);
			if (bus->busTypeId() == editBus->busTypeId() && bus->uuid() != editBus->uuid())
			{
				QMessageBox::critical(this, qAppName(), tr("A bus with type ID '%1' already exists!").arg(bus->busTypeId()));
				alreadyExists = true;
				break;
			}
		}

		// Skip if bus ID already exists

		if (alreadyExists == true)
		{
			refillPropeties = true;

			reloadBus(editBus->uuid());

			continue;
		}

		// Save and update bus information

		saveBus(editBus->uuid());

		for (QTreeWidgetItem* item : m_busTree->selectedItems())
		{
			QUuid uuid = item->data(0, Qt::UserRole).toUuid();

			if (editBus->uuid() == uuid)
			{
				updateBusTreeItemText(item, editBus);
			}
		}
	}

	if (refillPropeties == true)
	{
		fillBusProperties();
	}

	return;
}

void DialogBusEditor::onBusSelectionChanged()
{
	updateButtonsEnableState();
	fillBusProperties();
	fillBusSignals();

	return;
}

void DialogBusEditor::onBusCustomContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_popupMenu->exec(this->cursor().pos());
}

void DialogBusEditor::onBusSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_busEditorSortColumn = column;
	theSettings.m_busEditorSortOrder = order;

	return;
}

void DialogBusEditor::onSignalItemSelectionChanged()
{
	updateButtonsEnableState();

	return;
}

void DialogBusEditor::onSignalCustomContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_signalPopupMenu->exec(this->cursor().pos());
}

void DialogBusEditor::fillBusList()
{
	m_busTree->clear();

	int count = m_busses.count();

	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<VFrame30::Bus> bus = m_busses.get(i);

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setData(0, Qt::UserRole, bus->uuid());

		m_busTree->addTopLevelItem(item);

		updateBusTreeItemText(item);
	}

	for (int i = 0; i < m_busTree->columnCount(); i++)
	{
		m_busTree->resizeColumnToContents(i);
	}

	return;
}

void DialogBusEditor::fillBusProperties()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.empty() == true)
	{
		m_busPropertyEditor->clear();
		return;
	}

	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<std::shared_ptr<PropertyObject>> busObjects;

	for (QTreeWidgetItem* item : selectedItems)
	{
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		QVariant d = item->data(0, Qt::UserRole);
		if (d.isNull() || d.isValid() == false)
		{
			assert(false);
			return;
		}

		QUuid uuid = d.toUuid();

		std::shared_ptr<VFrame30::Bus> bus = m_busses.get(uuid);

		busObjects.push_back(bus);

		if (m_busses.fileInfo(bus->uuid()).state() == VcsState::CheckedOut)
		{
			checkedOutCount++;
		}
		else
		{
			checkedInCount++;
		}
	}

	bool readOnly = checkedInCount != 0 || checkedOutCount < selectedItems.size();

	m_busPropertyEditor->setReadOnly(readOnly);
	m_busPropertyEditor->setObjects(busObjects);

	return;

}

void DialogBusEditor::fillBusSignals()
{
	m_signalsTree->clear();

	VFrame30::Bus* bus = getCurrentBus();
	if (bus == nullptr)
	{
		return;
	}

	std::vector<VFrame30::BusSignal> busSignals = bus->busSignals();

	for (int i = 0; i < static_cast<int>(busSignals.size()); i++)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();

		updateSignalsTreeItemText(item, busSignals[i]);

		item->setData(0, Qt::UserRole, i);

		m_signalsTree->addTopLevelItem(item);
	}

	for (int i = 0; i < m_signalsTree->columnCount(); i++)
	{
		m_signalsTree->resizeColumnToContents(i);
	}

	return;
}

bool DialogBusEditor::addBus(const std::shared_ptr<VFrame30::Bus> bus)
{
	// Add bus, update UI
	//
	m_busses.add(bus->uuid(), bus);

	bool ok = saveBus(bus->uuid());
	if (ok == false)
	{
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(0, Qt::UserRole, bus->uuid());

	item->setFlags(item->flags() | Qt::ItemIsEditable);

	m_busTree->addTopLevelItem(item);

	updateBusTreeItemText(item);
	updateButtonsEnableState();

	m_busTree->setCurrentItem(item);
	m_busTree->clearSelection();
	item->setSelected(true);

	fillBusProperties();

	return true;
}

void DialogBusEditor::updateButtonsEnableState()
{
	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<QTreeWidgetItem*> selectedBusItems = m_busTree->selectedItems();

	int selectedBusCount = selectedBusItems.size();

	for (auto item : selectedBusItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		const std::shared_ptr<VFrame30::Bus> bus = m_busses.get(uuid);

		if (m_busses.fileInfo(bus->uuid()).state() == VcsState::CheckedOut)
		{
			checkedOutCount++;
		}
		else
		{
			checkedInCount++;
		}
	}

	QList<QTreeWidgetItem*> selectedSignalItems = m_signalsTree->selectedItems();

	int selectedSignalCount = selectedSignalItems.size();

	// --
	//
	m_removeAction->setEnabled(selectedBusCount > 0);
	m_cloneAction->setEnabled(selectedBusCount == 1);

	m_buttonCheckOut->setEnabled(selectedBusCount > 0 && checkedInCount > 0);
	m_checkOutAction->setEnabled(selectedBusCount > 0 && checkedInCount > 0);

	m_buttonCheckIn->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);
	m_checkInAction->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);

	m_buttonUndo->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);
	m_undoAction->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);

	// --
	//
	m_btnSignalAdd->setEnabled(checkedOutCount == 1);
	m_signalAddAction->setEnabled(checkedOutCount == 1);
	m_signalAddSubmenuAction->setEnabled(checkedOutCount == 1);

	m_btnSignalEdit->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);
	m_signalEditAction->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);

	m_btnSignalRemove->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);
	m_signalRemoveAction->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);

	m_btnSignalUp->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);
	m_signalUpAction->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);

	m_btnSignalDown->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);
	m_signalDownAction->setEnabled(checkedOutCount == 1 && selectedSignalCount > 0);

	return;
}

void DialogBusEditor::updateBusTreeItemText(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();

	const std::shared_ptr<VFrame30::Bus> bus = m_busses.get(uuid);

	updateBusTreeItemText(item, bus.get());

	DbFileInfo fi = m_busses.fileInfo(uuid);

	if (fi.state() == VcsState::CheckedOut)
	{
		item->setText(1, fi.action().text());

		int userId = fi.userId();
		item->setText(2, m_db->username(userId));
	}
	else
	{
		item->setText(1, "");
		item->setText(2, "");
	}

	return;
}

void DialogBusEditor::updateBusTreeItemText(QTreeWidgetItem* item, const VFrame30::Bus* bus)
{
	if (bus == nullptr)
	{
		assert(bus);
		return;
	}

	item->setText(0, bus->busTypeId());
	return;
}

void DialogBusEditor::updateSignalsTreeItemText(QTreeWidgetItem* item, const VFrame30::BusSignal& signal)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setText(0, signal.signalId());
	item->setText(1, E::valueToString<E::SignalType>(signal.type()));

	if (signal.type() == E::SignalType::Analog)
	{
		item->setText(2, E::valueToString<E::AnalogAppSignalFormat>(signal.analogFormat()));
	}
	else
	{
		assert(signal.type() == E::SignalType::Discrete);
		item->setText(2, "");
	}

	return;
}

VFrame30::Bus* DialogBusEditor::getCurrentBus(QUuid* uuid)
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();
	if (selectedItems.size() != 1)
	{
		return nullptr;
	}

	QTreeWidgetItem* item = selectedItems[0];
	if (item == nullptr)
	{
		return nullptr;
	}

	QUuid itemUuid = item->data(0, Qt::UserRole).toUuid();

	if (uuid != nullptr)
	{
		*uuid = itemUuid;
	}

	return m_busses.get(itemUuid).get();
}

bool DialogBusEditor::reloadBus(const QUuid& busUuid)
{
	bool ok = m_busses.reload(busUuid);

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to reload bus with UUID=").arg(busUuid.toString()));
		return false;
	}

	return true;
}

bool DialogBusEditor::saveBus(const QUuid& busUuid)
{
	QString errorMessage;

	bool ok = m_busses.save(busUuid, &errorMessage);

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save bus: ").arg(errorMessage));
		return false;
	}

	return true;
}
