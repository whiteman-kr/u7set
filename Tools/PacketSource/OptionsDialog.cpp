#include "OptionsDialog.h"

#include <QFile>
#include <QFileDialog>

#include "../../Builder/CfgFiles.h"
#include "../../lib/XmlHelper.h"

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
	QGroupBox* groupBuildPath = new QGroupBox(tr("Build path:"));
	QVBoxLayout *buildFilesLayout = new QVBoxLayout;
	QHBoxLayout *buildPathLayout = new QHBoxLayout;

	m_buildDirPathEdit = new QLineEdit(theOptions.build().buildDirPath(), this);
	m_selectBuildPathBtn = new QPushButton(tr("Select ..."), this);

	m_signalsFileEdit = new QLineEdit(theOptions.build().signalsFilePath(), this);
	m_signalsFileEdit->setReadOnly(true);
	m_signalsFileEdit->setFrame(false);
	m_sourceCfgFileEdit = new QLineEdit(theOptions.build().sourceCfgFilePath(), this);
	m_sourceCfgFileEdit->setReadOnly(true);
	m_sourceCfgFileEdit->setFrame(false);
	m_sourcesFileEdit = new QLineEdit(theOptions.build().sourcesFilePath(), this);
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
	if (buildDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Build directory path is empty!"));
		return;
	}


	QString signalsFile = buildDirPath + "/" + Builder::DIR_COMMON + "/" + Builder::FILE_APP_SIGNALS_ASGS;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(signalsFile));
		return;
	}


	// find path of AppDataSrv directory
	//
	QString appDataSrvDirPath;

	QDir dir(buildDirPath);
	QStringList listPathSubDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString pathSubDir, listPathSubDirs)
	{
		QString fileCfg = buildDirPath + "/" + pathSubDir + "/" + Builder::FILE_CONFIGURATION_XML;

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
			appDataSrvDirPath = buildDirPath + "/" + pathSubDir;
			break;
		}

		fileCfgXml.close();
	}

	if (appDataSrvDirPath.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("AppDataSrv directory is not found!"));
		return;
	}

	QString sourceCfgFile = appDataSrvDirPath + "/" + Builder::FILE_CONFIGURATION_XML;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourceCfgFile));
		return;
	}

	QString sourcesFile = appDataSrvDirPath + "/" + Builder::FILE_APP_DATA_SOURCES_XML;
	if (QFile::exists(signalsFile) == false)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("File \"%1\" is not found!").arg(sourcesFile));
		return;
	}

	m_buildDirPathEdit->setText(buildDirPath);
	m_signalsFileEdit->setText(signalsFile);
	m_sourceCfgFileEdit->setText(sourceCfgFile);
	m_sourcesFileEdit->setText(sourcesFile);
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
	if (buildDirPath.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input build path!"));
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
	theOptions.build().setSignalsFilePath(signalsFile);
	theOptions.build().setSourceCfgFilePath(sourceCfgFile);
	theOptions.build().setSourcesFilePath(sourcesFile);
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
