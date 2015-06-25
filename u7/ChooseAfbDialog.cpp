#include "Stable.h"
#include "ChooseAfbDialog.h"
#include "ui_ChooseAfbDialog.h"

int ChooseAfbDialog::m_lastSelectedIndex = -1;
Qt::SortOrder ChooseAfbDialog::m_lastSortOrder = Qt::SortOrder::AscendingOrder;

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
	columns << "Caption";
	ui->m_afbTree->setHeaderLabels(columns);

	ui->m_afbTree->setColumnWidth(0, 150);

	// add items
	//
	int index = 0;

	for (std::shared_ptr<Afbl::AfbElement>& e : this->elements)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, e->caption());

		item->setData(0, Qt::UserRole, index);
		index++;

		ui->m_afbTree->addTopLevelItem(item);
	}

	// select the last item
	//
	if (m_lastSelectedIndex != -1 && m_lastSelectedIndex < ui->m_afbTree->topLevelItemCount())
	{
		ui->m_afbTree->setCurrentItem(ui->m_afbTree->topLevelItem(m_lastSelectedIndex));
	}
	else
	{
		if (ui->m_afbTree->topLevelItemCount() > 0)
		{
			ui->m_afbTree->setCurrentItem(ui->m_afbTree->topLevelItem(0));
			m_lastSelectedIndex = 0;
		}
	}

	ui->m_afbTree->sortItems(0, m_lastSortOrder);
	ui->m_afbTree->setSortingEnabled(true);

}

ChooseAfbDialog::~ChooseAfbDialog()
{
	delete ui;
}

int ChooseAfbDialog::index()
{
	return m_lastSelectedIndex;
}

int ChooseAfbDialog::getSelectedIndex()
{
	QList <QTreeWidgetItem*> selectedItems = ui->m_afbTree->selectedItems();
	if (selectedItems.size() == 0 || selectedItems.size() > 1)
	{
		return -1;
	}

	int result = selectedItems[0]->data(0, Qt::UserRole).toInt();
	return result;

}

void ChooseAfbDialog::on_btnOk_clicked()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	m_lastSelectedIndex = getSelectedIndex();

	if (m_lastSelectedIndex == -1)
	{
		QMessageBox::warning(this, "Warning", "No AFB element is chosen!");
		return;
	}

	if (m_lastSelectedIndex < 0 || m_lastSelectedIndex >= elements.size())
	{
		assert(false);
		return;
	}

	accept();
}

void ChooseAfbDialog::reject()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	done(QDialog::Rejected);
}

void ChooseAfbDialog::closeEvent(QCloseEvent* e)
{
	Q_UNUSED(e);

	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();
}

void ChooseAfbDialog::on_btnCancel_clicked()
{
	m_lastSortOrder = ui->m_afbTree->header()->sortIndicatorOrder();

	done(QDialog::Rejected);
}

void ChooseAfbDialog::on_m_afbTree_itemSelectionChanged()
{
	int selectedIndex = getSelectedIndex();

	if (selectedIndex == -1)
	{
		ui->labelCaption->setText("");
		ui->labelDescription->setPlainText("");
		ui->labelInputCount->setText("");
		ui->labelOutputCount->setText("");

		ui->btnOk->setEnabled(false);
	}
	else
	{
		if (selectedIndex < 0 || selectedIndex >= elements.size())
		{
			assert(false);
			return;
		}

		std::shared_ptr<Afbl::AfbElement> e = elements[selectedIndex];

		ui->labelCaption->setText(e->caption());
		ui->labelDescription->setPlainText(e->description());
		ui->labelInputCount->setText(QString::number(e->inputSignals().size()));
		ui->labelOutputCount->setText(QString::number(e->outputSignals().size()));

		ui->btnOk->setEnabled(true);
	}
}

void ChooseAfbDialog::on_m_afbTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	on_btnOk_clicked();
}
