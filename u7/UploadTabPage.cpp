#include "UploadTabPage.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "DialogSettingsConfigurator.h"
#include "../lib/DbController.h"


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

	// Create Build list widget

	pLeftLayout->addWidget(new QLabel(tr("Choose the Build:")));
	m_pBuildList = new QListWidget();
	connect(m_pBuildList, &QListWidget::currentRowChanged, this, &UploadTabPage::buildChanged);
	pLeftLayout->addWidget(m_pBuildList);

	// Create Subsystems list widget

	pLeftLayout->addWidget(new QLabel(tr("Choose the Subsystem:")));
	m_pSubsystemsListWidget = new QTreeWidget();
	m_pSubsystemsListWidget->setRootIsDecorated(false);
	connect(m_pSubsystemsListWidget, &QTreeWidget::currentItemChanged, this, &UploadTabPage::subsystemChanged);
	pLeftLayout->addWidget(m_pSubsystemsListWidget, 2);

	QStringList l;
	l << tr("Subsystem");

	m_pSubsystemsListWidget->setColumnCount(l.size());
	m_pSubsystemsListWidget->setHeaderLabels(l);

	int il = 0;
	m_pSubsystemsListWidget->setColumnWidth(il++, 100);

	// Create UART list widget

	pLeftLayout->addWidget(new QLabel(tr("Firmware Types:")));
	m_pUartListWidget = new QTreeWidget();
	m_pUartListWidget->setRootIsDecorated(false);
	pLeftLayout->addWidget(m_pUartListWidget);

	l.clear();
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");
	l << tr("Status");

	m_pUartListWidget->setColumnCount(l.size());
	m_pUartListWidget->setHeaderLabels(l);

	il = 0;
	m_pUartListWidget->setColumnWidth(il++, 80);
	m_pUartListWidget->setColumnWidth(il++, 100);
	m_pUartListWidget->setColumnWidth(il++, 100);
	m_pUartListWidget->setColumnWidth(il++, 60);

	//

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

	m_pReadToFileButton = new QPushButton(tr("&Read to File"));
	pButtonsLayout->addWidget(m_pReadToFileButton);

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

	m_pDetectSubsystemButton = new QPushButton(tr("Detect Subsystem"));
	m_pDetectSubsystemButton->setEnabled(false);
	pButtonsLayout->addWidget(m_pDetectSubsystemButton);

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
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &UploadTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &UploadTabPage::projectClosed);

	connect(m_pReadToFileButton, &QAbstractButton::clicked, this, &UploadTabPage::read);
	connect(m_pDetectSubsystemButton, &QAbstractButton::clicked, this, &UploadTabPage::detectSubsystem);

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

	connect(this, &UploadTabPage::readFirmware, m_pConfigurator, &Configurator::readFirmware);
	connect(this, &UploadTabPage::detectSubsystem, m_pConfigurator, &Configurator::detectSubsystem_v1);

	connect(this, &UploadTabPage::loadBinaryFile, m_pConfigurator, &Configurator::loadBinaryFile);
	connect(this, &UploadTabPage::uploadFirmware, m_pConfigurator, &Configurator::uploadFirmware);
	connect(this, &UploadTabPage::eraseFlashMemory, m_pConfigurator, &Configurator::eraseFlashMemory);

	connect(m_pConfigurator, &Configurator::operationStarted, this, &UploadTabPage::disableControls);
	connect(m_pConfigurator, &Configurator::operationFinished, this, &UploadTabPage::enableControls);
	connect(m_pConfigurator, &Configurator::communicationReadFinished, this, &UploadTabPage::communicationReadFinished);

	connect(m_pConfigurator, &Configurator::loadBinaryFileHeaderComplete, this, &UploadTabPage::loadBinaryFileHeaderComplete);
	connect(m_pConfigurator, &Configurator::uartOperationStart, this, &UploadTabPage::uartOperationStart);
	connect(m_pConfigurator, &Configurator::uploadFirmwareComplete, this, &UploadTabPage::uploadComplete);
	connect(m_pConfigurator, &Configurator::detectSubsystemComplete, this, &UploadTabPage::detectSubsystemComplete);

	connect(m_pConfigurationThread, &QThread::finished, m_pConfigurator, &QObject::deleteLater);

	m_pConfigurator->moveToThread(m_pConfigurationThread);

	m_pConfigurationThread->start();

	emit setCommunicationSettings(theSettings.m_configuratorSerialPort, theSettings.m_configuratorShowDebugInfo, theSettings.m_configuratorVerify);

	// Start Timer
	//
	m_logTimerId = startTimer(100);

	return;
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

void UploadTabPage::refreshProjectBuilds()
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

	// Refresh builds list if it is changed
	//

	if (m_currentBuilds != builds)
	{
		m_currentBuilds = builds;

		m_pBuildList->clear();
		m_pBuildList->addItems(m_currentBuilds);

		m_pBuildList->blockSignals(true);
		selectBuild(m_currentBuild);
		m_pBuildList->blockSignals(false);
	}

	// Refresh binary file
	//

	refreshBinaryFile();
}

void UploadTabPage::configurationTypeChanged(const QString& s)
{
	Q_UNUSED(s);

	m_currentBuild.clear();
	m_currentFilePath.clear();

	refreshProjectBuilds();
}


void UploadTabPage::buildChanged(int index)
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

	refreshBinaryFile();
}

void UploadTabPage::subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2)
{
	Q_UNUSED(item1);
	Q_UNUSED(item2);


	QTreeWidgetItem* subsystemItem = m_pSubsystemsListWidget->currentItem();
	if (subsystemItem == nullptr)
	{
		return;
	}

	QString currentSubsystem = subsystemItem->data(columnSubsysId, Qt::UserRole).toString();

	bool ok = false;
	const ModuleFirmware& mf = m_firmware.firmware(currentSubsystem, &ok);
	if (ok == false)
	{
		assert(false);
		return;
	}

	const std::vector<UartPair>& uartList = mf.uartList();

	m_pUartListWidget->clear();

	for (auto it : uartList)
	{
		int uartID = it.first;
		QString uartType = it.second;

		QStringList l;
		l << tr("%1h").arg(QString::number(uartID, 16));
		l << uartType;
		l << "0";

		QTreeWidgetItem* uartItem = new QTreeWidgetItem(l);
		uartItem->setData(columnUartId, Qt::UserRole, uartID);
		uartItem->setData(columnUploadCount, Qt::UserRole, 0);

		m_pUartListWidget->addTopLevelItem(uartItem);
	}
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
	QString subsysId = selectedSubsystem();

	if (subsysId.isEmpty())
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Subsystem is not selected."));
		return;
	}

	try
	{
		emit uploadFirmware(&m_firmware, subsysId);
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

	m_pReadToFileButton->setEnabled(enable);
	m_pDetectSubsystemButton->setEnabled(enable);
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

	m_pReadToFileButton->setEnabled(enable);
	m_pDetectSubsystemButton->setEnabled(enable);
	m_pConfigureButton->setEnabled(enable);
	if (m_pEraseButton)
	{
		m_pEraseButton->setEnabled(enable);
	}
	m_pSettingsButton->setEnabled(enable);
	m_pCancelButton->setEnabled(!enable);
	m_pClearLogButton->setEnabled(enable);


	// Clear status of all Uarts
	//
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}
		item->setText(columnUartStatus, QString());
	}
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

QString UploadTabPage::selectedSubsystem()
{
	for (QTreeWidgetItem* item : m_pSubsystemsListWidget->selectedItems())
	{
		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid())
		{
			return vData.toString();
		}
	}

	return QString();
}

void UploadTabPage::selectBuild(const QString& id)
{
	m_pBuildList->clearSelection();

	int count = m_pBuildList->count();

	for (int i = 0; i < count; i++)
	{
		if (m_pBuildList->item(i)->text() == id)
		{
			m_pBuildList->setCurrentRow(i);
			return;
		}
	}
}

void UploadTabPage::selectSubsystem(const QString& id)
{
	m_pSubsystemsListWidget->clearSelection();

	int count = m_pSubsystemsListWidget->topLevelItemCount();

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pSubsystemsListWidget->topLevelItem(i);

		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid() && vData.toString() == id)
		{
			m_pSubsystemsListWidget->setCurrentItem(item);
			return;
		}
	}
}

void UploadTabPage::refreshBinaryFile()
{
	QString buildPath = m_buildSearchPath + QDir::separator() + m_currentBuild;

	if (m_buildSearchPath.isEmpty() == true || m_currentBuild.isEmpty() == true || QDir(buildPath).exists() == false)
	{
		m_currentFilePath.clear();
		clearSubsystemsUartData();
		return;
	}

	QStringList binaryFiles = QDir(buildPath).entryList(QStringList() << "*.bts",
									 QDir::Files| QDir::NoSymLinks);

	// Must be only one binary file

	if (binaryFiles.isEmpty() == true)
	{
		m_outputLog.writeError(tr("No Output Bitstream files found in %1!").arg(buildPath));

		m_currentFilePath.clear();
		clearSubsystemsUartData();
		return;
	}

	if (binaryFiles.size() > 1)
	{
		m_outputLog.writeError(tr("More than one Output Bitstream file found in %1!").arg(buildPath));

		m_currentFilePath.clear();
		clearSubsystemsUartData();
		return;
	}

	QString filePath = buildPath + QDir::separator() + binaryFiles[0];

	if (m_currentFilePath == filePath && m_currentFileModifiedTime == QFileInfo(filePath).lastModified())
	{
		return;	// File is the same, do not read it
	}

	clearSubsystemsUartData();

	m_currentFilePath = filePath;
	m_currentFileModifiedTime = QFileInfo(m_currentFilePath).lastModified();

	emit loadBinaryFile(m_currentFilePath, &m_firmware);
}

void UploadTabPage::clearSubsystemsUartData()
{
	m_pSubsystemsListWidget->clear();

	m_pUartListWidget->clear();
}

void UploadTabPage::resetUartData()
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setData(columnUploadCount, Qt::UserRole, 0);
		item->setText(columnUploadCount, "0");
	}
}

void UploadTabPage::loadBinaryFileHeaderComplete()
{
	clearSubsystemsUartData();

	QStringList subsystemsList = m_firmware.subsystems();

	for (const QString& subsystemId : subsystemsList)
	{
		QTreeWidgetItem* subsystemItem = new QTreeWidgetItem(QStringList() << subsystemId);
		subsystemItem->setData(columnSubsysId, Qt::UserRole, subsystemId);

		m_pSubsystemsListWidget->sortByColumn(columnUartId, Qt::AscendingOrder);
		m_pSubsystemsListWidget->addTopLevelItem(subsystemItem);
	}
}

void UploadTabPage::uartOperationStart(int uartID, QString operation)
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt();

		if (uartID == itemUartId)
		{
			item->setText(columnUartStatus, operation);
			return;
		}
	}
	return;
}

void UploadTabPage::uploadComplete(int uartID)
{
	int count = m_pUartListWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pUartListWidget->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt();

		if (uartID == itemUartId)
		{
			int itemUploadCount = item->data(columnUploadCount, Qt::UserRole).toInt();
			itemUploadCount++;

			item->setData(columnUploadCount, Qt::UserRole, itemUploadCount);
			item->setText(columnUploadCount, QString::number(itemUploadCount));

			return;
		}
	}
	return;
}

void UploadTabPage::detectSubsystemComplete(int subsystemId)
{
	m_outputLog.writeMessage(tr("Subsystem Key is %1").arg(subsystemId));

	bool subsystemFound = false;

	QStringList subsystems = m_firmware.subsystems();
	for (auto s : subsystems)
	{
		bool ok = false;
		const ModuleFirmware& fw = m_firmware.firmware(s, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		if (subsystemId == fw.ssKey())
		{
			// Select the subsystem
			//
			m_outputLog.writeMessage(tr("Subsystem ID is %1").arg(fw.subsysId()));
			selectSubsystem(fw.subsysId());
			subsystemFound = true;
			break;
		}
	}

	if (subsystemFound == false)
	{
		m_pSubsystemsListWidget->clearSelection();
		m_outputLog.writeMessage(tr("Subsystem ID is unknown"));
	}

	m_outputLog.writeSuccess(tr("Successful."));
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
