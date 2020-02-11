#include "DialogClientBehaviour.h"
#include "Settings.h"

//
//
// DialogClientBehaviour
//
//

DialogClientBehaviour::DialogClientBehaviour(DbController *pDbController, QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_dbController(pDbController)
{
	assert(db());

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	// Behavour list

	QHBoxLayout* topLayout = new QHBoxLayout();

	m_behaviourTree = new QTreeWidget();
	m_behaviourTree->setSelectionMode(QTreeWidget::ExtendedSelection);
	connect(m_behaviourTree, &QTreeWidget::itemSelectionChanged, this, &DialogClientBehaviour::on_behaviourSelectionChanged);
	topLayout->addWidget(m_behaviourTree);

	// Add/Remove buttons

	QVBoxLayout* addRemoveLayout = new QVBoxLayout();

	addRemoveLayout->addStretch(1);

	QPushButton* b = new QPushButton(tr("Add"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehaviour::on_add_clicked);

	b = new QPushButton(tr("Remove"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehaviour::on_remove_clicked);

	b = new QPushButton(tr("Clone"));
	addRemoveLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehaviour::on_clone_clicked);

	addRemoveLayout->addStretch(2);

	topLayout->addLayout(addRemoveLayout);

	// Propertry Editor

	m_propertyEditor = new ExtWidgets::PropertyEditor(this);
	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogClientBehaviour::on_behaviourPropertiesChanged);
	topLayout->addWidget(m_propertyEditor);

	mainLayout->addLayout(topLayout);

	//

	QHBoxLayout* bottomLayout = new QHBoxLayout();

	bottomLayout->addStretch();

	b = new QPushButton(tr("OK"));
	bottomLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehaviour::accept);

	b = new QPushButton(tr("Cancel"));
	bottomLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogClientBehaviour::reject);

	mainLayout->addLayout(bottomLayout);

	//

	setLayout(mainLayout);

	setWindowTitle(tr("Client Behaviour Editor"));

	QStringList l;
	l << tr("BehaviourID");
	l << tr("Type");
	m_behaviourTree->setColumnCount(static_cast<int>(l.size()));
	m_behaviourTree->setHeaderLabels(l);
	m_behaviourTree->setColumnWidth(static_cast<int>(Columns::ID), 120);
	m_behaviourTree->setColumnWidth(static_cast<int>(Columns::Type), 120);

	QString errorCode;

	QByteArray data;
	bool result = loadFileFromDatabase(db(), m_behaviourStorage.dbFileName(), &errorCode, &data);
	if (result == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't load behaviour file, error code: \n\n'%1'").arg(errorCode));
		return;
	}

	result = m_behaviourStorage.load(data, &errorCode);
	if (result == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't parse behaviour, error code: \n\n'%1'").arg(errorCode));
		return;
	}

	fillBehaviourList();

	for (int i = 0; i < m_behaviourTree->columnCount(); i++)
	{
		m_behaviourTree->resizeColumnToContents(i);
	}

	m_behaviourTree->sortByColumn(theSettings.m_behaviourEditorSortColumn, theSettings.m_behaviourEditorSortOrder);
	m_behaviourTree->setSortingEnabled(true);

	connect(m_behaviourTree->header(), &QHeaderView::sortIndicatorChanged, this, &DialogClientBehaviour::on_behaviourSortIndicatorChanged);

	return;
}

DialogClientBehaviour::~DialogClientBehaviour()
{

}

bool DialogClientBehaviour::askForSaveChanged()
{
	if (m_modified == false)
	{
		return true;
	}

	QMessageBox::StandardButton result = QMessageBox::warning(this, "Client Behaviour Editor", "Do you want to save your changes?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

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

bool DialogClientBehaviour::saveChanges()
{
	if (continueWithDuplicateId() == false)
	{
		return false;
	}

	bool ok = false;
	QString comment = QInputDialog::getText(this, tr("Client Behaviour Editor"),
											tr("Please enter comment:"), QLineEdit::Normal,
											tr("comment"), &ok);

	if (ok == false)
	{
		return false;
	}
	if (comment.isEmpty())
	{
		QMessageBox::warning(this, "Client Behaviour Editor", "No comment supplied!");
		return false;
	}

	// save data to XML
	//
	QByteArray data;
	m_behaviourStorage.save(data);

	// save to db
	//
	if (saveFileToDatabase(data, db(), m_behaviourStorage.dbFileName(), comment) == false)
	{
		QMessageBox::critical(this, QString("Error"), tr("Can't save client behavoiur file."));
		return false;
	}

	m_modified = false;

	return true;
}


bool DialogClientBehaviour::loadFileFromDatabase(DbController* db, const QString& fileName, QString *errorCode, QByteArray* data)
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

bool DialogClientBehaviour::saveFileToDatabase(const QByteArray& data, DbController* db, const QString& fileName, const QString &comment)
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

void DialogClientBehaviour::fillBehaviourList()
{
	m_behaviourTree->clear();

	int count = m_behaviourStorage.count();

	for (int i = 0; i < count; i++)
	{
		const std::shared_ptr<ClientBehaviour> behaviour = m_behaviourStorage.get(i);
		if (behaviour == nullptr)
		{
			assert(behaviour);
			break;
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setData(0, Qt::UserRole, i);
		updateBehaviuorItemText(item, behaviour.get());
		m_behaviourTree->addTopLevelItem(item);
	}

	return;
}

void DialogClientBehaviour::fillBehaviourProperties()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviourTree->selectedItems();

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

		std::shared_ptr<ClientBehaviour> behaviour = m_behaviourStorage.get(index);
		if(behaviour == nullptr)
		{
			Q_ASSERT(behaviour);
			return;
		}

		objects.push_back(behaviour);
	}

	m_propertyEditor->setObjects(objects);

	return;
}

void DialogClientBehaviour::updateBehaviuorItemText(QTreeWidgetItem* item, ClientBehaviour* behaviour)
{
	if (item == nullptr || behaviour == nullptr)
	{
		Q_ASSERT(item);
		Q_ASSERT(behaviour);
		return;
	}

	item->setText(static_cast<int>(Columns::ID), behaviour->behaviourId());

	if (behaviour->isMonitorBehaviour())
	{
		item->setText(static_cast<int>(Columns::Type), "Monitor");
	}
	else
	{
		if (behaviour->isTuningClientBehaviour())
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

void DialogClientBehaviour::addBehaviour(const std::shared_ptr<ClientBehaviour> behaviour)
{
	if (behaviour == nullptr)
	{
		Q_ASSERT(behaviour);
		return;
	}

	bool ok = false;

	QString id = QInputDialog::getText(this, qAppName(),
										 tr("Enter ID:"), QLineEdit::Normal,
	                                     behaviour->behaviourId(), &ok);

	if (ok == false || id.isEmpty() == true)
	{
		return;
	}

	const std::vector<std::shared_ptr<ClientBehaviour>> behaviours = m_behaviourStorage.behavoiurs();
	for (const auto existingBehaviour : behaviours)
	{
		if (existingBehaviour->behaviourId() == id)
		{
			QMessageBox::critical(this, qAppName(), tr("A client behaviour with specified ID already exists!"));
			return;
		}
	}

	behaviour->setBehaviourId(id);

	// Add bus, update UI
	//
	m_behaviourStorage.add(behaviour);

	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setData(0, Qt::UserRole, m_behaviourStorage.count() - 1);
	updateBehaviuorItemText(item, behaviour.get());
	m_behaviourTree->addTopLevelItem(item);

	m_behaviourTree->setCurrentItem(item);
	m_behaviourTree->clearSelection();
	item->setSelected(true);

	if (m_behaviourTree->topLevelItemCount() == 1)
	{
		m_behaviourTree->resizeColumnToContents(0);
	}

	fillBehaviourProperties();

	m_modified = true;

	return;
}

bool DialogClientBehaviour::continueWithDuplicateId()
{
	bool duplicated = false;
	QString duplicatedCaption;

	for (int i = 0; i < m_behaviourStorage.count(); i++)
	{
		ClientBehaviour* c = m_behaviourStorage.get(i).get();

		for (int j = 0; j < m_behaviourStorage.count(); j++)
		{
			ClientBehaviour* e = m_behaviourStorage.get(j).get();
			assert(e);

			if (i == j)
			{
				continue;
			}

			if (e->behaviourId() == c->behaviourId())
			{
				duplicated = true;
				duplicatedCaption = e->behaviourId();
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
		QString s = tr("ClientBehaviour with ID '%1' is duplicated.\r\n\r\nAre you sure you want to continue?").arg(duplicatedCaption);
		auto mbResult = QMessageBox::warning(this, tr("Client Behaviour Editor"), s, QMessageBox::Yes | QMessageBox::No);

		if (mbResult == QMessageBox::No)
		{
			return false;
		}
	}

	return true;
}

void DialogClientBehaviour::keyPressEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
	{
		return;
	}
	QDialog::keyPressEvent(evt);
	return;
}

void DialogClientBehaviour::showEvent(QShowEvent*)
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

void DialogClientBehaviour::closeEvent(QCloseEvent* e)
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

void DialogClientBehaviour::on_add_clicked()
{
	QMenu menu;

	QAction* addMonitorBehaviourAction = new QAction(tr("Add MonitorBehaviour"), this);
	connect(addMonitorBehaviourAction, &QAction::triggered, this, [this](){on_addMonitorBehaviour();});
	menu.addAction(addMonitorBehaviourAction);

	QAction* addTuningClientBehaviourAction = new QAction(tr("Add TuningClientBehaviour"), this);
	connect(addTuningClientBehaviourAction, &QAction::triggered, this, [this](){on_addTuningClientBehaviour();});
	menu.addAction(addTuningClientBehaviourAction);

	menu.exec(this->cursor().pos());

	return;
}


void DialogClientBehaviour::on_addMonitorBehaviour()
{
	std::shared_ptr<ClientBehaviour> behaviour = std::make_shared<MonitorBehaviour>();
	behaviour->setBehaviourId(tr("MONITOR_BEHAVIOURID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehaviour(behaviour);
}

void DialogClientBehaviour::on_addTuningClientBehaviour()
{
	std::shared_ptr<ClientBehaviour> behaviour = std::make_shared<TuningClientBehaviour>();
	behaviour->setBehaviourId(tr("TC_BEHAVIOURID_%1").arg(QString::number(db()->nextCounterValue()).rightJustified(4, '0')));
	addBehaviour(behaviour);
}

void DialogClientBehaviour::on_remove_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviourTree->selectedItems();

	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	auto mbResult = QMessageBox::warning(this, qAppName(), tr("Are you sure you want to remove selected behaviour?"), QMessageBox::Yes, QMessageBox::No);
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
		if (m_behaviourStorage.remove(i) == false)
		{
			Q_ASSERT(false);
			return;
		}
	}

	//

	fillBehaviourList();

	m_modified = true;

	return;
}

void DialogClientBehaviour::on_clone_clicked()
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviourTree->selectedItems();
	if (selectedItems.size() != 1)
	{
		return;
	}

	int index = selectedItems[0]->data(0, Qt::UserRole).toInt();
	const std::shared_ptr<ClientBehaviour> sourceBehaviour = m_behaviourStorage.get(index);
	if (sourceBehaviour == nullptr)
	{
		Q_ASSERT(sourceBehaviour);
		return;
	}

	std::shared_ptr<ClientBehaviour> clonedBehaviour;

	if (sourceBehaviour->isMonitorBehaviour())
	{
		clonedBehaviour = std::make_shared<MonitorBehaviour>();

		MonitorBehaviour* mbSource = dynamic_cast<MonitorBehaviour*>(sourceBehaviour.get());
		if (mbSource == nullptr)
		{
			Q_ASSERT(mbSource);
			return;
		}

		MonitorBehaviour* mbTarget = dynamic_cast<MonitorBehaviour*>(clonedBehaviour.get());
		if (mbTarget == nullptr)
		{
			Q_ASSERT(mbTarget);
			return;
		}

		*mbTarget = *mbSource;
	}
	else
	{
		if (sourceBehaviour->isTuningClientBehaviour())
		{
			clonedBehaviour = std::make_shared<TuningClientBehaviour>();

			TuningClientBehaviour* tbSource = dynamic_cast<TuningClientBehaviour*>(sourceBehaviour.get());
			if (tbSource == nullptr)
			{
				Q_ASSERT(tbSource);
				return;
			}

			TuningClientBehaviour* tTarget = dynamic_cast<TuningClientBehaviour*>(clonedBehaviour.get());
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

	clonedBehaviour->setBehaviourId(sourceBehaviour->behaviourId() + "_CLONE");

	addBehaviour(clonedBehaviour);

	return;
}

DbController* DialogClientBehaviour::db()
{
	return m_dbController;
}


void DialogClientBehaviour::accept()
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

void DialogClientBehaviour::reject()
{
	if (askForSaveChanged() == true)
	{
		QDialog::reject();
	}
	return;
}

void DialogClientBehaviour::on_behaviourSelectionChanged()
{
	fillBehaviourProperties();

	return;
}

void DialogClientBehaviour::on_behaviourSortIndicatorChanged(int column, Qt::SortOrder order)
{
	theSettings.m_behaviourEditorSortColumn = column;
	theSettings.m_behaviourEditorSortOrder = order;

	return;
}

void DialogClientBehaviour::on_behaviourPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	QList<QTreeWidgetItem*> selectedItems = m_behaviourTree->selectedItems();
	for (QTreeWidgetItem* item : selectedItems)
	{
		int index = item->data(0, Qt::UserRole).toInt();

		const std::shared_ptr<ClientBehaviour> behaviour = m_behaviourStorage.get(index);
		if (behaviour == nullptr)
		{
			Q_ASSERT(behaviour);
			return;
		}

		updateBehaviuorItemText(item, behaviour.get());
	}

	Q_UNUSED(objects);
	m_modified = true;

	return;
}
