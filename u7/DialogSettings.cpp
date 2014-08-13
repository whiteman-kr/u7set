#include "DialogSettings.h"
#include "ui_DialogSettings.h"

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

	ui->editIpAddress->setText(m_settings.serverIpAddress());
	ui->editPort->setText(QString().setNum(m_settings.serverPort()));
	ui->editUsername->setText(m_settings.serverUsername());
	ui->editPassword->setText(m_settings.serverPassword());

	return;
}

void DialogSettings::on_ok_clicked()
{
	// Check ip address
	//
	QString serverIpAddress = ui->editIpAddress->text();

	QHostAddress ha;
	if (ha.setAddress(serverIpAddress) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server IP Address."));
		mb.exec();

		ui->editIpAddress->setFocus();
		ui->editIpAddress->selectAll();
		return;
	}

	// Check port num
	//
	bool result = false;
	int serverPort = ui->editPort->text().toInt(&result);

	if (result == false || serverPort < 0 || serverPort > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editPort->setFocus();
		ui->editPort->selectAll();
		return;
	}

	// Set username
	//
	QString serverUsername = ui->editUsername->text();

	// Set password
	//
	QString serverPassword = ui->editPassword->text();

	// --
	//
	m_settings.setServerIpAddress(serverIpAddress);
	m_settings.setServerPort(serverPort);
	m_settings.setServerUsername(serverUsername);
	m_settings.setServerPassword(serverPassword);

	accept();
	return;
}

void DialogSettings::on_cancel_clicked()
{
	reject();
	return;
}
