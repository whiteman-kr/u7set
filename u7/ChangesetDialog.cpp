#include "ChangesetDialog.h"
#include "ui_ChangesetDialog.h"

ChangesetDialog::ChangesetDialog()
{
	assert(false);
}

ChangesetDialog::ChangesetDialog(const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::ChangesetDialog),
	m_fileHistory(fileHistory),
	m_changeset(-1)
{
	ui->setupUi(this);

	// Set changesetList
	//
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
		const DbChangesetInfo& ci = fileHistory[i];

		QStringList itemTextList;
		itemTextList << QString::number(ci.changeset());
		itemTextList << ci.user().username();
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

ChangesetDialog::~ChangesetDialog()
{
	delete ui;
}

int ChangesetDialog::changeset() const
{
	return m_changeset;
}

int ChangesetDialog::getChangeset(const std::vector<DbChangesetInfo>& fileHistory, QWidget* parent)
{
	ChangesetDialog cd(fileHistory, parent);
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

void ChangesetDialog::on_buttonBox_accepted()
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

void ChangesetDialog::on_changesetList_doubleClicked(const QModelIndex& /*index*/)
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
