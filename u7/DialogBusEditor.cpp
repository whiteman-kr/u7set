#include "DialogBusEditor.h"

#include "Settings.h"

#include <QTreeWidget>
#include <QMenu>
#include <QGridLayout>
#include "../lib/PropertyEditorDialog.h"

//
//
// DialogBusEditorDelegate
//
//

DialogBusEditorDelegate::DialogBusEditorDelegate(QObject *parent)
	:QItemDelegate(parent)
{
}

QWidget* DialogBusEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);

	if(index.column() == 0)
	{
		QLineEdit* edit = new QLineEdit(parent);

		QRegExp rx("^[A-Za-z][A-Za-z\\d]*$");
		edit->setValidator(new QRegExpValidator(rx, edit));

		return edit;
	}

	return nullptr;
}

void DialogBusEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
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

void DialogBusEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
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
// DialogBusEditor
//

DialogBusEditor* theDialogBusEditor = nullptr;

DialogBusEditor::DialogBusEditor(DbController* pDbController, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	m_db(pDbController)
{
	m_busses = new BusStorage(m_db, this);

	setWindowTitle(tr("Bus Types Editor"));

	setAttribute(Qt::WA_DeleteOnClose);

	setMinimumSize(900, 400);

	// Create user interface
	//

	m_peDialog = new PropertyEditorDialog(this);

	// Add signal context menu

	m_addSignalMenu = new QMenu(this);

	m_analogAction = new QAction("Analog signal", this);
	m_discreteAction = new QAction("Discrete signal", this);
	m_busAction = new QAction("Bus signal", this);

	connect(m_analogAction, &QAction::triggered, this, [this](){emit onSignalCreate(E::SignalType::Analog);});
	connect(m_discreteAction, &QAction::triggered, this, [this](){emit onSignalCreate(E::SignalType::Discrete);});
	connect(m_busAction, &QAction::triggered, this, [this](){emit onSignalCreate(E::SignalType::Bus);});

	m_addSignalMenu->addAction(m_analogAction);
	m_addSignalMenu->addAction(m_discreteAction);
	m_addSignalMenu->addAction(m_busAction);

	// m_busTree

	m_busTree = new QTreeWidget();

	QStringList l;
	l << tr("Bus Type ID");
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

	connect(m_busTree, &QTreeWidget::itemSelectionChanged, this, &DialogBusEditor::onBusItemSelectionChanged);
	connect(m_busTree, &QWidget::customContextMenuRequested, this, &DialogBusEditor::onBusCustomContextMenuRequested);

	connect(m_busTree, &QTreeWidget::itemChanged, this, &DialogBusEditor::onBusItemChanged);

	DialogBusEditorDelegate* editorDelegate = new DialogBusEditorDelegate(this);
	m_busTree->setItemDelegate(editorDelegate);

	//

	// m_signalsTree

	m_signalsTree = new QTreeWidget();

	l.clear();
	l << tr("Name");
	l << tr("Type");
	l << tr("Signal Format");
	l << tr("Bus Type ID");

	m_signalsTree->setColumnCount(l.size());
	m_signalsTree->setHeaderLabels(l);

	il = 0;
	m_signalsTree->setColumnWidth(il++, 140);
	m_signalsTree->setColumnWidth(il++, 40);
	m_signalsTree->setColumnWidth(il++, 40);
	m_signalsTree->setColumnWidth(il++, 40);

	m_signalsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_signalsTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_signalsTree->setRootIsDecorated(false);

	connect(m_signalsTree, &QTreeWidget::itemSelectionChanged, this, &DialogBusEditor::onSignalItemSelectionChanged);
	connect(m_signalsTree, &QWidget::customContextMenuRequested, this, &DialogBusEditor::onSignalCustomContextMenuRequested);
	connect(m_signalsTree, &QTreeWidget::itemDoubleClicked, this, &DialogBusEditor::onSignalItemDoubleClicked);

	// Left side

	QGridLayout* leftButtonsLayout = new QGridLayout();

	m_btnAdd = new QPushButton(tr("Add"));
	m_btnRemove = new QPushButton(tr("Remove"));
	m_btnCheckOut = new QPushButton(tr("Check Out"));
	m_btnCheckIn = new QPushButton(tr("Check In"));
	m_btnUndo = new QPushButton(tr("Undo"));
	m_btnRefresh = new QPushButton(tr("Refresh"));

	leftButtonsLayout->addWidget(m_btnAdd, 0, 0);
	leftButtonsLayout->addWidget(m_btnRemove, 0, 1);
	leftButtonsLayout->addWidget(m_btnCheckOut, 1, 0);
	leftButtonsLayout->addWidget(m_btnCheckIn, 1, 1);
	leftButtonsLayout->addWidget(m_btnUndo, 1, 2);
	leftButtonsLayout->addWidget(m_btnRefresh, 1, 3);
	leftButtonsLayout->addItem(new QSpacerItem(100, 10, QSizePolicy::Expanding), 1, 4);

	connect (m_btnAdd, &QPushButton::clicked, this, &DialogBusEditor::onAdd);
	connect (m_btnRemove, &QPushButton::clicked, this, &DialogBusEditor::onRemove);
	connect (m_btnCheckOut, &QPushButton::clicked, this, &DialogBusEditor::onCheckOut);
	connect (m_btnCheckIn, &QPushButton::clicked, this, &DialogBusEditor::onCheckIn);
	connect (m_btnUndo, &QPushButton::clicked, this, &DialogBusEditor::onUndo);
	connect (m_btnRefresh, &QPushButton::clicked, this, &DialogBusEditor::onRefresh);

	// Right side

	QGridLayout* rightButtonsLayout = new QGridLayout();

	m_btnSignalAdd = new QPushButton(tr("Add"));
	m_btnSignalEdit = new QPushButton(tr("Edit"));
	m_btnSignalRemove = new QPushButton(tr("Remove"));
	m_btnSignalUp = new QPushButton(tr("Up"));
	m_btnSignalDown = new QPushButton(tr("Down"));
	m_btnClose = new QPushButton(tr("Close"));

	rightButtonsLayout->addWidget(m_btnSignalAdd, 0, 0);
	rightButtonsLayout->addWidget(m_btnSignalEdit, 0, 1);
	rightButtonsLayout->addWidget(m_btnSignalRemove, 0, 2);
	rightButtonsLayout->addItem(new QSpacerItem(100, 10, QSizePolicy::Expanding), 0, 3);
	rightButtonsLayout->addWidget(m_btnSignalUp, 1, 0);
	rightButtonsLayout->addWidget(m_btnSignalDown, 1, 1);
	rightButtonsLayout->addItem(new QSpacerItem(m_btnSignalRemove->minimumSize().width(), 10, QSizePolicy::Minimum), 1, 2);
	rightButtonsLayout->addItem(new QSpacerItem(100, 10, QSizePolicy::Expanding), 1, 3);

	rightButtonsLayout->addWidget(m_btnClose, 1, 4);

	connect (m_btnSignalAdd, &QPushButton::clicked, this, &DialogBusEditor::onSignalAdd);
	connect (m_btnSignalEdit, &QPushButton::clicked, this, &DialogBusEditor::onSignalEdit);
	connect (m_btnSignalRemove, &QPushButton::clicked, this, &DialogBusEditor::onSignalRemove);
	connect (m_btnSignalUp, &QPushButton::clicked, this, &DialogBusEditor::onSignalUp);
	connect (m_btnSignalDown, &QPushButton::clicked, this, &DialogBusEditor::onSignalDown);
	connect (m_btnClose, &QPushButton::clicked, this, &DialogBusEditor::close);

	m_splitter = new QSplitter(Qt::Horizontal);

	//

	QWidget* lw = new QWidget();
	QWidget* rw = new QWidget();

	m_splitter->setChildrenCollapsible(false);

	//

	QVBoxLayout* leftLayout = new QVBoxLayout();
	lw->setLayout(leftLayout);

	leftLayout->setContentsMargins(0, 0, 0, 0);

	leftLayout->addWidget(m_busTree);
	leftLayout->addLayout(leftButtonsLayout);

	//

	QVBoxLayout* rightLayout = new QVBoxLayout();
	rw->setLayout(rightLayout);

	rightLayout->setContentsMargins(0, 0, 0, 0);

	rightLayout->addWidget(m_signalsTree);
	rightLayout->addLayout(rightButtonsLayout);

	m_splitter->addWidget(lw);
	m_splitter->addWidget(rw);


	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_splitter);

	setLayout(mainLayout);

	// Bus Popup menu
	//
	m_addAction = new QAction(tr("Add"), this);
	m_removeAction = new QAction(tr("Remove"), this);
	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkInAction = new QAction(tr("Check In"), this);
	m_undoAction = new QAction(tr("Undo"), this);
	m_refreshAction = new QAction(tr("Refresh"), this);

	connect(m_addAction, &QAction::triggered, this, &DialogBusEditor::onAdd);
	connect(m_removeAction, &QAction::triggered, this, &DialogBusEditor::onRemove);
	connect(m_checkOutAction, &QAction::triggered, this, &DialogBusEditor::onCheckOut);
	connect(m_checkInAction, &QAction::triggered, this, &DialogBusEditor::onCheckIn);
	connect(m_undoAction, &QAction::triggered, this, &DialogBusEditor::onUndo);
	connect(m_refreshAction, &QAction::triggered, this, &DialogBusEditor::onRefresh);

	m_popupMenu = new QMenu(this);
	m_popupMenu->addAction(m_addAction);
	m_popupMenu->addAction(m_removeAction);
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
	if (m_busses->load() == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Busses loading error!"));
		return;
	}

	// fill data
	//
	fillBusList();

	updateButtonsEnableState();

	// sort items
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

		m_splitter->restoreState(theSettings.m_busEditorSplitterState);
	}

	if (theSettings.m_busEditorPeWindowPos.x() != -1 && theSettings.m_busEditorPeWindowPos.y() != -1)
	{
		m_peDialog->move(theSettings.m_busEditorPeWindowPos);
		m_peDialog->restoreGeometry(theSettings.m_busEditorPeWindowGeometry);
		m_peDialog->setSplitterPosition(theSettings.m_busEditorPeSplitterPosition);
	}
	else
	{
		int w = 300;
		int h = 400;

		m_peDialog->move(qApp->desktop()->width() / 2 - w / 2, qApp->desktop()->height() / 2 - h / 2 );
		m_peDialog->resize(w, h);
	}
}

DialogBusEditor::~DialogBusEditor()
{
	theSettings.m_busEditorWindowPos = pos();
	theSettings.m_busEditorWindowGeometry = saveGeometry();
	theSettings.m_busEditorSplitterState = m_splitter->saveState();

	theSettings.m_busEditorPeWindowPos = m_peDialog->pos();
	theSettings.m_busEditorPeWindowGeometry = m_peDialog->saveGeometry();

	theSettings.m_busEditorPeSplitterPosition = m_peDialog->splitterPosition();

	::theDialogBusEditor = nullptr;

	return;

}

void DialogBusEditor::onAdd()
{
	VFrame30::Bus bus;

	bus.setUuid(QUuid::createUuid());
	bus.setBusTypeId(tr("BUSTYPEID_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')));

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

		bool ok = m_busses->removeFile(uuid, fileRemoved);
		if (ok == false)
		{
			assert(false);
			continue;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses->remove(uuid);

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
	fillBusSignals();

	return;
}

void DialogBusEditor::onCheckOut()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		if (m_busses->checkOut(uuid) == false)
		{
			continue;
		}

		updateBusTreeItemText(item);
	}

	updateButtonsEnableState();

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

		if (m_busses->checkIn(uuid, comment, fileWasRemoved) == false)
		{
			continue;
		}

		if (fileWasRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses->remove(uuid);

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

		if (m_busses->undo(uuid, fileRemoved) == false)
		{
			continue;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the connection from the list and from the storage
			//
			m_busses->remove(uuid);

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
			// read previous data from file

			std::shared_ptr<DbFile> file = nullptr;

			DbFileInfo fi = m_busses->fileInfo(uuid);

			bool ok = m_db->getLatestVersion(fi, &file, this);
			if (ok == true && file != nullptr)
			{
				QByteArray data;
				file->swapData(data);

				VFrame30::Bus* bus = m_busses->getPtr(uuid);

				QString errorMessage;

				if (bus != nullptr && bus->load(data, &errorMessage) == true)
				{
					updateBusTreeItemText(item);
					fillBusSignals();
				}
			}
		}
	}

	updateButtonsEnableState();
}

void DialogBusEditor::onRefresh()
{
	m_busses->clear();

	if (m_busses->load() == false)
	{
		return;
	}

	fillBusList();

	updateButtonsEnableState();
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

	bus->addSignal(bs);

	QTreeWidgetItem* item = new QTreeWidgetItem();

	updateSignalsTreeItemText(item, bs);

	m_signalsTree->addTopLevelItem(item);

	m_busses->save(uuid);
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

	m_peDialog->setReadOnly(m_busses->fileInfo(uuid).state() != VcsState::CheckedOut);
	m_peDialog->setObjects(editSignalsPointers);

	// Run property editor

	if (m_peDialog->exec() == QDialog::Accepted)
	{
		// Save data back to bus

		if(editIndexes.size() != editSignalsPointers.size())
		{
			assert(false);
			return;
		}

		for (int i = 0; i < editIndexes.size(); i++)
		{
			int signalIndex = editIndexes[i];

			busSignals[signalIndex] = *(dynamic_cast<VFrame30::BusSignal*>(editSignalsPointers[i].get()));

			updateSignalsTreeItemText(selectedItems[i], busSignals[signalIndex]);
		}

		bus->setBusSignals(busSignals);
	}

	m_busses->save(uuid);
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
		bus->removeSignal(indexesToDelete[i]);
	}

	fillBusSignals();

	m_busses->save(uuid);
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

	m_busses->save(uuid);
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

	m_busses->save(uuid);
}

void DialogBusEditor::onSignalItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	if (item != nullptr)
	{
		onSignalEdit();
	}
}

void DialogBusEditor::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void DialogBusEditor::reject()
{
	QDialog::reject();
}

void DialogBusEditor::onBusItemChanged(QTreeWidgetItem *item, int column)
{

	QUuid uuid;

	VFrame30::Bus* bus = getCurrentBus(&uuid);
	if (bus == nullptr)
	{
		return;
	}

	if (column != 0)
	{
		return;
	}

	QString text = item->text(0).trimmed();

	if (text.isEmpty() == true)
	{
		return;
	}

	bus->setBusTypeId(text);

	m_busses->save(uuid);

}

void DialogBusEditor::onBusItemSelectionChanged()
{
	updateButtonsEnableState();
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

	int count = m_busses->count();

	for (int i = 0; i < count; i++)
	{
		const VFrame30::Bus& bus = m_busses->get(i);

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setFlags(item->flags() | Qt::ItemIsEditable);

		item->setData(0, Qt::UserRole, bus.uuid());

		m_busTree->addTopLevelItem(item);

		updateBusTreeItemText(item);
	}

	for (int i = 0; i < m_busTree->columnCount(); i++)
	{
		m_busTree->resizeColumnToContents(i);
	}

	return;
}

void DialogBusEditor::fillBusSignals()
{
	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	if (selectedItems.size() != 1)
	{
		m_signalsTree->clear();
		return;
	}

	QTreeWidgetItem* item = selectedItems[0];
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

	const VFrame30::Bus bus = m_busses->get(uuid);

	std::vector<VFrame30::BusSignal> busSignals = bus.busSignals();

	m_signalsTree->clear();

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

bool DialogBusEditor::addBus(VFrame30::Bus bus)
{
	// Add bus, update UI
	//
	m_busses->add(bus.uuid(), bus);

	bool ok = m_busses->save(bus.uuid());
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save connection %1").arg(bus.busTypeId()));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(0, Qt::UserRole, bus.uuid());

	item->setFlags(item->flags() | Qt::ItemIsEditable);

	m_busTree->addTopLevelItem(item);

	updateBusTreeItemText(item);
	updateButtonsEnableState();

	m_busTree->clearSelection();
	item->setSelected(true);

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

		const VFrame30::Bus& bus = m_busses->get(uuid);

		if (m_busses->fileInfo(bus.uuid()).state() == VcsState::CheckedOut)
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

	//

	m_btnRemove->setEnabled(selectedBusCount > 0);
	m_removeAction->setEnabled(selectedBusCount > 0);

	m_btnCheckOut->setEnabled(selectedBusCount > 0 && checkedInCount > 0);
	m_checkOutAction->setEnabled(selectedBusCount > 0 && checkedInCount > 0);

	m_btnCheckIn->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);
	m_checkInAction->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);

	m_btnUndo->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);
	m_undoAction->setEnabled(selectedBusCount > 0 && checkedOutCount > 0);

	//

	m_btnSignalAdd->setEnabled(checkedOutCount == 1);
	m_signalAddAction->setEnabled(checkedOutCount == 1);
	m_signalAddSubmenuAction->setEnabled(checkedOutCount == 1);

	m_btnSignalEdit->setEnabled(selectedSignalCount > 0);
	m_signalEditAction->setEnabled(selectedSignalCount > 0);

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

	const VFrame30::Bus& bus = m_busses->get(uuid);

	int c = 0;
	item->setText(c++, bus.busTypeId());

	DbFileInfo fi = m_busses->fileInfo(uuid);

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

void DialogBusEditor::updateSignalsTreeItemText(QTreeWidgetItem* item, const VFrame30::BusSignal& signal)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setText(0, signal.name());
	item->setText(1, E::valueToString<E::SignalType>(signal.type()));

	if (signal.type() == E::SignalType::Analog)
	{
		item->setText(2, E::valueToString<E::AnalogAppSignalFormat>(signal.analogFormat()));
	}
	else
	{
		item->setText(2, "");
	}

	if (signal.type() == E::SignalType::Bus)
	{
		item->setText(3, signal.busTypeId());
	}
	else
	{
		item->setText(3, "");
	}

	return;
}


VFrame30::Bus* DialogBusEditor::getCurrentBus(QUuid* uuid)
{
	if (uuid == nullptr)
	{
		assert(uuid);
		return nullptr;
	}

	QTreeWidgetItem* item = m_busTree->currentItem();

	if (item == nullptr)
	{
		return nullptr;
	}

	*uuid = item->data(0, Qt::UserRole).toUuid();

	return m_busses->getPtr(*uuid);
}
