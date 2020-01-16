#include "PathOptionDialog.h"

#include <QFile>
#include <QFileDialog>

#include "../../Builder/CfgFiles.h"

// -------------------------------------------------------------------------------------------------------------------

PathOptionDialog::PathOptionDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

PathOptionDialog::~PathOptionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool PathOptionDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));

	// Signal Path
	//
	QGroupBox* groupSignalPath = new QGroupBox(tr("Path to file of signals (%1)").arg(Builder::FILE_APP_SIGNALS_ASGS));
	QHBoxLayout *signalPathLayout = new QHBoxLayout;

	m_signalPathEdit = new QLineEdit(theOptions.path().signalPath(), this);
	m_selectSignalPathBtn = new QPushButton(tr("Select ..."), this);

	signalPathLayout->addWidget(m_signalPathEdit);
	signalPathLayout->addWidget(m_selectSignalPathBtn);

	connect(m_selectSignalPathBtn, &QPushButton::clicked, this, &PathOptionDialog::onSelectSignalPath);

	groupSignalPath->setLayout(signalPathLayout);

	// Source Path
	//
	QGroupBox* groupSourcePath = new QGroupBox(tr("Path to file of Data Sources (%1)").arg(Builder::FILE_APP_DATA_SOURCES_XML));
	QHBoxLayout *sourcePathLayout = new QHBoxLayout;

	m_sourcePathEdit = new QLineEdit(theOptions.path().sourcePath(), this);
	m_selectSourcePathBtn = new QPushButton(tr("Select ..."), this);

	sourcePathLayout->addWidget(m_sourcePathEdit);
	sourcePathLayout->addWidget(m_selectSourcePathBtn);

	connect(m_selectSourcePathBtn, &QPushButton::clicked, this, &PathOptionDialog::onSelectSourcePath);

	groupSourcePath->setLayout(sourcePathLayout);


	// IP
	//
	QGroupBox* groupLocalIP = new QGroupBox(tr("IP-address"));
	QVBoxLayout *localIPLayout = new QVBoxLayout;

	// appDataSrv IP
	//
	QHBoxLayout *appDataSrvIPLayout = new QHBoxLayout;

	QLabel* appDataSrvIPLabel = new QLabel(tr("AppDataSrv IP"), this);
	m_appDataSrvIPEdit = new QLineEdit(theOptions.path().appDataSrvIP(), this);

	appDataSrvIPLayout->addWidget(appDataSrvIPLabel);
	appDataSrvIPLayout->addStretch();
	appDataSrvIPLayout->addWidget(m_appDataSrvIPEdit);

	// UalTester IP
	//
	QHBoxLayout *ualTesterIPLayout = new QHBoxLayout;

	QLabel* ualTesterIPLabel = new QLabel(tr("UalTester IP"), this);
	m_ualTesterIPEdit = new QLineEdit(theOptions.path().ualTesterIP(), this);

	ualTesterIPLayout->addWidget(ualTesterIPLabel);
	ualTesterIPLayout->addStretch();
	ualTesterIPLayout->addWidget(m_ualTesterIPEdit);


	localIPLayout->addLayout(appDataSrvIPLayout);
	localIPLayout->addLayout(ualTesterIPLayout);
	groupLocalIP->setLayout(localIPLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PathOptionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &PathOptionDialog::reject);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(groupSignalPath);
	mainLayout->addWidget(groupSourcePath);
	mainLayout->addWidget(groupLocalIP);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void PathOptionDialog::onSelectSignalPath()
{
	QString defaultDir = QDir::currentPath();

	if (theOptions.path().signalPath().isEmpty() == false)
	{
		if (QFile::exists(theOptions.path().signalPath()) == true)
		{
			defaultDir = theOptions.path().signalPath();
		}
	}

	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), defaultDir);
	if (path.isEmpty() == true)
	{
		return;
	}

	m_signalPathEdit->setText(path);
}

// -------------------------------------------------------------------------------------------------------------------

void PathOptionDialog::onSelectSourcePath()
{
	QString defaultDir = QDir::currentPath();

	if (theOptions.path().sourcePath().isEmpty() == false)
	{
		if (QFile::exists(theOptions.path().sourcePath()) == true)
		{
			defaultDir = theOptions.path().sourcePath();
		}
	}

	QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"), defaultDir);
	if (path.isEmpty() == true)
	{
		return;
	}

	m_sourcePathEdit->setText(path);
}

// -------------------------------------------------------------------------------------------------------------------

void PathOptionDialog::onOk()
{
	QString signalPath, sourcePath, appDataSrvIP, ualTesterIP;

	// signal path

	signalPath = m_signalPathEdit->text();
	if (signalPath.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input path to signals file!"));
		return;
	}

	if (QFile::exists(signalPath) == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Path to signals is not found!"));
		return;
	}

	// source path

	sourcePath = m_sourcePathEdit->text();
	if (sourcePath.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input path to sources file!"));
		return;
	}

	if (QFile::exists(sourcePath) == false)
	{
		QMessageBox::information(this, windowTitle(), tr("Path to sources is not found!"));
		return;
	}

	//

	appDataSrvIP = m_appDataSrvIPEdit->text();
	if (appDataSrvIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input AppDataSrv IP!"));
		return;
	}

	//

	ualTesterIP = m_ualTesterIPEdit->text();
	if (ualTesterIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input UalTester IP!"));
		return;
	}

	//

	theOptions.path().setSignalPath(signalPath);
	theOptions.path().setSourcePath(sourcePath);
	theOptions.path().setAppDataSrvIP(appDataSrvIP);
	theOptions.path().setUalTesterIP(ualTesterIP);

	theOptions.path().save();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
