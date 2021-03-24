#include "DialogClientBehavior.h"
#include "Settings.h"
#include "../lib/Ui/UiTools.h"

//
//
// TagsToColorDelegate
//
//
TagsToColorDelegate::TagsToColorDelegate(QWidget* parent):
	QItemDelegate(parent),
	m_parentWidget(parent)
{
}

QWidget* TagsToColorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);

	QLineEdit* edit = new QLineEdit(parent);

	if (index.column() == static_cast<int>(MonitorBehaviorEditWidget::Columns::Tag))
	{
		QRegExp rx("[A-Za-z\\d]*$");
		edit->setValidator(new QRegExpValidator(rx, edit));
	}

	if (index.column() == static_cast<int>(MonitorBehaviorEditWidget::Columns::Color1) ||
			index.column() == static_cast<int>(MonitorBehaviorEditWidget::Columns::Color2))
	{

		QRegExp rx("^[#][A-Fa-f\\d]*$");
		edit->setValidator(new QRegExpValidator(rx, edit));
	}

	return edit;
}

void TagsToColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if (index.column() == static_cast<int>(MonitorBehaviorEditWidget::Columns::Tag))
	{
		// Remember all tags except current row

		m_existingTags.clear();

		int rowCount = index.model()->rowCount();
		for (int i = 0; i < rowCount; i++)
		{
			if (i == index.row())
			{
				continue;
			}

			QModelIndex mi = index.siblingAtRow(i);

			QString s = index.model()->data(mi).toString();

			m_existingTags.push_back(s);
		}
	}

	QString s = index.model()->data(index, Qt::EditRole).toString();

	QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
	if (edit == nullptr)
	{
		Q_ASSERT(edit);
		return;
	}

	edit->setText(s);

	return;
}

void TagsToColorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	if (index.column() == static_cast<int>(MonitorBehaviorEditWidget::Columns::Tag))
	{
		QLineEdit* edit = qobject_cast<QLineEdit*>(editor);
		if (edit == nullptr)
		{
			Q_ASSERT(edit);
			return;
		}

		QString editText = edit->text();

		if (std::find(m_existingTags.begin(), m_existingTags.end(), editText) != m_existingTags.end())
		{
			QMessageBox::warning(m_parentWidget, "Client Behavior Editor", "This tag name already exists!");
			return;
		}
	}

	QItemDelegate::setModelData(editor, model, index);

	emit editingFinished(index);

	return;
}

//
// MonitorBehaviorEditWidget
//

MonitorBehaviorEditWidget::MonitorBehaviorEditWidget(QWidget* parent):
	QWidget(parent)
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Tree

	m_tagsTree = new QTreeWidget();
	mainLayout->addWidget(m_tagsTree);

	QStringList l;
	l << tr("Tag");
	l << tr("Color 1");
	l << tr("Color 2");
	m_tagsTree->setColumnCount(static_cast<int>(l.size()));
	m_tagsTree->setHeaderLabels(l);
	m_tagsTree->setColumnWidth(static_cast<int>(Columns::Tag), 120);
	m_tagsTree->setColumnWidth(static_cast<int>(Columns::Color1), 120);
	m_tagsTree->setColumnWidth(static_cast<int>(Columns::Color2), 120);
	m_tagsTree->setSelectionBehavior(QAbstractItemView::SelectRows);

	m_tagsToColorDelegate = new TagsToColorDelegate(this);
	connect(m_tagsToColorDelegate, &TagsToColorDelegate::editingFinished, this, &MonitorBehaviorEditWidget::onTagEditFinished);
	m_tagsTree->setItemDelegate(m_tagsToColorDelegate);

	// Buttons

	QVBoxLayout* tagButtonsLayout = new QVBoxLayout();

	QPushButton* b = new QPushButton(tr("Add Tag"));
	tagButtonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &MonitorBehaviorEditWidget::onAddTag);

	b = new QPushButton(tr("Remove Tag"));
	tagButtonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &MonitorBehaviorEditWidget::onRemoveTag);

	b = new QPushButton(tr("Move Up"));
	tagButtonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &MonitorBehaviorEditWidget::onTagUp);

	b = new QPushButton(tr("Move Down"));
	tagButtonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &MonitorBehaviorEditWidget::onTagDown);

	tagButtonsLayout->addStretch();

	mainLayout->addLayout(tagButtonsLayout);

	return;
}

void MonitorBehaviorEditWidget::setBehavior(std::shared_ptr<MonitorBehavior> mb)
{
	if (mb == nullptr)
	{
		Q_ASSERT(mb);
		return;
	}

	m_behavior = mb;

	fillTagToColor();

	return;
}

void MonitorBehaviorEditWidget::onAddTag()
{
	if (m_behavior == nullptr)
	{
		Q_ASSERT(m_behavior);
		return;
	}

	QStringList tags = m_behavior->tags();

	QString tag;

	do{
		bool ok = false;
		tag = QInputDialog::getText(this, tr("Client Behavior Editor"),
												tr("Enter the new tag name:"), QLineEdit::Normal,
											tr("tag"), &ok);

		if (ok == false)
		{
			return;
		}

		if (std::find(tags.begin(), tags.end(), tag) == tags.end())
		{
			break;
		}

		QMessageBox::warning(this, "Client Behavior Editor", "This tag name already exists!");

	}while(true);

	// Add tag and display it

	m_behavior->setTagToColors(tag, std::make_pair(QRgb(0xC00000), QRgb(0xFFFFFF)));

	addTagTreeItem(tag);

	// Select the created item

	m_tagsTree->clearSelection();

	QTreeWidgetItem* item = m_tagsTree->topLevelItem(m_tagsTree->topLevelItemCount() - 1);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	item->setSelected(true);

	emit behaviorModified();

	return;
}

void MonitorBehaviorEditWidget::onRemoveTag()
{
	if (m_behavior == nullptr)
	{
		Q_ASSERT(m_behavior);
		return;
	}

	QModelIndexList selectedRows = m_tagsTree->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		return;
	}

	int row = selectedRows[0].row();

	QTreeWidgetItem* item = m_tagsTree->topLevelItem(row);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	// Prevent user from removing default tags
	//
	QString tag = item->text(static_cast<int>(Columns::Tag));
	if (tag == MonitorBehavior::criticalTag ||
		tag == MonitorBehavior::attentionTag ||
		tag == MonitorBehavior::generalTag ||
		tag == MonitorBehavior::nonValidityTag ||
		tag == MonitorBehavior::simulatedTag ||
		tag == MonitorBehavior::blockedTag ||
		tag == MonitorBehavior::mismatchTag ||
		tag == MonitorBehavior::outOfLimitsTag)
	{
		QMessageBox::critical(this, qAppName(), tr("This tag can't be removed!"));
		return;
	}


	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected tag?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	if (m_behavior->removeTagToColors(row) == false)
	{
		Q_ASSERT(false);
		return;
	}

	item = m_tagsTree->takeTopLevelItem(row);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}
	delete item;

	emit behaviorModified();

	return;
}

void MonitorBehaviorEditWidget::onTagUp()
{
	moveTag(-1);
}

void MonitorBehaviorEditWidget::onTagDown()
{
	moveTag(1);
}

void MonitorBehaviorEditWidget::onTagEditFinished(const QModelIndex& index)
{
	if (m_behavior == nullptr)
	{
		Q_ASSERT(m_behavior);
		return;
	}

	if (index.row() < 0 || index.row() >= m_tagsTree->topLevelItemCount())
	{
		Q_ASSERT(false);
		return;
	}

	QTreeWidgetItem* item = m_tagsTree->topLevelItem(index.row());
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	switch (index.column())
	{
	case static_cast<int>(Columns::Tag):
	{
		m_behavior->setTag(index.row(), item->text(index.column()));
		break;
	}
	case static_cast<int>(Columns::Color1):
	case static_cast<int>(Columns::Color2):
	{
		QString tag = item->text(static_cast<int>(Columns::Tag));
		QColor color1 = QColor(item->text(static_cast<int>(Columns::Color1)));
		QColor color2 = QColor(item->text(static_cast<int>(Columns::Color2)));

		m_behavior->setTagToColors(tag, std::make_pair(color1.rgb(), color2.rgb()));

		item->setIcon(static_cast<int>(Columns::Color1), UiTools::drawColorBox(color1));
		item->setIcon(static_cast<int>(Columns::Color2), UiTools::drawColorBox(color2));
		break;
	}
	default:
		Q_ASSERT(false);
		return;

	}

	emit behaviorModified();

	return;
}

void MonitorBehaviorEditWidget::fillTagToColor()
{
	if (m_behavior == nullptr)
	{
		Q_ASSERT(m_behavior);
		return;
	}

	m_tagsTree->clear();

	QStringList tags = m_behavior->tags();
	for (const QString& tag : tags)
	{
		addTagTreeItem(tag);
	}

	return;
}

void MonitorBehaviorEditWidget::addTagTreeItem(const QString& tag)
{
	auto colors = m_behavior->tagToColors(tag);
	if (colors.has_value() == false)
	{
		Q_ASSERT(false);
		return;
	}

	std::pair<QRgb, QRgb> colorPair = colors.value();

	QColor color1(colorPair.first);
	QColor color2(colorPair.second);

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setText(static_cast<int>(Columns::Tag), tag);
	item->setText(static_cast<int>(Columns::Color1), color1.name());
	item->setText(static_cast<int>(Columns::Color2), color2.name());
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	item->setIcon(static_cast<int>(Columns::Color1), UiTools::drawColorBox(color1));
	item->setIcon(static_cast<int>(Columns::Color2), UiTools::drawColorBox(color2));

	m_tagsTree->addTopLevelItem(item);

	return;
}

void MonitorBehaviorEditWidget::moveTag(int step)
{
	if (m_behavior == nullptr)
	{
		Q_ASSERT(m_behavior);
		return;
	}

	if (step != -1 && step != 1)
	{
		Q_ASSERT(false);
		return;
	}

	QModelIndexList selectedRows = m_tagsTree->selectionModel()->selectedRows();
	if (selectedRows.size() != 1)
	{
		return;
	}

	int row = selectedRows[0].row();

	int newRow = row + step;
	if (newRow < 0 || newRow >= m_tagsTree->topLevelItemCount())
	{
		return;
	}

	if (m_behavior->moveTagToColors(row, step) == false)
	{
		Q_ASSERT(false);
		return;
	}

	fillTagToColor();


	// Select the new row

	m_tagsTree->clearSelection();

	QTreeWidgetItem* item = m_tagsTree->topLevelItem(newRow);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	item->setSelected(true);

	emit behaviorModified();

	return;
}

//
// TuningClientBehaviorEditWidget
//

TuningClientBehaviorEditWidget::TuningClientBehaviorEditWidget(QWidget* parent)
	:QWidget(parent)
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QLabel* l = new QLabel("No special properties exist for TuningClient behavior");
	l->setAlignment(Qt::AlignCenter);
	l->setStyleSheet("border: 1px solid black");
	mainLayout->addWidget(l);

	return;
}

//
//
// DialogClientBehavior
//
//

DialogClientBehavior::DialogClientBehavior(DbController *pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_dbController(pDbController)
{
	assert(db());

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	m_hSplitter = new QSplitter(Qt::Horizontal);

	// Left side

	QWidget* leftWidget = new QWidget(this);

	QHBoxLayout* leftLayout = new QHBoxLayout(leftWidget);

	leftLayout->setContentsMargins(0, 0, 0, 0);

	// Behavour list

	m_behaviorTree = new QTreeWidget();
	m_behaviorTree->setSelectionMode(QTreeWidget::SingleSelection);
	connect(m_behaviorTree, &QTreeWidget::itemSelectionChanged, this, &DialogClientBehavior::onBehaviorSelectionChanged);
	leftLayout->addWidget(m_behaviorTree);

	// Add/Remove buttons

	QVBoxLayout* addRemoveLayout = new QVBoxLayout();

	addRemoveLayout->addStretch(1);

	QPushButton* b = new QPushButton(tr("Add"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::onAddClicked);

	b = new QPushButton(tr("Remove"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::onRemoveClicked);

	b = new QPushButton(tr("Clone"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::onCloneClicked);

	addRemoveLayout->addStretch(2);

	leftLayout->addLayout(addRemoveLayout);

	m_hSplitter->addWidget(leftWidget);

	// Right side

	m_vSplitter = new QSplitter(Qt::Vertical);
	m_hSplitter->addWidget(m_vSplitter);

	// Propertry Editor

	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogClientBehavior::onBehaviorPropertiesChanged);
	m_vSplitter->addWidget(m_propertyEditor);

	// Editor widget

	QWidget* editorWidget = new QWidget(this);
	m_vSplitter->addWidget(editorWidget);

	QHBoxLayout* editorLayout = new QHBoxLayout(editorWidget);
	editorLayout->setContentsMargins(0, 0, 0, 0);

	// MonitorBehaviorEditWidget

	m_monitorBehaviorEditWidget = new MonitorBehaviorEditWidget(this);
	m_monitorBehaviorEditWidget->setVisible(false);
	connect(m_monitorBehaviorEditWidget, &MonitorBehaviorEditWidget::behaviorModified, this, &DialogClientBehavior::onBehaviorModified);
	editorLayout->addWidget(m_monitorBehaviorEditWidget);

	// TuningClientBehaviorEditWidget

	m_tuningClientBehaviorEditWidget = new TuningClientBehaviorEditWidget(this);
	m_tuningClientBehaviorEditWidget->setVisible(false);
	connect(m_tuningClientBehaviorEditWidget, &TuningClientBehaviorEditWidget::behaviorModified, this, &DialogClientBehavior::onBehaviorModified);
	editorLayout->addWidget(m_tuningClientBehaviorEditWidget);

	m_emptyEditorWidget = new QLabel(tr("No client behavior selected"), this);
	m_emptyEditorWidget->setStyleSheet("border: 1px solid black");
	m_emptyEditorWidget->setAlignment(Qt::AlignCenter);
	m_emptyEditorWidget->setVisible(true);
	editorLayout->addWidget(m_emptyEditorWidget);

	//

	mainLayout->addWidget(m_hSplitter);

	// Bottom side

	QHBoxLayout* bottomLayout = new QHBoxLayout();
	bottomLayout->addStretch();

	b = new QPushButton(tr("OK"));
	bottomLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::accept);

	b = new QPushButton(tr("Cancel"));
	bottomLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::reject);

	mainLayout->addLayout(bottomLayout);

	//

	setLayout(mainLayout);

	setWindowTitle(tr("Client Behavior Editor"));

	QStringList l;
	l << tr("BehaviorID");
	l << tr("Type");
	m_behaviorTree->setColumnCount(static_cast<int>(l.size()));
	m_behaviorTree->setHeaderLabels(l);
	m_behaviorTree->setColumnWidth(static_cast<int>(Columns::ID), 120);
	m_behaviorTree->setColumnWidth(static_cast<int>(Columns::Type), 120);

	QString errorCode;

	QByteArray data;
	bool result = loadFileFromDatabase(db(), m_behaviorStorage.dbFileName(), &errorCode, &data);
	if (result == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load behavior file, error code: \n\n'%1'").arg(errorCode));
		return;
	}

	result = m_behaviorStorage.load(data, &errorCode);
	if (result == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't parse behavior, error code: \n\n'%1'").arg(errorCode));
		return;
	}

	fillBehaviorList();

	for (int i = 0; i < m_behaviorTree->columnCount(); i++)
	{
		m_behaviorTree->resizeColumnToContents(i);
	}

	// Sort

	m_behaviorTree->sortByColumn(theSettings.m_behaviorEditorSortColumn, theSettings.m_behaviorEditorSortOrder);
	m_behaviorTree->setSortingEnabled(true);

	connect(m_behaviorTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogClientBehavior::onBehaviorSortIndicatorChanged);

	// Select first item

	if (m_behaviorTree->topLevelItemCount() != 0)
	{
		QTreeWidgetItem* item = m_behaviorTree->topLevelItem(0);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}
		item->setSelected(true);
	}

	// Restore splitters state

	m_hSplitter->restoreState(theSettings.m_behaviorEditorHSplitterState);
	m_vSplitter->restoreState(theSettings.m_behaviorEditorVSplitterState);

	return;
}

DialogClientBehavior::~DialogClientBehavior()
{
	theSettings.m_behaviorEditorHSplitterState = m_hSplitter->saveState();
	theSettings.m_behaviorEditorVSplitterState = m_vSplitter->saveState();
}

bool DialogClientBehavior::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, "Client Behavior Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

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

bool DialogClientBehavior::saveChanges()
{
	if (continueWithDuplicateId() == false)
	{
		return false;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, tr("Client Behavior Editor"),
											tr("Please enter check-in comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return false;
	}
	if (comment.isEmpty())
	{
		QMessageBox::warning(this, "Client Behavior Editor", "No comment supplied! Please provide a comment.");
		return false;
	}

	// save data to XML
	//
	QByteArray data;
	m_behaviorStorage.save(&data);

	// save to db
	//
	if (saveFileToDatabase(data, db(), m_behaviorStorage.dbFileName(), comment) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save client behavoiur file."));
		return false;
	}

	m_modified = false;

	return true;
}


bool DialogClientBehavior::loadFileFromDatabase(DbController* db, const QString& fileName, QString *errorCode, QByteArray* data)
{
	if (db == nullptr || errorCode == nullptr || data == nullptr)
	{
		assert(errorCode);
		assert(db);
		assert(data);
		return false;
	}

	// Load the file from the database
	//
	std::vector<DbFileInfo> fileList;

	bool ok = db->getFileList(&fileList, DbDir::EtcDir, fileName, true, nullptr);
	if (ok == false || fileList.size() != 1)
	{
		*errorCode = QObject::tr("File %1 is not found.").arg(fileName);
		return false;
	}

	std::shared_ptr<DbFile> file = nullptr;
	ok = db->getLatestVersion(fileList[0], &file, nullptr);
	if (ok == false || file == nullptr)
	{
		*errorCode = QObject::tr("Get latest version of %1 failed.").arg(fileName);
		return false;
	}

	file->swapData(*data);

	return true;
}

bool DialogClientBehavior::saveFileToDatabase(const QByteArray& data, DbController* db, const QString& fileName, const QString &comment)
{
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	// save to db
	//
	std::shared_ptr<DbFile> file = nullptr;
	std::vector<DbFileInfo> fileList;
	int etcFileId = db->systemFileId(DbDir::EtcDir);

	bool ok = db->getFileList(&fileList, etcFileId, fileName, true, nullptr);

	if (ok == false || fileList.size() != 1)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
		pf->setFileName(fileName);

		if (db->addFile(pf, etcFileId, nullptr) == false)
		{
			return false;
		}

		ok = db->getFileList(&fileList, etcFileId, fileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			return false;
		}
	}

	ok = db->getLatestVersion(fileList[0], &file, nullptr);
	if (ok == false || file == nullptr)
	{
		return false;
	}

	if (file->state() != VcsState::CheckedOut)
	{
		if (db->checkOut(fileList[0], nullptr) == false)
		{
			return false;
		}
	}

	QByteArray fileData(data);
	file->swapData(fileData);

	if (db->setWorkcopy(file, nullptr) == false)
	{
		return false;
	}

	if (db->checkIn(fileList[0], comment, nullptr) == false)
	{
		return false;
	}

	return true;
}

void DialogClientBehavior::fillBehaviorList()
{
	m_behaviorTree->clear();

	int count = m_behaviorStorage.count();

	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<ClientBehavior> behavior = m_behaviorStorage.get(i);
		if (behavior == nullptr)
		{
			assert(behavior);
			break;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setData(0, Qt::UserRole, i);
		updateBehaviuorItemText(item, behavior.get());
		m_behaviorTree->addTopLevelItem(item);
	}

	return;
}

void DialogClientBehavior::fillBehaviorProperties()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviorTree->selectedItems();

	if (selectedItems.empty() == true)
	{
		m_propertyEditor->clear();
		return;
	}

	QList<std::shared_ptr<PropertyObject>> objects;

	for (QTreeWidgetItem* item : selectedItems)
	{
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int index = item->data(0, Qt::UserRole).toInt();

		std::shared_ptr<ClientBehavior> behavior = m_behaviorStorage.get(index);
		if(behavior == nullptr)
		{
			Q_ASSERT(behavior);
			return;
		}

		objects.push_back(behavior);
	}

	m_propertyEditor->setObjects(objects);

	return;
}

void DialogClientBehavior::updateBehaviuorItemText(QTreeWidgetItem* item, ClientBehavior* behavior)
{
	if (item == nullptr || behavior == nullptr)
	{
		Q_ASSERT(item);
		Q_ASSERT(behavior);
		return;
	}

	item->setText(static_cast<int>(Columns::ID), behavior->behaviorId());

	if (behavior->isMonitorBehavior())
	{
		item->setText(static_cast<int>(Columns::Type), "Monitor");
	}
	else
	{
		if (behavior->isTuningClientBehavior())
		{
			item->setText(static_cast<int>(Columns::Type), "TuningClient");
		}
		else
		{
			Q_ASSERT(false);
			item->setText(static_cast<int>(Columns::Type), "Unknown");
		}
	}

	return;
}

void DialogClientBehavior::addBehavior(const std::shared_ptr<ClientBehavior> behavior)
{
	if (behavior == nullptr)
	{
		Q_ASSERT(behavior);
		return;
	}

	bool ok = false;

	QString id = QInputDialog::getText(this, qAppName(),
										 tr("Enter ID:"), QLineEdit::Normal,
										 behavior->behaviorId(), &ok);

	if (ok == false || id.isEmpty() == true)
	{
		return;
	}

	const std::vector<std::shared_ptr<ClientBehavior>> behaviors = m_behaviorStorage.behaviors();
	for (const auto& existingBehavior : behaviors)
	{
		if (existingBehavior->behaviorId() == id)
		{
			QMessageBox::critical(this, qAppName(), tr("A client behavior with specified ID already exists!"));
			return;
		}
	}

	behavior->setBehaviorId(id);

	// Add bus, update UI
	//
	m_behaviorStorage.add(behavior);

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setData(0, Qt::UserRole, m_behaviorStorage.count() - 1);
	updateBehaviuorItemText(item, behavior.get());
	m_behaviorTree->addTopLevelItem(item);

	m_behaviorTree->setCurrentItem(item);
	m_behaviorTree->clearSelection();
	item->setSelected(true);

	if (m_behaviorTree->topLevelItemCount() == 1)
	{
		m_behaviorTree->resizeColumnToContents(0);
	}

	fillBehaviorProperties();

	m_modified = true;

	return;
}

bool DialogClientBehavior::continueWithDuplicateId()
{
	bool duplicated = false;
	QString duplicatedCaption;

	for (int i = 0; i < m_behaviorStorage.count(); i++)
	{
		ClientBehavior* c = m_behaviorStorage.get(i).get();

		for (int j = 0; j < m_behaviorStorage.count(); j++)
		{
			ClientBehavior* e = m_behaviorStorage.get(j).get();
			assert(e);

			if (i == j)
			{
				continue;
			}

			if (e->behaviorId() == c->behaviorId())
			{
				duplicated = true;
				duplicatedCaption = e->behaviorId();
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
		QString s = tr("ClientBehavior with ID '%1' is duplicated.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
		auto mbResult = QMessageBox::warning(this, tr("Client Behavior Editor"), s, QMessageBox::Yes | QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}

void DialogClientBehavior::keyPressEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
	{
		return;
	}
	QDialog::keyPressEvent(evt);
	return;
}

void DialogClientBehavior::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.60),
		   static_cast<int>(screen.height() * 0.50));
	move(screen.center() - rect().center());

	if (m_propertyEditor == nullptr)
	{
		Q_ASSERT(m_propertyEditor);
	}
	else
	{
		m_propertyEditor->setSplitterPosition(static_cast<int>(m_propertyEditor->width() * 0.4));
	}

	return;
}

void DialogClientBehavior::closeEvent(QCloseEvent* e)
{
	if (askForSaveChanged() == true)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
	return;
}

void DialogClientBehavior::onAddClicked()
{
	QMenu menu;

	QAction* addMonitorBehaviorAction = new QAction(tr("Add MonitorBehavior"), this);
	connect(addMonitorBehaviorAction, &QAction::triggered, this, [this](){onAddMonitorBehavior();});
	menu.addAction(addMonitorBehaviorAction);

	QAction* addTuningClientBehaviorAction = new QAction(tr("Add TuningClientBehavior"), this);
	connect(addTuningClientBehaviorAction, &QAction::triggered, this, [this](){onAddTuningClientBehavior();});
	menu.addAction(addTuningClientBehaviorAction);

	menu.exec(this->cursor().pos());

	return;
}


void DialogClientBehavior::onAddMonitorBehavior()
{
	std::shared_ptr<ClientBehavior> behavior = std::make_shared<MonitorBehavior>();
	behavior->setBehaviorId(tr("MONITOR_BEHAVIORID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehavior(behavior);
}

void DialogClientBehavior::onAddTuningClientBehavior()
{
	std::shared_ptr<ClientBehavior> behavior = std::make_shared<TuningClientBehavior>();
	behavior->setBehaviorId(tr("TC_BEHAVIORID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehavior(behavior);
}

void DialogClientBehavior::onRemoveClicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviorTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected behavior?"), QMessageBox::Yes, QMessageBox::No);
	if (mbResult == QMessageBox::No)
	{
		return;
	}

	std::vector<int> itemsToDelete;

	for (auto item : selectedItems)
	{
		int index = item->data(0, Qt::UserRole).toInt();
		itemsToDelete.push_back(index);
	}

	std::sort(itemsToDelete.begin(), itemsToDelete.end(), std::greater<int>());

	for (int i : itemsToDelete)
	{
		if (m_behaviorStorage.remove(i) == false)
		{
			Q_ASSERT(false);
			return;
		}
	}

	//

	fillBehaviorList();

	m_modified = true;

	return;
}

void DialogClientBehavior::onCloneClicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviorTree->selectedItems();
	if (selectedItems.size() != 1)
	{
		return;
	}

	int index = selectedItems[0]->data(0, Qt::UserRole).toInt();
	const std::shared_ptr<ClientBehavior> sourceBehavior = m_behaviorStorage.get(index);
	if (sourceBehavior == nullptr)
	{
		Q_ASSERT(sourceBehavior);
		return;
	}

	std::shared_ptr<ClientBehavior> clonedBehavior;

	if (sourceBehavior->isMonitorBehavior())
	{
		clonedBehavior = std::make_shared<MonitorBehavior>();

		MonitorBehavior* mbSource = dynamic_cast<MonitorBehavior*>(sourceBehavior.get());
		if (mbSource == nullptr)
		{
			Q_ASSERT(mbSource);
			return;
		}

		MonitorBehavior* mbTarget = dynamic_cast<MonitorBehavior*>(clonedBehavior.get());
		if (mbTarget == nullptr)
		{
			Q_ASSERT(mbTarget);
			return;
		}

		*mbTarget = *mbSource;
	}
	else
	{
		if (sourceBehavior->isTuningClientBehavior())
		{
			clonedBehavior = std::make_shared<TuningClientBehavior>();

			TuningClientBehavior* tbSource = dynamic_cast<TuningClientBehavior*>(sourceBehavior.get());
			if (tbSource == nullptr)
			{
				Q_ASSERT(tbSource);
				return;
			}

			TuningClientBehavior* tTarget = dynamic_cast<TuningClientBehavior*>(clonedBehavior.get());
			if (tTarget == nullptr)
			{
				Q_ASSERT(tTarget);
				return;
			}

			*tTarget = *tbSource;
		}
		else
		{
			Q_ASSERT(false);
			return;
		}

	}

	clonedBehavior->setBehaviorId(sourceBehavior->behaviorId() + "_CLONE");

	addBehavior(clonedBehavior);

	return;
}

DbController* DialogClientBehavior::db()
{
	return m_dbController;
}


void DialogClientBehavior::accept()
{
	if (m_modified == true)
	{
		if (saveChanges() == false)
		{
			return;
		}
	}

	QDialog::accept();

	return;
}

void DialogClientBehavior::reject()
{
	if (askForSaveChanged() == true)
	{
		QDialog::reject();
	}
	return;
}

void DialogClientBehavior::onBehaviorSelectionChanged()
{
	fillBehaviorProperties();

	// Show/hide behaviour editors

	QList<QTreeWidgetItem*> selectedItems = m_behaviorTree->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		m_emptyEditorWidget->setVisible(true);

		m_monitorBehaviorEditWidget->setVisible(false);
		m_tuningClientBehaviorEditWidget->setVisible(false);
	}
	else
	{
		QTreeWidgetItem* item = selectedItems[0];

		int index = item->data(0, Qt::UserRole).toInt();

		const std::shared_ptr<ClientBehavior> behavior = m_behaviorStorage.get(index);
		if (behavior == nullptr)
		{
			Q_ASSERT(behavior);
			return;
		}

		m_emptyEditorWidget->setVisible(false);

		if (behavior->isMonitorBehavior() == true)
		{
			m_monitorBehaviorEditWidget->setVisible(true);
			m_tuningClientBehaviorEditWidget->setVisible(false);

			std::shared_ptr<MonitorBehavior> mb = std::dynamic_pointer_cast<MonitorBehavior>(behavior);
			if (mb == nullptr)
			{
				Q_ASSERT(mb);
				return;
			}

			m_monitorBehaviorEditWidget->setBehavior(mb);
		}
		else
		{
			if (behavior->isTuningClientBehavior() == true)
			{
				m_monitorBehaviorEditWidget->setVisible(false);
				m_tuningClientBehaviorEditWidget->setVisible(true);
			}
			else
			{
				// Unknown behavior
				Q_ASSERT(false);
			}
		}
	}

	return;
}

void DialogClientBehavior::onBehaviorSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_behaviorEditorSortColumn = column;
	theSettings.m_behaviorEditorSortOrder = order;

	return;
}

void DialogClientBehavior::onBehaviorPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviorTree->selectedItems();

	for (QTreeWidgetItem* item : selectedItems)
	{
		int index = item->data(0, Qt::UserRole).toInt();

		const std::shared_ptr<ClientBehavior> behavior = m_behaviorStorage.get(index);
		if (behavior == nullptr)
		{
			Q_ASSERT(behavior);
			return;
		}

		updateBehaviuorItemText(item, behavior.get());
	}

	Q_UNUSED(objects);
	m_modified = true;

	return;
}

void DialogClientBehavior::onBehaviorModified()
{
	m_modified = true;
}
