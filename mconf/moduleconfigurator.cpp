#include "stable.h"
#include "moduleconfigurator.h"
#include "configurator.h"
#include "settingsform.h"
#include "diagtabpage.h"
#include "applicationtabpage.h"

ModuleConfigurator::ModuleConfigurator(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	qDebug() << "GUI thread id is " << QThread::currentThreadId();

	m_settings.load();


	// Tab widget
	//
	m_tabWidget = new QTabWidget(this);
	
	DiagTabPage* diagTabPage = new DiagTabPage();
	ApplicationTabPage* appTabPage = new ApplicationTabPage();

	m_tabWidget->addTab(diagTabPage, "Diag");
	m_tabWidget->addTab(appTabPage, "Application");
	
	// Log
	//
	m_pLog = new QTextEdit();
	m_pLog->setReadOnly(true);
	m_pLog->document()->setUndoRedoEnabled(false);
	m_pLog->document()->setMaximumBlockCount(600);
	
	
	// Read data from module button
	//
	m_pReadButton = new QPushButton(tr("&Read"));

	// Write Data to module (Configure)
	//
	m_pConfigureButton = new QPushButton(tr("&Configure"));
	m_pConfigureButton->setDefault(true);

	// Erase flash memory
	//
	m_pEraseButton = new QPushButton(tr("&Erase"));

	// Show setting dialog
	//
	m_pSettingsButton = new QPushButton(tr("&Settings..."));

	// Clear Log button
	//
	m_pClearLogButton = new QPushButton(tr("Clear Log"));

	//
	// Layouts
	//

	// Left Layout (Edit controls)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	
	pLeftLayout->addWidget(m_tabWidget);
	pLeftLayout->addWidget(m_pLog, 1);
		
	// Right Layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();
	pRightLayout->addWidget(m_pConfigureButton);
	pRightLayout->addWidget(m_pReadButton);

	if (m_settings.expertMode() == true)
	{
		pRightLayout->addWidget(m_pEraseButton);
	}

	pRightLayout->addStretch();

	pRightLayout->addWidget(m_pSettingsButton);
	pRightLayout->addWidget(m_pClearLogButton);
		
	// Main, dialog layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);
	
	QWidget* pCentralWidget = new QWidget();

	pCentralWidget->setLayout(pMainLayout);
	setCentralWidget(pCentralWidget);

	// GUI
	//
	connect(m_pConfigureButton, &QAbstractButton::clicked, this, &ModuleConfigurator::configureClicked);
	connect(m_pReadButton, &QAbstractButton::clicked, this, &ModuleConfigurator::readClicked);

	if (m_settings.expertMode() == true)
	{
		connect(m_pEraseButton, &QAbstractButton::clicked, this, &ModuleConfigurator::eraseClicked);
	}
	
	connect(m_pSettingsButton, &QAbstractButton::clicked, this, &ModuleConfigurator::settingsClicked);
	connect(m_pClearLogButton, &QAbstractButton::clicked, this, &ModuleConfigurator::clearLogClicked);

	// Logic
	//
	qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");

	m_pConfigurator = new Configurator(m_settings.serialPort());
	m_pConfigurationThread = new QThread(this);
	
	connect(this, &ModuleConfigurator::setCommunicationSettings, m_pConfigurator, &Configurator::setSettings);
	
	connect(this, &ModuleConfigurator::readConfiguration, m_pConfigurator, &Configurator::readConfiguration);
	connect(this, SIGNAL(writeDiagData(quint32, QDate, quint32, quint32)), m_pConfigurator, SLOT(writeDiagData(quint32, QDate, quint32, quint32)));
	connect(this, &ModuleConfigurator::writeConfData, m_pConfigurator, &Configurator::writeConfData);
	//connect(this, &ModuleConfigurator::writeDiagData, m_pConfigurator, &Configurator::writeDiagData);	// Template version in 5.0.1 has a bug, will be resolved in 5.0.2
	connect(this, &ModuleConfigurator::eraseFlashMemory, m_pConfigurator, &Configurator::eraseFlashMemory);
	
	connect(m_pConfigurator, &Configurator::communicationStarted, this, &ModuleConfigurator::disableControls);
	connect(m_pConfigurator, &Configurator::communicationFinished, this, &ModuleConfigurator::enableControls);
	connect(m_pConfigurator, &Configurator::communicationReadFinished, this, &ModuleConfigurator::communicationReadFinished);
	
	connect(m_pConfigurationThread, &QThread::finished, m_pConfigurator, &QObject::deleteLater);
	
	m_pConfigurator->moveToThread(m_pConfigurationThread);

	m_pConfigurationThread->start();

	emit setCommunicationSettings(m_settings.serialPort(), m_settings.showDebugInfo());

	// Start Timer
	//
	m_logTimerId = startTimer(10);

	// --
	//
	theLog.writeMessage(tr("Programm is started"));
	return;
}

ModuleConfigurator::~ModuleConfigurator()
{
	killTimer(m_logTimerId);

	if (m_pConfigurationThread != nullptr)
	{
		m_pConfigurationThread->exit();
		m_pConfigurationThread->wait(10000);
	}
}

void ModuleConfigurator::timerEvent(QTimerEvent* pTimerEvent)
{
	if (pTimerEvent == nullptr)
	{
		assert(pTimerEvent != nullptr);
		return;
	}

	if (pTimerEvent->timerId() == m_logTimerId && 
		theLog.windowMessageListEmpty() == false && 
		m_pLog != nullptr)
	{
		std::list<OutputLogItem> messages;
		for (int i = 0; i < 10 && theLog.windowMessageListEmpty() == false; i++)
		{
			messages.push_back(theLog.popWindowMessages());
		}

		for (auto m = messages.begin(); m != messages.end(); ++m)
		{
			writeLog(*m);
		}
	}

	return;
}

void ModuleConfigurator::writeLog(const OutputLogItem& logItem)
{
	if (m_pLog == nullptr)
	{
		assert(m_pLog != nullptr);
		return;
	}

	// --
	//
	QString color; 
	switch (logItem.level)
	{
	case Message:
		color = "black";
		break;
	case Success:
		color = "green";
		break;
	case Warning:
		color = "orange";
		break;
	case Error:
		color = "red";
		break;
		
	default:
		assert(false);
		color = "black";
	}

	QString s;
	if (logItem.message.isEmpty() == false)
	{
		s = "<font face=\"Verdana\" size=3 color=#808080>" + logItem.time.toString("hh:mm:ss:zzz   ") + "<font color=" + color + ">" + (logItem.bold ? QString("<b>") : QString()) + logItem.message;
	}

	m_pLog->append(s);
	return;
}

void ModuleConfigurator::configureClicked()
{
	assert(m_tabWidget != nullptr);

	try
	{
		// Write diag info, like factory no, crc, etc
		//
		if (dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			DiagTabPage* page = dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget());

			if (page->isFactoryNoValid() == false)
			{
				QMessageBox msgBox(this);
				msgBox.setText(tr("Invalid FactoryNo."));
				msgBox.setInformativeText(tr("Enter valid data."));
				msgBox.exec();
				return;
			}

			if (page->isFirmwareCrc1Valid() == false)
			{
				QMessageBox msgBox(this);
				msgBox.setText(tr("Invalid Firmware CRC1."));
				msgBox.setInformativeText(tr("Enter valid data."));
				msgBox.exec();
				return;
			}

			if (page->isFirmwareCrc2Valid() == false)
			{
				QMessageBox msgBox(this);
				msgBox.setText(tr("Invalid Firmware CRC2."));
				msgBox.setInformativeText(tr("Enter valid data."));
				msgBox.exec();
				return;
			}

			uint32_t factoryNo = page->factoryNo();
			QDate manufactureDate = page->manufactureDate();
			uint32_t firmwareCrc1 = page->firmwareCrc1();
			uint32_t firmwareCrc2 = page->firmwareCrc2();

			if (factoryNo == 0)
			{
				QMessageBox msgBox(this);
				msgBox.setText(tr("Invalid FactoryNo."));
				msgBox.setInformativeText(tr("Enter valid data."));

				msgBox.exec();
				return;
			}

			// --
			//
			theLog.writeMessage("");
			theLog.writeMessage(tr("Writing configuration..."), true);

			// send write in commuinication thread...
			//
			disableControls();

			emit writeDiagData(factoryNo, manufactureDate, firmwareCrc1, firmwareCrc2);
		}

		// Write diag info, like factory no, crc, etc
		//
		if (dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			ApplicationTabPage* page = dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget());

			if (page->isFileLoaded() == false)
			{
				QMessageBox mb(this);
				mb.setText(tr("Configuration file was not loaded."));
				mb.exec();
				return;
			}

			// send write in commuinication thread...
			//
			disableControls();

			emit writeConfData(page->configuration());
		}
	}
	catch(QString message)
	{
		theLog.writeError(message);
		return;
	}

	return;
}

void ModuleConfigurator::readClicked()
{
	try
	{
		// Read diag info, like factory no, crc, etc
		//
		if (dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			//DiagTabPage* page = dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget());

			theLog.writeMessage("");
			theLog.writeMessage(tr("Reading configuration..."), true);

			// Read
			//
			disableControls();

			emit readConfiguration(0);
		}

	}
	catch(QString message)
	{
		theLog.writeError(message);
		return;
	}

	return;
}

void ModuleConfigurator::eraseClicked()
{
	if (m_settings.expertMode() == false)
	{
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("Are you sure you want to erase the flash memory?"));
	mb.setInformativeText(tr("All data will be lost."));
	mb.addButton(tr("Cancel"), QMessageBox::RejectRole);
	mb.addButton(tr("Erase"), QMessageBox::AcceptRole);
	
	if (mb.exec() == QMessageBox::Rejected)
	{
		return;
	}
	
	try
	{
		// --
		//
		theLog.writeMessage("");
		theLog.writeMessage(tr("Erasing flash memory..."), true);
		
		// Read
		//
		disableControls();

		emit eraseFlashMemory(0);
	}
	catch(QString message)
	{
		theLog.writeError(message);
		return;
	}
	
	return;
}

void ModuleConfigurator::settingsClicked()
{
	SettingsForm settingsForm(m_settings, this);
	
	int result = settingsForm.exec();
	if (result == QDialog::Accepted)
	{
		m_settings = settingsForm.settings();
		m_settings.save();

		emit setCommunicationSettings(m_settings.serialPort(), m_settings.showDebugInfo());
	}

	return;
}

void ModuleConfigurator::clearLogClicked()
{
	if (m_pLog == nullptr)
	{
		assert(m_pLog != nullptr);
		return;
	}

	m_pLog->clear();

	return;
}

void ModuleConfigurator::disableControls()
{
	m_tabWidget->setEnabled(false);

	m_pReadButton->setEnabled(false);
	m_pConfigureButton->setEnabled(false);
	
	if (m_pEraseButton != nullptr)
	{
		m_pEraseButton->setEnabled(false);
	}

	m_pSettingsButton->setEnabled(false);
	m_pClearLogButton->setEnabled(false);
}

void ModuleConfigurator::enableControls()
{
	m_tabWidget->setEnabled(true);

	m_pReadButton->setEnabled(true);
	m_pConfigureButton->setEnabled(true);

	if (m_pEraseButton != nullptr)
	{
		m_pEraseButton->setEnabled(true);
	}

	m_pSettingsButton->setEnabled(true);
	m_pClearLogButton->setEnabled(true);
}

void ModuleConfigurator::communicationReadFinished(int protocolVersion, std::vector<uint8_t> data)
{
	// Read diag info, like factory no, crc, etc
	//
	if (dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget()) != nullptr)
	{
		DiagTabPage* page = dynamic_cast<DiagTabPage*>(m_tabWidget->currentWidget());

		//CONF_SERVICE_DATA_V1 serviceDataVersion = *reinterpret_cast<CONF_SERVICE_DATA_V1*>(data.data());

		switch (protocolVersion)
		{
		case 1:
			{
				CONF_SERVICE_DATA_V1 serviceDataVersion = *reinterpret_cast<CONF_SERVICE_DATA_V1*>(data.data());

				page->setFactoryNo(serviceDataVersion.factoryNo());
				page->setManufactureDate(QDate(serviceDataVersion.manufactureYear(), serviceDataVersion.manufactureMonth(), serviceDataVersion.manufactureDay()));

				page->setFirmwareCrc1(serviceDataVersion.firmwareCrc1());
				page->setFirmwareCrc2(serviceDataVersion.firmwareCrc2());
			}
			break;
		default:
			assert(false);
		}
	}

	return;
}

