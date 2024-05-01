#include "DialogAppSignalLists.h"
#include "Settings.h"
#include "AppSignalSetProvider.h"

#include "../lib/StandardColors.h"
#include "../UtilsLib/Ui/UiTools.h"
#include "../Proto/AppSignalList.pb.h"


//
//
// AppSignalListsProvider - this calss is used to provide app signals for editing signal lists
//
//

AppSignalListsProvider::AppSignalListsProvider(AppSignalSetProvider* signalSetProvider) :
	m_signalSetProvider(signalSetProvider)
{
	Q_ASSERT(signalSetProvider);
}

int AppSignalListsProvider::signalsCount() const
{
	return m_signalSetProvider->signalCount();
}

std::vector<Hash> AppSignalListsProvider::signalHashes() const
{
	const std::vector<AppSignal*>& sv = m_signalSetProvider->signalsVector();

	std::vector<Hash> result;
	result.reserve(sv.size());

	for (const auto& v : sv)
	{
		result.push_back(::calcHash(v->appSignalID()));
	}
	return result;
}

std::vector<AppSignalParam> AppSignalListsProvider::signalList() const
{
	const std::vector<AppSignal*>& sv = m_signalSetProvider->signalsVector();

	std::vector<AppSignalParam> result;
	result.reserve(sv.size());

	for (const auto& v : sv)
	{
		result.push_back({*v});
	}

	return result;
}

bool AppSignalListsProvider::signalExists(Hash hash) const
{
	return m_signalSetProvider->signalSet().getSignalByHash(hash) != nullptr;
}

bool AppSignalListsProvider::signalExists(const QString& appSignalId) const
{
	return m_signalSetProvider->signalExists(appSignalId);
}

bool AppSignalListsProvider::signalsExist(const QStringList& signalIds) const
{
	return std::all_of(signalIds.begin(), signalIds.end(), [this](const QString& appSignalId) {
		return m_signalSetProvider->signalExists(appSignalId);
	});
}

AppSignalParam AppSignalListsProvider::signalParam(Hash signalHash, bool* found) const
{
	AppSignalParam result;

	const AppSignal* s = m_signalSetProvider->signalSet().getSignalByHash(signalHash);

	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		result.load(*s);
	}

	return result;


}

AppSignalParam AppSignalListsProvider::signalParam(const QString& appSignalId, bool* found) const
{
	AppSignalParam result;

	AppSignal* s = m_signalSetProvider->getSignal(appSignalId);

	if (found != nullptr)
	{
		*found = s != nullptr;
	}

	if (s != nullptr)
	{
		result.load(*s);
	}

	return result;
}


void DialogAppSignalLists::showDialog(DbController* db, QWidget* parent)
{
	Q_ASSERT(db);

	if (s_instance == nullptr)
	{
		s_instance = new DialogAppSignalLists(db, parent);
		s_instance->show();
	}
	else
	{
		s_instance->activateWindow();
	}
	UiTools::adjustDialogPlacement(s_instance);

	return;
}

DialogAppSignalLists::DialogAppSignalLists(DbController* db, QWidget* parent)
	: QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
	m_db(db),
	m_lists(db),
	m_signalProvider(AppSignalSetProvider::getInstance())
{
	assert(m_db);

	setWindowTitle(tr("AppSignalLists Editor"));

	setAttribute(Qt::WA_DeleteOnClose);

	// Create user interface
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QHBoxLayout* maskLayout = new QHBoxLayout();

	m_mask = new QLineEdit();
	m_mask->setClearButtonEnabled(true);

	connect(m_mask, &QLineEdit::returnPressed, this, &DialogAppSignalLists::onMaskReturn);
	connect(m_mask, &QLineEdit::textChanged, this, [this](const QString& text) 
		{
			if (text.isEmpty() == true) 
			{
				onMaskApply();
			}
		}
	);

	m_maskApply = new QPushButton(tr("Filter"));
	connect(m_maskApply, &QPushButton::clicked, this, &DialogAppSignalLists::onMaskApply);

	m_listsTree = new QTreeWidget();

	QStringList l;
	l << tr("ListID");
	l << tr("State");
	l << tr("User");

	m_listsTree->setColumnCount(static_cast<int>(l.size()));
	m_listsTree->setHeaderLabels(l);

	int il = 0;
	m_listsTree->setColumnWidth(il++, 180);
	m_listsTree->setColumnWidth(il++, 40);
	m_listsTree->setColumnWidth(il++, 40);
	m_listsTree->setSortingEnabled(true);
	m_listsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_listsTree->setContextMenuPolicy(Qt::CustomContextMenu);

	m_listsTree->setRootIsDecorated(false);

	connect(m_listsTree, &QTreeWidget::itemSelectionChanged, this, &DialogAppSignalLists::onItemSelectionChanged);
	connect(m_listsTree, &QWidget::customContextMenuRequested, this, &DialogAppSignalLists::onCustomContextMenuRequested);

	m_listPropertyEditor = new ExtWidgets::PropertyEditor(this);
	connect(m_listPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogAppSignalLists::onPropertiesChanged);

	m_signalListWidget = new AppSignalLists::AppSignalListWidget(m_signalProvider, false, this);
	connect(m_signalListWidget, &AppSignalLists::AppSignalListWidget::signalsChanged, this, &DialogAppSignalLists::onSignalsChanged);

	maskLayout->addWidget(m_mask);
	maskLayout->addWidget(m_maskApply);

	m_splitter = new QSplitter(Qt::Horizontal);

	m_splitter->addWidget(m_listsTree);

	QTabWidget* tab = new QTabWidget();
	tab->addTab(m_listPropertyEditor, "Properties");
	tab->addTab(m_signalListWidget, "Signals");
	m_splitter->addWidget(tab);

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

	connect (m_btnAdd, &QPushButton::clicked, this, &DialogAppSignalLists::onAdd);
	connect (m_btnRemove, &QPushButton::clicked, this, &DialogAppSignalLists::onRemove);
	connect (m_btnCheckOut, &QPushButton::clicked, this, &DialogAppSignalLists::onCheckOut);
	connect (m_btnCheckIn, &QPushButton::clicked, this, &DialogAppSignalLists::onCheckIn);
	connect (m_btnUndo, &QPushButton::clicked, this, &DialogAppSignalLists::onUndo);
	connect (m_btnRefresh, &QPushButton::clicked, this, &DialogAppSignalLists::onRefresh);
	connect (m_btnClose, &QPushButton::clicked, this, &DialogAppSignalLists::close);

	mainLayout->addLayout(maskLayout);
	mainLayout->addWidget(m_splitter);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);

	// Shortcuts
	//
	QShortcut* removeShortcut = new QShortcut(QKeySequence(QKeySequence::Delete), this);
	connect(removeShortcut, &QShortcut::activated, this, &DialogAppSignalLists::onRemoveShortcut);

	QShortcut* copyShortcut = new QShortcut(QKeySequence(QKeySequence::Copy), this);
	connect(copyShortcut, &QShortcut::activated, this, &DialogAppSignalLists::onCopyShortcut);

	QShortcut* pasteShortcut = new QShortcut(QKeySequence(QKeySequence::Paste), this);
	connect(pasteShortcut, &QShortcut::activated, this, &DialogAppSignalLists::onPasteShortcut);

	QShortcut* refreshShortcut = new QShortcut(QKeySequence(QKeySequence::Refresh), this);
	connect(refreshShortcut, &QShortcut::activated, this, &DialogAppSignalLists::onRefresh);

	// Mask and completer
	//
	m_completer = new QCompleter(QSettings().value("DialogAppSignalLists/masks").toString().split('\n'), this);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_mask->setCompleter(m_completer);

    connect(m_mask, &QLineEdit::textEdited, [this](){m_completer->complete();});
	connect(m_completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_mask, &QLineEdit::setText);

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

	connect(m_addAction, &QAction::triggered, this, &DialogAppSignalLists::onAdd);
	connect(m_removeAction, &QAction::triggered, this, &DialogAppSignalLists::onRemove);
	connect(m_copyAction, &QAction::triggered, this, &DialogAppSignalLists::onCopy);
	connect(m_pasteAction, &QAction::triggered, this, &DialogAppSignalLists::onPaste);
	connect(m_checkOutAction, &QAction::triggered, this, &DialogAppSignalLists::onCheckOut);
	connect(m_checkInAction, &QAction::triggered, this, &DialogAppSignalLists::onCheckIn);
	connect(m_undoAction, &QAction::triggered, this, &DialogAppSignalLists::onUndo);
	connect(m_refreshAction, &QAction::triggered, this, &DialogAppSignalLists::onRefresh);

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

	// Load lists
	//
	QString errorMessage;

	bool ok = m_lists.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(parent, qAppName(), errorMessage);
		return;
	}

	// Load deprecated connections
	//
	/*
	Builder::ConnectionStorage xmlConnections(m_db);

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
	*/
	// fill data
	//
	fillAppSignalLists();

	updateButtonsEnableState();

	// sort items
	//
	for (int i = 0; i < m_listsTree->columnCount(); i++)
	{
		m_listsTree->resizeColumnToContents(i);
	}

	

	m_listsTree->sortByColumn(QSettings().value("DialogAppSignalLists/sortColumn", 0).toInt(), 
		static_cast<Qt::SortOrder>(QSettings().value("DialogAppSignalLists/sortOrder", Qt::AscendingOrder).toInt()));

	connect(m_listsTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogAppSignalLists::onSortIndicatorChanged);

	// Restore settings
	//
	QByteArray ba = QSettings().value("DialogAppSignalLists/geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}

	ba = QSettings().value("DialogAppSignalLists/splitterState").toByteArray();
	if (ba.isEmpty() == false)
	{
		m_splitter->restoreState(ba);
	}

	m_listPropertyEditor->setSplitterPosition(QSettings().value("DialogAppSignalLists/splitterPosition", 0).toInt());

	return;
}

DialogAppSignalLists::~DialogAppSignalLists()
{
	QSettings().setValue("DialogAppSignalLists/geometry", saveGeometry());
	QSettings().setValue("DialogAppSignalLists/splitterState", m_splitter->saveState());
	QSettings().setValue("DialogAppSignalLists/splitterPosition", m_listPropertyEditor->splitterPosition());

	s_instance = nullptr;

	return;
}

void DialogAppSignalLists::setFilter(QString filter)
{
	assert(m_mask);

	m_mask->setText(filter);
	onMaskApply();

	return;
}

void DialogAppSignalLists::onMaskReturn()
{
	onMaskApply();
	return;
}

void DialogAppSignalLists::onMaskApply()
{
	// Get mask
	//
	QString maskText = m_mask->text();

	if (maskText.isEmpty() == false)
	{
		m_masks = maskText.split(';', Qt::SkipEmptyParts);
		m_masks.removeDuplicates();

		for (QString& mask : m_masks)
		{
			mask = mask.trimmed();
		}
		
		QStringList masksHistory = QSettings().value("DialogAppSignalLists/masks").toString().split('\n');
		bool masksHistoryUpdate = false;

		for (const auto& mask : m_masks)
		{
			// Save filter history
			//
			if (masksHistory.contains(mask) == false)
			{
				masksHistory.append(mask);
				masksHistoryUpdate = true;

				QStringListModel* model = dynamic_cast<QStringListModel*>(m_completer->model());
				if (model == nullptr)
				{
					assert(model);
					return;
				}
				model->setStringList(masksHistory);
			}
		}
		if (masksHistoryUpdate == true)
		{
			QSettings().setValue("DialogAppSignalLists/masks", masksHistory.join('\n'));
		}
	}
	else
	{
		m_masks.clear();
	}

	fillAppSignalLists();

	return;
}

bool DialogAppSignalLists::addList(std::shared_ptr<AppSignalLists::AppSignalList> list)
{
	if (list == nullptr)
	{
		assert(list);
		return false;
	}

	// Add list, update UI
	//
	QString errorMessage;

	m_lists.add(list->uuid(), list);

	bool ok = m_lists.save(list->uuid(), &errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save list %1: %2").arg(list->id()).arg(errorMessage));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(0, Qt::UserRole, list->uuid());

	m_listsTree->addTopLevelItem(item);

	updateTreeItemText(item);
	updateButtonsEnableState();

	m_listsTree->clearSelection();
	item->setSelected(true);

	return true;
}

bool DialogAppSignalLists::pasteList(std::shared_ptr<AppSignalLists::AppSignalList> list)
{
	if (list == nullptr)
	{
		assert(list);
		return false;
	}

	// Add list, update UI
	//
	QString errorMessage;

	m_lists.add(list->uuid(), list);

	bool ok = m_lists.save(list->uuid(), &errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save list %1: %2").arg(list->id()).arg(errorMessage));
		return false;
	}

	QTreeWidgetItem* item = new QTreeWidgetItem();

	item->setData(0, Qt::UserRole, list->uuid());

	m_listsTree->addTopLevelItem(item);

	updateTreeItemText(item);

	item->setSelected(true);

	return true;

}

void DialogAppSignalLists::fillAppSignalLists()
{
	m_listsTree->clear();

	int count = m_lists.count();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(i);
		if (list == nullptr)
		{
			assert(list);
			break;
		}

		if (m_masks.empty() == false)
		{
			bool maskResult = false;

			for (const QString& mask : m_masks)
			{
				if (list->id().contains(mask, Qt::CaseInsensitive) == true)
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

		item->setData(0, Qt::UserRole, list->uuid());

		m_listsTree->addTopLevelItem(item);

		updateTreeItemText(item);
	}

	return;
}

void DialogAppSignalLists::setPropertyEditorObjects()
{
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		m_listPropertyEditor->clear();
		m_signalListWidget->setList(nullptr);

		updateButtonsEnableState();

		return;
	}

	bool readOnly = false;

	QList<std::shared_ptr<PropertyObject>> objects;

	AppSignalLists::AppSignalList* firstList = nullptr;

	for (const auto& item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(uuid);
		if (list == nullptr)
		{
			assert(list);
			return;
		}

		if (m_lists.fileInfo(list->uuid()).state() != E::VcsState::CheckedOut)
		{
			readOnly = true;
		}

		objects.push_back(list);

		if (firstList == nullptr)
		{
			firstList = list.get();
		}
	}

	m_listPropertyEditor->setExpertMode(theSettings.isExpertMode());
	m_listPropertyEditor->setReadOnly(readOnly);
	m_listPropertyEditor->setObjects(objects);

	if (objects.size() == 1 && firstList != nullptr)
	{
		m_signalListWidget->setList(firstList);
	}
	else
	{
		m_signalListWidget->setList(nullptr);
	}
	
	m_signalListWidget->setReadOnly(objects.size() > 1 || readOnly == true || firstList == nullptr);
		
	return;
}

bool DialogAppSignalLists::continueWithDuplicateIds()
{
	bool duplicated = false;
	QString duplicatedId;

	for (int i = 0; i < m_lists.count(); i++)
	{
		AppSignalLists::AppSignalList* c = m_lists.get(i).get();

		for (int j = 0; j < m_lists.count(); j++)
		{
			AppSignalLists::AppSignalList* e = m_lists.get(j).get();
			assert(e);

			if (i == j)
			{
				continue;
			}

			if (e->id() == c->id())
			{
				duplicated = true;
				duplicatedId = e->id();
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
		QString s = tr("AppSignalList with ID '%1' already exists.\r\n\r\nAre you sure you want to continue?").arg(duplicatedId);
		auto mbResult = QMessageBox::warning(this, qAppName(), s, QMessageBox::Yes | QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}

void DialogAppSignalLists::onSortIndicatorChanged(int column, Qt::SortOrder order)
{
	QSettings().setValue("DialogAppSignalLists/sortColumn", column);
	QSettings().setValue("DialogAppSignalLists/sortOrder", order);

	return;
}

void DialogAppSignalLists::onItemSelectionChanged()
{
	updateButtonsEnableState();
	setPropertyEditorObjects();

	return;
}

void DialogAppSignalLists::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	// Save modified objects
	//
	for (auto& object : objects)
	{
		AppSignalLists::AppSignalList* c = dynamic_cast<AppSignalLists::AppSignalList*>(object.get());
		if (c == nullptr)
		{
			assert(c);
			continue;
		}

		QString errorMessage;
		bool ok = m_lists.save(c->uuid(), &errorMessage);

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save list %1: %2").arg(c->id()).arg(errorMessage));
			continue;
		}
	}

	// update tree items
	//
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();
	for (auto item : selectedItems)
	{
		updateTreeItemText(item);
	}

	return;
}

void DialogAppSignalLists::onSignalsChanged()
{
	AppSignalLists::AppSignalList* l = m_signalListWidget->list();
	if (l == nullptr)
	{
		assert(l);
		return;
	}

	QString errorMessage;
	bool ok = m_lists.save(l->uuid(), &errorMessage);

	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Failed to save list %1: %2").arg(l->id()).arg(errorMessage));
	}
}

void DialogAppSignalLists::onAdd()
{
	std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();
	list->setId(tr("LIST_%1").arg(QString::number(m_db->nextCounterValue()).rightJustified(4, '0')));
	addList(list);
	return;
}

void DialogAppSignalLists::onRemove()
{
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected lists?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;

		bool ok = m_lists.removeFile(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			break;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the list from the list and from the storage
			//
			m_lists.remove(uuid);

			int index = m_listsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_listsTree->takeTopLevelItem(index);
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

void DialogAppSignalLists::onCopy()
{
	
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	Proto::EnvelopeSet envelopeSet;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(uuid);
		if (list == nullptr)
		{
			assert(list);
			return;
		}

		Proto::Envelope* envelope = envelopeSet.add_items();
		list->SaveData(envelope);
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
		mime->setData(AppSignalLists::AppSignalList::mimeType, data);

		QClipboard* clipboard = QApplication::clipboard();
		clipboard->clear();
		clipboard->setMimeData(mime);
	}
	
	return;
}


void DialogAppSignalLists::onPaste()
{
	QClipboard* clipboard = QApplication::clipboard();

	const QMimeData *mimeData = clipboard->mimeData();
	if (mimeData->hasFormat(AppSignalLists::AppSignalList::mimeType) == false)
	{
		return;
	}

	QByteArray data = mimeData->data(AppSignalLists::AppSignalList::mimeType);
	if (data.isEmpty() == true)
	{
		return;
	}

	Proto::EnvelopeSet envelopeSet;
	if (envelopeSet.ParseFromArray(data.constData(), static_cast<int>(data.size())) == false)
	{
		return;
	}

	m_listsTree->clearSelection();

	m_listsTree->blockSignals(true);

	m_listPropertyEditor->clear();

	for (int i = 0; i < envelopeSet.items_size(); i++)
	{
		const Proto::Envelope& envelope = envelopeSet.items(i);

		if (envelope.HasExtension(::Proto::appSignalList) == false)
		{
			Q_ASSERT(envelope.HasExtension(::Proto::appSignalList));
			continue;
		}

		std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

		if (list->LoadData(envelope) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		list->setUuid(QUuid::createUuid());
		list->setId(tr("%1 (Copy)").arg(list->id()));
		pasteList(list);
	}

	m_listsTree->blockSignals(false);

	updateButtonsEnableState();

	setPropertyEditorObjects();
	
	return;

}

void DialogAppSignalLists::onCheckOut()
{
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QString errorMessage;

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool ok = m_lists.checkOut(uuid, &errorMessage);
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

void DialogAppSignalLists::onCheckIn()
{
	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, tr("AppSignalLists Editor"),
											tr("Please enter the comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return;
	}

	if (comment.isEmpty())
	{
		QMessageBox::warning(this, tr("AppSignalLists Editor"), tr("No comment supplied! Please provide a comment."));
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileWasRemoved = false;
		QString errorMessage;

		ok = m_lists.checkIn(uuid, comment, &fileWasRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileWasRemoved == true)
		{
			// File was removed, delete the list from the list and from the storage
			//
			m_lists.remove(uuid);

			int index = m_listsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_listsTree->takeTopLevelItem(index);
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

void DialogAppSignalLists::onUndo()
{
	QList <QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, tr("AppSignalLists Editor"), tr("Are you sure you want to undo changes on selected lists?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		bool fileRemoved = false;
		QString errorMessage;

		bool ok = m_lists.undo(uuid, &fileRemoved, &errorMessage);
		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
			continue;
		}

		if (fileRemoved == true)
		{
			// File was removed, delete the list from the list and from the storage
			//
			m_lists.remove(uuid);

			int index = m_listsTree->indexOfTopLevelItem(item);
			if (index == -1)
			{
				assert(false);
				continue;
			}

			QTreeWidgetItem* deleteItem = m_listsTree->takeTopLevelItem(index);
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

			DbFileInfo fi = m_lists.fileInfo(uuid);

			ok = m_db->getLatestVersion(fi, &file, this);
			if (ok == true && file != nullptr)
			{
				QByteArray data;
				file->swapData(data);

				std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(uuid);
				if (list != nullptr)
				{
					Proto::Envelope envelope;
					if (envelope.ParseFromArray(data.constData(), static_cast<int>(data.size())) == false)
					{
						Q_ASSERT(false);
						return;
					}

					ok = list->LoadData(envelope);
					if (ok == false)
					{
						Q_ASSERT(ok);
						return;
					}
					updateTreeItemText(item);
				}
			}
		}
	}

	updateButtonsEnableState();

	setPropertyEditorObjects();
}

void DialogAppSignalLists::onRefresh()
{
	m_lists.clear();

	QString errorMessage;

	bool ok = m_lists.load(&errorMessage);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), errorMessage);
		return;
	}

	fillAppSignalLists();
	updateButtonsEnableState();

	return;
}

void DialogAppSignalLists::closeEvent(QCloseEvent* e)
{
	if (continueWithDuplicateIds() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
}

void DialogAppSignalLists::reject()
{
	if (continueWithDuplicateIds() == true)
	{
		QDialog::reject();
	}
}

void DialogAppSignalLists::onCopyShortcut()
{
	if (m_listsTree->hasFocus() == false)
	{
		return;
	}

	onCopy();
}

void DialogAppSignalLists::onPasteShortcut()
{
	if (m_listsTree->hasFocus() == false)
	{
		return;
	}

	onPaste();
}

void DialogAppSignalLists::onRemoveShortcut()
{
	if (m_listsTree->hasFocus() == false)
	{
		return;
	}

	onRemove();
}

void DialogAppSignalLists::onCustomContextMenuRequested(const QPoint &pos)
{
	Q_UNUSED(pos);

	m_popupMenu->exec(this->cursor().pos());
}


void DialogAppSignalLists::updateTreeItemText(QTreeWidgetItem* item)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();

	std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(uuid);
	if (list == nullptr)
	{
		assert(list);
		return;
	}

	int c = 0;
	item->setText(c++, list->id());

	DbFileInfo fi = m_lists.fileInfo(list->uuid());

	QBrush b(StandardColors::VcsCheckedIn);

	if (fi.state() == E::VcsState::CheckedOut)
	{
		item->setText(c++, E::valueToString<E::VcsItemAction>(fi.action()));

		int userId = fi.userId();
		item->setText(c++, m_db->username(userId));

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
		}
	}
	else
	{
		item->setText(c++, "");
		item->setText(c++, "");
	}

	for (int i = 0; i < m_listsTree->header()->count(); i++)
	{
		item->setBackground(i, b);
	}

	return;
}

void DialogAppSignalLists::updateButtonsEnableState()
{
	qsizetype selectedCount = 0;
	int checkedInCount = 0;
	int checkedOutCount = 0;

	QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

	selectedCount = selectedItems.size();

	for (auto item : selectedItems)
	{
		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<AppSignalLists::AppSignalList> list = m_lists.get(uuid);
		if (list == nullptr)
		{
			assert(list);
			return;
		}

		if (m_lists.fileInfo(list->uuid()).state() == E::VcsState::CheckedOut)
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

	return;
}
