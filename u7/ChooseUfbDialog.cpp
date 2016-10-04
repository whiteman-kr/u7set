#include "ChooseUfbDialog.h"
#include "ui_ChooseUfbDialog.h"

ChooseUfbDialog::ChooseUfbDialog(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::ChooseUfbDialog)
{
	ui->setupUi(this);

	m_ufbs = ufbs;

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

	QList<QTreeWidgetItem*> items;

	ui->ufbElements->clear();
	QTreeWidgetItem* allSection = new QTreeWidgetItem(QStringList(QString("All")));

	ui->ufbElements->insertTopLevelItem(0, allSection);

	if (searchMask.isEmpty() == true)
	{
		for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
		{
			items.append(new QTreeWidgetItem(allSection, QStringList(ufb->caption())));
		}
	}
	else
	{
		for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
		{
			bool found = ufb->caption().contains(searchMask, Qt::CaseInsensitive);

			if (found == true)
			{
				items.append(new QTreeWidgetItem(allSection, QStringList(ufb->caption())));
			}
		}

		ui->ufbElements->expandAll();
	}
}

void ChooseUfbDialog::itemSelected(QTreeWidgetItem* item, int column)
{
	Q_UNUSED(column);

	for (std::shared_ptr<VFrame30::UfbSchema> ufb : m_ufbs)
	{
		if (ufb->caption() == item->text(0))
		{
			m_selectedUfb = ufb;

			ui->caption->setText(ufb->caption());
			ui->description->setPlainText(ufb->description());
			ui->ok->setEnabled(true);

			break;
		}
	}

	return;
}

std::shared_ptr<VFrame30::UfbSchema> ChooseUfbDialog::result()
{
	return m_selectedUfb;
}
