#include "Stable.h"
#include "ModuleConfigurator.h"
#include "../lib/Configurator.h"
#include "SettingsForm.h"
#include "DiagTabPage.h"
#include "ApplicationTabPage.h"

ModuleConfigurator::ModuleConfigurator(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	qDebug() << "GUI thread id is " << QThread::currentThreadId();

	m_settings.load();


	// Tab widget
	//
	m_tabWidget = new QTabWidget(this);
	
	ApplicationTabPage* appTabPage = new ApplicationTabPage();
	DiagTabPage* diagTabPage = new DiagTabPage();

	m_tabWidget->addTab(appTabPage, "Output Bitstream Files");
	m_tabWidget->addTab(diagTabPage, "Service Information");

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
	pMainLayout->addLayout(pLeftLayout, 1);
	pMainLayout->addWidget(m_pLog, 2);
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

	m_pConfigurator = new Configurator(m_settings.serialPort(), &theLog);
	m_pConfigurationThread = new QThread(this);
	
	connect(this, &ModuleConfigurator::setCommunicationSettings, m_pConfigurator, &Configurator::setSettings);
	
	connect(this, &ModuleConfigurator::readServiceInformation, m_pConfigurator, &Configurator::readServiceInformation);
	connect(this, &ModuleConfigurator::readFirmware, m_pConfigurator, &Configurator::readFirmware);

	//connect(this, SIGNAL(writeDiagData(quint32, QDate, quint32, quint32)), m_pConfigurator, SLOT(writeDiagData(quint32, QDate, quint32, quint32)));
	connect(this, &ModuleConfigurator::writeConfData, m_pConfigurator, &Configurator::uploadFirmware);
	connect(this, &ModuleConfigurator::writeDiagData, m_pConfigurator, &Configurator::uploadServiceInformation);	// Template version in 5.0.1 has a bug, will be resolved in 5.0.2
	connect(this, &ModuleConfigurator::eraseFlashMemory, m_pConfigurator, &Configurator::eraseFlashMemory);
	
	connect(m_pConfigurator, &Configurator::operationStarted, this, &ModuleConfigurator::disableControls);
	connect(m_pConfigurator, &Configurator::operationFinished, this, &ModuleConfigurator::enableControls);
	connect(m_pConfigurator, &Configurator::communicationReadFinished, this, &ModuleConfigurator::communicationReadFinished);

	connect(appTabPage, &ApplicationTabPage::loadBinaryFile, m_pConfigurator, &Configurator::loadBinaryFile);

	connect(m_pConfigurator, &Configurator::loadBinaryFileHeaderComplete, appTabPage, &ApplicationTabPage::loadBinaryFileHeaderComplete);
	connect(m_pConfigurator, &Configurator::uploadFirmwareComplete, appTabPage, &ApplicationTabPage::uploadComplete);
	
	connect(m_pConfigurationThread, &QThread::finished, m_pConfigurator, &QObject::deleteLater);
	
	m_pConfigurator->moveToThread(m_pConfigurationThread);

	m_pConfigurationThread->start();

	emit setCommunicationSettings(m_settings.serialPort(), m_settings.showDebugInfo(), m_settings.verify());

	// Start Timer
	//
	m_logTimerId = startTimer(2);

	setMinimumSize(1280, 768);

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
        theLog.isEmpty() == false &&
		m_pLog != nullptr)
	{
		std::list<OutputLogItem> messages;
		for (int i = 0; i < 30 && theLog.isEmpty() == false; i++)
		{
            messages.push_back(theLog.popMessages());
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

	QString s = logItem.toHtml();
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
				QMessageBox::critical(this, qApp->applicationName(), tr("Invalid FactoryNo."));
				return;
			}

            if (page->isFirmwareCrcValid() == false)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Invalid Firmware CRC."));
				return;
			}


			uint32_t factoryNo = page->factoryNo();
			QDate manufactureDate = page->manufactureDate();
            uint32_t firmwareCrc = page->firmwareCrc();

			if (factoryNo == 0)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Invalid FactoryNo."));
				return;
			}

			// --
			//
            theLog.writeMessage("");
            theLog.writeMessage(tr("Writing configuration..."));

			// send write in commuinication thread...
			//
			disableControls();

            emit writeDiagData(factoryNo, manufactureDate, firmwareCrc);
		}

		// Write diag info, like factory no, crc, etc
		//
		if (dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			ApplicationTabPage* page = dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget());

			if (page->isFileLoaded() == false)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Configuration file was not loaded."));
				return;
			}

			if (page->subsystemId().isEmpty() == true)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Subsystem is not selected."));
				return;
			}

			// send write in commuinication thread...
			//
			disableControls();

			emit writeConfData(page->configuration(), page->subsystemId());
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
            theLog.writeMessage(tr("Reading configuration..."));

			// Read
			//
			disableControls();

			emit readServiceInformation(0);
		}

		if (dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			//ApplicationTabPage* page = dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget());

			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));

			if (fileName.isEmpty() == true)
			{
				return;
			}

			theLog.writeMessage("");
			theLog.writeMessage(tr("Reading firmware to file %1...").arg(fileName));

			// Read
			//
			disableControls();

			emit readFirmware(fileName);
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
        theLog.writeMessage(tr("Erasing flash memory..."));
		
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

		emit setCommunicationSettings(m_settings.serialPort(), m_settings.showDebugInfo(), m_settings.verify());
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

void ModuleConfigurator::communicationReadFinished(int protocolVersion, std::vector<quint8> data)
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

                page->setFirmwareCrc(serviceDataVersion.firmwareCrc());
			}
			break;
		default:
			assert(false);
		}
	}

	return;
}

