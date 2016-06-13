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

    bool result = m_confFirmware.load(fileName);

	if (result == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("File %1 wasn't loaded").arg(fileName));
		mb.setIcon(QMessageBox::Critical);
		mb.exec();
		return;
	}

	ui.fileNameEdit->setText(fileName);
    ui.UartIdEdit->setText(QString::number(m_confFirmware.uartId(), 16));
	
    theLog.writeMessage(tr("File %1 was loaded.").arg(fileName));

    theLog.writeMessage(tr("SubsysID: %1").arg(m_confFirmware.subsysId()));
    //theLog.writeMessage(tr("Changeset: %1").arg(m_confFirmware.changeset()));
    theLog.writeMessage(tr("UartID: %1h").arg(QString::number(m_confFirmware.uartId(), 16)));
    theLog.writeMessage(tr("MinimumFrameSize: %1").arg(QString::number(m_confFirmware.frameSize())));
    theLog.writeMessage(tr("FrameCount: %1").arg(QString::number(m_confFirmware.frameCount())));

	return;
}
