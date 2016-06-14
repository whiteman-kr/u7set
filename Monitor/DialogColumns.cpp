#include "DialogColumns.h"
#include "ui_DialogColumns.h"
#include <QMessageBox>


DialogColumns::DialogColumns(QWidget *parent, const QStringList& columnsNames, const std::vector<int>& columnsIndexes) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_columnsNames(columnsNames),
	m_columnsIndexes(columnsIndexes),
	ui(new Ui::DialogColumns)
{
	ui->setupUi(this);

	for (int i : columnsIndexes)
	{
		QListWidgetItem* item = new QListWidgetItem(columnsNames.at(i));
		item->setData(Qt::UserRole, i);

		ui->listSelectedColumns->addItem(item);

	}

	int i = 0;
	for (const QString& s : columnsNames)
	{

		if (std::find(columnsIndexes.begin(), columnsIndexes.end(), i) == columnsIndexes.end())
		{
			QListWidgetItem* item = new QListWidgetItem(s);
			item->setData(Qt::UserRole, i);
			ui->listExistingColumns->addItem(item);
		}

		i++;
	}

	ui->listExistingColumns->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->listSelectedColumns->setSelectionMode(QAbstractItemView::SingleSelection);

}

DialogColumns::~DialogColumns()
{
	delete ui;
}

std::vector<int> DialogColumns::columnsIndexes()
{
	return m_columnsIndexes;
}

void DialogColumns::accept()
{
	if (ui->listSelectedColumns->count() == 0)
	{
		QMessageBox::warning(this, tr("Columns"), tr("Please select at least one column!"));
		return;
	}

	m_columnsIndexes.clear();

	for (int i = 0; i < ui->listSelectedColumns->count(); i++)
	{
		QListWidgetItem* item = ui->listSelectedColumns->item(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int index = item->data(Qt::UserRole).toInt();

		if (index < 0 || index >= m_columnsNames.size())
		{
			assert(false);
			return;
		}

		m_columnsIndexes.push_back(index);
	}

	QDialog::accept();
}

void DialogColumns::on_buttonAdd_clicked()
{
	int row = ui->listExistingColumns->currentRow();
	if (row == -1)
	{
		return;
	}

	QListWidgetItem* item = ui->listExistingColumns->takeItem(row);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	ui->listSelectedColumns->addItem(item);

}

void DialogColumns::on_buttonAddAll_clicked()
{
	while (ui->listExistingColumns->count() > 0)
	{
		QListWidgetItem* item = ui->listExistingColumns->takeItem(0);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		ui->listSelectedColumns->addItem(item);
	}
}

void DialogColumns::on_buttonRemove_clicked()
{
	int row = ui->listSelectedColumns->currentRow();
	if (row == -1)
	{
		return;
	}

	QListWidgetItem* item = ui->listSelectedColumns->takeItem(row);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	ui->listExistingColumns->addItem(item);
}

void DialogColumns::on_buttonRemoveAll_clicked()
{
	ui->listSelectedColumns->clear();
	ui->listExistingColumns->clear();

	int i = 0;
	for (const QString& s : m_columnsNames)
	{
		QListWidgetItem* item = new QListWidgetItem(s);
		item->setData(Qt::UserRole, i);

		ui->listExistingColumns->addItem(item);

		i++;
	}
}

void DialogColumns::on_buttonUp_clicked()
{
	int row = ui->listSelectedColumns->currentRow();
	if (row < 1)
	{
		return;
	}

	QListWidgetItem * item = ui->listSelectedColumns->takeItem(row);
	ui->listSelectedColumns->insertItem(row - 1, item);
	ui->listSelectedColumns->setCurrentRow(row - 1);
}

void DialogColumns::on_buttonDown_clicked()
{
	int row = ui->listSelectedColumns->currentRow();
	if (row >= ui->listSelectedColumns->count() - 1)
	{
		return;
	}

	QListWidgetItem * item = ui->listSelectedColumns->takeItem(row);
	ui->listSelectedColumns->insertItem(row + 1, item);
	ui->listSelectedColumns->setCurrentRow(row + 1);
}

void DialogColumns::on_listExistingColumns_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_buttonAdd_clicked();
}

void DialogColumns::on_listSelectedColumns_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_buttonRemove_clicked();

}
