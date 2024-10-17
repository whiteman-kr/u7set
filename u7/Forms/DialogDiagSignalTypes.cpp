#include "DialogDiagSignalTypes.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "Settings.h"

#include <UiLib/PropertyEditor.h>
#include <UiLib/StandardColors.h>

//
// DialogDiagSignalTypes
//
DialogDiagSignalTypes::DialogDiagSignalTypes(DbController* db, QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	m_db(db),
	m_diagSignalTypes(db)
{
	Q_ASSERT(m_db);

	setWindowTitle(tr("Diagnostics Signal Types Editor"));

	setAttribute(Qt::WA_DeleteOnClose);

	setMinimumSize(1024, 600);

	// Create user interface
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	m_diagSignalTypesTree = new QTreeWidget();

	QStringList l;
	l << tr("DiagSignalTypeID");
	l << tr("State");
	l << tr("User");

	m_diagSignalTypesTree->setColumnCount(l.size());
	m_diagSignalTypesTree->setHeaderLabels(l);

	int il = 0;
	m_diagSignalTypesTree->header()->setMinimumSectionSize(40);
	m_diagSignalTypesTree->header()->resizeSection(il++, 240);
	m_diagSignalTypesTree->header()->resizeSection(il++, 80);
	m_diagSignalTypesTree->header()->resizeSection(il++, 80);
	m_diagSignalTypesTree->header()->setStretchLastSection(true);

	m_diagSignalTypesTree->setSortingEnabled(true);
	m_diagSignalTypesTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_diagSignalTypesTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_diagSignalTypesTree->setRootIsDecorated(false);

	connect(m_diagSignalTypesTree, &QTreeWidget::itemSelectionChanged, this, &DialogDiagSignalTypes::onItemSelectionChanged);
	connect(m_diagSignalTypesTree, &QWidget::customContextMenuRequested, this, &DialogDiagSignalTypes::onCustomContextMenuRequested);

	m_diagSignalTypesPropertyEditor = new ExtWidgets::PropertyEditor(this);

	connect(m_diagSignalTypesPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogDiagSignalTypes::onPropertiesChanged);

	m_splitter = new QSplitter(Qt::Horizontal);

	m_splitter->addWidget(m_diagSignalTypesTree);
	m_splitter->addWidget(m_diagSignalTypesPropertyEditor);

	m_splitter->setChildrenCollapsible(false);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();

	m_btnAdd = new QPushButton(tr("Add"));
	m_btnRemove = new QPushButton(tr("Remove"));
	m_btnCheckOut = new QPushButton(tr("Check Out"));
	m_btnCheckIn = new QPushButton(tr("Check In"));
	m_btnUndo = new QPushButton(tr("Undo"));
	m_btnRefresh = new QPushButton(tr("Refresh"));
	m_btnClose = new QPushButton(tr("Close"));

	buttonsLayout->addWidget(m_btnAdd);
	buttonsLayout->addWidget(m_btnRemove);
	buttonsLayout->addWidget(m_btnCheckOut);
	buttonsLayout->addWidget(m_btnCheckIn);
	buttonsLayout->addWidget(m_btnUndo);
	buttonsLayout->addWidget(m_btnRefresh);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(m_btnClose);

	connect(m_btnAdd, &QPushButton::clicked, this, &DialogDiagSignalTypes::onAdd);
	connect(m_btnRemove, &QPushButton::clicked, this, &DialogDiagSignalTypes::onRemove);
	connect(m_btnCheckOut, &QPushButton::clicked, this, &DialogDiagSignalTypes::onCheckOut);
	connect(m_btnCheckIn, &QPushButton::clicked, this, &DialogDiagSignalTypes::onCheckIn);
	connect(m_btnUndo, &QPushButton::clicked, this, &DialogDiagSignalTypes::onUndo);
	connect(m_btnRefresh, &QPushButton::clicked, this, &DialogDiagSignalTypes::onRefresh);
	connect(m_btnClose, &QPushButton::clicked, this, &DialogDiagSignalTypes::close);

	mainLayout->addWidget(m_splitter);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);

	// Shortcuts
	//
	QShortcut* removeShortcut = new QShortcut(QKeySequence(QKeySequence::Delete), this);
	connect(removeShortcut, &QShortcut::activated, this, &DialogDiagSignalTypes::onRemoveShortcut);

	QShortcut* copyShortcut = new QShortcut(QKeySequence(QKeySequence::Copy), this);
	connect(copyShortcut, &QShortcut::activated, this, &DialogDiagSignalTypes::onCopyShortcut);

	QShortcut* pasteShortcut = new QShortcut(QKeySequence(QKeySequence::Paste), this);
	connect(pasteShortcut, &QShortcut::activated, this, &DialogDiagSignalTypes::onPasteShortcut);

	QShortcut* refreshShortcut = new QShortcut(QKeySequence(QKeySequence::Refresh), this);
	connect(refreshShortcut, &QShortcut::activated, this, &DialogDiagSignalTypes::onRefresh);

	// Popup menu
	//
	m_addAction = new QAction(tr("Add"), this);
	m_removeAction = new QAction(tr("Remove"), this);
	m_removeAction->setShortcut(QKeySequence::Delete);

	m_copyAction = new QAction(tr("Copy"), this);
	m_copyAction->setShortcut(QKeySequence::Copy);

	m_pasteAction = new QAction(tr("Paste"), this);
	m_pasteAction->setShortcut(QKeySequence::Paste);

	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkInAction = new QAction(tr("Check In"), this);
	m_undoAction = new QAction(tr("Undo"), this);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setShortcut(QKeySequence::Refresh);

	m_importAction = new QAction(tr("Import..."), this);
	m_exportAction = new QAction(tr("Export..."), this);

	connect(m_addAction, &QAction::triggered, this, &DialogDiagSignalTypes::onAdd);
	connect(m_removeAction, &QAction::triggered, this, &DialogDiagSignalTypes::onRemove);
	connect(m_copyAction, &QAction::triggered, this, &DialogDiagSignalTypes::onCopy);
	connect(m_pasteAction, &QAction::triggered, this, &DialogDiagSignalTypes::onPaste);
	connect(m_checkOutAction, &QAction::triggered, this, &DialogDiagSignalTypes::onCheckOut);
	connect(m_checkInAction, &QAction::triggered, this, &DialogDiagSignalTypes::onCheckIn);
	connect(m_undoAction, &QAction::triggered, this, &DialogDiagSignalTypes::onUndo);
	connect(m_refreshAction, &QAction::triggered, this, &DialogDiagSignalTypes::onRefresh);
	connect(m_importAction, &QAction::triggered, this, &DialogDiagSignalTypes::onImport);
	connect(m_exportAction, &QAction::triggered, this, &DialogDiagSignalTypes::onExport);

	m_popupMenu = new QMenu(this);
	m_popupMenu->addAction(m_addAction);
	m_popupMenu->addAction(m_removeAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_copyAction);
	m_popupMenu->addAction(m_pasteAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_checkOutAction);
	m_popupMenu->addAction(m_checkInAction);
	m_popupMenu->addAction(m_undoAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_refreshAction);
	m_popupMenu->addSeparator();
	m_popupMenu->addAction(m_importAction);
	m_popupMenu->addAction(m_exportAction);

	// Load connections
	//
	QString errorMessage;

	bool ok = m_diagSignalTypes.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(parent, qAppName(), errorMessage);
		return;
	}

	// fill data
	//
	fillDiagSignalTypesList();

	updateButtonsEnableState();

	// sort items
	//
	int sortColumn = QSettings().value("DialogDiagSignalTypes/sortColumn", 0).toInt();
	Qt::SortOrder sortOrder = static_cast<Qt::SortOrder>(QSettings().value("DialogDiagSignalTypes/sortOrder", Qt::AscendingOrder).toInt());
	m_diagSignalTypesTree->sortByColumn(sortColumn, sortOrder);

	// Restore settings
	//
	QPoint dialogPos = QSettings().value("DialogDiagSignalTypes/pos", QPoint(-1, -1)).toPoint();
	if (dialogPos.x() != -1 && dialogPos.y() != -1)
	{
		move(dialogPos);
	}

	QByteArray dialogGeometry = QSettings().value("DialogDiagSignalTypes/geometry").toByteArray();
	if (dialogGeometry.isEmpty() == false)
	{
		restoreGeometry(dialogGeometry);
	}

	QByteArray splitterState = QSettings().value("DialogDiagSignalTypes/splitterState").toByteArray();
	if (splitterState.isEmpty() == false)
	{
		m_splitter->restoreState(splitterState);
	}

	int peSplitterPosition = QSettings().value("DialogDiagSignalTypes/peSplitterPosition", -1).toInt();
	if (peSplitterPosition != -1)
	{
		m_diagSignalTypesPropertyEditor->setSplitterPosition(peSplitterPosition);
	}

	return;
}


DialogDiagSignalTypes::~DialogDiagSignalTypes()
{
	QSettings().setValue("DialogDiagSignalTypes/pos", pos());
	QSettings().setValue("DialogDiagSignalTypes/geometry", saveGeometry());
	QSettings().setValue("DialogDiagSignalTypes/splitterState", m_splitter->saveState());
	QSettings().setValue("DialogDiagSignalTypes/peSplitterPosition", m_diagSignalTypesPropertyEditor->splitterPosition());

	QSettings().setValue("DialogDiagSignalTypes/sortColumn", m_diagSignalTypesTree->sortColumn());
	QSettings().setValue("DialogDiagSignalTypes/sortOrder", static_cast<int>(m_diagSignalTypesTree->header()->sortIndicatorOrder()));

	s_instance = nullptr;

	return;
}

void DialogDiagSignalTypes::showDialog(DbController* db, QWidget* parent)
{
	Q_ASSERT(db);

	if (s_instance == nullptr)
	{
		s_instance = new DialogDiagSignalTypes(db, parent);
		s_instance->show();
	}
	else
	{
		s_instance->activateWindow();
	}
	UiTools::adjustDialogPlacement(s_instance);

	return;
}

void DialogDiagSignalTypes::onItemSelectionChanged()
{
	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogDiagSignalTypes::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{

	// Check for duplicate signalTypeId
	//
	for (const auto& object : objects)
	{
		Hardware::DiagSignalTypeObject* dst = dynamic_cast<Hardware::DiagSignalTypeObject*>(object.get());
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			continue;
		}

		for (int i = 0; i < m_diagSignalTypes.count(); i++)
		{
			auto otherObject = m_diagSignalTypes.get(i);
			if (otherObject == nullptr)
			{
				Q_ASSERT(otherObject);
				return;
			}

			if (otherObject->uuid() == dst->uuid())
			{
				continue;
			}

			if (otherObject->signalTypeId() == dst->signalTypeId())
			{
				QMessageBox::critical(this, qAppName(), tr("Error: signal type ID '%1' already exists.").arg(dst->signalTypeId()));

				QString oldSignalId = signalTypeIdFromItem(dst->uuid());
				if (oldSignalId.isEmpty() == true)
				{
					Q_ASSERT(false);
				}
				else
				{
					dst->setSignalTypeId(oldSignalId);
				}
				break;
			}
		}
	}

	// Save modified objects
	//
	for (const auto& object : objects)
	{
		Hardware::DiagSignalTypeObject* dst = dynamic_cast<Hardware::DiagSignalTypeObject*>(object.get());
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			continue;
		}

		QString errorMessage;
		bool ok = m_diagSignalTypes.save(dst->uuid(), &errorMessage);

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save diag signal type %1: %2").arg(dst->signalTypeId()).arg(errorMessage));
			continue;
		}
	}

	// update tree items
	//
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();
	for (auto item : selectedItems)
	{
		updateTreeItemText(item);
	}

	return;
}

void DialogDiagSignalTypes::onAdd()
{
	std::shared_ptr<Hardware::DiagSignalTypeObject> dst = Hardware::DiagSignalTypeObject::CreateObject();
	if (dst == nullptr)
	{
		Q_ASSERT(dst);
		return;
	}

	dst->setUuid(QUuid::createUuid());
	dst->setSignalTypeId(tr("ST_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')));

	addDiagSignalType(dst);
	return;
}

void DialogDiagSignalTypes::onRemove()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected signal types?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		bool fileRemoved = false;

		bool ok = m_diagSignalTypes.removeFile(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the type from the list and from the storage
			//
			m_diagSignalTypes.remove(uuid);

			int index = m_diagSignalTypesTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				Q_ASSERT(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_diagSignalTypesTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				Q_ASSERT(deleteItem);
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

void DialogDiagSignalTypes::onCopy()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	Proto::EnvelopeSet envelopeSet;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			return;
		}


		auto envelope = envelopeSet.add_items();
		dst->Save(envelope);
	}

	QByteArray data;
	data.resize(static_cast<int>(envelopeSet.ByteSizeLong()));

	bool result = envelopeSet.SerializeToArray(data.data(), static_cast<int>(envelopeSet.ByteSizeLong()));
	if (result == false)
	{
		Q_ASSERT(result);
		return;
	}

	if (data.isEmpty() == false)
	{
		QMimeData* mime = new QMimeData();
		mime->setData(Hardware::DiagSignalTypeObject::mimeType, data);

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	return;
}


void DialogDiagSignalTypes::onPaste()
{
	QClipboard* clipboard = QApplication::clipboard();

	const QMimeData* mimeData = clipboard->mimeData();
	if (mimeData->hasFormat(Hardware::DiagSignalTypeObject::mimeType) == false)
	{
		return;
	}

	QByteArray data = mimeData->data(Hardware::DiagSignalTypeObject::mimeType);
	if (data.isEmpty() == true)
	{
		return;
	}

	Proto::EnvelopeSet envelopeSet;
	if (envelopeSet.ParseFromArray(data.constData(), data.size()) == false)
	{
		return;
	}

	m_diagSignalTypesTree->clearSelection();

	m_diagSignalTypesTree->blockSignals(true);

	m_diagSignalTypesPropertyEditor->clear();

	for (int i = 0; i < envelopeSet.items_size(); i++)
	{
		const auto& envelope = envelopeSet.items(i);

		if (envelope.HasExtension(::Proto::diagSignalType) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = Hardware::DiagSignalTypeObject::CreateObject(envelope);
		if (dst == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		QString newSignalTypeId = tr("%1 (Copy)").arg(dst->signalTypeId());
		int copyNumber = 1;
		while (m_diagSignalTypes.hasSignalTypeId(newSignalTypeId) == true)
		{
			newSignalTypeId = tr("%1 (Copy %2)").arg(dst->signalTypeId()).arg(copyNumber++);
		}

		dst->setUuid(QUuid::createUuid());
		dst->setSignalTypeId(newSignalTypeId);
		pasteDiagSignalType(dst);
	}

	m_diagSignalTypesTree->blockSignals(false);

	updateButtonsEnableState();

	setPropertyEditorObjects();

	return;
}

void DialogDiagSignalTypes::onCheckOut()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		bool ok = m_diagSignalTypes.checkOut(uuid, &errorMessage);
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

void DialogDiagSignalTypes::onCheckIn()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, qAppName(), tr("Please enter the comment:"), QLineEdit::Normal, tr("comment"), &ok);

	if (ok == false)
	{
		return;
	}

	if (comment.isEmpty())
	{
		QMessageBox::warning(this, qAppName(), tr("No comment supplied! Please provide a comment."));
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		bool fileWasRemoved = false;
		QString errorMessage;

		ok = m_diagSignalTypes.checkIn(uuid, comment, &fileWasRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileWasRemoved == true)
		{
			// File was removed, delete the type from the list and from the storage
			//
			m_diagSignalTypes.remove(uuid);

			int index = m_diagSignalTypesTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				Q_ASSERT(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_diagSignalTypesTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				Q_ASSERT(deleteItem);
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

void DialogDiagSignalTypes::onUndo()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to undo changes on selected signal types?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		bool fileRemoved = false;
		QString errorMessage;

		bool ok = m_diagSignalTypes.undo(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the type from the list and from the storage
			//
			m_diagSignalTypes.remove(uuid);

			int index = m_diagSignalTypesTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				Q_ASSERT(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_diagSignalTypesTree->takeTopLevelItem(index);
			if (deleteItem == nullptr)
			{
				Q_ASSERT(deleteItem);
				continue;
			}

			delete deleteItem;
		}
		else
		{
			// read previous data from file

			std::shared_ptr<DbFile> file = nullptr;

			DbFileInfo fi = m_diagSignalTypes.fileInfo(uuid);

			ok = m_db->getLatestVersion(fi, &file, this);
			if (ok == true && file != nullptr)
			{
				QByteArray data;
				file->swapData(data);

				std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);

				if (dst != nullptr && dst->Load(data) == true)
				{
					updateTreeItemText(item);
				}
			}
		}
	}

	updateButtonsEnableState();

	setPropertyEditorObjects();
}

void DialogDiagSignalTypes::onRefresh()
{
	m_diagSignalTypes.clear();

	QString errorMessage;

	bool ok = m_diagSignalTypes.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMessage);
		return;
	}

	fillDiagSignalTypesList();
	updateButtonsEnableState();

	return;
}

void DialogDiagSignalTypes::onExport()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	Proto::EnvelopeSet envelopeSet;

	QString defaultFileName;
	if (selectedItems.size() > 1)
	{
		defaultFileName = tr("DiagSignalTypes.%1").arg(File::DiagSignalTypeSetFileExtension);
	}

	for (const auto& item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			return;
		}

		if (defaultFileName.isEmpty() == true)
		{
			defaultFileName = tr("%1.%2").arg(dst->signalTypeId()).arg(File::DiagSignalTypeSetFileExtension);
		}

		auto envelope = envelopeSet.add_items();
		dst->Save(envelope);
	}

	QByteArray data;
	data.resize(static_cast<int>(envelopeSet.ByteSizeLong()));

	bool result = envelopeSet.SerializeToArray(data.data(), static_cast<int>(envelopeSet.ByteSizeLong()));
	if (result == false)
	{
		Q_ASSERT(result);
		return;
	}

	if (data.isEmpty() == true)
	{
		return;
	}

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export"), path + QDir::separator() + defaultFileName, tr("Diagnostics Signal Types Set (*.%1)").arg(File::DiagSignalTypeSetFileExtension));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QFile file(fileName);
	if (file.open(QFile::WriteOnly | QFile::Truncate) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error opening file %1!").arg(QDir::toNativeSeparators(fileName)));
		return;
	}

	if (file.write(data) != data.length())
	{
		QMessageBox::critical(this, qAppName(), tr("Error writing data to file %1!").arg(QDir::toNativeSeparators(fileName)));
		return;
	}

	QMessageBox::information(this, qAppName(), tr("File %1 saved successfully.").arg(QDir::toNativeSeparators(fileName)));

	return;
}

void DialogDiagSignalTypes::onImport()
{
	static QString path{"."};
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), path + QDir::separator(), tr("Diagnostics Signal Types Set (*.%1)").arg(File::DiagSignalTypeSetFileExtension));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	QByteArray data;

	QFile file(fileName);
	if (file.open(QFile::ReadOnly) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error opening file %1!").arg(QDir::toNativeSeparators(fileName)));
		return;
	}

	data = file.readAll();

	Proto::EnvelopeSet envelopeSet;
	if (data.isEmpty() == true || envelopeSet.ParseFromArray(data.constData(), data.size()) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Error parsing file %1 contents!").arg(QDir::toNativeSeparators(fileName)));
		return;
	}

	m_diagSignalTypesTree->clearSelection();

	m_diagSignalTypesTree->blockSignals(true);

	m_diagSignalTypesPropertyEditor->clear();

	for (int i = 0; i < envelopeSet.items_size(); i++)
	{
		const auto& envelope = envelopeSet.items(i);

		if (envelope.HasExtension(::Proto::diagSignalType) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = Hardware::DiagSignalTypeObject::CreateObject(envelope);
		if (dst == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		QString newSignalTypeId = dst->signalTypeId();
		int copyNumber = 1;
		while (m_diagSignalTypes.hasSignalTypeId(newSignalTypeId) == true)
		{
			newSignalTypeId = tr("%1 (Import %2)").arg(dst->signalTypeId()).arg(copyNumber++);
		}

		dst->setUuid(QUuid::createUuid());
		dst->setSignalTypeId(newSignalTypeId);
		pasteDiagSignalType(dst);
	}

	m_diagSignalTypesTree->blockSignals(false);

	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogDiagSignalTypes::onCopyShortcut()
{
	if (m_diagSignalTypesTree->hasFocus() == false)
	{
		return;
	}

	onCopy();
}

void DialogDiagSignalTypes::onPasteShortcut()
{
	if (m_diagSignalTypesTree->hasFocus() == false)
	{
		return;
	}

	onPaste();
}

void DialogDiagSignalTypes::onRemoveShortcut()
{
	if (m_diagSignalTypesTree->hasFocus() == false)
	{
		return;
	}

	onRemove();
}

void DialogDiagSignalTypes::onCustomContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	m_popupMenu->exec(this->cursor().pos());
}

bool DialogDiagSignalTypes::addDiagSignalType(std::shared_ptr<Hardware::DiagSignalTypeObject> dst)
{
	if (dst == nullptr)
	{
		Q_ASSERT(dst);
		return false;
	}

	// Check if signal type is not used
	//
	auto found = m_diagSignalTypes.get(dst->signalTypeId());
	if (found != nullptr)
	{
		QMessageBox::critical(this, windowTitle(), tr("Diagnostic signal type '%1' already exists.").arg(dst->signalTypeId()));
		return false;
	}

	// Add type, update UI
	//
	QString errorMessage;

	m_diagSignalTypes.add(dst->uuid(), dst);

	bool ok = m_diagSignalTypes.save(dst->uuid(), &errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save DiagSignalType %1: %2").arg(dst->signalTypeId()).arg(errorMessage));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(static_cast<int>(Columns::SignalTypeId), Qt::UserRole, dst->uuid());

	m_diagSignalTypesTree->addTopLevelItem(item);

	updateTreeItemText(item);
	updateButtonsEnableState();

	m_diagSignalTypesTree->clearSelection();
	item->setSelected(true);


	return true;
}

bool DialogDiagSignalTypes::pasteDiagSignalType(std::shared_ptr<Hardware::DiagSignalTypeObject> dst)
{
	if (dst == nullptr)
	{
		Q_ASSERT(dst);
		return false;
	}

	// Add connection, update UI
	//
	QString errorMessage;

	m_diagSignalTypes.add(dst->uuid(), dst);

	bool ok = m_diagSignalTypes.save(dst->uuid(), &errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save diag signal type %1: %2").arg(dst->signalTypeId()).arg(errorMessage));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(static_cast<int>(Columns::SignalTypeId), Qt::UserRole, dst->uuid());

	m_diagSignalTypesTree->addTopLevelItem(item);

	updateTreeItemText(item);

	item->setSelected(true);

	return true;
}

QString DialogDiagSignalTypes::signalTypeIdFromItem(const QUuid& uuid) const
{
	for (int i = 0; i < m_diagSignalTypesTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_diagSignalTypesTree->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return QString();
		}

		QUuid itemUuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		if (itemUuid == uuid)
		{
			return item->text(0);
		}
	}

	return QString();
}

void DialogDiagSignalTypes::fillDiagSignalTypesList()
{
	m_diagSignalTypesTree->clear();

	int count = m_diagSignalTypes.count();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(i);
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			break;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setData(static_cast<int>(Columns::SignalTypeId), Qt::UserRole, dst->uuid());

		m_diagSignalTypesTree->addTopLevelItem(item);

		updateTreeItemText(item);
	}

	return;
}

void DialogDiagSignalTypes::setPropertyEditorObjects()
{
	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		m_diagSignalTypesPropertyEditor->clear();

		updateButtonsEnableState();

		return;
	}

	bool readOnly = false;

	QList<std::shared_ptr<PropertyObject>> objects;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			return;
		}

		if (m_diagSignalTypes.fileInfo(dst->uuid()).state() != E::VcsState::CheckedOut)
		{
			readOnly = true;
		}

		objects.push_back(dst);
	}

	m_diagSignalTypesPropertyEditor->setExpertMode(theSettings.isExpertMode());
	m_diagSignalTypesPropertyEditor->setReadOnly(readOnly);
	m_diagSignalTypesPropertyEditor->setObjects(objects);

	return;
}

bool DialogDiagSignalTypes::continueWithDuplicateCaptions()
{
	bool duplicated = false;
	QString duplicatedCaption;

	for (int i = 0; i < m_diagSignalTypes.count(); i++)
	{
		Hardware::DiagSignalTypeObject* c = m_diagSignalTypes.get(i).get();

		for (int j = 0; j < m_diagSignalTypes.count(); j++)
		{
			Hardware::DiagSignalTypeObject* e = m_diagSignalTypes.get(j).get();
			Q_ASSERT(e);

			if (i == j)
			{
				continue;
			}

			if (e->signalTypeId() == c->signalTypeId())
			{
				duplicated = true;
				duplicatedCaption = e->signalTypeId();
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
		QString s = tr("Diagnostic signal type with ID '%1' already exists.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
		auto mbResult = QMessageBox::warning(this, qAppName(), s, QMessageBox::Yes | QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}

void DialogDiagSignalTypes::updateTreeItemText(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

	std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);
	if (dst == nullptr)
	{
		Q_ASSERT(dst);
		return;
	}

	item->setText(static_cast<int>(Columns::SignalTypeId), dst->signalTypeId());

	DbFileInfo fi = m_diagSignalTypes.fileInfo(dst->uuid());

	QBrush b(StandardColors::VcsCheckedIn);

	if (fi.state() == E::VcsState::CheckedOut)
	{
		item->setText(static_cast<int>(Columns::Action), E::valueToString<E::VcsItemAction>(fi.action()));

		int userId = fi.userId();
		item->setText(static_cast<int>(Columns::UserId), m_db->username(userId));

		switch (fi.action())
		{
		case E::VcsItemAction::Added:
			b.setColor(StandardColors::VcsAdded);
			break;
		case E::VcsItemAction::Modified:
			b.setColor(StandardColors::VcsModified);
			break;
		case E::VcsItemAction::Deleted:
			b.setColor(StandardColors::VcsDeleted);
			break;
		case E::VcsItemAction::Unknown:
			Q_ASSERT(false);
			b.setColor(StandardColors::VcsDeleted);
			break;
		}
	}
	else
	{
		item->setText(static_cast<int>(Columns::Action), "");
		item->setText(static_cast<int>(Columns::UserId), "");
	}

	for (int i = 0; i < static_cast<int>(Columns::Count); i++)
	{
		item->setBackground(i, b);
	}

	return;
}

void DialogDiagSignalTypes::updateButtonsEnableState()
{
	int selectedCount = 0;
	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<QTreeWidgetItem*> selectedItems = m_diagSignalTypesTree->selectedItems();

	selectedCount = selectedItems.size();

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(static_cast<int>(Columns::SignalTypeId), Qt::UserRole).toUuid();

		std::shared_ptr<Hardware::DiagSignalTypeObject> dst = m_diagSignalTypes.get(uuid);
		if (dst == nullptr)
		{
			Q_ASSERT(dst);
			return;
		}

		if (m_diagSignalTypes.fileInfo(dst->uuid()).state() == E::VcsState::CheckedOut)
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

	m_copyAction->setEnabled(selectedCount > 0);

	m_btnCheckOut->setEnabled(selectedCount > 0 && checkedInCount > 0);
	m_checkOutAction->setEnabled(selectedCount > 0 && checkedInCount > 0);

	m_btnCheckIn->setEnabled(selectedCount > 0 && checkedOutCount > 0);
	m_checkInAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);

	m_btnUndo->setEnabled(selectedCount > 0 && checkedOutCount > 0);
	m_undoAction->setEnabled(selectedCount > 0 && checkedOutCount > 0);

	m_exportAction->setEnabled(selectedCount > 0);

	return;
}

void DialogDiagSignalTypes::closeEvent(QCloseEvent* e)
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

void DialogDiagSignalTypes::reject()
{
	if (continueWithDuplicateCaptions() == true)
	{
		QDialog::reject();
	}
}
