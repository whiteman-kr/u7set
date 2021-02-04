#include "OptionsDialog.h"

#include <QFileDialog>

#include "../../lib/XmlHelper.h"
#include "../../lib/Types.h"
#include "../../lib/ConstStrings.h"
#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(const BuildOption& buildOption, QWidget* parent) :
	QDialog(parent) ,
	m_buildOption(buildOption)
{
	createInterface();
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

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());
	resize(static_cast<int>(screen.width() * 0.25), static_cast<int>(screen.height() * 0.2));
	move(screen.center() - rect().center());

	// CfgSrv
	//
	QGroupBox* serviceGroup = new QGroupBox(tr("Connection to Configuration Service"));
	QVBoxLayout *serviceLabelLayout = new QVBoxLayout;


	QLabel* cfgSrvIDLabel = new QLabel(tr("TestClient EquipmentID"), this);
	QLabel* cfgSrvIPLabel = new QLabel(tr("Configuration Service IP"), this);
	QLabel* cfgSrvPortLabel = new QLabel(tr("Configuration Service Port"), this);
	QLabel* appDataSrvIDLabel = new QLabel(tr("Application Data Service EquipmentID"), this);

	serviceLabelLayout->addWidget(cfgSrvIDLabel);
	serviceLabelLayout->addWidget(cfgSrvIPLabel);
	serviceLabelLayout->addWidget(cfgSrvPortLabel);
	serviceLabelLayout->addWidget(appDataSrvIDLabel);

	QVBoxLayout *serviceEditLayout = new QVBoxLayout;

	m_cfgSrvIDEdit = new QLineEdit(m_buildOption.cfgSrvEquipmentID(), this);
	m_cfgSrvIPEdit = new QLineEdit(m_buildOption.cfgSrvIP().addressStr(), this);
	m_cfgSrvPortEdit = new QLineEdit(QString::number(m_buildOption.cfgSrvIP().port()), this);
	m_appDataSrvIDEdit = new QLineEdit(m_buildOption.appDataSrvEquipmentID(), this);

	serviceEditLayout->addWidget(m_cfgSrvIDEdit);
	serviceEditLayout->addWidget(m_cfgSrvIPEdit);
	serviceEditLayout->addWidget(m_cfgSrvPortEdit);
	serviceEditLayout->addWidget(m_appDataSrvIDEdit);

	QHBoxLayout *serviceLayout = new QHBoxLayout;

	serviceLayout->addLayout(serviceLabelLayout);
	serviceLayout->addLayout(serviceEditLayout);

	serviceGroup->setLayout(serviceLayout);

	// UalTester
	//
	QGroupBox* groupLocalIP = new QGroupBox(tr("UalTester"));
	QVBoxLayout *localIPLayout = new QVBoxLayout;

	// UalTester IP
	//
	QHBoxLayout *ualTesterIPLayout = new QHBoxLayout;

	QLabel* ualTesterIPLabel = new QLabel(tr("IP for listening commands from UalTester"), this);
	m_ualTesterIPEdit = new QLineEdit(m_buildOption.ualTesterIP().addressStr(), this);

	connect(m_ualTesterIPEdit, &QLineEdit::textChanged, this, &OptionsDialog::onUalTesterIP);

	ualTesterIPLayout->addWidget(ualTesterIPLabel);
	ualTesterIPLayout->addWidget(m_ualTesterIPEdit);

	localIPLayout->addLayout(ualTesterIPLayout);
	groupLocalIP->setLayout(localIPLayout);

	// buttons
	//
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &OptionsDialog::onOk);

	// Main Layout
	//
	QVBoxLayout *mainLayout = new QVBoxLayout;

	mainLayout->addWidget(serviceGroup);
	mainLayout->addWidget(groupLocalIP);
	mainLayout->addStretch();
	mainLayout->addWidget(m_buttonBox);

	setLayout(mainLayout);

	setMinimumSize(200,200);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onUalTesterIP(const QString &ip)
{
	m_buildOption.setUalTesterIP(HostAddressPort(ip, m_buildOption.ualTesterIP().port()));
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
	QString cfgSrvID = m_cfgSrvIDEdit->text();
	if (cfgSrvID.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field cfgSrvID is empty!"));
		return;
	}

	QString cfgSrvIP = m_cfgSrvIPEdit->text();
	if (cfgSrvIP.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field cfgSrvIP is empty!"));
		return;
	}

	QString cfgSrvPort = m_cfgSrvPortEdit->text();
	if (cfgSrvIP.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field cfgSrvIP is empty!"));
		return;
	}

	QString appDataSrvID = m_appDataSrvIDEdit->text();
	if (appDataSrvID.isEmpty() == true)
	{
		QMessageBox::information(nullptr, windowTitle(), tr("Field appDataSrvID is empty!"));
		return;
	}

	QString ualTesterIP = m_ualTesterIPEdit->text();
	if (ualTesterIP.isEmpty() == true)
	{
		QMessageBox::information(this, windowTitle(), tr("Please, input UalTester IP!"));
		return;
	}

	m_buildOption.setСfgSrvEquipmentID(cfgSrvID);
	m_buildOption.setCfgSrvIP(HostAddressPort(cfgSrvIP, cfgSrvPort.toInt()));
	m_buildOption.setAppDataSrvEquipmentID(appDataSrvID);
	m_buildOption.setUalTesterIP(HostAddressPort(ualTesterIP, PORT_TUNING_SERVICE_CLIENT_REQUEST));

	accept();
}

// -------------------------------------------------------------------------------------------------------------------
