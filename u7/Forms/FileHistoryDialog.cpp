#include "FileHistoryDialog.h"
#include "ui_FileHistoryDialog.h"
#include "ChangesetDetailsDialog.h"

FileHistoryDialog::FileHistoryDialog()
{
	assert(false);
}

FileHistoryDialog::FileHistoryDialog(QString title, DbController* db, const std::vector<DbChangeset>& fileHistory, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::FileHistoryDialog),
	m_fileHistory(fileHistory),
	m_db(db)
{
	assert(m_db);

	ui->setupUi(this);

	setWindowTitle(title);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	// Set changesetList
	//
	ui->changesetList->setColumnCount(4);

	QStringList headerLabels;

	headerLabels << tr("Changeset") << tr("User") << tr("Date") << tr("Comment");

	ui->changesetList->setHeaderLabels(headerLabels);

	// Fill users combo box
	//
	{
		ui->userComboBox->blockSignals(true);

		std::vector<DbUser> users;
		db->getUserList(&users, parent);

		for (const DbUser& user : users)
		{
			ui->userComboBox->addItem(user.username(), user.userId());
		}

		ui->userComboBox->addItem(AllUsersText, AllUsersUserId);

		ui->userComboBox->model()->sort(0, Qt::AscendingOrder);
		ui->userComboBox->setCurrentText(AllUsersText);
		ui->userComboBox->setCurrentIndex(ui->userComboBox->findText(AllUsersText));

		ui->userComboBox->blockSignals(false);
	}

	fillList();

	return;
}

FileHistoryDialog::~FileHistoryDialog()
{
	delete ui;
}

// Modalless dfialogbox
//
void FileHistoryDialog::showHistory(DbController* db, QString objectName, const std::vector<DbChangeset>& fileHistory, QWidget* parent)
{
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	FileHistoryDialog* dialog = new FileHistoryDialog("History - " + objectName, db, fileHistory, parent);

	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();

	return;
}

void FileHistoryDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.40),
		   static_cast<int>(screen.height() * 0.35));

	move(screen.center() - rect().center());

	return;
}

void FileHistoryDialog::fillList()
{
	int userId = ui->userComboBox->currentData().toInt();

	ui->changesetList->clear();

	// Fill changeset list
	//
	QList<QTreeWidgetItem*> items;
	items.reserve(static_cast<int>(m_fileHistory.size()));

	for (unsigned int i = 0; i < m_fileHistory.size(); i++)
	{
		const DbChangeset& ci = m_fileHistory[i];

		if (userId == AllUsersUserId || userId == ci.userId())
		{
			QStringList itemTextList;
			itemTextList << QString::number(ci.changeset());
			itemTextList << ci.username();
			itemTextList << ci.date().toString("dd MMM yyyy HH:mm:ss");
			itemTextList << ci.comment();

			QTreeWidgetItem* item = new QTreeWidgetItem(itemTextList);

			items.push_back(item);
		}
	}

	ui->changesetList->insertTopLevelItems(0, items);

	// Select the first item
	//
	if (items.isEmpty() == false)
	{
		QItemSelectionModel* sm = ui->changesetList->selectionModel();

		QModelIndex mi = ui->changesetList->model()->index(0, 0);
		sm->select(mi, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	return;
}

void FileHistoryDialog::on_changesetList_doubleClicked(const QModelIndex& /*index*/)
{
	QTreeWidgetItem* item = ui->changesetList->currentItem();

	if (item == nullptr)
	{
		return;
	}

	bool ok = false;
	int changeset = item->text(0).toUInt(&ok);

	if (ok == false)
	{
		return;
	}

	FileHistoryDialog::changesetDetails(changeset);

	return;
}

void FileHistoryDialog::on_changesetList_customContextMenuRequested(const QPoint& /*pos*/)
{
	QMenu menu(ui->changesetList);

	QTreeWidgetItem* item = ui->changesetList->currentItem();

	if (item == nullptr)
	{
		return;
	}

	bool ok = false;
	int changeset = item->text(0).toUInt(&ok);

	if (ok == false)
	{
		return;
	}

	// Changeset Details
	//
	QAction* changesetDetailsAction = new QAction(tr("Changeset Details..."), &menu);
	connect(changesetDetailsAction, &QAction::triggered, this,
			[this, changeset]()
			{
				FileHistoryDialog::changesetDetails(changeset);
			});

	menu.addAction(changesetDetailsAction);

	// Show menu
	//
	menu.exec(cursor().pos());

	return;
}

void FileHistoryDialog::changesetDetails(int changeset)
{
	QWidget* parentWidget = dynamic_cast<QWidget*>(this->parent());
	assert(parentWidget);

	ChangesetDetailsDialog::showChangesetDetails(m_db, changeset, parentWidget);
}

void FileHistoryDialog::on_buttonBox_clicked(QAbstractButton* /*button*/)
{
	accept();
}

void FileHistoryDialog::on_userComboBox_currentIndexChanged(int /*index*/)
{
	fillList();
}
