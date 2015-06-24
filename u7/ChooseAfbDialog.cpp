#include "Stable.h"
#include "ChooseAfbDialog.h"
#include "ui_ChooseAfbDialog.h"

int ChooseAfbDialog::m_lastSelectedIndex = -1;

ChooseAfbDialog::ChooseAfbDialog(const std::vector<std::shared_ptr<Afbl::AfbElement> >& elements, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ChooseAfbDialog)
{
	ui->setupUi(this);

	for (auto& e : elements)
	{
		this->elements.push_back(e);
	}

	QStringList columns;
	columns << "StrID";
	columns << "Caption";
	columns << "OpCode";
	ui->m_afbTree->setHeaderLabels(columns);

	ui->m_afbTree->setColumnWidth(0, 150);
	ui->m_afbTree->setColumnWidth(1, 150);
	ui->m_afbTree->setColumnWidth(2, 70);

	for (std::shared_ptr<Afbl::AfbElement>& e : this->elements)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, e->strID());
		item->setText(1, e->caption());
		item->setText(2, QString::number(e->opcode()));

		ui->m_afbTree->addTopLevelItem(item);
	}

	if (m_lastSelectedIndex != -1 && m_lastSelectedIndex < ui->m_afbTree->topLevelItemCount())
	{
		ui->m_afbTree->setCurrentItem(ui->m_afbTree->topLevelItem(m_lastSelectedIndex));
	}

}

ChooseAfbDialog::~ChooseAfbDialog()
{
	delete ui;
}


int ChooseAfbDialog::index()
{
	return m_index;
}

void ChooseAfbDialog::on_btnOk_clicked()
{
	m_index = -1;

	QModelIndexList selectedIndexList = ui->m_afbTree->selectionModel()->selectedIndexes();

	for (const QModelIndex& i : selectedIndexList)
	{
		m_index = i.row();
		break;	//only one item must be selected
	}

	if (m_index == -1)
	{
		QMessageBox::warning(this, "Warning", "No AFB element is chosen!");
		return;
	}

	m_lastSelectedIndex = m_index;

	accept();
}

void ChooseAfbDialog::on_btnCancel_clicked()
{
	reject();
}
