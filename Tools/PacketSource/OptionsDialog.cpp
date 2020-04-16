#include "OptionsDialog.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>

#include "../../Builder/CfgFiles.h"
#include "../../lib/XmlHelper.h"
#include "../../lib/Types.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(const BuildInfo& buildInfo, QWidget* parent) :
	QDialog(parent) ,
	m_buildInfo(buildInfo)
{
	createInterface();
	restoreWindowState();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::~OptionsDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));

	// Build Path
	//
	QGroupBox* groupBuildPath = new QGroupBox(tr("Build path"));
	QVBoxLayout *buildFilesLayout = new QVBoxLayout;
	QHBoxLayout *buildPathLayout = new QHBoxLayout;

	m_buildDirPathEdit = new QLineEdit(m_buildInfo.buildDirPath(), this);
	m_buildDirPathEdit->setReadOnly(true);
	m_selectBuildPathBtn = new QPushButton(tr("Select ..."), this);

	m_signalsFileEdit = new QLineEdit(m_buildInfo.buildFile(BUILD_FILE_TYPE_SIGNALS).path(), this);
	m_signalsFileEdit->setReadOnly(true);
	m_signalsFileEdit->setFrame(false);
	m_sourceCfgFileEdit = new QLineEdit(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCE_CFG).path(), this);
	m_sourceCfgFileEdit->setReadOnly(true);
	m_sourceCfgFileEdit->setFrame(false);
	m_sourcesFileEdit = new QLineEdit(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCES).path(), this);
	m_sourcesFileEdit->setReadOnly(true);
	m_sourcesFileEdit->setFrame(false);

	buildPathLayout->addWidget(m_buildDirPathEdit);
	buildPathLayout->addWidget(m_selectBuildPathBtn);

	connect(m_selectBuildPathBtn, &QPushButton::clicked, this, &OptionsDialog::onSelectBuildDirPath);

	buildFilesLayout->addLayout(buildPathLayout);
	buildFilesLayout->addWidget(m_signalsFileEdit);
	buildFilesLayout->addWidget(m_sourceCfgFileEdit);
	buildFilesLayout->addWidget(m_sourcesFileEdit);

	groupBuildPath->setLayout(buildFilesLayout);

	// reload build
	//
	QGroupBox* groupReloadBuild = new QGroupBox(tr("Reload build files automatically"));
	QVBoxLayout *reloadBuildLayout = new QVBoxLayout;
	QHBoxLayout *timeoutReloadBuildLayout = new QHBoxLayout;

	m_enableReloadCheck = new QCheckBox(tr("Enable reload build files automatically"), this);
	m_enableReloadCheck->setCheckState(m_buildInfo.enableReload() == true ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

	connect(m_enableReloadCheck, &QCheckBox::clicked, this, &OptionsDialog::onEnableReload);

	QLabel* timeoutReloadLabel = new QLabel(tr("Timeout for check new build files"), this);
	m_timeoutReloadEdit = new QLineEdit(QString::number(m_buildInfo.timeoutReload(), 10), this);
	m_timeoutReloadEdit->setValidator( new QIntValidator(0, 1000, this) );
	m_timeoutReloadEdit->setAlignment(Qt::AlignCenter);
	m_timeoutReloadEdit->setEnabled(m_enableReloadCheck->checkState() == Qt::CheckState::Checked);
	QLabel* timeoutSecLabel = new QLabel(tr("second(s)"), this);

	connect(m_timeoutReloadEdit, &QLineEdit::textChanged, this, &OptionsDialog::onTimeoutReload);

	timeoutReloadBuildLayout->addWidget(timeoutReloadLabel);
	timeoutReloadBuildLayout->addWidget(m_timeoutReloadEdit);
	timeoutReloadBuildLayout->addWidget(timeoutSecLabel);
	timeoutReloadBuildLayout->addStretch();

	reloadBuildLayout->addWidget(m_enableReloadCheck);
	reloadBuildLayout->addLayout(timeoutReloadBuildLayout);

	groupReloadBuild->setLayout(reloadBuildLayout);

	// IP
	//
	QGroupBox* groupLocalIP = new QGroupBox(tr("IP-address"));
	QVBoxLayout *localIPLayout = new QVBoxLayout;

	// appDataSrv IP
	//
	QHBoxLayout *appDataSrvIPLayout = new QHBoxLayout;

	QLabel* appDataSrvIPLabel = new QLabel(tr("AppDataReceivingIP for send packets to AppDataSrv"), this);
	m_appDataSrvIPEdit = new QLineEdit(m_buildInfo.appDataSrvIP().addressStr(), this);

	connect(m_appDataSrvIPEdit, &QLineEdit::textChanged, this, &OptionsDialog::onAppDataSrvIP);

	appDataSrvIPLayout->addWidget(appDataSrvIPLabel);
	appDataSrvIPLayout->addStretch();
	appDataSrvIPLayout->addWidget(m_appDataSrvIPEdit);

	// UalTester IP
	//
	QHBoxLayout *ualTesterIPLayout = new QHBoxLayout;

	QLabel* ualTesterIPLabel = new QLabel(tr("IP for listening commands from UalTester"), this);
	m_ualTesterIPEdit = new QLineEdit(m_buildInfo.ualTesterIP().addressStr(), this);

	connect(m_ualTesterIPEdit, &QLineEdit::textChanged, this, &OptionsDialog::onUalTesterIP);

	ualTesterIPLayout->addWidget(ualTesterIPLabel);
	ualTesterIPLayout->addStretch();
	ualTesterIPLayout->addWidget(m_ualTesterIPEdit);

	localIPLayout->addLayout(appDataSrvIPLayout);
	localIPLayout->addLayout(ualTesterIPLayout);
	groupLocalIP->setLayout(localIPLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &OptionsDialog::onCancel);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(groupBuildPath);
	mainLayout->addWidget(groupReloadBuild);
	mainLayout->addWidget(groupLocalIP);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	setMinimumSize(200,200);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onSelectBuildDirPath()
{
	QString defaultDir = QDir::currentPath();

	if (m_buildInfo.buildDirPath().isEmpty() == false)
	{
		if (QDir(m_buildInfo.buildDirPath()).exists() == true)
		{
			defaultDir = m_buildInfo.buildDirPath();
		}
	}

	QString buildDirPath = QFileDialog::getExistingDirectory(this, tr("Select build directory"), defaultDir);
	if (buildDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Build directory path is empty!"));
		return;
	}

	m_signalsFileEdit->setText(QString());
	m_sourceCfgFileEdit->setText(QString());
	m_sourcesFileEdit->setText(QString());

	loadBuildDirPath(buildDirPath);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onEnableReload()
{
	m_timeoutReloadEdit->setEnabled(m_enableReloadCheck->checkState() == Qt::CheckState::Checked);
	m_buildInfo.setEnableReload(m_enableReloadCheck->checkState() == Qt::CheckState::Checked);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onTimeoutReload(const QString &sec)
{
	m_buildInfo.setTimeoutReload(sec.toInt());
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onAppDataSrvIP(const QString &ip)
{
	m_buildInfo.setAppDataSrvIP(HostAddressPort(ip, m_buildInfo.appDataSrvIP().port()));
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onUalTesterIP(const QString &ip)
{
	m_buildInfo.setUalTesterIP(HostAddressPort(ip, m_buildInfo.ualTesterIP().port()));
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::loadBuildDirPath(const QString& buildDirPath)
{
	if (buildDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Build directory path is empty!"));
		return false;
	}

	if (QDir(buildDirPath).exists() == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Build directory is not exist!"));
		return false;
	}

	m_buildDirPathEdit->setText(buildDirPath);

	m_buildInfo.setBuildDirPath(buildDirPath);

	if (QFile::exists(m_buildInfo.buildFile(BUILD_FILE_TYPE_SIGNALS).path()) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(Builder::FILE_APP_SIGNALS_ASGS));
		return false;
	}

	m_signalsFileEdit->setText(m_buildInfo.buildFile(BUILD_FILE_TYPE_SIGNALS).path());

	if (QFile::exists(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCE_CFG).path()) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(Builder::FILE_CONFIGURATION_XML));
		return false;
	}

	m_sourceCfgFileEdit->setText(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCE_CFG).path());

	if (QFile::exists(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCES).path()) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(Builder::FILE_APP_DATA_SOURCES_XML));
		return false;
	}

	m_sourcesFileEdit->setText(m_buildInfo.buildFile(BUILD_FILE_TYPE_SOURCES).path());

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::saveWindowState()
{
//	theOptions.windows().m_optionsWindowPos = pos();
//	theOptions.windows().m_optionsWindowGeometry = saveGeometry();

//	theOptions.windows().save();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::restoreWindowState()
{
//	move(theOptions.windows().m_optionsWindowPos);
//	restoreGeometry(theOptions.windows().m_optionsWindowGeometry);

//	// Ensure widget is visible
//	//
//	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
	// build dir path
	//
	QString buildDirPath = m_buildDirPathEdit->text();
	if (QDir(buildDirPath).exists() == false)
	{
		return;
	}

	QString signalsFile = m_signalsFileEdit->text();
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(signalsFile));
		return;
	}

	QString sourceCfgFile = m_sourceCfgFileEdit->text();
	if (QFile::exists(sourceCfgFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourceCfgFile));
		return;
	}

	QString sourcesFile = m_sourcesFileEdit->text();
	if (QFile::exists(sourcesFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourcesFile));
		return;
	}

	// reload build
	//
	QString timeoutReload = m_timeoutReloadEdit->text();
	if (timeoutReload.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Timeout for reload is empty!"));
		return;
	}

	// IP
	//
	QString appDataSrvIP = m_appDataSrvIPEdit->text();
	if (appDataSrvIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input AppDataSrv IP!"));
		return;
	}

	QString ualTesterIP = m_ualTesterIPEdit->text();
	if (ualTesterIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input UalTester IP!"));
		return;
	}

	saveWindowState();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onCancel()
{
	saveWindowState();

	reject();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::closeEvent(QCloseEvent* e)
{
	saveWindowState();

	QDialog::closeEvent(e);
}

// -------------------------------------------------------------------------------------------------------------------
