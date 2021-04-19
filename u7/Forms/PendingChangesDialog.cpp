#include "PendingChangesDialog.h"
#include <QGridLayout>
#include <QSplitter>
#include "GlobalMessanger.h"

//
// PendingChangesModel
//
PendingChangesModel::PendingChangesModel()
{
}

QModelIndex PendingChangesModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
	if (hasIndex(row, column, parent) == false)
	{
		return {};
	}

	if (parent.isValid() == false)
	{
	}
	else
	{
		assert(false);
		return {};
	}

	return createIndex(row, column);
}

QModelIndex PendingChangesModel::parent(const QModelIndex& /*index*/) const
{
	return {};
}

int PendingChangesModel::rowCount(const QModelIndex& parentIndex /*= QModelIndex()*/) const
{
	if (m_objects.empty() == true ||
		parentIndex.column() > 0)
	{
		return 0;
	}

	if (parentIndex.isValid() == false)
	{
		return static_cast<int>(m_objects.size());
	}

	return 0;
}

int PendingChangesModel::columnCount(const QModelIndex& /*parent*/ /*= QModelIndex()*/) const
{
	return static_cast<int>(Columns::Count);
}


QVariant PendingChangesModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	if (index.isValid() == false)
	{
		return {};
	}

	int row = index.row();
	Columns column = static_cast<Columns>(index.column());

	int objectIndex = row;
	if (objectIndex < 0 || objectIndex >= m_objects.size())
	{
		assert(objectIndex >=0 && objectIndex < m_objects.size());
		return {};
	}

	const PendingChangesObject& object = m_objects[objectIndex];

	if (std::holds_alternative<DbFileInfo>(object) == false &&
		std::holds_alternative<AppSignal>(object) == false)
	{
		assert(false);
		return {};
	}

	if (std::holds_alternative<DbFileInfo>(object) == true)
	{
		const DbFileInfo& file = std::get<DbFileInfo>(object);

		if (role == Qt::DisplayRole)
		{
			switch (column)
			{
			case Columns::Index:
				return {row + 1};

			case Columns::Type:
				return {"F"};

			case Columns::Id:
				return {file.fileId()};

			case Columns::NameOrAppSignalId:
				return {file.fileName()};

			case Columns::Caption:
				{
					QByteArray data = file.details().toUtf8();

					QJsonParseError parseError;
					QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

					if (parseError.error != QJsonParseError::NoError)
					{
						qDebug() << "file details pasing error: " << parseError.errorString();
						return {};
					}

					if (jsonDoc.isObject() == false)
					{
						assert(jsonDoc.isObject());		// have a look at json doc, it is supposed to be an object
						return {};
					}

					QJsonObject jsonObject = jsonDoc.object();
					QJsonValue captionValue = jsonObject.value(QLatin1String("Caption"));

					if (captionValue.type() == QJsonValue::String)
					{
						return captionValue.toString();
					}

					return {};
				}

			case Columns::State:
				return {E::valueToString<E::VcsState>(file.state())};

			case Columns::Action:
				return {E::valueToString<E::VcsItemAction>(file.action())};

			case Columns::User:
				if (auto it = m_users.find(file.userId());
					it != m_users.end())
				{
					return it->second.username();
				}
				else
				{
					return file.userId();
				}

			default:
				assert(false);
				return {};
			}
		}
	}	// Is file

	if (std::holds_alternative<AppSignal>(object) == true)
	{
		const AppSignal& signal = std::get<AppSignal>(object);

		if (role == Qt::DisplayRole)
		{
			switch (column)
			{
			case Columns::Index:
				return {row + 1};

			case Columns::Type:
				return {"S"};

			case Columns::Id:
				return {signal.ID()};

			case Columns::NameOrAppSignalId:
				return {signal.appSignalID()};

			case Columns::Caption:
				return {signal.customAppSignalID() + ": " + signal.caption()};

			case Columns::State:
				assert(signal.checkedOut() == true);
				return {"Checked Out"};

			case Columns::Action:
				return {E::valueToString<E::VcsItemAction>(signal.instanceAction())};

			case Columns::User:
				if (auto it = m_users.find(signal.userID());
					it != m_users.end())
				{
					return it->second.username();
				}
				else
				{
					return signal.userID();
				}

			default:
				assert(false);
				return {};
			}
		}
	}	// Is file


	return {};
}

QVariant PendingChangesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (static_cast<Columns>(section))
			{
			case Columns::Index:				return QStringLiteral("Index");
			case Columns::Type:					return QStringLiteral("Type");
			case Columns::Id:					return QStringLiteral("ID");
			case Columns::NameOrAppSignalId:	return QStringLiteral("State");
			case Columns::Caption:				return QStringLiteral("Caption");
			case Columns::State:				return QStringLiteral("State");
			case Columns::Action:				return QStringLiteral("Action");
			case Columns::User:					return QStringLiteral("User");
			default:
				assert(false);
			}
		}

		return {};
	}

	return {};
}


void PendingChangesModel::setData(std::vector<PendingChangesObject>&& objects, const std::vector<DbUser>& users)
{
	beginResetModel();

	m_objects = std::move(objects);
	m_users.clear();

	for (const DbUser& u : users)
	{
		m_users[u.userId()] = u;
	}

	endResetModel();

	return;
}

void PendingChangesModel::resetData()
{
	beginResetModel();

	m_objects.clear();
	m_users.clear();

	endResetModel();

	return;
}

std::vector<PendingChangesObject> PendingChangesModel::objectsByModelIndex(QModelIndexList& indexes)
{
	std::vector<PendingChangesObject> result;
	result.reserve(indexes.size());

	for (const QModelIndex& index : indexes)
	{
		if (index.isValid() == false)
		{
			assert(index.isValid());
			continue;
		}

		if (index.column() != 0)
		{
			continue;
		}

		int objectIndex = index.row();

		if (objectIndex < 0 || objectIndex > m_objects.size())
		{
			assert(objectIndex >= 0 && objectIndex < m_objects.size());
			continue;
		}

		result.push_back(m_objects[objectIndex]);
	}

	return result;
}


//
// PendingChangesDialog
//
PendingChangesDialog::PendingChangesDialog(DbController* db, QWidget* parent) :
	QDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint},
	HasDbController(db)
{
	setWindowTitle(tr("Pending Changes"));

#ifdef _DEBUG
	[[maybe_unused]]QAbstractItemModelTester* modelTester = new QAbstractItemModelTester(&m_model,
																		 QAbstractItemModelTester::FailureReportingMode::Fatal,
																		 this);
#endif

	// --
	//
	m_treeView = new QTreeView;

	m_treeView->setUniformRowHeights(true);
	m_treeView->setWordWrap(false);
	m_treeView->setExpandsOnDoubleClick(false);
	m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_treeView->setModel(&m_model);

	connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PendingChangesDialog::selectionChanged);

	// --
	//

	m_commentEdit = new QPlainTextEdit{this};
	m_commentEdit->setObjectName(QString::fromUtf8("commentEdit"));

	m_splitter = new QSplitter{Qt::Orientation::Vertical, this};
	m_splitter->addWidget(m_treeView);
	m_splitter->addWidget(m_commentEdit);
	m_splitter->setStretchFactor(0, 2);
	m_splitter->setChildrenCollapsible(false);

	m_checkInButton = new QPushButton{tr("Check In"), this};
	connect(m_checkInButton, &QPushButton::clicked, this, &PendingChangesDialog::checkIn);

	m_undoButton = new QPushButton{tr("Undo Changes"), this};
	connect(m_undoButton, &QPushButton::clicked, this, &PendingChangesDialog::undoChanges);

	m_refershButton = new QPushButton{tr("Refresh"), this};
	m_refershButton->setShortcut(QKeySequence::Refresh);
	connect(m_refershButton, &QPushButton::clicked, this, &PendingChangesDialog::updateData);

	// --
	//
	QGridLayout* layout = new QGridLayout{this};

	layout->addWidget(m_splitter, 0, 0, 6, 8);

	layout->addWidget(m_checkInButton, 6, 0, 1, 1);
	layout->addWidget(m_undoButton, 6, 1, 1, 1);
	layout->addWidget(m_refershButton, 6, 7, 1, 1);

	setLayout(layout);

	setTabOrder(m_treeView, m_commentEdit);
	setTabOrder(m_commentEdit, m_commentEdit);
	setTabOrder(m_checkInButton, m_undoButton);
	setTabOrder(m_undoButton, m_refershButton);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &PendingChangesDialog::hide);

	// --
	//
	QSettings settings{};

	QRect lastGeomentry = settings.value("PendingChangesDialog/geometry").toRect();
	resize(lastGeomentry.width(), lastGeomentry.height());

	QByteArray lastState = settings.value("PendingChangesDialog/TreeView/State").toByteArray();
	m_treeView->header()->restoreState(lastState);

	QByteArray splitterLastState = settings.value("PendingChangesDialog/Splitter").toByteArray();
	m_splitter->restoreState(splitterLastState);

	return;
}

PendingChangesDialog::~PendingChangesDialog()
{
	QSettings settings{};

	settings.setValue("PendingChangesDialog/geometry", this->geometry());
	settings.setValue("PendingChangesDialog/TreeView/State", m_treeView->header()->saveState());
	settings.setValue("PendingChangesDialog/Splitter", this->m_splitter->saveState());

	return;
}

void PendingChangesDialog::checkIn()
{
	QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty() == true)
	{
		assert(selectedIndexes.isEmpty() == false);
	}

	// --
	//
	QString comment = m_commentEdit->toPlainText();

	if (comment.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Check In comment cannot be empty!"));

		m_commentEdit->setFocus();
		return;
	}

	// Separate files and signals
	//
	std::vector<PendingChangesObject> objects = m_model.objectsByModelIndex(selectedIndexes);

	std::vector<DbFileInfo> checkInFiles;
	checkInFiles.reserve(objects.size());

	QVector<int> checkInSignals;
	checkInSignals.reserve(static_cast<int>(objects.size()));

	for (const PendingChangesObject& o : objects)
	{
		if (std::holds_alternative<DbFileInfo>(o) == true)
		{
			const DbFileInfo& file = std::get<DbFileInfo>(o);
			checkInFiles.push_back(file);

			continue;
		}

		if (std::holds_alternative<AppSignal>(o) == true)
		{
			const AppSignal& signal = std::get<AppSignal>(o);
			checkInSignals.push_back(signal.ID());
			continue;
		}

		// What kind of object is it?
		//
		assert(std::holds_alternative<DbFileInfo>(o) == true ||
			   std::holds_alternative<AppSignal>(o) == true);
	}

	// CheckIn files
	//
	if (checkInFiles.empty() == false)
	{
		db()->checkIn(checkInFiles, comment, this);
	}

	// CheckIn signals
	//
	if (checkInSignals.empty() == false)
	{
		QVector<ObjectState> signalObjectState;
		db()->checkinSignals(&checkInSignals, comment, &signalObjectState, this);
	}

	// --
	//
	updateData();

	return;
}

void PendingChangesDialog::undoChanges()
{
	QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty() == true)
	{
		assert(selectedIndexes.isEmpty() == false);
	}

	// Separate files and signals
	//
	std::vector<PendingChangesObject> objects = m_model.objectsByModelIndex(selectedIndexes);

	std::vector<DbFileInfo> undoFilesFiles;
	undoFilesFiles.reserve(objects.size());

	QVector<int> undoSignals;
	undoSignals.reserve(static_cast<int>(objects.size()));

	for (const PendingChangesObject& o : objects)
	{
		if (std::holds_alternative<DbFileInfo>(o) == true)
		{
			const DbFileInfo& file = std::get<DbFileInfo>(o);
			undoFilesFiles.push_back(file);

			continue;
		}

		if (std::holds_alternative<AppSignal>(o) == true)
		{
			const AppSignal& signal = std::get<AppSignal>(o);
			undoSignals.push_back(signal.ID());
			continue;
		}

		// What kind of object is it?
		//
		assert(std::holds_alternative<DbFileInfo>(o) == true ||
			   std::holds_alternative<AppSignal>(o) == true);
	}

	// CheckIn files
	//
	if (undoFilesFiles.empty() == false)
	{
		db()->undoChanges(undoFilesFiles, this);
	}

	// CheckIn signals
	//
	if (undoSignals.empty() == false)
	{
		QVector<ObjectState> signalObjectState;
		db()->undoSignalsChanges(undoSignals, &signalObjectState, this);
	}

	// --
	//
	updateData();

	return;
}


void PendingChangesDialog::updateData()
{
	m_model.resetData();

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	// Get checked out files
	//
	std::vector<DbFileInfo> checkedOutFiles;

	DbFileInfo rootFile{db()->rootFileId()};
	if (bool ok = db()->getCheckedOutFiles(rootFile, &checkedOutFiles, parentWidget());
		ok == false)
	{
		return;
	}

	std::vector<PendingChangesObject> objects;
	objects.reserve(checkedOutFiles.size());

	for (const DbFileInfo& fi : checkedOutFiles)
	{
		objects.push_back(fi);
	}

	// Get checked out signals
	//
	QVector<int> checkedOutSignalsIds;
	db()->getCheckedOutSignalsIDs(&checkedOutSignalsIds, this);

	if (checkedOutSignalsIds.empty() == false)
	{
		QVector<AppSignal> checkedOutSignals;
		checkedOutSignals.reserve(checkedOutSignalsIds.size());

		if (bool ok = db()->getLatestSignals(checkedOutSignalsIds, &checkedOutSignals, this);
			ok == true)
		{
			for (const AppSignal& s : checkedOutSignals)
			{
				objects.push_back(s);
			}
		}
	}

	// Get users
	//
	std::vector<DbUser> users;

	if (bool ok = db()->getUserList(&users, this);
		ok == false)
	{
		return;
	}

	// Update data in model
	//
	m_model.setData(std::move(objects), users);

	selectionChanged({}, {});

	return;
}

void PendingChangesDialog::selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	assert(m_treeView);
	assert(m_checkInButton);
	assert(m_undoButton);

	QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

	m_checkInButton->setEnabled(selectedIndexes.empty() == false);
	m_undoButton->setEnabled(selectedIndexes.empty() == false);

	return;
}

void PendingChangesDialog::show(DbController* db, QWidget* parent)
{
	if (m_instance == nullptr)
	{
		m_instance = new PendingChangesDialog{db, parent};
	}

	m_instance->updateData();

	m_instance->hide();		// Hiding dialog and showing it again will place it on the center of parent
	static_cast<QDialog*>(m_instance)->show();

	return;
}
