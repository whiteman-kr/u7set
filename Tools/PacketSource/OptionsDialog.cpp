#include "OptionsDialog.h"

#include <QFile>
#include <QFileDialog>

#include "../../Builder/CfgFiles.h"
#include "../../lib/XmlHelper.h"
#include "../../lib/Types.h"


// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(QWidget* parent) :
	QDialog(parent)
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

	m_buildDirPathEdit = new QLineEdit(theOptions.build().buildDirPath(), this);
	m_selectBuildPathBtn = new QPushButton(tr("Select ..."), this);

	m_signalsFileEdit = new QLineEdit(theOptions.build().buildFile(BUILD_FILE_TYPE_SIGNALS).path(), this);
	m_signalsFileEdit->setReadOnly(true);
	m_signalsFileEdit->setFrame(false);
	m_sourceCfgFileEdit = new QLineEdit(theOptions.build().buildFile(BUILD_FILE_TYPE_SOURCE_CFG).path(), this);
	m_sourceCfgFileEdit->setReadOnly(true);
	m_sourceCfgFileEdit->setFrame(false);
	m_sourcesFileEdit = new QLineEdit(theOptions.build().buildFile(BUILD_FILE_TYPE_SOURCES).path(), this);
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
	m_enableReloadCheck->setCheckState(theOptions.build().enableReload() == true ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

	connect(m_enableReloadCheck, &QCheckBox::clicked, this, &OptionsDialog::onEnableReload);

	QLabel* timeoutReloadLabel = new QLabel(tr("Timeout for check new build files"), this);
	m_timeoutReloadEdit = new QLineEdit(QString::number(theOptions.build().timeoutReload(), 10), this);
	m_timeoutReloadEdit->setValidator( new QIntValidator(0, 1000, this) );
	m_timeoutReloadEdit->setAlignment(Qt::AlignCenter);
	m_timeoutReloadEdit->setEnabled(m_enableReloadCheck->checkState() == Qt::CheckState::Checked);
	QLabel* timeoutSecLabel = new QLabel(tr("second(s)"), this);

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
	m_appDataSrvIPEdit = new QLineEdit(theOptions.build().appDataSrvIP(), this);

	appDataSrvIPLayout->addWidget(appDataSrvIPLabel);
	appDataSrvIPLayout->addStretch();
	appDataSrvIPLayout->addWidget(m_appDataSrvIPEdit);

	// UalTester IP
	//
	QHBoxLayout *ualTesterIPLayout = new QHBoxLayout;

	QLabel* ualTesterIPLabel = new QLabel(tr("IP for listening commands from UalTester"), this);
	m_ualTesterIPEdit = new QLineEdit(theOptions.build().ualTesterIP(), this);

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

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onSelectBuildDirPath()
{
	QString defaultDir = QDir::currentPath();

	if (theOptions.build().buildDirPath().isEmpty() == false)
	{
		if (QFile::exists(theOptions.build().buildDirPath()) == true)
		{
			defaultDir = theOptions.build().buildDirPath();
		}
	}

	QString buildDirPath = QFileDialog::getExistingDirectory(this, tr("Select build directory"), defaultDir);
	if (loadBuildDirPath(buildDirPath) == true)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onEnableReload()
{
	m_timeoutReloadEdit->setEnabled(m_enableReloadCheck->checkState() == Qt::CheckState::Checked);
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::loadBuildDirPath(const QString& buildDirPath)
{
	if (buildDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Build directory path is empty!"));
		return false;
	}


	QString signalsFile = buildDirPath + BUILD_FILE_SEPARATOR + Builder::DIR_COMMON + BUILD_FILE_SEPARATOR + Builder::FILE_APP_SIGNALS_ASGS;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(signalsFile));
		return false;
	}


	// find path of AppDataSrv directory
	//
	QString appDataSrvDirPath;

	QDir dir(buildDirPath);
	QStringList listPathSubDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString pathSubDir, listPathSubDirs)
	{
		QString fileCfg = buildDirPath + BUILD_FILE_SEPARATOR + pathSubDir + BUILD_FILE_SEPARATOR + Builder::FILE_CONFIGURATION_XML;

		QFile fileCfgXml(fileCfg);
		if (fileCfgXml.open(QIODevice::ReadOnly) == false)
		{
			continue;
		}

		QByteArray&& cfgData = fileCfgXml.readAll();
		XmlReadHelper xmlCfg(cfgData);

		if (xmlCfg.findElement("Software") == false)
		{
			continue;
		}

		int softwareType = 0;
		if (xmlCfg.readIntAttribute("Type", &softwareType) == false)
		{
			continue;
		}

		if (softwareType == E::SoftwareType::AppDataService)
		{
			appDataSrvDirPath = buildDirPath + BUILD_FILE_SEPARATOR + pathSubDir;
			break;
		}

		fileCfgXml.close();
	}

	if (appDataSrvDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("AppDataSrv directory is not found!"));
		return false;
	}

	QString sourceCfgFile = appDataSrvDirPath + BUILD_FILE_SEPARATOR + Builder::FILE_CONFIGURATION_XML;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourceCfgFile));
		return false;
	}

	QString sourcesFile = appDataSrvDirPath + BUILD_FILE_SEPARATOR + Builder::FILE_APP_DATA_SOURCES_XML;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourcesFile));
		return false;
	}

	m_buildDirPathEdit->setText(buildDirPath);
	m_signalsFileEdit->setText(signalsFile);
	m_sourceCfgFileEdit->setText(sourceCfgFile);
	m_sourcesFileEdit->setText(sourcesFile);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::saveWindowState()
{
	theOptions.windows().m_optionsWindowPos = pos();
	theOptions.windows().m_optionsWindowGeometry = saveGeometry();

	theOptions.windows().save();

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::restoreWindowState()
{
	move(theOptions.windows().m_optionsWindowPos);
	restoreGeometry(theOptions.windows().m_optionsWindowGeometry);

	// Ensure widget is visible
	//
	setVisible(true);	// Widget must be visible for correct work of QApplication::desktop()->screenGeometry

	return;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
	// build dir path
	//
	QString buildDirPath = m_buildDirPathEdit->text();
	if (loadBuildDirPath(buildDirPath) == false)
	{
		return;
	}

	QString signalsFile = m_signalsFileEdit->text();
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(signalsFile));
		return;
	}

	BuildFile signalsBuildFile = theOptions.build().buildFile(BUILD_FILE_TYPE_SIGNALS);
	signalsBuildFile.setPath(signalsFile);

	QString sourceCfgFile = m_sourceCfgFileEdit->text();
	if (QFile::exists(sourceCfgFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourceCfgFile));
		return;
	}

	BuildFile sourceCfgBuildFile = theOptions.build().buildFile(BUILD_FILE_TYPE_SOURCE_CFG);
	sourceCfgBuildFile.setPath(sourceCfgFile);

	QString sourcesFile = m_sourcesFileEdit->text();
	if (QFile::exists(sourcesFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourcesFile));
		return;
	}

	BuildFile sourcesBuildFile = theOptions.build().buildFile(BUILD_FILE_TYPE_SOURCES);
	sourcesBuildFile.setPath(sourcesFile);

	// reload build
	//

	bool enableReload = m_enableReloadCheck->checkState() == Qt::CheckState::Checked;

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


	// save
	//
	theOptions.build().setBuildDirPath(buildDirPath);
	theOptions.build().setBuildFile(BUILD_FILE_TYPE_SIGNALS, signalsBuildFile);
	theOptions.build().setBuildFile(BUILD_FILE_TYPE_SOURCE_CFG, sourceCfgBuildFile);
	theOptions.build().setBuildFile(BUILD_FILE_TYPE_SOURCES, sourcesBuildFile);

	theOptions.build().setEnableReload(enableReload);
	theOptions.build().setTimeoutReload(timeoutReload.toInt());

	theOptions.build().setAppDataSrvIP(appDataSrvIP);
	theOptions.build().setUalTesterIP(ualTesterIP);

	theOptions.build().save();

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
