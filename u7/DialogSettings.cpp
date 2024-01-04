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

	QString hostHistoryString = QSettings().value("DialogSettings/hostHistory", QString()).toString();
	QStringList hostHistory = hostHistoryString.split(';', Qt::SkipEmptyParts);

	ui->comboHost->addItems(hostHistory);
	ui->comboHost->setCurrentText(m_settings.serverHost());
	ui->editPort->setText(QString().setNum(m_settings.serverPort()));
	ui->editUsername->setText(m_settings.serverUsername());
	ui->editPassword->setText(m_settings.serverPassword());
	ui->editOutputPath->setText(m_settings.buildOutputPath());
	ui->checkExpertMode->setChecked(m_settings.isExpertMode());

	return;
}

void DialogSettings::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.23), height());

	move(screen.center() - rect().center());

	return;
}

void DialogSettings::on_ok_clicked()
{
	// Check ip address
	//
	QString serverHost = ui->comboHost->currentText().toUpper();
	if (serverHost.isEmpty() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("Incorrect server IP Address."));
		mb.exec();

		ui->comboHost->setFocus();
		return;
	}

	// Check Instance StrID
	//
	QStringList hostHistory;
	for (int i = 0; i < ui->comboHost->count(); i++)
	{
		hostHistory.push_back(ui->comboHost->itemText(i));

		if (hostHistory.size() >= 10)
		{
			break;
		}
	}

	if (hostHistory.contains(serverHost) == false)
	{
		hostHistory.push_front(serverHost);
	}

	QSettings().setValue("DialogSettings/hostHistory", hostHistory.join(';'));

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

	QString buildOutputPath = ui->editOutputPath->text();

	// --
	//
	m_settings.setServerHost(serverHost);
	m_settings.setServerPort(serverPort);
	m_settings.setServerUsername(serverUsername);
	m_settings.setServerPassword(serverPassword);
	m_settings.setBuildOutputPath(buildOutputPath);
	m_settings.setExpertMode(ui->checkExpertMode->checkState() == Qt::CheckState::Checked);

	accept();
	return;
}

void DialogSettings::on_cancel_clicked()
{
	reject();
	return;
}

void DialogSettings::on_browseOutputPath_clicked()
{
	QString dir = QDir().toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Open Directory"),
													m_settings.buildOutputPath(),
													QFileDialog::ShowDirsOnly
													| QFileDialog::DontResolveSymlinks));

    if (dir.isEmpty() == false)
    {
        ui->editOutputPath->setText(dir);
    }
}
