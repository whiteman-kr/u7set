#include "Stable.h"
#include "UploadTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"
#include "DialogSettingsConfigurator.h"

//
//
// UploadTabPage
//
//


UploadTabPage::UploadTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)//,
{
	assert(dbcontroller != nullptr);

	//
	// Controls
	//

	//Left layout
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();

	pLeftLayout->addWidget(new QLabel(tr("Choose Configuration:")));
	m_pConfigurationCombo = new QComboBox();
	m_pConfigurationCombo->addItem(tr("Debug"), tr("debug"));
	m_pConfigurationCombo->addItem(tr("Release"), tr("release"));
	m_pConfigurationCombo->setCurrentIndex(0);
	connect(m_pConfigurationCombo, &QComboBox::currentTextChanged, this, &UploadTabPage::configurationTypeChanged);
	pLeftLayout->addWidget(m_pConfigurationCombo);

	pLeftLayout->addWidget(new QLabel(tr("Choose Build:")));
	m_pBuildList = new QListWidget();
	connect(m_pBuildList, &QListWidget::currentRowChanged, this, &UploadTabPage::findSubsystemsInBuild);
	pLeftLayout->addWidget(m_pBuildList);

	pLeftLayout->addWidget(new QLabel(tr("Choose Subsystem:")));
	m_pSubsystemList = new QListWidget();
	connect(m_pSubsystemList, &QListWidget::currentRowChanged, this, &UploadTabPage::subsystemChanged);
	pLeftLayout->addWidget(m_pSubsystemList);

	pLeftLayout->addWidget(new QLabel(tr("Firmware Types:")));
	m_pFirmwareListWidget = new QTreeWidget();
	pLeftLayout->addWidget(m_pFirmwareListWidget);

	QStringList l;
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");

	m_pFirmwareListWidget->setColumnCount(l.size());
	m_pFirmwareListWidget->setHeaderLabels(l);

	int il = 0;
	m_pFirmwareListWidget->setColumnWidth(il++, 80);
	m_pFirmwareListWidget->setColumnWidth(il++, 140);
	m_pFirmwareListWidget->setColumnWidth(il++, 140);

	m_pFirmwareListWidget->setRootIsDecorated(false);

	QHBoxLayout* bl = new QHBoxLayout();

	bl->addStretch();

	QPushButton* b = new QPushButton(tr("Reset Upload Counters"));
	bl->addWidget(b);
	connect(b, &QPushButton::clicked, this, &UploadTabPage::resetUartData);

	pLeftLayout->addLayout(bl);

	pLeftLayout->addStretch();

	QWidget* pLeftWidget = new QWidget();
	pLeftWidget->setLayout(pLeftLayout);

	// Log
	//
	m_pLog = new QTextEdit();
	m_pLog->setReadOnly(true);
	m_pLog->document()->setUndoRedoEnabled(false);
	m_pLog->document()->setMaximumBlockCount(600);

	// Buttons
	//
	QHBoxLayout* pButtonsLayout = new QHBoxLayout();

	m_pReadButton = new QPushButton(tr("&Read"));
	pButtonsLayout->addWidget(m_pReadButton);


	if (theSettings.isExpertMode() == true)
	{
		m_pEraseButton = new QPushButton(tr("&Erase"));
		pButtonsLayout->addWidget(m_pEraseButton);
	}

	m_pClearLogButton = new QPushButton(tr("Clear Log"));
	pButtonsLayout->addWidget(m_pClearLogButton);

	m_pSettingsButton = new QPushButton(tr("&Settings..."));
	pButtonsLayout->addWidget(m_pSettingsButton);

	pButtonsLayout->addStretch();

	m_pConfigureButton = new QPushButton(tr("&Configure"));
	m_pConfigureButton->setEnabled(false);
	m_pConfigureButton->setDefault(true);
	pButtonsLayout->addWidget(m_pConfigureButton);

	m_pCancelButton = new QPushButton(tr("Cancel"));
	m_pCancelButton->setEnabled(false);
	pButtonsLayout->addWidget(m_pCancelButton);

	// Right layout
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();
	pRightLayout->addWidget(m_pLog);
	pRightLayout->addLayout(pButtonsLayout);

	QWidget* pRightWidget = new QWidget();
	pRightWidget->setLayout(pRightLayout);

	// --
	//
	m_vsplitter = new QSplitter();

	m_vsplitter->addWidget(pLeftWidget);
	m_vsplitter->addWidget(pRightWidget);

	m_vsplitter->setStretchFactor(0, 2);
	m_vsplitter->setStretchFactor(1, 1);

	m_vsplitter->restoreState(theSettings.m_UploadTabPageSplitterState);

	QHBoxLayout* pMainLayout = new QHBoxLayout(this);

	pMainLayout->addWidget(m_vsplitter);

	setLayout(pMainLayout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &UploadTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &UploadTabPage::projectClosed);

	connect(m_pReadButton, &QAbstractButton::clicked, this, &UploadTabPage::read);
	connect(m_pConfigureButton, &QAbstractButton::clicked, this, &UploadTabPage::upload);
	if (m_pEraseButton != nullptr)
	{
		connect(m_pEraseButton, &QAbstractButton::clicked, this, &UploadTabPage::erase);
	}
	connect(m_pCancelButton, &QAbstractButton::clicked, this, &UploadTabPage::cancel);
	connect(m_pClearLogButton, &QAbstractButton::clicked, this, &UploadTabPage::clearLog);
	connect(m_pSettingsButton, &QAbstractButton::clicked, this, &UploadTabPage::settings);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);




	// Logic
	//
	qRegisterMetaType<std::vector<uint8_t>>("std::vector<uint8_t>");

	m_pConfigurator = new Configurator(theSettings.m_configuratorSerialPort, &m_outputLog);
	m_pConfigurationThread = new QThread(this);

	m_pConfigurator->setVerify(theSettings.m_configuratorVerify);

	connect(this, &UploadTabPage::setCommunicationSettings, m_pConfigurator, &Configurator::setSettings);

	//connect(this, &UploadTabPage::readConfiguration, m_pConfigurator, &Configurator::readConfiguration);
	connect(this, &UploadTabPage::readFirmware, m_pConfigurator, &Configurator::readFirmware);

	connect(this, &UploadTabPage::showConfDataFileInfo, m_pConfigurator, &Configurator::showBinaryFileInfo);
	connect(this, &UploadTabPage::writeConfDataFile, m_pConfigurator, &Configurator::uploadBinaryFile);
	connect(this, &UploadTabPage::eraseFlashMemory, m_pConfigurator, &Configurator::eraseFlashMemory);

	connect(m_pConfigurator, &Configurator::communicationStarted, this, &UploadTabPage::disableControls);
	connect(m_pConfigurator, &Configurator::communicationFinished, this, &UploadTabPage::enableControls);
	connect(m_pConfigurator, &Configurator::communicationReadFinished, this, &UploadTabPage::communicationReadFinished);

	connect(m_pConfigurator, &Configurator::loadHeaderComplete, this, &UploadTabPage::loadHeaderComplete);
	connect(m_pConfigurator, &Configurator::uploadSuccessful, this, &UploadTabPage::uploadSuccessful);

	connect(m_pConfigurationThread, &QThread::finished, m_pConfigurator, &QObject::deleteLater);

	m_pConfigurator->moveToThread(m_pConfigurationThread);

	m_pConfigurationThread->start();

	emit setCommunicationSettings(theSettings.m_configuratorSerialPort, theSettings.m_configuratorShowDebugInfo, theSettings.m_configuratorVerify);

	// Start Timer
	//
	m_logTimerId = startTimer(100);


}

UploadTabPage::~UploadTabPage()
{
	if (m_logTimerId != -1)
	{
		killTimer(m_logTimerId);
	}

	if (m_pConfigurationThread != nullptr)
	{
		m_pConfigurationThread->exit();
		m_pConfigurationThread->wait(10000);
	}

	theSettings.m_UploadTabPageSplitterState = m_vsplitter->saveState();
	theSettings.writeUserScope();
}

bool UploadTabPage::isUploading()
{
	return m_uploading;
}

void UploadTabPage::findProjectBuilds()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	if (isUploading() == true)
	{
		return;
	}

	QString projectName = dbController()->currentProject().projectName();
	if (projectName.isEmpty() == true)
	{
		return;
	}

	QVariant data = m_pConfigurationCombo->currentData();
	if (data.isNull() == true || data.isValid() == false)
	{
		assert(false);
		return;
	}

	QString configurationType = data.toString();
	if (configurationType.isEmpty() == true)
	{
		assert(false);
		return;
	}

	m_buildSearchPath = QString("%1%2%3-%4").arg(theSettings.buildOutputPath()).arg(QDir::separator()).arg(projectName).arg(configurationType);

	QStringList builds = QDir(m_buildSearchPath).entryList(QStringList("build*"), QDir::Dirs|QDir::NoSymLinks);

	m_pBuildList->clear();
	m_pBuildList->addItems(builds);

	m_pSubsystemList->clear();

	if (m_currentBuildIndex != -1 && m_currentBuildIndex < m_pBuildList->count())
	{
		m_pBuildList->setCurrentRow(m_currentBuildIndex);
	}
}

void UploadTabPage::configurationTypeChanged(const QString& s)
{
	Q_UNUSED(s);

	findProjectBuilds();
}


void UploadTabPage::findSubsystemsInBuild(int index)
{
	Q_UNUSED(index);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QListWidgetItem* item = m_pBuildList->currentItem();
	if (item == nullptr)
	{
		return;
	}

	m_currentBuild = item->text();

	m_currentBuildIndex = m_pBuildList->currentRow();

	QString buildPath = m_buildSearchPath + QDir::separator() + m_currentBuild;

	QStringList subsystems = QDir(buildPath).entryList(QStringList("*"),
									 QDir::Dirs | QDir::NoSymLinks);

	m_pSubsystemList->clear();

	// add only directories that contain binary files
	//

	for (QString s : subsystems)
	{
		QDir subsystemPath = buildPath + QDir::separator() + s;

		QStringList binaryFiles = QDir(subsystemPath).entryList(QStringList() << "*.bts",
										 QDir::Files| QDir::NoSymLinks);

		if (binaryFiles.isEmpty() == false)
		{
			m_pSubsystemList->addItem(s);
		}
	}

	if (m_currentSubsystemIndex != -1 && m_currentSubsystemIndex < m_pSubsystemList->count())
	{
		m_pSubsystemList->setCurrentRow(m_currentSubsystemIndex);
	}
}

void UploadTabPage::subsystemChanged(int index)
{
	Q_UNUSED(index);

	//clearLog();

	m_currentFileName.clear();

	QListWidgetItem* item = m_pSubsystemList->currentItem();
	if (item == nullptr)
	{
		m_pConfigureButton->setEnabled(false);
		return;
	}

	m_pConfigureButton->setEnabled(true);
	m_currentSubsystem = item->text();
	m_currentSubsystemIndex = m_pSubsystemList->currentRow();

	QString searchPath = m_buildSearchPath + QDir::separator() + m_currentBuild + QDir::separator() + m_currentSubsystem;

	QStringList binaryFiles = QDir(searchPath).entryList(QStringList() << "*.bts",
									 QDir::Files| QDir::NoSymLinks);

	if (binaryFiles.isEmpty() == true)
	{
		m_outputLog.writeError(tr("No Output Bitstream files found in %1!").arg(searchPath));
		return;
	}

	if (binaryFiles.size() > 1)
	{
		m_outputLog.writeError(tr("More than one Output Bitstream file found in %1!").arg(searchPath));
		return;
	}

	m_currentFileName = searchPath + QDir::separator() + binaryFiles[0];

	clearUartData();

	emit showConfDataFileInfo(m_currentFileName);

}

void UploadTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}


void UploadTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void UploadTabPage::projectClosed()
{
	m_pBuildList->clear();
	m_pSubsystemList->clear();

	this->setEnabled(false);
	return;
}

void UploadTabPage::read()
{
	clearLog();

	try
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"));

		if (fileName.isEmpty() == true)
		{
			return;
		}

		m_outputLog.writeMessage("");
		m_outputLog.writeMessage(tr("Reading firmware to file %1...").arg(fileName));

		emit readFirmware(fileName);

	}
	catch(QString message)
	{
		m_outputLog.writeError(message);
		return;
	}
}

void UploadTabPage::upload()
{
	if (m_currentFileName.isEmpty())
	{
		m_outputLog.writeError(tr("No Output Bitstream File selected!"));
		return;
	}

	try
	{

		emit writeConfDataFile(m_currentFileName);
	}
	catch(QString message)
	{
		m_outputLog.writeError(message);
		return;
	}
}

void UploadTabPage::erase()
{
	if (theSettings.isExpertMode() == false)
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
		m_outputLog.writeMessage("");
		m_outputLog.writeMessage(tr("Erasing flash memory..."));

		// Read
		//
		//disableControls();

		emit eraseFlashMemory(0);
	}
	catch(QString message)
	{
		m_outputLog.writeError(message);
		return;
	}

}

void UploadTabPage::cancel()
{
	m_pConfigurator->cancelOperation();
}

void UploadTabPage::clearLog()
{
	if (m_pLog == nullptr)
	{
		assert(m_pLog != nullptr);
		return;
	}

	m_pLog->clear();
}


void UploadTabPage::settings()
{
	DialogSettingsConfigurator dlg(this);
	if (dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	emit setCommunicationSettings(theSettings.m_configuratorSerialPort, theSettings.m_configuratorShowDebugInfo, theSettings.m_configuratorVerify);
}

void UploadTabPage::disableControls()
{
	m_uploading = true;

	bool enable = false;

	m_pReadButton->setEnabled(enable);
	m_pConfigureButton->setEnabled(enable);
	if (m_pEraseButton)
	{
		m_pEraseButton->setEnabled(enable);
	}
	m_pSettingsButton->setEnabled(enable);
	m_pCancelButton->setEnabled(!enable);
	m_pClearLogButton->setEnabled(enable);

}

void UploadTabPage::enableControls()
{
	m_uploading = false;

	bool enable = true;

	m_pReadButton->setEnabled(enable);
	m_pConfigureButton->setEnabled(enable);
	if (m_pEraseButton)
	{
		m_pEraseButton->setEnabled(enable);
	}
	m_pSettingsButton->setEnabled(enable);
	m_pCancelButton->setEnabled(!enable);
	m_pClearLogButton->setEnabled(enable);
}

void UploadTabPage::communicationReadFinished()
{

}


void UploadTabPage::writeLog(const OutputLogItem& logItem)
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

void UploadTabPage::clearUartData()
{
	m_pFirmwareListWidget->clear();
}

void UploadTabPage::resetUartData()
{
	int count = m_pFirmwareListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pFirmwareListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setData(2, Qt::UserRole, 0);
		item->setText(2, "0");
	}
}

void UploadTabPage::loadHeaderComplete(std::vector<UartPair> uartList)
{
	clearUartData();

	for (auto it : uartList)
	{
		int uartID = it.first;
		QString uartType = it.second;

		QStringList l;
		l << tr("%1h").arg(QString::number(uartID, 16));
		l << uartType;
		l << "0";

		QTreeWidgetItem* item = new QTreeWidgetItem(l);

		item->setData(0, Qt::UserRole, uartID);
		item->setData(2, Qt::UserRole, 0);

		m_pFirmwareListWidget->addTopLevelItem(item);
	}

	m_pFirmwareListWidget->sortByColumn(0, Qt::AscendingOrder);
}

void UploadTabPage::uploadSuccessful(int uartID)
{
	int count = m_pFirmwareListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pFirmwareListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int itemUartId = item->data(0, Qt::UserRole).toInt();
		if (uartID == itemUartId)
		{
			int itemUploadCount = item->data(2, Qt::UserRole).toInt();
			itemUploadCount++;

			item->setData(2, Qt::UserRole, itemUploadCount);
			item->setText(2, QString::number(itemUploadCount));

			break;
		}
	}

	return;
}

void UploadTabPage::timerEvent(QTimerEvent* pTimerEvent)
{
	if (pTimerEvent == nullptr)
	{
		assert(pTimerEvent != nullptr);
		return;
	}

	if (pTimerEvent->timerId() == m_logTimerId &&
		m_outputLog.isEmpty() == false &&
		m_pLog != nullptr)
	{
		std::list<OutputLogItem> messages;
		for (int i = 0; i < 30 && m_outputLog.isEmpty() == false; i++)
		{
			messages.push_back(m_outputLog.popMessages());
		}

		for (auto m = messages.begin(); m != messages.end(); ++m)
		{
			writeLog(*m);
		}
	}

	return;
}
