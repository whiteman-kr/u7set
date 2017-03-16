#include "FindData.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

QString FindData::m_findText = QString();

// -------------------------------------------------------------------------------------------------------------------

FindData::FindData(QTableView *pView) :
	QObject(pView),
	m_pView(pView)
{
	createInterface(pView);
}

// -------------------------------------------------------------------------------------------------------------------

FindData::~FindData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::createInterface(QTableView *pView)
{
	if (pView == nullptr)
	{
		return;
	}

	m_pFindDialog = new QDialog(pView->parentWidget());

	m_pFindDialog->setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	m_pFindDialog->setWindowTitle(tr("Find"));
	m_pFindDialog->setWindowIcon(QIcon(":/icons/Find.png"));

		m_pFindTextEdit = new QLineEdit(m_findText, m_pFindDialog);
		m_findNextButton = new QPushButton(tr("Find Next"), m_pFindDialog);

		QHBoxLayout *mainLayout = new QHBoxLayout ;

		mainLayout->addWidget(m_pFindTextEdit);
		mainLayout->addWidget(m_findNextButton);

	m_pFindDialog->setLayout(mainLayout);

	connect(m_pFindTextEdit, &QLineEdit::textChanged, this, &FindData::findTextChanged);
	connect(m_findNextButton, &QPushButton::clicked, this, &FindData::findNext);

	emit findTextChanged();
}

// -------------------------------------------------------------------------------------------------------------------

int FindData::exec()
{
	if (m_pFindDialog == nullptr)
	{
		return QDialog::Rejected;
	}

	return m_pFindDialog->exec();
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::findTextChanged()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->clearSelection();

	m_findText = m_pFindTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		return;
	}

	int foundRow = find(-1);
	if (foundRow != -1)
	{
		m_pView->setCurrentIndex(m_pView->model()->index(foundRow, 0));
	}

	enableFindNextButton(foundRow);
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::findNext()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int startRow = m_pView->currentIndex().row();
	if (startRow == 0)
	{
		startRow = -1;
	}

	int foundRow = find(startRow);
	if (foundRow != -1)
	{
		m_pView->setCurrentIndex(m_pView->model()->index(foundRow, 0));
	}

	enableFindNextButton(foundRow);
}

// -------------------------------------------------------------------------------------------------------------------

int FindData::find(int start)
{
	if (m_pView == nullptr)
	{
		return - 1;
	}

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	if (rowCount == 0 || columnCount == 0)
	{
		return -1;
	}

	m_findText = m_pFindTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		return -1;
	}

	int foundRow = -1;

	for(int row = start + 1; row < rowCount; row++)
	{
		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			QString text = m_pView->model()->data(m_pView->model()->index(row, column)).toString();

			int pos = text.indexOf(m_findText);
			if (pos == -1)
			{
				continue;
			}

			foundRow = row;

			break;
		}

		if (foundRow != -1)
		{
			break;
		}
	}

	return foundRow;
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::enableFindNextButton(int foundRow)
{
	if (foundRow == -1)
	{
		m_findNextButton->setEnabled(false);
		return;
	}

	int foundNextRow = find(foundRow);
	if (foundNextRow != -1)
	{
		m_findNextButton->setEnabled(true);
	}
	else
	{
		m_findNextButton->setEnabled(false);
	}
}

// -------------------------------------------------------------------------------------------------------------------
