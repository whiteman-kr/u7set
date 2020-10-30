#include "CopyData.h"

#include <QClipboard>

// -------------------------------------------------------------------------------------------------------------------

CopyData::CopyData(QTableView *pView, bool copyHiddenColumn) :
	QObject(pView),
	m_pView(pView),
	m_copyHiddenColumn(copyHiddenColumn)
{
}

// -------------------------------------------------------------------------------------------------------------------

CopyData::~CopyData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::exec()
{
	if (m_pView == nullptr)
	{
		return;
	}

	copyToMemory();
}

// -------------------------------------------------------------------------------------------------------------------

bool CopyData::copyToMemory()
{
	if (m_pView == nullptr)
	{
		return false;
	}

	QString textClipboard;

	m_copyCancel = false;

	int columnCount = m_pView->model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_copyHiddenColumn == false)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}
		}

		textClipboard.append(m_pView->model()->headerData(column, Qt::Horizontal).toString());
		textClipboard.append("\t");
	}

	textClipboard.append("\n");

	int rowCount = m_pView->model()->rowCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_copyCancel == true)
		{
			break;
		}

		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_copyHiddenColumn == false)
			{
				if (m_pView->isColumnHidden(column) == true)
				{
					continue;
				}
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString());
			textClipboard.append("\t");
		}

		textClipboard.append("\n");
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::copyCancel()
{
	m_copyCancel = true;
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::copyComplited()
{
	qDebug() << "Copy is complited!";
}

// -------------------------------------------------------------------------------------------------------------------


