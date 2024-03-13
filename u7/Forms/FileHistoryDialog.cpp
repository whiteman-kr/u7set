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

	// Set hideUpdatesCheckBox
	//
	bool hideUpdateEvents = QSettings{}.value("FileHistoryDialog/hideUpdates", false).toBool();
	ui->hideUpdatesCheckBox->setChecked(hideUpdateEvents);

	connect(ui->hideUpdatesCheckBox, &QCheckBox::toggled, this,
			[this](bool checked)
			{
				QSettings{}.setValue("FileHistoryDialog/hideUpdates", checked);
				fillList();
			});

	// Export button
	//
	connect(ui->exportPushButton, &QPushButton::clicked, this, &FileHistoryDialog::exportHistory);
	
	// Filter/Find
	//
	connect(ui->filterPushButton, &QPushButton::clicked, this, &FileHistoryDialog::filterText);
	connect(ui->findPushButton, &QPushButton::clicked, this, &FileHistoryDialog::findText);

	// Fill list
	//
	fillList();

	return;
}

FileHistoryDialog::~FileHistoryDialog()
{
	delete ui;
}

// Non modal dialog box
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
	QRect screen = parentWidget()->screen()->availableGeometry();

	resize(static_cast<int>(screen.width() * 0.40),
		   static_cast<int>(screen.height() * 0.35));
	move(screen.center() - rect().center());

	return;
}

void FileHistoryDialog::fillList(QString filterText)
{
	int userId = ui->userComboBox->currentData().toInt();
	bool hideUpdateEvents = ui->hideUpdatesCheckBox->isChecked();

	ui->changesetList->clear();

	// Fill changeset list
	//
	QList<QTreeWidgetItem*> items;
	items.reserve(std::ssize(m_fileHistory));

	for (unsigned int i = 0; i < m_fileHistory.size(); i++)
	{
		const DbChangeset& ci = m_fileHistory[i];

		// Filter out update events
		//
		if (const QString& comment = ci.comment();
			hideUpdateEvents == true && 
			(comment.startsWith(QLatin1StringView{"Update: "}) == true) || (comment.startsWith(QLatin1StringView{"Upgrade: "}) == true))
		{
			continue;
		}
		
		// Filter out user
		//
		if (userId != AllUsersUserId && userId != ci.userId())
		{
			continue;
		}

		// Filter out text
		//
		if (filterText.isEmpty() == false && ci.comment().contains(filterText) == false)
		{
			continue;
		}

		QStringList itemTextList;
		itemTextList << QString::number(ci.changeset());
		itemTextList << ci.username();
		itemTextList << ci.date().toString("dd MMM yyyy HH:mm:ss");
		itemTextList << ci.comment();
			
		items.push_back(new QTreeWidgetItem(itemTextList));
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

void FileHistoryDialog::exportHistory()
{
	// Export content of ui->changesetList to the csv file, semicolon separated.
	// The first line is a column header.
	//
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("CSV Files (*.csv);;All Files (*.*)"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	QFile file{fileName};
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("Cannot open file %1 for writing.").arg(fileName));
		return;
	}

	QTextStream out{&file};

	// Write header, take column names from the header of the ui->changesetList
	//
	for (int i = 0, size = ui->changesetList->columnCount(); i < size; i++)
	{
		out << ui->changesetList->headerItem()->text(i);

		if (i < size - 1)
		{
			out << ";";
		}
	}

	// Write content
	//
	for (int recordIndex = 0, size = ui->changesetList->topLevelItemCount(); recordIndex < size; recordIndex++)
	{
		QTreeWidgetItem* item = ui->changesetList->topLevelItem(recordIndex);

		out << "\n";

		for (int columnIndex = 0, headerSize = ui->changesetList->columnCount(); columnIndex < headerSize; columnIndex++)
		{
			out << item->text(columnIndex);

			if (columnIndex < headerSize - 1)
			{
				out << ";";
			}
		}
	}

	return;
}

void FileHistoryDialog::filterText()
{
	QString filterText = ui->lineEdit->text().trimmed();
	fillList(filterText);
	return;
}

void FileHistoryDialog::findText()
{
	// Find next occurrence of the text in the list
	//
	QString findText = ui->lineEdit->text().trimmed();

	if (m_lastFoundText != findText)
	{
		// New search
		//
		m_lastFoundText = findText;
		m_lastFoundLineIndex = -1;
	}

	// Find next occurrence or new search if the text has changed
	//
	for (int i = m_lastFoundLineIndex + 1, rowSize = ui->changesetList->topLevelItemCount(); i < rowSize; i++)
	{
		QTreeWidgetItem* item = ui->changesetList->topLevelItem(i);

		for (int j = 0, columnSize = ui->changesetList->columnCount(); j < columnSize; j++)
		{
			if (item->text(j).contains(findText, Qt::CaseInsensitive) == true)
			{
				ui->changesetList->setCurrentItem(item);
				ui->changesetList->scrollToItem(item);
				m_lastFoundLineIndex = i;
				return;
			}
		}
	}

	// If that was the last occurrence, show message - No more occurrence found.
	//
	m_lastFoundLineIndex = -1;
	findText.clear();

	QMessageBox::information(this, tr("Find"), tr("No more occurrence found."));

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
