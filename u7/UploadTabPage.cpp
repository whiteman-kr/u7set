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

	// Choose Configuration/Build Widget
	//

	QWidget* pConfigurationWidget = new QWidget();

	QVBoxLayout *pConfigurationLayout = new QVBoxLayout(pConfigurationWidget);

	// Create Build list widget

	pConfigurationLayout->addWidget(new QLabel(tr("Choose the Build:")));
	m_pBuildTree = new QTreeWidget();

	QStringList l;
	l << tr("Build");
	l << tr("Date");
	l << tr("Result");

	m_pBuildTree->setColumnCount(l.size());
	m_pBuildTree->setHeaderLabels(l);
	m_pBuildTree->setRootIsDecorated(false);
	m_pBuildTree->header()->hide();

	connect(m_pBuildTree, &QTreeWidget::itemSelectionChanged, this, &UploadTabPage::buildChanged);
	pConfigurationLayout->addWidget(m_pBuildTree);

	// Choose Subsystem and Uart Widget
	//

	QWidget* pSubsystemWidget = new QWidget();

	QVBoxLayout *pSubsystemLayout = new QVBoxLayout(pSubsystemWidget);

	// Create Subsystems list widget

	pSubsystemLayout->addWidget(new QLabel(tr("Choose the Subsystem:")));
	m_pSubsystemsListWidget = new QTreeWidget();
	m_pSubsystemsListWidget->setRootIsDecorated(false);
	connect(m_pSubsystemsListWidget, &QTreeWidget::currentItemChanged, this, &UploadTabPage::subsystemChanged);
	pSubsystemLayout->addWidget(m_pSubsystemsListWidget, 2);

	l.clear();
	l << tr("Subsystem");

	m_pSubsystemsListWidget->setColumnCount(l.size());
	m_pSubsystemsListWidget->setHeaderLabels(l);

	int il = 0;
	m_pSubsystemsListWidget->setColumnWidth(il++, 100);

	QWidget* pUartWidget = new QWidget();

	QVBoxLayout *pUartLayout = new QVBoxLayout(pUartWidget);

	// Create UART list widget

	pUartLayout->addWidget(new QLabel(tr("Firmware Types:")));
	m_pUartListWidget = new QTreeWidget();
	m_pUartListWidget->setRootIsDecorated(false);
	pUartLayout->addWidget(m_pUartListWidget);

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

	pUartLayout->addLayout(bl);

	//pSubsystemUartLayout->addStretch();

	//Left Splitter
	//
	m_pLeftSplitter = new QSplitter(Qt::Vertical);

	m_pLeftSplitter->addWidget(pConfigurationWidget);
	m_pLeftSplitter->addWidget(pSubsystemWidget);
	m_pLeftSplitter->addWidget(pUartWidget);

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

	if (theSettings.isExpertMode() == true)
	{
		QString mconfPath = QApplication::applicationDirPath() + "/mconf.exe";
		if (QFile::exists(mconfPath) == true)
		{
			m_pMconfButton = new QPushButton(tr("&Run mconf..."));
			pButtonsLayout->addWidget(m_pMconfButton);
		}
	}

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
	m_pRightSplitter = new QSplitter();

	m_pRightSplitter->addWidget(m_pLeftSplitter);
	m_pRightSplitter->addWidget(pRightWidget);

	m_pRightSplitter->setStretchFactor(0, 2);
	m_pRightSplitter->setStretchFactor(1, 1);

	m_pLeftSplitter->restoreState(theSettings.m_UploadTabPageLeftSplitterState);
	m_pRightSplitter->restoreState(theSettings.m_UploadTabPageRightSplitterState);

	QHBoxLayout* pMainLayout = new QHBoxLayout(this);

	pMainLayout->addWidget(m_pRightSplitter);

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
	if (m_pMconfButton != nullptr)
	{
		connect(m_pMconfButton, &QAbstractButton::clicked, this, &UploadTabPage::mconf);
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
	connect(this, &UploadTabPage::detectSubsystem, m_pConfigurator, &Configurator::detectSubsystem);

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

	theSettings.m_UploadTabPageLeftSplitterState = m_pLeftSplitter->saveState();
	theSettings.m_UploadTabPageRightSplitterState = m_pRightSplitter->saveState();

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

	m_buildSearchPath = QString("%1%2%3").arg(theSettings.buildOutputPath()).arg(QDir::separator()).arg(projectName);

	// Get builds list

	std::vector<std::pair<QString, QDateTime>> builds;

	QStringList buildList = QDir(m_buildSearchPath).entryList(QStringList("build*"), QDir::Dirs|QDir::NoSymLinks);

	for (const QString& build : buildList)
	{
		QFile buildXml(QString("%1/%2/build.xml").arg(m_buildSearchPath).arg(build));

		QDateTime tm;

		if (buildXml.exists() == true)
		{
			tm = buildXml.fileTime(QFile::FileModificationTime);
		}

		builds.push_back(std::make_pair(build, tm));
	}

	// Compare builds list with current and refresh is required (number of builds, their names or modification time is changed)

	if (m_builds != builds)
	{
		m_builds = builds;

		m_pBuildTree->blockSignals(true);

		m_pBuildTree->clear();

		for (const QString& buildName : buildList)
		{
			Builder::BuildInfo buildInfo;
			bool buildSuccess = false;

			QString buildPath = QString("%1/%2").arg(m_buildSearchPath).arg(buildName);

			bool result = readBuildInfo(buildPath, &buildInfo, &buildSuccess);

			QStringList strings;
			strings << buildName;

			if (result == true)
			{
				strings << buildInfo.date.toString("dd.MM.yyyy hh:mm:ss");
				strings << (buildSuccess == true ? tr("Success") : tr("Failed"));
			}
			else
			{
				strings << QString();
				strings << tr("Failed");
			}

			QTreeWidgetItem* item = new QTreeWidgetItem(strings);
			m_pBuildTree->addTopLevelItem(item);
		}

		selectBuild(m_currentBuild);

		m_pBuildTree->blockSignals(false);

		for (int i = 0; i < m_pBuildTree->header()->count(); i++)
		{
			m_pBuildTree->resizeColumnToContents(i);
		}
		m_pBuildTree->setColumnWidth(static_cast<int>(BuildColumns::Name), m_pBuildTree->columnWidth(static_cast<int>(BuildColumns::Name)) + 20);
		m_pBuildTree->setColumnWidth(static_cast<int>(BuildColumns::Date), m_pBuildTree->columnWidth(static_cast<int>(BuildColumns::Date)) + 20);

	}

	// Refresh binary file
	//

	refreshBinaryFile();
}

void UploadTabPage::buildChanged()
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QTreeWidgetItem* item = m_pBuildTree->currentItem();
	if (item == nullptr)
	{
		return;
	}

	m_currentBuild = item->text(static_cast<int>(BuildColumns::Name));

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

	clearLog();

	return;
}

void UploadTabPage::projectClosed()
{
	m_pBuildTree->clear();

	m_currentBuild.clear();

	m_builds.clear();

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

		emit readFirmware(fileName, {});

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
		emit uploadFirmware(&m_firmware, subsysId, {});
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

		emit eraseFlashMemory(0, {});
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

void UploadTabPage::mconf()
{
	QString mconfPath = QApplication::applicationDirPath() + "/mconf.exe";

	if (QFile::exists(mconfPath) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("File mconf.exe does not exist!"));
		return;
	}

	if (QProcess::startDetached(mconfPath, {}) == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Could not run mconf.exe!"));
		return;
	}
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
	if (m_pMconfButton)
	{
		m_pMconfButton->setEnabled(enable);
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
	if (m_pMconfButton)
	{
		m_pMconfButton->setEnabled(enable);
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
	m_pBuildTree->clearSelection();

	int count = m_pBuildTree->topLevelItemCount();

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_pBuildTree->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			continue;
		}

		if (item->text(static_cast<int>(BuildColumns::Name)) == id)
		{
			item->setSelected(true);

			m_pBuildTree->scrollToItem(item);

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

bool UploadTabPage::readBuildInfo(const QString& buildPath, Builder::BuildInfo* buildInfo, bool* buildSuccess)
{
	if (buildInfo == nullptr || buildSuccess == nullptr)
	{
		Q_ASSERT(buildInfo);
		Q_ASSERT(buildSuccess);
		return false;
	}

	// Read build information from build.xml

	QString buildXmlFileName = buildPath + "/build.xml";

	QFile buildXml(buildXmlFileName);

	if (buildXml.exists() == false)
	{
		return false;
	}

	if (buildXml.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	QByteArray data = buildXml.readAll();

	buildXml.close();

	if (data.isEmpty())
	{
		return false;
	}

	bool buildInfoFound = false;
	bool buildResultFound = false;

	QXmlStreamReader xmlReader(data);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == "BuildInfo")
		{
			buildInfo->readFromXml(xmlReader);
			buildInfoFound = true;
		}

		if (xmlReader.name() == "BuildResult")
		{
			bool ok = false;

			int errorCount = xmlReader.attributes().value("Errors").toInt(&ok);

			if (ok == false || errorCount != 0)
			{
				*buildSuccess = false;
			}
			else
			{
				*buildSuccess = true;
			}


			buildResultFound = true;
		}
	}

	if (buildInfoFound == false || buildResultFound == false)
	{
		return false;	// Required sections were not found in build.xml
	}

	// Check if .bts file exists

	QString buildOutputFileName = QString("%1/%2-%3.bts").arg(buildPath).arg(buildInfo->project).arg(QString::number(buildInfo->id).rightJustified(6, '0'));

	QFile buildOutput(buildOutputFileName);

	if (buildOutput.exists() == false)
	{
		*buildSuccess = false;
	}

	return true;
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
