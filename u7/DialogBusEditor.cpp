#include "DialogBusEditor.h"

#include "Settings.h"

#include <QTreeWidget>

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

	//m_signalsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	//m_signalsTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_signalsTree->setRootIsDecorated(false);

	//connect(m_signalsTree, &QTreeWidget::itemSelectionChanged, this, &DialogBusEditor::onItemSelectionChanged);
	//connect(m_signalsTree, &QWidget::customContextMenuRequested, this, &DialogBusEditor::onCustomContextMenuRequested);

	//

	QHBoxLayout* leftButtonsLayout = new QHBoxLayout();

	m_btnAdd = new QPushButton(tr("Add"));
	m_btnRemove = new QPushButton(tr("Remove"));
	m_btnCheckOut = new QPushButton(tr("Check Out"));
	m_btnCheckIn = new QPushButton(tr("Check In"));
	m_btnUndo = new QPushButton(tr("Undo"));
	m_btnRefresh = new QPushButton(tr("Refresh"));

	leftButtonsLayout->addWidget(m_btnAdd);
	leftButtonsLayout->addWidget(m_btnRemove);
	leftButtonsLayout->addWidget(m_btnCheckOut);
	leftButtonsLayout->addWidget(m_btnCheckIn);
	leftButtonsLayout->addWidget(m_btnUndo);
	leftButtonsLayout->addWidget(m_btnRefresh);
	leftButtonsLayout->addStretch();

	connect (m_btnAdd, &QPushButton::clicked, this, &DialogBusEditor::onAdd);
	connect (m_btnRemove, &QPushButton::clicked, this, &DialogBusEditor::onRemove);
	connect (m_btnCheckOut, &QPushButton::clicked, this, &DialogBusEditor::onCheckOut);
	connect (m_btnCheckIn, &QPushButton::clicked, this, &DialogBusEditor::onCheckIn);
	connect (m_btnUndo, &QPushButton::clicked, this, &DialogBusEditor::onUndo);
	connect (m_btnRefresh, &QPushButton::clicked, this, &DialogBusEditor::onRefresh);

	QHBoxLayout* rightButtonsLayout = new QHBoxLayout();

	rightButtonsLayout->addStretch();

	m_btnClose = new QPushButton(tr("Close"));

	rightButtonsLayout->addWidget(m_btnClose);

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

	// Popup menu
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
		//m_connectionPropertyEditor->setSplitterPosition(theSettings.m_connectionEditorPeSplitterPosition);
	}
}

DialogBusEditor::~DialogBusEditor()
{
	theSettings.m_busEditorWindowPos = pos();
	theSettings.m_busEditorWindowGeometry = saveGeometry();
	theSettings.m_busEditorSplitterState = m_splitter->saveState();
	//theSettings.m_busEditorPeSplitterPosition = m_busPropertyEditor->splitterPosition();

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
	//setPropertyEditorObjects();

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
	//setPropertyEditorObjects();

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
				}
			}
		}
	}

	updateButtonsEnableState();

	//setPropertyEditorObjects();
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
	if (item == nullptr)
	{
		assert(item);
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

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();

	VFrame30::Bus* bus = m_busses->getPtr(uuid);

	if (bus == nullptr)
	{
		assert(bus);
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

	return;
}

void DialogBusEditor::fillBusSignals()
{
	QTreeWidgetItem* item = m_busTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	QVariant d = item->data(0, Qt::UserRole);
	if (d.isNull() || d.isValid() == false)
	{
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

	m_busTree->addTopLevelItem(item);

	updateBusTreeItemText(item);
	updateButtonsEnableState();

	m_busTree->clearSelection();
	item->setSelected(true);

	return true;
}

void DialogBusEditor::updateButtonsEnableState()
{
	int selectedCount = 0;
	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<QTreeWidgetItem*> selectedItems = m_busTree->selectedItems();

	selectedCount = selectedItems.size();

	for (auto item : selectedItems)
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

	if (signal.type() == E::SignalType::Bus)
	{
		item->setText(3, signal.busTypeId());
	}

	return;
}
