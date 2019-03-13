#include "DialogSourceInfo.h"

QColor DialogSourceInfo::dataItemErrorColor = QColor(0xc0, 0, 0);

DialogSourceInfo::DialogSourceInfo(QWidget *parent, Hash sourceHash)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	  m_sourceHash(sourceHash)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_updateStateTimerId = startTimer(250);
}

DialogSourceInfo::~DialogSourceInfo()
{
}


void DialogSourceInfo::createDataItem(QTreeWidgetItem* parentItem, const QString& caption)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << caption);
	parentItem->addChild(item);
	m_treeWidgetItemsMap[caption] = item;
}

QTreeWidgetItem* DialogSourceInfo::dataItem(const QString& caption)
{
	auto it = m_treeWidgetItemsMap.find(caption);
	if (it == m_treeWidgetItemsMap.end())
	{
		assert(false);
		return nullptr;
	}

	QTreeWidgetItem* item = it->second;
	if (item == nullptr)
	{
		assert(item);
	}

	return item;
}

void DialogSourceInfo::setDataItemNumberCompare(QTreeWidgetItem* parentItem, const QString& caption, quint64 number, quint64 previousNumber)
{
	setDataItemText(caption, QString::number(number));

	// Highlight increasing number
	//

	QTreeWidgetItem* item = dataItem(caption);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	if (number > previousNumber)
	{
		bool ok = false;
		int errorCount = parentItem->data(0, Qt::UserRole).toInt(&ok);

		if (ok == true)
		{
			errorCount++;
			parentItem->setData(0, Qt::UserRole, errorCount);
		}
		else
		{
			assert(ok);
		}

		item->setForeground(1, QBrush(dataItemErrorColor));
	}
	else
	{
		item->setForeground(1, QBrush(Qt::black));
	}
}

void DialogSourceInfo::setDataItemNumber(const QString& caption, quint64 number)
{
	setDataItemText(caption, QString::number(number));
}

void DialogSourceInfo::setDataItemText(const QString& caption, const QString& text)
{
	QTreeWidgetItem* item = dataItem(caption);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	item->setText(1, text);
}

void DialogSourceInfo::updateParentItemState(QTreeWidgetItem* item)
{
	bool ok = false;
	int errorCount = item->data(0, Qt::UserRole).toInt(&ok);
	if (ok == false)
	{
		assert(ok);
		return;
	}

	if (errorCount > 0)
	{
		item->setForeground(1, QBrush(dataItemErrorColor));
		item->setText(1, tr("E: %1").arg(errorCount));
	}
	else
	{
		item->setForeground(1, QBrush(Qt::black));
		item->setText(1, QString());

	}
}

void DialogSourceInfo::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId)
	{
		updateData();
	}
}

void DialogSourceInfo::accept()
{
	emit dialogClosed(m_sourceHash);

	QDialog::accept();
}

void DialogSourceInfo::reject()
{
	emit dialogClosed(m_sourceHash);

	QDialog::reject();
}
