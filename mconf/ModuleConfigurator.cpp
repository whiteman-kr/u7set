#include "Stable.h"
#include "ModuleConfigurator.h"
#include "../lib/Configurator.h"
#include "SettingsForm.h"
#include "DiagTabPage.h"
#include "ApplicationTabPage.h"
#include "../lib/Ui/DialogAbout.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

ModuleConfigurator::ModuleConfigurator(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	qDebug() << "GUI thread id is " << QThread::currentThreadId();

	m_settings.load();


	// Tab widget
	//
	m_tabWidget = new QTabWidget(this);
	
	ApplicationTabPage* appTabPage = new ApplicationTabPage(m_settings.expertMode());
	DiagTabPage* diagTabPage = new DiagTabPage();

	m_tabWidget->addTab(appTabPage, "Output Bitstream Files");
	m_tabWidget->addTab(diagTabPage, "Service Information");

	connect(m_tabWidget, &QTabWidget::currentChanged, [this](int tabIndex)
	{
		if (m_pEraseButton != nullptr)
		{
			m_pEraseButton->setEnabled(tabIndex == 0);
		}
	});

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
	if (m_settings.expertMode() == true)
	{
		m_pEraseButton = new QPushButton(tr("&Erase"));
	}

	m_pCancelButton = new QPushButton(tr("Cancel"));
	m_pCancelButton->setEnabled(false);

	// Show setting dialog
	//
	m_pSettingsButton = new QPushButton(tr("&Settings..."));

	// Clear Log button
	//
	m_pClearLogButton = new QPushButton(tr("Clear Log"));

	// About button
	//
	m_pAboutButton = new QPushButton(tr("About mconf..."));

	// About Qt button
	//
	m_pAboutQtButton = new QPushButton(tr("About Qt..."));

	//
	// Layouts
	//

	// Left Layout (Edit controls)
	//

	m_pSplitter = new QSplitter();
	m_pSplitter->addWidget(m_tabWidget);
	m_pSplitter->addWidget(m_pLog);
	m_pSplitter->setChildrenCollapsible(false);
		
	// Right Layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();
	pRightLayout->addWidget(m_pConfigureButton);
	pRightLayout->addWidget(m_pReadButton);

	if (m_pEraseButton != nullptr)
	{
		pRightLayout->addWidget(m_pEraseButton);
	}

	pRightLayout->addWidget(m_pCancelButton);

	pRightLayout->addStretch();

	pRightLayout->addWidget(m_pSettingsButton);
	pRightLayout->addWidget(m_pClearLogButton);
		
	pRightLayout->addWidget(m_pAboutQtButton);
	pRightLayout->addWidget(m_pAboutButton);

	// Main, dialog layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_pSplitter);
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

	connect(m_pCancelButton, &QAbstractButton::clicked, this, &ModuleConfigurator::cancelClicked);

	connect(m_pSettingsButton, &QAbstractButton::clicked, this, &ModuleConfigurator::settingsClicked);
	connect(m_pClearLogButton, &QAbstractButton::clicked, this, &ModuleConfigurator::clearLogClicked);

	connect(m_pAboutQtButton, &QAbstractButton::clicked, this, &ModuleConfigurator::aboutQtClicked);
	connect(m_pAboutButton, &QAbstractButton::clicked, this, &ModuleConfigurator::aboutClicked);

	// Logic
	//
	qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");
	qRegisterMetaType<std::vector<int>>("std::vector<int>");
	qRegisterMetaType<std::optional<std::vector<int>>>("std::optional<std::vector<int>>");

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
	connect(m_pConfigurator, &Configurator::operationFinished, appTabPage, &ApplicationTabPage::enableControls);
	connect(m_pConfigurator, &Configurator::detectSubsystemComplete, appTabPage, &ApplicationTabPage::detectSubsystemComplete);
	connect(m_pConfigurator, &Configurator::detectUartsComplete, appTabPage, &ApplicationTabPage::detectUartsComplete);

	connect(m_pConfigurator, &Configurator::communicationReadFinished, this, &ModuleConfigurator::communicationReadFinished);

	connect(appTabPage, &ApplicationTabPage::loadBinaryFile, m_pConfigurator, &Configurator::loadBinaryFile);
	connect(appTabPage, &ApplicationTabPage::detectSubsystem, m_pConfigurator, &Configurator::detectSubsystem);
	connect(appTabPage, &ApplicationTabPage::detectUarts, m_pConfigurator, &Configurator::detectUarts);

	connect(m_pConfigurator, &Configurator::loadBinaryFileHeaderComplete, appTabPage, &ApplicationTabPage::loadBinaryFileHeaderComplete);
	connect(m_pConfigurator, &Configurator::uartOperationStart, appTabPage, &ApplicationTabPage::uartOperationStart);
	connect(m_pConfigurator, &Configurator::uploadFirmwareComplete, appTabPage, &ApplicationTabPage::uploadComplete);
	
	connect(m_pConfigurationThread, &QThread::finished, m_pConfigurator, &QObject::deleteLater);
	
	m_pConfigurator->moveToThread(m_pConfigurationThread);

	m_pConfigurationThread->start();

	emit setCommunicationSettings(m_settings.serialPort(), m_settings.showDebugInfo(), m_settings.verify());

	// Start Timer
	//
	m_logTimerId = startTimer(2);

	setMinimumSize(1280, 768);

	// Restore window and splitter state
	//
	QByteArray data = QSettings().value("m_splitterState").toByteArray();
	if (data.isEmpty() == false)
	{
		m_pSplitter->restoreState(data);
	}
	else
	{
		m_pSplitter->setStretchFactor(0, 30);
		m_pSplitter->setStretchFactor(1, 70);
	}


	data = QSettings().value("m_windowState").toByteArray();
	if (data.isEmpty() == false)
	{
		restoreState(data);
	}

	data = QSettings().value("m_windowGeometry").toByteArray();
	if (data.isEmpty() == false)
	{
		restoreGeometry(data);
	}

	// --
	//
    theLog.writeMessage(tr("Programm is started"));
	theLog.writeMessage(tr("Version %1").arg(qApp->applicationVersion()));

#ifdef GITLAB_CI_BUILD
	theLog.writeMessage(tr("Commit SHA: %1").arg(CI_COMMIT_SHA));
	theLog.writeMessage(tr("Branch: %1").arg(CI_BUILD_REF_SLUG));
	theLog.writeMessage(tr("Build Date: %1").arg(BUILD_DATE));
	theLog.writeMessage(tr("Build Host: %1").arg(COMPUTERNAME));
#else
#endif

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

	// Save window and splitter state
	//
	QSettings().setValue("m_splitterState", m_pSplitter->saveState());
	QSettings().setValue("m_windowState", saveState());
	QSettings().setValue("m_windowGeometry", saveGeometry());
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
			theLog.writeMessage(tr("Writing service information..."));

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

			if (page->selectedSubsystem().isEmpty() == true)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Subsystem is not selected."));
				return;
			}

			std::optional<std::vector<int>> selectedUarts = page->selectedUarts();

			if (selectedUarts.has_value() == true && selectedUarts.value().empty() == true)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Please select at least one UartID."));
				return;
			}

			theLog.writeMessage("");
			theLog.writeMessage(tr("Writing configuration..."));

			// send write in commuinication thread...
			//
			disableControls();

			emit writeConfData(page->configuration(), page->selectedSubsystem(), selectedUarts);
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
			theLog.writeMessage(tr("Reading service information..."));

			// Read
			//
			disableControls();

			emit readServiceInformation(0);
		}

		if (dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			ApplicationTabPage* page = dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget());

			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));

			if (fileName.isEmpty() == true)
			{
				return;
			}

			std::optional<std::vector<int>> selectedUarts = page->selectedUarts();

			if (selectedUarts.has_value() == true && selectedUarts.value().empty() == true)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Please select at least one UartID."));
				return;
			}

			theLog.writeMessage("");
			theLog.writeMessage(tr("Reading configuration to file %1...").arg(fileName));

			// Read
			//
			disableControls();

			emit readFirmware(fileName, selectedUarts);
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
	mb.setIcon(QMessageBox::Warning);
	
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

		if (dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget()) != nullptr)
		{
			ApplicationTabPage* page = dynamic_cast<ApplicationTabPage*>(m_tabWidget->currentWidget());

			std::optional<std::vector<int>> selectedUarts = page->selectedUarts();

			if (selectedUarts.has_value() == true && selectedUarts.value().empty() == true)
			{
				QMessageBox::critical(this, qApp->applicationName(), tr("Please select at least one UartID."));
				return;
			}

			disableControls();

			emit eraseFlashMemory(0, selectedUarts);
		}
		else
		{
			Q_ASSERT(false);
			throw (tr("Attempt to erase service information"));
		}
	}
	catch(QString message)
	{
        theLog.writeError(message);
		return;
	}
	
	return;
}

void ModuleConfigurator::cancelClicked()
{
	m_pConfigurator->cancelOperation();

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

void ModuleConfigurator::aboutQtClicked()
{
	QMessageBox::aboutQt(this, qAppName());
	return;
}

void ModuleConfigurator::aboutClicked()
{
	QString text = qApp->applicationName() + tr(" allows user to upload firmware to flash memory of logic modules.<br>");
	DialogAbout::show(this, text, ":/Images/Images/Logo.png");

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
	
	if (m_tabWidget->currentIndex() == 0)
	{
		if (m_pEraseButton != nullptr)
		{
			m_pEraseButton->setEnabled(false);
		}

		m_pCancelButton->setEnabled(true);
	}

	m_pSettingsButton->setEnabled(false);
	m_pClearLogButton->setEnabled(false);
}

void ModuleConfigurator::enableControls()
{
	m_tabWidget->setEnabled(true);

	m_pReadButton->setEnabled(true);
	m_pConfigureButton->setEnabled(true);

	if (m_tabWidget->currentIndex() == 0)
	{
		if (m_pEraseButton != nullptr)
		{
			m_pEraseButton->setEnabled(true);
		}

		m_pCancelButton->setEnabled(false);
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

