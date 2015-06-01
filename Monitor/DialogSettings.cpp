#include "DialogSettings.h"
#include "ui_DialogSettings.h"

#include <QFileDialog>

DialogSettings::DialogSettings(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

const Settings& DialogSettings::settings() const
{
	return m_settings;
}

void DialogSettings::setSettings(const Settings& value)
{
	m_settings = value;

	ui->editConfiguratorIpAddress->setText(m_settings.configuratorIpAddress());
	ui->editConfiguratorPort->setText(QString().setNum(m_settings.configuratorPort()));

	return;
}

void DialogSettings::on_ok_clicked()
{
	// Check ip address
	//
	QString configuratorIpAddress = ui->editConfiguratorIpAddress->text();

	QHostAddress ha;
	if (ha.setAddress(configuratorIpAddress) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect format of the configurator IP Address."));
		mb.exec();

		ui->editConfiguratorIpAddress->setFocus();
		ui->editConfiguratorIpAddress->selectAll();
		return;
	}

	// Check port num
	//
	bool result = false;
	int serverPort = ui->editConfiguratorPort->text().toInt(&result);

	if (result == false || serverPort < 0 || serverPort > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editConfiguratorPort->setFocus();
		ui->editConfiguratorPort->selectAll();
		return;
	}

	// --
	//
	m_settings.setConfiguratorIpAddress(configuratorIpAddress);
	m_settings.setConfiguratorPort(serverPort);

	accept();
	return;
}

void DialogSettings::on_cancel_clicked()
{
	reject();
	return;
}
