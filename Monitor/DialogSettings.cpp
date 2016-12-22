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

	connect (ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::ok_clicked);
	connect (ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogSettings::cancel_clicked);

	// Resize depends on monitor size, DPI, resolution
	//
	setVisible(true);	//	if this widget is not visible yet, QDesktopWidget().availableGeometry returns resilution just to 1st screen

	QRect screen = QDesktopWidget().availableGeometry(this);
	resize(screen.width() * 0.23, height());

	move(screen.center() - rect().center());

	return;
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

	ui->instanceStrId->setText(m_settings.instanceStrId());

	ui->editConfiguratorIpAddress1->setText(m_settings.configuratorIpAddress1());
	ui->editConfiguratorPort1->setText(QString().setNum(m_settings.configuratorPort1()));

	ui->editConfiguratorIpAddress2->setText(m_settings.configuratorIpAddress2());
	ui->editConfiguratorPort2->setText(QString().setNum(m_settings.configuratorPort2()));

	ui->checkShowLogo->setChecked(m_settings.showLogo());
	ui->checkShowItemsLabels->setChecked(m_settings.showItemsLabels());
	ui->checkSingleInstance->setChecked(m_settings.singleInstance());

	return;
}

void DialogSettings::ok_clicked()
{
	// Check Instance StrID
	//
	QString instanceStrId = ui->instanceStrId->text();

	if (instanceStrId.isEmpty() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("Instance StrID cannot be empty"));
		mb.exec();

		ui->instanceStrId->setFocus();
		ui->instanceStrId->selectAll();
		return;
	}

	// Check ip address 1
	//
	QString configuratorIpAddress1 = ui->editConfiguratorIpAddress1->text();

	QHostAddress ha;
	if (ha.setAddress(configuratorIpAddress1) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect format of the configurator IP Address."));
		mb.exec();

		ui->editConfiguratorIpAddress1->setFocus();
		ui->editConfiguratorIpAddress1->selectAll();
		return;
	}

	// Check port num 1
	//
	bool result = false;
	int serverPort1 = ui->editConfiguratorPort1->text().toInt(&result);

	if (result == false || serverPort1 < 0 || serverPort1 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editConfiguratorPort1->setFocus();
		ui->editConfiguratorPort1->selectAll();
		return;
	}

	// Check ip address 2
	//
	QString configuratorIpAddress2 = ui->editConfiguratorIpAddress2->text();

	if (ha.setAddress(configuratorIpAddress2) == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect format of the configurator IP Address."));
		mb.exec();

		ui->editConfiguratorIpAddress2->setFocus();
		ui->editConfiguratorIpAddress2->selectAll();
		return;
	}

	// Check port num 2
	//
	result = false;
	int serverPort2 = ui->editConfiguratorPort2->text().toInt(&result);

	if (result == false || serverPort2 < 0 || serverPort2 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editConfiguratorPort2->setFocus();
		ui->editConfiguratorPort2->selectAll();
		return;
	}

	// --
	//
	m_settings.setInstanceStrId(instanceStrId);

	m_settings.setConfiguratorIpAddress1(configuratorIpAddress1);
	m_settings.setConfiguratorPort1(serverPort1);

	m_settings.setConfiguratorIpAddress2(configuratorIpAddress2);
	m_settings.setConfiguratorPort2(serverPort2);

	m_settings.setShowLogo(ui->checkShowLogo->isChecked());
	m_settings.setShowItemsLabels(ui->checkShowItemsLabels->isChecked());
	m_settings.setSingleInstance(ui->checkSingleInstance->isChecked());

	accept();
	return;
}

void DialogSettings::cancel_clicked()
{
	reject();
	return;
}
