#include "ChooseUfbDialog.h"
#include "ui_ChooseUfbDialog.h"

ChooseUfbDialog::ChooseUfbDialog(QWidget *parent, std::vector<std::shared_ptr<VFrame30::UfbSchema> > *ufbs) :
    QDialog(parent),
    ui(new Ui::ChooseUfbDialog)
{
	ui->setupUi(this);

	m_ufbs = *ufbs;

	ui->ufbElements->setHeaderLabel(tr("Caption"));
	ui->ufbElements->setSortingEnabled(true);

	ui->caption->setReadOnly(true);
	ui->description->setReadOnly(true);

	ui->ok->setEnabled(false);

	connect (ui->quickSearch, &QLineEdit::textChanged, this, &ChooseUfbDialog::fillTree);
	connect (ui->ufbElements, &QTreeWidget::itemClicked, this, &ChooseUfbDialog::itemSelected);

	connect (ui->ok, &QPushButton::clicked, this, &ChooseUfbDialog::accept);
	connect (ui->cancel, &QPushButton::clicked, this, &ChooseUfbDialog::reject);

	fillTree();
}

ChooseUfbDialog::~ChooseUfbDialog()
{
	delete ui;
}

void ChooseUfbDialog::fillTree()
{
	ui->ok->setEnabled(false);
	ui->caption->clear();
	ui->description->clear();

	QString searchMask = ui->quickSearch->text();

	QList<QTreeWidgetItem *> items;

	ui->ufbElements->clear();
	QTreeWidgetItem *allSection = new QTreeWidgetItem(QStringList(QString("All")));
	ui->ufbElements->insertTopLevelItem(0, allSection);

	if (searchMask.isEmpty())
	{
		for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
		{
			items.append(new QTreeWidgetItem(allSection, QStringList(ufb.get()->caption())));
		}
	}
	else
	{
		for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
		{
			if (ufb.get()->caption().contains(searchMask, Qt::CaseSensitive))
			{
				items.append(new QTreeWidgetItem(allSection, QStringList(ufb.get()->caption())));
			}
		}

		ui->ufbElements->expandAll();
	}
}

void ChooseUfbDialog::itemSelected(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	int index = 0;

	for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
	{
		if (ufb.get()->caption() == item->text(0))
		{
			selectedUfb = index;

			ui->caption->setText(ufb.get()->caption());
			ui->description->setPlainText(ufb.get()->description());
			ui->ok->setEnabled(true);
		}
		index++;
	}
}
