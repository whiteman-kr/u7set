#include "Stable.h"
#include "SettingsForm.h"

SettingsForm::SettingsForm(const Settings& settings, QWidget* parent)
	: QDialog(parent),
	m_settings(settings)
{
	setWindowTitle(tr("Settings"));

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setSizeGripEnabled(true);

	// Enumerate all com ports
	//
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	for (const QSerialPortInfo& pi : ports)
	{
		qDebug() << "Port";
		qDebug() << pi.description();
		qDebug() << pi.manufacturer();
		qDebug() << pi.portName();
		qDebug() << pi.serialNumber();
		qDebug() << pi.systemLocation();
	}

	// ComPort
	//
	m_pSerialPort = new QComboBox();
	m_pSerialPortLabel = new QLabel(tr("&Serial Port"));
	m_pSerialPortLabel->setBuddy(m_pSerialPort);

    if (ports.size() != 0)
    {

        bool serialPortFound = false;

        for (const QSerialPortInfo& pi : ports)
        {
            QString port = pi.systemLocation();
            m_pSerialPort->addItem(port);

            if (port == m_settings.serialPort())
            {
                serialPortFound = true;
                m_pSerialPort->setCurrentText(port);
            }
        }

        if (serialPortFound == false)
        {
            m_pSerialPort->setCurrentIndex(0);
            m_settings.setSerialPort(ports[0].systemLocation());
        }
    }




/*
	std::list<QString> serialPorts;

	HKEY hSerialCommKey = 0;

	LONG openRegKeyResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hSerialCommKey);
	if (openRegKeyResult ==  ERROR_SUCCESS)
	{
		LONG enumResult = 0;
		int valueIndex = 0;
		do 
		{
			wchar_t valueName[1024];
			wchar_t value[1024];
			DWORD valueSize = sizeof(value);
			//DWORD valueNameSize = sizeof(valueName);
			DWORD valueType = REG_SZ;

			enumResult = ::RegEnumValue(hSerialCommKey, valueIndex, valueName, &valueSize, NULL, &valueType, (LPBYTE)value, &valueSize);
			if (enumResult == ERROR_SUCCESS)
			{
				if (valueType == REG_SZ && valueSize + 1 < sizeof(value))
				{
					QString s = "\\\\.\\" + QString::fromWCharArray(value);
					serialPorts.push_back(s);
				}
			}

			valueIndex ++;
		}
		while (enumResult == ERROR_SUCCESS);

		::RegCloseKey(hSerialCommKey);
		hSerialCommKey = 0;
	}
	else
	{
		// Cannot enumerate comport, just add 256 com ports
		//
		for (int i = 0; i < 255; i++)
		{
			QString s = "\\\\.\\COM" + QString().setNum(i);
			serialPorts.push_back(s);
		}
	}
	
	// ComPort 
	//
	m_pSerialPort = new QComboBox();
	m_pSerialPortLabel = new QLabel(tr("&Serial Port"));
	m_pSerialPortLabel->setBuddy(m_pSerialPort);

	for (auto it = serialPorts.begin(); it != serialPorts.end(); ++it)
	{
		m_pSerialPort->addItem(*it);
	}

	m_pSerialPort->setCurrentText(m_settings.serialPort());
	*/

	// ShowDebugInfo Check Box
	//
	m_pUseMultipleUartProtocol = new QCheckBox();
	m_pUseMultipleUartProtocol->setText(tr("Use Multiple-UART Protocol if possible"));
	m_pUseMultipleUartProtocol->setChecked(m_settings.useMultipleUartProtocol());

	m_pShowDebugInfo = new QCheckBox();
	m_pShowDebugInfo->setText(tr("Show debug information"));
	m_pShowDebugInfo->setChecked(m_settings.showDebugInfo());

	m_pVerify = new QCheckBox();
	m_pVerify->setText(tr("Verify written data"));
	m_pVerify->setChecked(m_settings.verify());


	// ExpertMode Check Box
	//
	m_pExpertMode = new QCheckBox();
	m_pExpertMode->setText(tr("Expert mode"));
	m_pExpertMode->setChecked(m_settings.expertMode());

	// Server
	//
	m_pServer = new QLineEdit();
	m_pServer->setInputMask("000.000.000.000");
	m_pServer->setText(m_settings.server());
	m_pServerLabel = new QLabel(tr("Server"));

	// Server username
	//
	m_pServerUsername = new QLineEdit();
	m_pServerUsername->setText(m_settings.serverUsername());
	m_pServerUsernameLabel = new QLabel(tr("Username"));

	// Server password
	//
	m_pServerPassword = new QLineEdit();
	m_pServerPassword->setEchoMode(QLineEdit::Password);
	m_pServerPassword->setText(m_settings.serverPassword());
	m_pServerPasswordLabel = new QLabel(tr("Password"));

	// Ok buttons
	//
	m_pOkButton = new QPushButton(tr("Ok"));

	// Cancel button
	//
	m_pCancelButton = new QPushButton(tr("Cancel"));

	//
	// Layouts
	//

	// Left Layout (Edit controls)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();

	QGridLayout* pLeftGridLayout = new QGridLayout();
	pLeftLayout->addLayout(pLeftGridLayout);

	pLeftGridLayout->addWidget(m_pSerialPortLabel, 0, 0);
	pLeftGridLayout->addWidget(m_pSerialPort, 0, 1, 1, 3);

	pLeftGridLayout->addWidget(m_pUseMultipleUartProtocol, 1, 0, 1, 3);
	pLeftGridLayout->addWidget(m_pShowDebugInfo, 2, 0, 1, 3);
	pLeftGridLayout->addWidget(m_pVerify, 3, 0, 1, 3);
	pLeftGridLayout->addWidget(m_pExpertMode, 4, 0, 1, 3);

	pLeftGridLayout->addWidget(m_pServerLabel, 5, 0);
	pLeftGridLayout->addWidget(m_pServer, 5, 1, 1, 3);

	pLeftGridLayout->addWidget(m_pServerUsernameLabel, 6, 0);
	pLeftGridLayout->addWidget(m_pServerUsername, 6, 1, 1, 3);

	pLeftGridLayout->addWidget(m_pServerPasswordLabel, 7, 0);
	pLeftGridLayout->addWidget(m_pServerPassword, 7, 1, 1, 3);

	pLeftLayout->addStretch();
		
	// Right Layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();
	pRightLayout->addWidget(m_pOkButton);
	pRightLayout->addWidget(m_pCancelButton);
	pRightLayout->addStretch();
		
	// Main, dialog layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout, 1);
	pMainLayout->addLayout(pRightLayout);
	
	setLayout(pMainLayout);

	setMinimumSize(static_cast<int>(sizeHint().width() * 1.25), static_cast<int>(sizeHint().height() * 1.0));

	// --
	//
	connect(m_pOkButton, &QPushButton::clicked, this, &QDialog::accept);
	connect(m_pCancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(m_pSerialPort, &QComboBox::currentTextChanged, this, &SettingsForm::currentSerialPortChanged);
	
	connect(m_pUseMultipleUartProtocol, &QCheckBox::stateChanged, this, &SettingsForm::useMultipleUartProtocolChanged);
	connect(m_pShowDebugInfo, &QCheckBox::stateChanged, this, &SettingsForm::showDebugInfoChanged);
	connect(m_pVerify, &QCheckBox::stateChanged, this, &SettingsForm::verifyChanged);
	connect(m_pExpertMode, &QCheckBox::stateChanged, this, &SettingsForm::expertModeChanged);

	return;
}

SettingsForm::~SettingsForm()
{

}

const Settings& SettingsForm::settings() const
{
	return m_settings;
}

void SettingsForm::currentSerialPortChanged(const QString & text)
{
	m_settings.setSerialPort(text);
}

void SettingsForm::useMultipleUartProtocolChanged(int state)
{
	m_settings.setUseMultipleUartProtocol(state == static_cast<int>(Qt::Checked));
}

void SettingsForm::showDebugInfoChanged(int state)
{
	m_settings.setShowDebugInfo(state == static_cast<int>(Qt::Checked));
}

void SettingsForm::verifyChanged(int state)
{
	m_settings.setVerify(state == static_cast<int>(Qt::Checked));
}

void SettingsForm::expertModeChanged(int state)
{
	m_settings.setExpertMode(state == static_cast<int>(Qt::Checked));
}

void SettingsForm::accept()
{
	m_settings.setServer(m_pServer->text());
	m_settings.setServerUsername(m_pServerUsername->text());
	m_settings.setServerPassword(m_pServerPassword->text());

	QDialog::accept();
}
