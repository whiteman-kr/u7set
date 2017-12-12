#include "Stable.h"
#include "ApplicationTabPage.h"
#include "../lib/OutputLog.h"

ApplicationTabPage::ApplicationTabPage(QWidget *parent)
	: QWidget(parent)
{

	QVBoxLayout* pLeftLayout = new QVBoxLayout();

	//
	pLeftLayout->addWidget(new QLabel(tr("Bitstream File Name:")));

	QHBoxLayout* bl = new QHBoxLayout();

	m_pFileNameEdit = new QLineEdit();
	m_pFileNameEdit->setReadOnly(true);

	QPushButton* b = new QPushButton(tr("Browse..."));
	bl->addWidget(m_pFileNameEdit);
	bl->addWidget(b);

	connect(b, &QPushButton::clicked, this, &ApplicationTabPage::openFileClicked);

	pLeftLayout->addLayout(bl);

	// Create Subsystems list widget

	pLeftLayout->addWidget(new QLabel(tr("Choose the Subsystem:")));
	m_pSubsystemsListWidget = new QTreeWidget();
	m_pSubsystemsListWidget->setRootIsDecorated(false);
	connect(m_pSubsystemsListWidget, &QTreeWidget::currentItemChanged, this, &ApplicationTabPage::subsystemChanged);
	pLeftLayout->addWidget(m_pSubsystemsListWidget, 2);

	QStringList l;
	l << tr("Subsystem");

	m_pSubsystemsListWidget->setColumnCount(l.size());
	m_pSubsystemsListWidget->setHeaderLabels(l);

	int il = 0;
	m_pSubsystemsListWidget->setColumnWidth(il++, 100);

	// Create UART list widget

	pLeftLayout->addWidget(new QLabel(tr("Firmware Types:")));
	m_pUartListWidget = new QTreeWidget();
	m_pUartListWidget->setRootIsDecorated(false);
	pLeftLayout->addWidget(m_pUartListWidget);

	l.clear();
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");
	l << tr("Status");

	m_pUartListWidget->setColumnCount(l.size());
	m_pUartListWidget->setHeaderLabels(l);

	il = 0;
	m_pUartListWidget->setColumnWidth(il++, 80);
	m_pUartListWidget->setColumnWidth(il++, 100);
	m_pUartListWidget->setColumnWidth(il++, 100);
	m_pUartListWidget->setColumnWidth(il++, 60);

	//
	bl = new QHBoxLayout();
	bl->addStretch();

	b = new QPushButton(tr("Reset Upload Counters"));
	bl->addWidget(b);
	connect(b, &QPushButton::clicked, this, &ApplicationTabPage::on_resetCountersButton_clicked);

	pLeftLayout->addLayout(bl);

	pLeftLayout->addStretch();


	//
	setLayout(pLeftLayout);


}

ApplicationTabPage::~ApplicationTabPage()
{

}

bool ApplicationTabPage::isFileLoaded() const
{
	return m_firmware.isEmpty() == false;
}

ModuleFirmwareStorage *ApplicationTabPage::configuration()
{
	return &m_firmware;
}

QString ApplicationTabPage::selectedSubsystem()
{
	for (QTreeWidgetItem* item : m_pSubsystemsListWidget->selectedItems())
	{
		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid())
		{
			return vData.toString();
		}
	}

	return QString();
}

void ApplicationTabPage::selectSubsystem(const QString& id)
{
	m_pSubsystemsListWidget->clearSelection();

	int count = m_pSubsystemsListWidget->topLevelItemCount();

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pSubsystemsListWidget->topLevelItem(i);

		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid() && vData.toString() == id)
		{
			m_pSubsystemsListWidget->setCurrentItem(item);
			return;
		}
	}
}


void ApplicationTabPage::subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2)
{
	Q_UNUSED(item1);
	Q_UNUSED(item2);


	QTreeWidgetItem* subsystemItem = m_pSubsystemsListWidget->currentItem();
	if (subsystemItem == nullptr)
	{
		return;
	}

	QString subsystemId = subsystemItem->data(columnSubsysId, Qt::UserRole).toString();

	bool ok = false;
	const ModuleFirmware& mf = m_firmware.firmware(subsystemId, &ok);
	if (ok == false)
	{
		assert(false);
		return;
	}

	const std::vector<UartPair>& uartList = mf.uartList();

	m_pUartListWidget->clear();

	for (auto it : uartList)
	{
		int uartID = it.first;
		QString uartType = it.second;

		QStringList l;
		l << tr("%1h").arg(QString::number(uartID, 16));
		l << uartType;
		l << "0";

		QTreeWidgetItem* uartItem = new QTreeWidgetItem(l);
		uartItem->setData(columnUartId, Qt::UserRole, uartID);
		uartItem->setData(columnUploadCount, Qt::UserRole, 0);

		m_pUartListWidget->addTopLevelItem(uartItem);
	}
}

void ApplicationTabPage::openFileClicked()
{
	QFileDialog fd(this);

	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setFileMode(QFileDialog::ExistingFile);

	QStringList filters;
	filters << "Bitstream files (*.bts)"
			<< "All files (*.*)";

	fd.setNameFilters(filters);

	if (fd.exec() == QDialog::Rejected)
	{
		return;
	}

	QStringList fileList = fd.selectedFiles();

	if (fileList.size() != 1)
	{
		return;
	}

	QString fileName = fileList[0];

	if (QFile::exists(fileName) == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("File %1 doesn't exists.").arg(fileName));
		return;
	}

	m_pFileNameEdit->setText(fileName);

	clearSubsystemsUartData();

	emit loadBinaryFile(fileName, &m_firmware);

	return;
}

void ApplicationTabPage::loadBinaryFileHeaderComplete()
{
	clearSubsystemsUartData();

	QStringList subsystemsList = m_firmware.subsystems();

	for (const QString& subsystemId : subsystemsList)
	{
		QTreeWidgetItem* subsystemItem = new QTreeWidgetItem(QStringList() << subsystemId);
		subsystemItem->setData(columnSubsysId, Qt::UserRole, subsystemId);

		m_pSubsystemsListWidget->sortByColumn(columnUartId, Qt::AscendingOrder);
		m_pSubsystemsListWidget->addTopLevelItem(subsystemItem);
	}

	// User must think
	//
	//if (m_pSubsystemsListWidget->topLevelItemCount() > 0)
	//{
		//m_pSubsystemsListWidget->setCurrentItem(m_pSubsystemsListWidget->topLevelItem(0));
	//}
}

void ApplicationTabPage::on_resetCountersButton_clicked()
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setData(columnUploadCount, Qt::UserRole, 0);
		item->setText(columnUploadCount, "0");
	}
}

void ApplicationTabPage::uploadStart(int uartID)
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt();

		if (uartID == itemUartId)
		{
			item->setText(columnUartStatus, tr("Uploading"));
			return;
		}
	}
	return;
}

void ApplicationTabPage::uploadComplete(int uartID)
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt();

		if (uartID == itemUartId)
		{
			int itemUploadCount = item->data(columnUploadCount, Qt::UserRole).toInt();
			itemUploadCount++;

			item->setData(columnUploadCount, Qt::UserRole, itemUploadCount);
			item->setText(columnUploadCount, QString::number(itemUploadCount));

			return;
		}
	}
	return;
}

void ApplicationTabPage::detectSubsystemComplete(int subsystemId)
{
	theLog.writeMessage(tr("Subsystem Key is %1").arg(subsystemId));

	bool subsystemFound = false;

	QStringList subsystems = m_firmware.subsystems();
	for (auto s : subsystems)
	{
		bool ok = false;
		const ModuleFirmware& fw = m_firmware.firmware(s, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		if (subsystemId == fw.ssKey())
		{
			// Select the subsystem
			//
			theLog.writeMessage(tr("Subsystem ID is %1").arg(fw.subsysId()));
			selectSubsystem(fw.subsysId());
			subsystemFound = true;
			break;
		}
	}

	if (subsystemFound == false)
	{
		m_pSubsystemsListWidget->clearSelection();
		theLog.writeMessage(tr("Subsystem ID is unknown"));
	}

	theLog.writeSuccess(tr("Successful."));
}

void ApplicationTabPage::enableControls()
{
	// Clear status of all Uarts
	//
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}
		item->setText(columnUartStatus, QString());
	}
}

void ApplicationTabPage::clearSubsystemsUartData()
{
	m_pSubsystemsListWidget->clear();
	m_pUartListWidget->clear();
}

