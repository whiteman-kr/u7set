#include "Stable.h"
#include "ApplicationTabPage.h"
#include "../lib/OutputLog.h"

ApplicationTabPage::ApplicationTabPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.openButton, &QAbstractButton::clicked, this, &ApplicationTabPage::openFileClicked);


	QStringList l;
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");

	ui.firmwareListWidget->setColumnCount(l.size());
	ui.firmwareListWidget->setHeaderLabels(l);

	int il = 0;
	ui.firmwareListWidget->setColumnWidth(il++, 80);
	ui.firmwareListWidget->setColumnWidth(il++, 140);
	ui.firmwareListWidget->setColumnWidth(il++, 140);

	ui.firmwareListWidget->setRootIsDecorated(false);

}

ApplicationTabPage::~ApplicationTabPage()
{

}

bool ApplicationTabPage::isFileLoaded() const
{
    return m_confFirmware.isEmpty() == false;
}

ModuleFirmware *ApplicationTabPage::configuration()
{
    return &m_confFirmware;
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
	theLog.writeMessage(tr("SubsysID: %1").arg(m_confFirmware.subsysId()));
	theLog.writeMessage(tr("ChangesetID: %1").arg(m_confFirmware.changesetId()));
	theLog.writeMessage(tr("Build User: %1").arg(m_confFirmware.userName()));
	theLog.writeMessage(tr("Build No: %1").arg(QString::number(m_confFirmware.buildNumber())));
	theLog.writeMessage(tr("Build Config: %1").arg(m_confFirmware.buildConfig()));
	theLog.writeMessage(tr("LM Description Number: %1").arg(m_confFirmware.lmDescriptionNumber()));

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

		item->setData(2, Qt::UserRole, 0);
		item->setText(2, "0");
	}
}

void ApplicationTabPage::uploadSuccessful(int uartID)
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

		int itemUartId = item->data(0, Qt::UserRole).toInt();
		if (uartID == itemUartId)
		{
			int itemUploadCount = item->data(2, Qt::UserRole).toInt();
			itemUploadCount++;

			item->setData(2, Qt::UserRole, itemUploadCount);
			item->setText(2, QString::number(itemUploadCount));

			break;
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
	std::vector<std::pair<int, QString>> uartList = m_confFirmware.uartList();

	clearUartData();

	for (auto it : uartList)
	{
		int uartID = it.first;
		QString uartType = it.second;

		QStringList l;
		l << tr("%1h").arg(QString::number(uartID, 16));
		l << uartType;
		l << "0";

		QTreeWidgetItem* item = new QTreeWidgetItem(l);

		item->setData(0, Qt::UserRole, uartID);
		item->setData(2, Qt::UserRole, 0);

		ui.firmwareListWidget->addTopLevelItem(item);
	}
	ui.firmwareListWidget->sortByColumn(0, Qt::AscendingOrder);
}



