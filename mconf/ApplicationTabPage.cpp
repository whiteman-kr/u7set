#include "Stable.h"
#include "ApplicationTabPage.h"
#include "../lib/OutputLog.h"

ApplicationTabPage::ApplicationTabPage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.openButton, &QAbstractButton::clicked, this, &ApplicationTabPage::openFileClicked);
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
    filters << "Firmwares (*.mcb *.alb *.tub)"
            << "Module configuration files (*.mcb)"
			<< "Application logic files (*.alb)"
            << "Tuning parameters files (*.tub)"
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

	QString errorCode;

	bool result = m_confFirmware.load(fileName, errorCode);

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
    ui.UartIdEdit->setText(QString::number(m_confFirmware.uartId(), 16));
	
    theLog.writeMessage(tr("File %1 was loaded.").arg(fileName));

	theLog.writeMessage(tr("File Version: %1").arg(m_confFirmware.fileVersion()));
	theLog.writeMessage(tr("SubsysID: %1").arg(m_confFirmware.subsysId()));
	theLog.writeMessage(tr("ChangesetID: %1").arg(m_confFirmware.changesetId()));
	theLog.writeMessage(tr("Build User: %1").arg(m_confFirmware.userName()));
	theLog.writeMessage(tr("Build No: %1").arg(QString::number(m_confFirmware.buildNumber())));
	theLog.writeMessage(tr("Build Config: %1").arg(m_confFirmware.buildConfig()));
	theLog.writeMessage(tr("LM Description Number: %1").arg(m_confFirmware.lmDescriptionNumber()));
	theLog.writeMessage(tr("UartID: %1h").arg(QString::number(m_confFirmware.uartId(), 16)));
	theLog.writeMessage(tr("FrameSize: %1").arg(QString::number(m_confFirmware.frameSize())));
	theLog.writeMessage(tr("FrameSize with CRC: %1").arg(QString::number(m_confFirmware.frameSizeWithCRC())));
	theLog.writeMessage(tr("FrameCount: %1").arg(QString::number(m_confFirmware.frameCount())));

	return;
}
