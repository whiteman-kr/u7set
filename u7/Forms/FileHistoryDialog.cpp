#include "FileHistoryDialog.h"
#include "ui_FileHistoryDialog.h"

FileHistoryDialog::FileHistoryDialog()
{
	assert(false);
}

FileHistoryDialog::FileHistoryDialog(QString title, const std::vector<DbChangeset>& fileHistory, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::FileHistoryDialog),
	m_fileHistory(fileHistory)
{
	ui->setupUi(this);

	setWindowTitle(title);

	// Set changesetList
	//
//	auto p = qApp->palette("QTreeView");

//	QColor highlight = p.highlight().color();
//	QColor highlightText = p.highlightedText().color();

//	QString selectionColor = QString("QTreeWidget {selection-background-color: red; selection-color: %2; }")
//							 .arg(highlight.name())
//							 .arg(highlightText.name());

//	ui->changesetList->setStyleSheet(selectionColor);

	ui->changesetList->setColumnCount(4);


	QStringList headerLabels;

	headerLabels << tr("Changeset") << tr("User") << tr("Date") << tr("Comment");

	ui->changesetList->setHeaderLabels(headerLabels);

	// Fill changeset list
	//
	QList<QTreeWidgetItem*> items;
	items.reserve(static_cast<int>(fileHistory.size()));

	for (unsigned int i = 0; i < fileHistory.size(); i++)
	{
		const DbChangeset& ci = fileHistory[i];

		QStringList itemTextList;
		itemTextList << QString::number(ci.changeset());
		itemTextList << ci.username();
		itemTextList << ci.date().toString(Qt::SystemLocaleShortDate);
		itemTextList << ci.comment();

		QTreeWidgetItem* item = new QTreeWidgetItem(itemTextList);

		items.push_back(item);
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

FileHistoryDialog::~FileHistoryDialog()
{
	delete ui;
}

// Modalless dfialogbox
//
void FileHistoryDialog::showHistory(QString fileName, const std::vector<DbChangeset>& fileHistory, QWidget* parent)
{
	FileHistoryDialog* dialog = new FileHistoryDialog("History - " + fileName, fileHistory, parent);

	dialog->setAttribute(Qt::WA_DeleteOnClose);

	dialog->show();

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
			[changeset]()
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
	// SET PARENT OF THE CREATED WINDOW  this->parent() !!!!!!!!!!!!!!!1
	assert(false);
}
