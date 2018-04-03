#include "SourceOptionDialog.h"

#include <QFile>
#include <QFileDialog>

// -------------------------------------------------------------------------------------------------------------------

SourceOptionDialog::SourceOptionDialog(QWidget* parent) :
	QDialog(parent)
{
	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

SourceOptionDialog::~SourceOptionDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SourceOptionDialog::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setWindowTitle(tr("Options"));
	setFixedSize(300, 200);

	// Server IP
	//
	QHBoxLayout *serverIPLayout = new QHBoxLayout;

	m_serverIPLabel = new QLabel(tr("IP"), this);
	m_serverIPEdit = new QLineEdit(theOptions.source().serverIP(), this);

	serverIPLayout->addWidget(m_serverIPLabel);
	serverIPLayout->addStretch();
	serverIPLayout->addWidget(m_serverIPEdit);


	// Server port
	//
	QHBoxLayout *serverPortLayout = new QHBoxLayout;

	m_serverPortLabel = new QLabel(tr("Port"), this);
	m_serverPortEdit = new QLineEdit(QString::number(theOptions.source().serverPort()), this);
	m_serverPortEdit->setValidator(new QIntValidator(0, 65535, this));

	serverPortLayout->addWidget(m_serverPortLabel);
	serverPortLayout->addStretch();
	serverPortLayout->addWidget(m_serverPortEdit);

	// Path
	//
	QHBoxLayout *serverPathLayout = new QHBoxLayout;

	m_pathEdit = new QLineEdit(theOptions.source().path(), this);
	m_selectPathBtn = new QPushButton(tr("Select ..."), this);

	serverPathLayout->addWidget(m_pathEdit);
	//serverPathLayout->addStretch();
	serverPathLayout->addWidget(m_selectPathBtn);

	connect(m_selectPathBtn, &QPushButton::clicked, this, &SourceOptionDialog::onSelectPath);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SourceOptionDialog::onOk);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SourceOptionDialog::reject);

	// Main Layout
	//

	QVBoxLayout *optionConnectLayout = new QVBoxLayout;

	optionConnectLayout->addLayout(serverIPLayout);
	optionConnectLayout->addLayout(serverPortLayout);

	QGroupBox* groupIP = new QGroupBox(tr("Server connect"));
	groupIP->setLayout(optionConnectLayout);


	QGroupBox* groupPath = new QGroupBox(tr("Path to file sources (AppDataSources.xml)"));
	groupPath->setLayout(serverPathLayout);


	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(groupIP);
	mainLayout->addWidget(groupPath);
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOptionDialog::onSelectPath()
{

	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(), tr("XML files (*.xml)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

	m_pathEdit->setText(fileName);
}

// -------------------------------------------------------------------------------------------------------------------

void SourceOptionDialog::onOk()
{

	QString serverIP = m_serverIPEdit->text();
	if (serverIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input server IP!"));
		return;
	}

	if (m_serverPortEdit->text().isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input server port!"));
		return;
	}

	int port = m_serverPortEdit->text().toInt();
	if (port <= 0 || port > 65535)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, correct server port!"));
		return;
	}

	QString path = m_pathEdit->text();
	if (path.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input path to sources file!"));
		return;
	}

	if (QFile::exists(path) == false)
	{
		QMessageBox::information(this, windowTitle(), tr("File of sources is not found!"));
		return;
	}

	theOptions.source().setServerIP(serverIP);
	theOptions.source().setServerPort(port);
	theOptions.source().setPath(path);

	theOptions.source().save();

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
