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
	connect (ui->saveAsButton, &QPushButton::clicked, this, &DialogSettings::saveAs_clicked);

	return;
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

const MonitorAppSettings::Data& DialogSettings::settings() const
{
	return m_settings;
}

void DialogSettings::setSettings(const MonitorAppSettings::Data& value)
{
	m_settings = value;

	ui->instanceStrId->setText(m_settings.equipmentId);

	ui->editConfiguratorIpAddress1->setText(m_settings.cfgSrvIpAddress1);
	ui->editConfiguratorPort1->setText(QString().setNum(m_settings.cfgSrvPort1));

	ui->editConfiguratorIpAddress2->setText(m_settings.cfgSrvIpAddress2);
	ui->editConfiguratorPort2->setText(QString().setNum(m_settings.cfgSrvPort2));

	ui->checkShowLogo->setChecked(m_settings.showLogo);
	ui->checkShowItemsLabels->setChecked(m_settings.showItemsLabels);
	ui->checkSingleInstance->setChecked(m_settings.singleInstance);
	ui->windowCaptionEdit->setText(m_settings.windowCaption);

	return;
}

void DialogSettings::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.23), height());
	move(screen.center() - rect().center());

	return;
}

std::optional<MonitorAppSettings::Data> DialogSettings::parseData()
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
		return {};
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
		return {};
	}

	// Check port num 1
	//
	bool convResult = false;
	int serverPort1 = ui->editConfiguratorPort1->text().toInt(&convResult);

	if (convResult == false || serverPort1 < 0 || serverPort1 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editConfiguratorPort1->setFocus();
		ui->editConfiguratorPort1->selectAll();
		return {};
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
		return {};
	}

	// Check port num 2
	//
	convResult = false;
	int serverPort2 = ui->editConfiguratorPort2->text().toInt(&convResult);

	if (convResult == false || serverPort2 < 0 || serverPort2 > 65535)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server port."));
		mb.exec();

		ui->editConfiguratorPort2->setFocus();
		ui->editConfiguratorPort2->selectAll();
		return {};
	}

	// --
	//
	MonitorAppSettings::Data data;

	data.equipmentId = instanceStrId;

	data.cfgSrvIpAddress1 = configuratorIpAddress1;
	data.cfgSrvPort1 = serverPort1;

	data.cfgSrvIpAddress2 = configuratorIpAddress2;
	data.cfgSrvPort2 = serverPort2;

	data.showLogo = ui->checkShowLogo->isChecked();
	data.showItemsLabels = ui->checkShowItemsLabels->isChecked();
	data.windowCaption = ui->windowCaptionEdit->text();

	data.singleInstance = ui->checkSingleInstance->isChecked();

	return {data};
}


void DialogSettings::ok_clicked()
{
	auto d = parseData();

	if (d.has_value() == true)
	{
		m_settings = d.value();
		accept();
	}

	return;
}

void DialogSettings::cancel_clicked()
{
	reject();
	return;
}

void DialogSettings::saveAs_clicked()
{
	auto d = parseData();

	if (d.has_value() == false)
	{
		return;
	}

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													QString{},
													tr("ini File (*.ini);;All Files (*.*)"));

	MonitorAppSettings ms;
	ms.set(d.value());

	if (bool ok = ms.saveToFile(fileName);
		ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("File %1 saving error.").arg(fileName));
	}

	return;
}
