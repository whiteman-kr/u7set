#include "ChangesetDetailsDialog.h"
#include "ui_ChangesetDetailsDialog.h"
#include "CompareDialog.h"

QByteArray ChangesetDetailsDialog::m_splitterState = QByteArray();

ChangesetDetailsDialog::ChangesetDetailsDialog(DbController* db, const DbChangesetDetails& changesetDetails, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ChangesetDetailsDialog),
	m_db(db),
	m_changesetDetails(changesetDetails)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

	if (m_splitterState.isEmpty() == false)
	{
		ui->splitter->restoreState(m_splitterState);
	}

	ui->changeset->setText(QString::number(m_changesetDetails.changeset()));
	ui->date->setText(m_changesetDetails.date().toString("dd MMM yyyy HH:mm:ss"));
	ui->user->setText(m_changesetDetails.username());
	ui->comment->document()->setPlainText(m_changesetDetails.comment());

	connect(ui->splitter, &QSplitter::splitterMoved,
			[this]()
			{
				m_splitterState = ui->splitter->saveState();
			});

	// Fill objects table
	//
	ui->objects->setColumnCount(4);

	QStringList columns;

	columns << tr("Type");
	columns << tr("Name");
	columns << tr("Caption");
	columns << tr("Action");
	columns << tr("Parent");

	ui->objects->setHeaderLabels(columns);

	// Fill list
	//
	const std::vector<DbChangesetObject>& objects = m_changesetDetails.objects();

	QList<QTreeWidgetItem*> items;
	items.reserve(static_cast<int>(objects.size()));

	int fileCount = 0;
	int signalCount = 0;

	for (size_t i = 0; i < objects.size(); i++)
	{
		const DbChangesetObject& co = objects[i];

		QStringList itemTextList;

		switch (co.type())
		{
		case DbChangesetObject::Type::File:
			fileCount ++;
			itemTextList << "F";
			break;
		case DbChangesetObject::Type::Signal:
			signalCount ++;
			itemTextList << "S";
			break;
		default:
			assert(false);
		}

		itemTextList << co.name();
		itemTextList << co.caption();
		itemTextList << co.action().text();
		itemTextList << co.parent();

		QTreeWidgetItem* item = new QTreeWidgetItem(itemTextList);
		item->setData(0, Qt::UserRole, i);

		items.push_back(item);
	}

	ui->objects->insertTopLevelItems(0, items);

	ui->labelObjects->setText(tr("Objects, %1 file(s), %2 signal(s)").arg(fileCount).arg(signalCount));

	return;
}

ChangesetDetailsDialog::~ChangesetDetailsDialog()
{
	m_splitterState = ui->splitter->saveState();
	delete ui;
}

void ChangesetDetailsDialog::showChangesetDetails(DbController* db, int changeset, QWidget* parent)
{
	if (db == nullptr)
	{
		assert(db);
		return;
	}

	// Get changeset details from the database
	//
	DbChangesetDetails changesetDetails;

	bool ok = db->getChangesetDetails(changeset, &changesetDetails, parent);
	if (ok == false)
	{
		return;
	}

	// Show dialog
	//
	ChangesetDetailsDialog* dialog = new ChangesetDetailsDialog(db, changesetDetails, parent);

	dialog->setWindowTitle(tr("Changeset #%1").arg(changeset));

	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();

	return;
}

void ChangesetDetailsDialog::on_objects_customContextMenuRequested(const QPoint& /*pos*/)
{
	QMenu menu(ui->objects);

	QTreeWidgetItem* item = ui->objects->currentItem();
	if (item == nullptr)
	{
		return;
	}

	bool ok = false;
	size_t selectionIndex = item->data(0, Qt::UserRole).toInt(&ok);

	if (ok == false)
	{
		return;
	}

	const std::vector<DbChangesetObject>& objects = m_changesetDetails.objects();

	if (selectionIndex < 0 || selectionIndex >= objects.size())
	{
		assert(false);
		return;
	}

	int changeset = m_changesetDetails.changeset();
	const DbChangesetObject& object = objects[selectionIndex];

	QAction* compareAction = new QAction(tr("Compare..."), &menu);
	connect(compareAction, &QAction::triggered, this,
			[this, object, changeset]()
			{
				ChangesetDetailsDialog::compare(object, changeset);
			});

	menu.addAction(compareAction);

	// Show menu
	//
	menu.exec(cursor().pos());

	return;
}

void ChangesetDetailsDialog::compare(DbChangesetObject object, int changeset)
{
	CompareDialog::showCompare(m_db, object, changeset,dynamic_cast<QWidget*>(this->parent()));
}
