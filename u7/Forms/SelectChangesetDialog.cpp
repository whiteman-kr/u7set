#include "SelectChangesetDialog.h"
#include "ui_SelectChangesetDialog.h"
#include "ChangesetDetailsDialog.h"

SelectChangesetDialog::SelectChangesetDialog()
{
	assert(false);
}

SelectChangesetDialog::SelectChangesetDialog(QString title, DbController* db, const std::vector<DbChangeset>& history, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SelectChangesetDialog),
	m_db(db),
	m_history(history),
	m_changeset(-1)
{
	assert(m_db);

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

	headerLabels << tr("Changeset") << tr("User") << tr("Action") << tr("Date") << tr("Comment");
	ui->changesetList->setHeaderLabels(headerLabels);

	// Fill changeset list
	//
	QList<QTreeWidgetItem*> items;
	items.reserve(static_cast<int>(history.size()));

	for (unsigned int i = 0; i < history.size(); i++)
	{
		const DbChangeset& ci = history[i];

		QStringList itemTextList;
		itemTextList << QString::number(ci.changeset());
		itemTextList << ci.username();
		itemTextList << ci.action().text();
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

	// --
	//
	return;
}

SelectChangesetDialog::~SelectChangesetDialog()
{
	delete ui;
}

int SelectChangesetDialog::changeset() const
{
	return m_changeset;
}

void SelectChangesetDialog::setFile(const DbFileInfo& file)
{
	m_file = file;
}

DbFileInfo SelectChangesetDialog::file() const
{
	return m_file;
}

// Modal dialogbox, gets selected changest on Ok
//
int SelectChangesetDialog::getChangeset(DbController* db, const DbFileInfo& file, const std::vector<DbChangeset>& history, QWidget* parent)
{
	if (db == nullptr)
	{
		assert(db);
		return -1;
	}

	SelectChangesetDialog cd("Select Changeset - " + file.fileName(), db, history, parent);
	cd.setFile(file);

	int result = cd.exec();

	if (result == QDialog::Accepted)
	{
		return cd.changeset();
	}
	else
	{
		return -1;
	}
}

void SelectChangesetDialog::on_buttonBox_accepted()
{
	QTreeWidgetItem* curItem = ui->changesetList->currentItem();

	if (curItem != nullptr)
	{
		bool ok = false;
		int c = curItem->text(0).toUInt(&ok);

		m_changeset = ok ? c : -1;
	}

	QDialog::accept();
}

void SelectChangesetDialog::on_changesetList_doubleClicked(const QModelIndex& /*index*/)
{
	QTreeWidgetItem* item = ui->changesetList->currentItem();

	if (item != nullptr)
	{
		bool ok = false;
		int c = item->text(0).toUInt(&ok);

		m_changeset = ok ? c : -1;

		QDialog::accept();
	}

	return;
}

void SelectChangesetDialog::on_changesetList_customContextMenuRequested(const QPoint& /*pos*/)
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

	QAction* selectChangesetAction = new QAction(tr("Select Changeset"), &menu);
	connect(selectChangesetAction, &QAction::triggered, this, &SelectChangesetDialog::on_buttonBox_accepted);

	QAction* changesetDetailsAction = new QAction(tr("Changeset Details..."), &menu);
	connect(changesetDetailsAction, &QAction::triggered, this,
			[this, changeset]()
			{
				changesetDetails(changeset);
			});

	QAction* compareAction = new QAction(tr("Compare..."), &menu);
	connect(compareAction, &QAction::triggered, this,
			[this, changeset]()
			{
				SelectChangesetDialog::compareFile(m_file, changeset);
			});

	menu.addAction(selectChangesetAction);
	menu.addAction(changesetDetailsAction);
	menu.addAction(compareAction);

	// Show menu
	//
	menu.exec(cursor().pos());
}

void SelectChangesetDialog::changesetDetails(int changeset)
{
	QWidget* parentWidget = dynamic_cast<QWidget*>(this->parent());
	assert(parentWidget);

	ChangesetDetailsDialog::showChangesetDetails(m_db, changeset, parentWidget);
}

void SelectChangesetDialog::compareFile(DbFileInfo& file, int changeset)
{
	// SET PARENT OF THE CREATED WINDOW  this->parent() !!!!!!!!!!!!!!!1
	assert(false);
}