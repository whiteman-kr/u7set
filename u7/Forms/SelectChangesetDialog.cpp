#include "SelectChangesetDialog.h"
#include "ui_SelectChangesetDialog.h"

SelectChangesetDialog::SelectChangesetDialog()
{
	assert(false);
}

SelectChangesetDialog::SelectChangesetDialog(QString title, const std::vector<DbChangeset>& fileHistory, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::SelectChangesetDialog),
	m_fileHistory(fileHistory),
	m_changeset(-1)
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

	headerLabels << tr("Changeset") << tr("User") << tr("Action") << tr("Date") << tr("Comment");
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

// Modal dialogbox, gets selected changest on Ok
//
int SelectChangesetDialog::getChangeset(QString fileName, const std::vector<DbChangeset>& fileHistory, QWidget* parent)
{
	SelectChangesetDialog cd("Select Changeset - " + fileName, fileHistory, parent);

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
