#include "stable.h"
#include "applicationtabpage.h"
#include "log.h"

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
	return m_reader.isLoaded();
}

const ConfigDataReader& ApplicationTabPage::configuration() const
{
	return m_reader;
}

void ApplicationTabPage::openFileClicked()
{
	QFileDialog fd(this);

	//fd.setOption(QFileDialog::Option::DontUseNativeDialog);

	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setFileMode(QFileDialog::ExistingFile);
	fd.setNameFilter("Conf Binary Files (*.cdb);; All Files (*.*)");
	
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

	bool result = m_reader.load(fileName);

	if (result == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("File %1 wasn't loaded").arg(fileName));
		mb.setIcon(QMessageBox::Critical);
		mb.exec();
		return;
	}

	ui.fileNameEdit->setText(fileName);
	ui.UartIdEdit->setText(QString::number(m_reader.uartID(), 16));
	
	theLog.writeMessage(tr("File %1 was loaded.").arg(fileName));

	theLog.writeMessage(tr("Name: %1").arg(m_reader.name()));
	theLog.writeMessage(tr("Changeset: %1").arg(m_reader.changeset()));
	theLog.writeMessage(tr("Template: %1").arg(m_reader.fileName()));
	theLog.writeMessage(tr("UartID: %1h").arg(QString::number(m_reader.uartID(), 16)));
	theLog.writeMessage(tr("MinimumFrameSize: %1").arg(QString::number(m_reader.minFrameSize())));
	theLog.writeMessage(tr("FrameCount: %1").arg(QString::number(m_reader.framesCount())));

	return;
}
