#include "Stable.h"
#include "ApplicationTabPage.h"
#include "../lib/OutputLog.h"

ApplicationTabPage::ApplicationTabPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.openButton, &QAbstractButton::clicked, this, &ApplicationTabPage::openFileClicked);


	QStringList l;
	l << tr("Subsystem");
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");

	ui.firmwareListWidget->setColumnCount(l.size());
	ui.firmwareListWidget->setHeaderLabels(l);

	int il = 0;
	ui.firmwareListWidget->setColumnWidth(il++, 140);
	ui.firmwareListWidget->setColumnWidth(il++, 80);
	ui.firmwareListWidget->setColumnWidth(il++, 140);
	ui.firmwareListWidget->setColumnWidth(il++, 140);

	ui.firmwareListWidget->setSelectionMode(QTreeWidget::SingleSelection);

}

ApplicationTabPage::~ApplicationTabPage()
{

}

bool ApplicationTabPage::isFileLoaded() const
{
	return m_confFirmware.isEmpty() == false;
}

ModuleFirmwareStorage *ApplicationTabPage::configuration()
{
    return &m_confFirmware;
}

QString ApplicationTabPage::subsystemId()
{
	for (QTreeWidgetItem* item : ui.firmwareListWidget->selectedItems())
	{
		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid())
		{
			return vData.toString();
		}
	}

	return QString();
}

void ApplicationTabPage::openFileClicked()
{
	QFileDialog fd(this);

	//fd.setOption(QFileDialog::Option::DontUseNativeDialog);

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

	clearUartData();

	QString errorCode;
	bool result = m_confFirmware.load(fileName, &errorCode);

	if (result == false)
	{
		QMessageBox mb(this);
		QString str = tr("File %1 wasn't loaded!").arg(fileName);
		if (errorCode.isEmpty() == false)
		{
			str += "\r\n\r\n" + errorCode;
		}
		mb.setText(str);
		mb.setIcon(QMessageBox::Critical);
		mb.exec();
		return;
	}

	ui.fileNameEdit->setText(fileName);

	theLog.writeMessage(tr("File %1 was loaded.").arg(fileName));

	theLog.writeMessage(tr("File Version: %1").arg(m_confFirmware.fileVersion()));
	theLog.writeMessage(tr("ChangesetID: %1").arg(m_confFirmware.changesetId()));
	theLog.writeMessage(tr("Build User: %1").arg(m_confFirmware.userName()));
	theLog.writeMessage(tr("Build No: %1").arg(QString::number(m_confFirmware.buildNumber())));
	theLog.writeMessage(tr("Build Config: %1").arg(m_confFirmware.buildConfig()));
	theLog.writeMessage(tr("Subsystems: %1").arg(m_confFirmware.subsystemsString()));


	fillUartData();

	return;
}

void ApplicationTabPage::on_resetCountersButton_clicked()
{

	int count = ui.firmwareListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = ui.firmwareListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int childCount = item->childCount();
		for (int c = 0; c < childCount; c++)
		{
			QTreeWidgetItem* childItem = item->child(c);

			childItem->setData(columnUploadCount, Qt::UserRole, 0);
			childItem->setText(columnUploadCount, "0");
		}
	}
}

void ApplicationTabPage::uploadSuccessful(int uartID)
{
	QString subsystem = subsystemId();

	int count = ui.firmwareListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = ui.firmwareListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int childCount = item->childCount();

		for (int c = 0; c < childCount; c++)
		{
			QTreeWidgetItem* childItem = item->child(c);

			QString itemSubsystemId = childItem->data(columnSubsysId, Qt::UserRole).toString();
			int itemUartId = childItem->data(columnUartId, Qt::UserRole).toInt();

			if (subsystem == itemSubsystemId && uartID == itemUartId)
			{
				int itemUploadCount = childItem->data(columnUploadCount, Qt::UserRole).toInt();
				itemUploadCount++;

				childItem->setData(columnUploadCount, Qt::UserRole, itemUploadCount);
				childItem->setText(columnUploadCount, QString::number(itemUploadCount));

				return;
			}
		}
	}

	return;
}

void ApplicationTabPage::clearUartData()
{
	ui.firmwareListWidget->clear();
}

void ApplicationTabPage::fillUartData()
{
	clearUartData();

	QStringList subsystems = m_confFirmware.subsystems();

	for (const QString& subsystemId : subsystems)
	{
		QTreeWidgetItem* subsystemItem = new QTreeWidgetItem(QStringList() << subsystemId);
		subsystemItem->setData(columnSubsysId, Qt::UserRole, subsystemId);

		bool ok = false;
		ModuleFirmware& fw = m_confFirmware.firmware(subsystemId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		std::vector<UartPair> uartList = fw.uartList();

		for (auto it : uartList)
		{
			int uartID = it.first;
			QString uartType = it.second;

			QStringList l;
			l << QString();
			l << tr("%1h").arg(QString::number(uartID, 16));
			l << uartType;
			l << "0";

			QTreeWidgetItem* uartItem = new QTreeWidgetItem(l);
			uartItem->setData(columnSubsysId, Qt::UserRole, subsystemId);
			uartItem->setData(columnUartId, Qt::UserRole, uartID);
			uartItem->setData(columnUploadCount, Qt::UserRole, 0);

			subsystemItem->addChild(uartItem);
		}
		ui.firmwareListWidget->sortByColumn(columnUartId, Qt::AscendingOrder);

		ui.firmwareListWidget->addTopLevelItem(subsystemItem);
		subsystemItem->setExpanded(true);
	}
}



