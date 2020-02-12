#include "DialogClientBehavior.h"
#include "Settings.h"

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

	// Behavour list

	QHBoxLayout* topLayout = new QHBoxLayout();

	m_behaviorTree = new QTreeWidget();
	m_behaviorTree->setSelectionMode(QTreeWidget::ExtendedSelection);
	connect(m_behaviorTree, &QTreeWidget::itemSelectionChanged, this, &DialogClientBehavior::on_behaviorSelectionChanged);
	topLayout->addWidget(m_behaviorTree);

	// Add/Remove buttons

	QVBoxLayout* addRemoveLayout = new QVBoxLayout();

	addRemoveLayout->addStretch(1);

	QPushButton* b = new QPushButton(tr("Add"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::on_add_clicked);

	b = new QPushButton(tr("Remove"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::on_remove_clicked);

	b = new QPushButton(tr("Clone"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehavior::on_clone_clicked);

	addRemoveLayout->addStretch(2);

	topLayout->addLayout(addRemoveLayout);

	// Propertry Editor

	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogClientBehavior::on_behaviorPropertiesChanged);
	topLayout->addWidget(m_propertyEditor);

	mainLayout->addLayout(topLayout);

	//

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

	m_behaviorTree->sortByColumn(theSettings.m_behaviorEditorSortColumn, theSettings.m_behaviorEditorSortOrder);
	m_behaviorTree->setSortingEnabled(true);

	connect(m_behaviorTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogClientBehavior::on_behaviorSortIndicatorChanged);

	return;
}

DialogClientBehavior::~DialogClientBehavior()
{

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
		QMessageBox::warning(this, "Client Behavior Editor", "No comment supplied!");
		return false;
	}

	// save data to XML
	//
	QByteArray data;
	m_behaviorStorage.save(data);

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
	bool ok = db->getFileList(&fileList, db->etcFileId(), fileName, true, nullptr);
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

	bool ok = db->getFileList(&fileList, db->etcFileId(), fileName, true, nullptr);

	if (ok == false || fileList.size() != 1)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
		pf->setFileName(fileName);

		if (db->addFile(pf, db->etcFileId(), nullptr) == false)
		{
			return false;
		}

		ok = db->getFileList(&fileList, db->etcFileId(), fileName, true, nullptr);
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

	const std::vector<std::shared_ptr<ClientBehavior>> behaviors = m_behaviorStorage.behavoiurs();
	for (const auto existingBehavior : behaviors)
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

void DialogClientBehavior::on_add_clicked()
{
	QMenu menu;

	QAction* addMonitorBehaviorAction = new QAction(tr("Add MonitorBehavior"), this);
	connect(addMonitorBehaviorAction, &QAction::triggered, this, [this](){on_addMonitorBehavior();});
	menu.addAction(addMonitorBehaviorAction);

	QAction* addTuningClientBehaviorAction = new QAction(tr("Add TuningClientBehavior"), this);
	connect(addTuningClientBehaviorAction, &QAction::triggered, this, [this](){on_addTuningClientBehavior();});
	menu.addAction(addTuningClientBehaviorAction);

	menu.exec(this->cursor().pos());

	return;
}


void DialogClientBehavior::on_addMonitorBehavior()
{
	std::shared_ptr<ClientBehavior> behavior = std::make_shared<MonitorBehavior>();
	behavior->setBehaviorId(tr("MONITOR_BEHAVIORID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehavior(behavior);
}

void DialogClientBehavior::on_addTuningClientBehavior()
{
	std::shared_ptr<ClientBehavior> behavior = std::make_shared<TuningClientBehavior>();
	behavior->setBehaviorId(tr("TC_BEHAVIORID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehavior(behavior);
}

void DialogClientBehavior::on_remove_clicked()
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

void DialogClientBehavior::on_clone_clicked()
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

void DialogClientBehavior::on_behaviorSelectionChanged()
{
	fillBehaviorProperties();

	return;
}

void DialogClientBehavior::on_behaviorSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_behaviorEditorSortColumn = column;
	theSettings.m_behaviorEditorSortOrder = order;

	return;
}

void DialogClientBehavior::on_behaviorPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
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
