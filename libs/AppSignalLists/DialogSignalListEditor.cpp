#include "../../AppSignalLib/ISignalManager.h"
#include <AppSignalLists/DialogSignalListEditor.h>
#include <UiLib/PropertyEditor.h>
#include <UiLib/UiTools.h>

namespace AppSignalLists
{
	void DialogSignalListEditor::showDialog(AppSignalListSet& appSignalListSet,
											ISignalManager& signalManager,
											ITuningSignalManager* tuningSignalManager,
											QWidget* parent)
	{
		if (s_instance == nullptr)
		{
			s_instance = new DialogSignalListEditor(appSignalListSet, signalManager, tuningSignalManager, parent);
			s_instance->show();
		}
		else
		{
			s_instance->activateWindow();
		}
		UiTools::adjustDialogPlacement(s_instance);

		return;
	}

	DialogSignalListEditor* DialogSignalListEditor::instance()
	{
		return s_instance;
	}

	DialogSignalListEditor::DialogSignalListEditor(AppSignalListSet& appSignalListSet,
												   ISignalManager& signalManager,
												   ITuningSignalManager* tuningSignalManager,
												   QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint),
		m_appLists(appSignalListSet),
		m_signalManager(signalManager)
	{
		m_editLists = m_appLists;

		setWindowTitle(tr("AppSignalLists Editor"));

		setAttribute(Qt::WA_DeleteOnClose);

		// Create user interface
		//
		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		QHBoxLayout* maskLayout = new QHBoxLayout();

		m_mask = new QLineEdit();
		m_mask->setClearButtonEnabled(true);

		connect(m_mask, &QLineEdit::returnPressed, this, &DialogSignalListEditor::onMaskReturn);
		connect(m_mask,
				&QLineEdit::textChanged,
				this,
				[this](const QString& text)
				{
					if (text.isEmpty() == true)
					{
						onMaskApply();
					}
				});

		m_maskApply = new QPushButton(tr("Filter"));
		connect(m_maskApply, &QPushButton::clicked, this, &DialogSignalListEditor::onMaskApply);

		m_listsTree = new QTreeWidget();

		QStringList l;
		l << tr("ID");
		l << tr("Caption");
		l << tr("Type");

		m_listsTree->setColumnCount(static_cast<int>(l.size()));
		m_listsTree->setHeaderLabels(l);

		int il = 0;
		m_listsTree->setColumnWidth(il++, 180);
		m_listsTree->setColumnWidth(il++, 100);
		m_listsTree->setSortingEnabled(true);
		m_listsTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_listsTree->setContextMenuPolicy(Qt::CustomContextMenu);

		m_listsTree->setRootIsDecorated(false);

		connect(m_listsTree, &QTreeWidget::itemSelectionChanged, this, &DialogSignalListEditor::onItemSelectionChanged);
		connect(m_listsTree, &QWidget::customContextMenuRequested, this, &DialogSignalListEditor::onCustomContextMenuRequested);

		m_listPropertyEditor = new ExtWidgets::PropertyEditor(this);
		connect(m_listPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogSignalListEditor::onPropertiesChanged);

		m_signalListWidget = new AppSignalLists::AppSignalListWidget(m_signalManager, tuningSignalManager, this);
		connect(m_signalListWidget, &AppSignalLists::AppSignalListWidget::signalsChanged, this, &DialogSignalListEditor::onSignalsChanged);

		maskLayout->addWidget(m_mask);
		maskLayout->addWidget(m_maskApply);

		m_splitter = new QSplitter(Qt::Horizontal);

		m_splitter->addWidget(m_listsTree);

		QTabWidget* tab = new QTabWidget();
		tab->addTab(m_listPropertyEditor, tr("Properties"));
		tab->addTab(m_signalListWidget, tr("Signals"));
		m_splitter->addWidget(tab);

		m_splitter->setChildrenCollapsible(false);

		QHBoxLayout* buttonsLayout = new QHBoxLayout();

		m_btnAdd = new QPushButton(tr("Add"));
		m_btnRemove = new QPushButton(tr("Remove"));

		m_btnOk = new QPushButton(tr("OK"));
		m_btnCancel = new QPushButton(tr("Cancel"));

		buttonsLayout->addWidget(m_btnAdd);
		buttonsLayout->addWidget(m_btnRemove);
		buttonsLayout->addStretch();
		buttonsLayout->addWidget(m_btnOk);
		buttonsLayout->addWidget(m_btnCancel);

		connect(m_btnAdd, &QPushButton::clicked, this, &DialogSignalListEditor::onAdd);
		connect(m_btnRemove, &QPushButton::clicked, this, &DialogSignalListEditor::onRemove);

		connect(m_btnOk, &QPushButton::clicked, this, &DialogSignalListEditor::accept);
		connect(m_btnCancel, &QPushButton::clicked, this, &DialogSignalListEditor::reject);

		mainLayout->addLayout(maskLayout);
		mainLayout->addWidget(m_splitter);
		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);

		// Shortcuts
		//
		QShortcut* removeShortcut = new QShortcut(QKeySequence(QKeySequence::Delete), this);
		connect(removeShortcut, &QShortcut::activated, this, &DialogSignalListEditor::onRemoveShortcut);

		QShortcut* copyShortcut = new QShortcut(QKeySequence(QKeySequence::Copy), this);
		connect(copyShortcut, &QShortcut::activated, this, &DialogSignalListEditor::onCopyShortcut);

		QShortcut* pasteShortcut = new QShortcut(QKeySequence(QKeySequence::Paste), this);
		connect(pasteShortcut, &QShortcut::activated, this, &DialogSignalListEditor::onPasteShortcut);

		// Mask and completer
		//
		m_completer = new QCompleter(QSettings().value("DialogSignalListEditor/masks").toString().split('\n'), this);
		m_completer->setCaseSensitivity(Qt::CaseInsensitive);
		m_mask->setCompleter(m_completer);

		connect(m_mask,
				&QLineEdit::textEdited,
				[this]()
				{
					m_completer->complete();
				});
		connect(m_completer, static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_mask, &QLineEdit::setText);

		// Popup menu
		//
		m_addAction = new QAction(tr("Add"), this);
		m_removeAction = new QAction(tr("Remove"), this);
		m_removeAction->setShortcut(QKeySequence::Delete);

		m_copyAction = new QAction(tr("Copy"), this);
		m_copyAction->setShortcut(QKeySequence::Copy);

		m_pasteAction = new QAction(tr("Paste"), this);
		m_pasteAction->setShortcut(QKeySequence::Paste);

		connect(m_addAction, &QAction::triggered, this, &DialogSignalListEditor::onAdd);
		connect(m_removeAction, &QAction::triggered, this, &DialogSignalListEditor::onRemove);
		connect(m_copyAction, &QAction::triggered, this, &DialogSignalListEditor::onCopy);
		connect(m_pasteAction, &QAction::triggered, this, &DialogSignalListEditor::onPaste);

		m_popupMenu = new QMenu(this);
		m_popupMenu->addAction(m_addAction);
		m_popupMenu->addAction(m_removeAction);
		m_popupMenu->addSeparator();
		m_popupMenu->addAction(m_copyAction);
		m_popupMenu->addAction(m_pasteAction);
		m_popupMenu->addSeparator();

		// fill data
		//
		fillAppSignalLists();

		updateListEditorEnableState();

		// sort items
		//
		for (int i = 0; i < m_listsTree->columnCount(); i++)
		{
			m_listsTree->resizeColumnToContents(i);
		}


		m_listsTree->sortByColumn(
			QSettings().value("DialogSignalListEditor/sortColumn", 0).toInt(),
			static_cast<Qt::SortOrder>(QSettings().value("DialogSignalListEditor/sortOrder", Qt::AscendingOrder).toInt()));

		connect(m_listsTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogSignalListEditor::onSortIndicatorChanged);

		// Restore settings
		//
		QByteArray ba = QSettings().value("DialogSignalListEditor/geometry").toByteArray();
		if (ba.isEmpty() == false)
		{
			restoreGeometry(ba);
		}

		ba = QSettings().value("DialogSignalListEditor/headerState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_listsTree->header()->restoreState(ba);
		}

		ba = QSettings().value("DialogSignalListEditor/splitterState").toByteArray();
		if (ba.isEmpty() == false)
		{
			m_splitter->restoreState(ba);
		}

		m_listPropertyEditor->setSplitterPosition(QSettings().value("DialogSignalListEditor/splitterPosition", 200).toInt());

		if (m_listsTree->topLevelItemCount() != 0)
		{
			m_listsTree->topLevelItem(0)->setSelected(true);
		}

		return;
	}

	DialogSignalListEditor::~DialogSignalListEditor()
	{
		QSettings().setValue("DialogSignalListEditor/geometry", saveGeometry());
		QSettings().setValue("DialogSignalListEditor/headerState", m_listsTree->header()->saveState());
		QSettings().setValue("DialogSignalListEditor/splitterState", m_splitter->saveState());
		QSettings().setValue("DialogSignalListEditor/splitterPosition", m_listPropertyEditor->splitterPosition());

		s_instance = nullptr;

		return;
	}

	void DialogSignalListEditor::setFilter(QString filter)
	{
		assert(m_mask);

		m_mask->setText(filter);
		onMaskApply();

		return;
	}

	void DialogSignalListEditor::onMaskReturn()
	{
		onMaskApply();
		return;
	}

	void DialogSignalListEditor::onMaskApply()
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

			QStringList masksHistory = QSettings().value("DialogSignalListEditor/masks").toString().split('\n');
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
				QSettings().setValue("DialogSignalListEditor/masks", masksHistory.join('\n'));
			}
		}
		else
		{
			m_masks.clear();
		}

		fillAppSignalLists();

		return;
	}

	bool DialogSignalListEditor::addList(std::shared_ptr<AppSignalLists::AppSignalList> list)
	{
		if (list == nullptr)
		{
			assert(list);
			return false;
		}

		// Add list, update UI
		//
		QString errorMessage;

		m_editLists.add(list);

		QTreeWidgetItem* item = new QTreeWidgetItem();

		item->setData(0, Qt::UserRole, list->uuid());

		m_listsTree->addTopLevelItem(item);

		updateTreeItemText(item);

		m_listsTree->clearSelection();
		item->setSelected(true);

		m_modified = true;

		return true;
	}

	bool DialogSignalListEditor::pasteList(std::shared_ptr<AppSignalLists::AppSignalList> list)
	{
		if (list == nullptr)
		{
			assert(list);
			return false;
		}

		// Add list, update UI
		//
		QString errorMessage;

		m_editLists.add(list);

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setData(0, Qt::UserRole, list->uuid());
		m_listsTree->addTopLevelItem(item);

		updateTreeItemText(item);
		item->setSelected(true);

		m_modified = true;

		return true;
	}

	void DialogSignalListEditor::fillAppSignalLists()
	{
		m_listsTree->clear();

		int count = m_editLists.count();
		for (int i = 0; i < count; i++)
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list = m_editLists.get(i);
			if (list == nullptr)
			{
				assert(list);
				break;
			}

			// Do not add UI lists
			//
			if (list->systemTagsList().contains(AppSignalLists::AppSignalList::tagUi) == true ||
				list->systemTagsList().contains(AppSignalLists::AppSignalList::tagEquipment) == true ||
				list->systemTagsList().contains(AppSignalLists::AppSignalList::tagSchema) == true)
			{
				continue;
			}

			if (m_masks.empty() == false)
			{
				bool maskResult = false;

				for (const QString& mask : m_masks)
				{
					if (list->id().contains(mask, Qt::CaseInsensitive) == true ||
						list->caption().contains(mask, Qt::CaseInsensitive) == true)
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

	void DialogSignalListEditor::setPropertyEditorObjects()
	{
		QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

		if (selectedItems.isEmpty() == true)
		{
			m_listPropertyEditor->clear();
			m_signalListWidget->setList(nullptr);

			updateListEditorEnableState();

			return;
		}

		bool readOnly = false;

		QList<std::shared_ptr<PropertyObject>> objects;

		AppSignalLists::AppSignalList* firstList = nullptr;

		for (const auto& item : selectedItems)
		{
			QUuid uuid = item->data(0, Qt::UserRole).toUuid();

			std::shared_ptr<AppSignalLists::AppSignalList> list = m_editLists.get(uuid);
			if (list == nullptr)
			{
				assert(list);
				return;
			}

			if (list->systemTagsList().contains(AppSignalList::tagIde) == true)
			{
				readOnly = true;
			}

			objects.push_back(list);

			if (firstList == nullptr)
			{
				firstList = list.get();
			}
		}

		m_btnRemove->setEnabled(readOnly == false);
		m_removeAction->setEnabled(readOnly == false);

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

	bool DialogSignalListEditor::continueWithDuplicateIds()
	{
		std::vector<std::pair<QString, QString>> nonUniqueIds = m_editLists.checkForSameIds();

		QStringList duplicatedIds;
		for (const auto& [id, caption] : nonUniqueIds)
		{
			if (caption.isEmpty() == false)
			{
				duplicatedIds.push_back(QString("%1 ('%2')").arg(id).arg(caption));
			}
			else
			{
				duplicatedIds.push_back(id);
			}
		}

		if (duplicatedIds.empty() == false)
		{
			QString s =
				tr("Signal lists with duplicated IDs found:\n\n%1\n\nAre you sure you want to continue?").arg(duplicatedIds.join('\n'));
			auto mbResult = QMessageBox::warning(this, qAppName(), s, QMessageBox::Yes | QMessageBox::No);

			if (mbResult == QMessageBox::No)
			{
				return false;
			}
		}

		return true;
	}

	void DialogSignalListEditor::onSortIndicatorChanged(int column, Qt::SortOrder order)
	{
		QSettings().setValue("DialogSignalListEditor/sortColumn", column);
		QSettings().setValue("DialogSignalListEditor/sortOrder", order);

		return;
	}

	void DialogSignalListEditor::onItemSelectionChanged()
	{
		updateListEditorEnableState();
		setPropertyEditorObjects();

		return;
	}

	void DialogSignalListEditor::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> /*objects*/)
	{
		// update tree items
		//
		QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();
		for (auto item : selectedItems)
		{
			updateTreeItemText(item);
		}

		m_modified = true;

		return;
	}

	void DialogSignalListEditor::onSignalsChanged()
	{
		m_modified = true;
	}

	void DialogSignalListEditor::onAdd()
	{
		std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

		int counter = 1;
		QString id;
		do
		{
			id = tr("LIST_%1").arg(QString::number(counter++).rightJustified(4, '0'));

		} while (m_editLists.get(id) != nullptr);

		list->setId(id);
		list->setCaption(id.toLower());
		addList(list);

		return;
	}

	void DialogSignalListEditor::onRemove()
	{
		QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();

		if (selectedItems.isEmpty() == true)
		{
			return;
		}

		auto mbResult = QMessageBox::warning(this,
											 qAppName(),
											 tr("Are you sure you want to remove selected lists?"),
											 QMessageBox::Yes,
											 QMessageBox::No);
		if (mbResult == QMessageBox::No)
		{
			return;
		}

		QString errorMessage;

		for (auto item : selectedItems)
		{
			QUuid uuid = item->data(0, Qt::UserRole).toUuid();

			// File was removed, delete the list from the list and from the storage
			//
			m_editLists.remove(uuid);

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

		setPropertyEditorObjects();

		m_modified = true;

		return;
	}

	void DialogSignalListEditor::onCopy()
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

			std::shared_ptr<AppSignalLists::AppSignalList> list = m_editLists.get(uuid);
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


	void DialogSignalListEditor::onPaste()
	{
		QClipboard* clipboard = QApplication::clipboard();

		const QMimeData* mimeData = clipboard->mimeData();
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
			list->setId(tr("%1_Copy").arg(list->id()));

			// Remove IDE tag
			{
				QStringList tags = list->systemTagsList();
				tags.removeAll(AppSignalLists::AppSignalList::tagIde);
				list->setSystemTags(tags.join(' '));
			}

			pasteList(list);
		}

		m_listsTree->blockSignals(false);

		updateListEditorEnableState();

		setPropertyEditorObjects();

		return;
	}

	void DialogSignalListEditor::saveChanges()
	{
		m_appLists = m_editLists;

		// Count cached hashes for all user lists
		//
		std::vector<Hash> allHashes = m_signalManager.signalHashes();

		auto lists = m_appLists.lists();
		for (auto& list : lists)
		{
			Q_ASSERT(list);

			if (list->systemTagsList().contains(AppSignalList::tagIde) == true)
			{
				continue; // We process only user-created lists!!!
			}

			auto& appListHashesCache = list->mutableAppListHashesCache();
			auto& tuningListHashesCache = list->mutableTuningListHashesCache();
			appListHashesCache.clear();
			tuningListHashesCache.clear();

			for (Hash hash : allHashes)
			{
				auto asp = m_signalManager.signalParam(hash);
				if (asp.has_value() == false)
				{
					assert(asp.has_value());
					continue;
				}

				// Add filtered signals to the list
				//
				if (list->appSignalMatch(*asp) == true)
				{
					appListHashesCache.insert(hash);

					if (asp->enableTuning() == true)
					{
						tuningListHashesCache.insert(hash);
					}
				}
			}
		}

		QString errorMessage;
		if (m_appLists.save(&errorMessage) == false)
		{
			QMessageBox::critical(this, qAppName(), errorMessage);
		}

		m_appLists.updatePerformed();
	}

	void DialogSignalListEditor::closeEvent(QCloseEvent* e)
	{
		if (m_modified == true)
		{
			int reply = QMessageBox::warning(this,
											 qAppName(),
											 tr("Warning! Changes are not saved. Do you wish to save them?"),
											 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

			if (reply == QMessageBox::Yes)
			{
				// Save changes
				//
				if (continueWithDuplicateIds() == false)
				{
					e->ignore();
					return;
				}
				saveChanges();

				e->accept();
				return;
			}

			if (reply == QMessageBox::Cancel)
			{
				// Cancel
				//
				e->ignore();
				return;
			}
		}

		e->accept();
		return;
	}

	void DialogSignalListEditor::accept()
	{
		if (continueWithDuplicateIds() == false)
		{
			return;
		}
		saveChanges();
		QDialog::accept();
	}

	void DialogSignalListEditor::reject()
	{
		if (m_modified == true)
		{
			int reply = QMessageBox::warning(this,
											 qAppName(),
											 tr("Warning! Changes are not saved. Do you wish to save them?"),
											 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

			if (reply == QMessageBox::Yes)
			{
				accept();
				return;
			}

			if (reply == QMessageBox::Cancel)
			{
				return;
			}
		}

		QDialog::reject();
	}

	void DialogSignalListEditor::onCopyShortcut()
	{
		if (m_listsTree->hasFocus() == false)
		{
			return;
		}

		onCopy();
	}

	void DialogSignalListEditor::onPasteShortcut()
	{
		if (m_listsTree->hasFocus() == false)
		{
			return;
		}

		onPaste();
	}

	void DialogSignalListEditor::onRemoveShortcut()
	{
		if (m_listsTree->hasFocus() == false)
		{
			return;
		}

		onRemove();
	}

	void DialogSignalListEditor::onCustomContextMenuRequested(const QPoint& pos)
	{
		Q_UNUSED(pos);

		m_popupMenu->exec(this->cursor().pos());
	}


	void DialogSignalListEditor::updateTreeItemText(QTreeWidgetItem* item)
	{
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		QUuid uuid = item->data(0, Qt::UserRole).toUuid();

		std::shared_ptr<AppSignalLists::AppSignalList> list = m_editLists.get(uuid);
		if (list == nullptr)
		{
			assert(list);
			return;
		}

		int c = 0;
		item->setText(c++, list->id());
		item->setText(c++, list->caption());
		item->setText(c++, list->systemTags());

		return;
	}

	void DialogSignalListEditor::updateListEditorEnableState()
	{
		qsizetype selectedCount = 0;

		QList<QTreeWidgetItem*> selectedItems = m_listsTree->selectedItems();
		selectedCount = selectedItems.size();

		m_listPropertyEditor->setEnabled(selectedCount > 0);
		m_signalListWidget->setEnabled(selectedCount > 0);


		m_btnRemove->setEnabled(selectedCount > 0);
		m_removeAction->setEnabled(selectedCount > 0);
		m_copyAction->setEnabled(selectedCount > 0);

		return;
	}
} // namespace AppSignalLists