#include "DialogSettings.h"
#include "ui_DialogSettings.h"

#include "./ProjectsTabPage/ProjectBackup.h"


DialogSettings::DialogSettings(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogSettings)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~Qt::WindowMinimizeButtonHint & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowContextHelpButtonHint) |
				   Qt::CustomizeWindowHint);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogSettings::on_ok_clicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogSettings::on_cancel_clicked);

	connect(ui->browsePgDump, &QPushButton::clicked, this, &DialogSettings::browsePgDump);
	connect(ui->detectPgDump, &QPushButton::clicked, this, &DialogSettings::detectPgDump);

	connect(ui->browsePsql, &QPushButton::clicked, this, &DialogSettings::browsePsql);
	connect(ui->detectPsql, &QPushButton::clicked, this, &DialogSettings::detectPsql);

	return;
}

DialogSettings::~DialogSettings()
{
	delete ui;
}

const AppSettings& DialogSettings::settings() const
{
	return m_settings;
}

void DialogSettings::setSettings(const AppSettings& value)
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

	ui->editPgDump->setText(m_settings.pgDumpCommand());
	ui->editPlsql->setText(m_settings.psqlCommand());

	return;
}

void DialogSettings::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = parentWidget()->screen()->availableGeometry();
	resize(static_cast<int>(screen.width() * 0.25), height());

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

	m_settings.setPgDumpCommand(ui->editPgDump->text());
	m_settings.setPsqlCommand(ui->editPlsql->text());

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
	QString dir =
		QDir().toNativeSeparators(QFileDialog::getExistingDirectory(this,
																	tr("Open Directory"),
																	m_settings.buildOutputPath(),
																	QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (dir.isEmpty() == false)
	{
		ui->editOutputPath->setText(dir);
	}
}

void DialogSettings::browsePgDump()
{
#ifdef Q_OS_WIN32
	QString filter = tr("pg_dump (pg_dump.exe);;All Files (*)");
#else
	QString filter = tr("pg_dump (pg_dump);;All Files (*)");
#endif

	QString file = QFileDialog::getOpenFileName(this, tr("pg_dump"), m_settings.pgDumpCommand(), filter);
	if (file.isEmpty() == false)
	{
		ui->editPgDump->setText(file);
	}

	return;
}

void DialogSettings::detectPgDump()
{
	ui->editPgDump->setText(ProjectBackup::autoDetectExecutable("pg_dump"));
}

void DialogSettings::browsePsql()
{
#ifdef Q_OS_WIN32
	QString filter = tr("psql (psql.exe);;All Files (*)");
#else
	QString filter = tr("psql (psql);;All Files (*)");
#endif

	QString file = QFileDialog::getOpenFileName(this, tr("psql"), m_settings.pgDumpCommand(), filter);
	if (file.isEmpty() == false)
	{
		ui->editPlsql->setText(file);
	}

	return;
}

void DialogSettings::detectPsql()
{
	ui->editPlsql->setText(ProjectBackup::autoDetectExecutable("psql"));
}
