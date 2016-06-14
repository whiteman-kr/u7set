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

	int i = 0;
	for (const QString& s : columnsNames)
	{
		QListWidgetItem* item = new QListWidgetItem(s);
		item->setData(Qt::UserRole, i);

		if (std::find(columnsIndexes.begin(), columnsIndexes.end(), i) != columnsIndexes.end())
		{
			ui->listSelectedColumns->addItem(item);
		}
		else
		{
			ui->listExistingColumns->addItem(item);
		}

		i++;
	}

}

DialogColumns::~DialogColumns()
{
	delete ui;
}

std::vector<int> DialogColumns::columnIndexes()
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
