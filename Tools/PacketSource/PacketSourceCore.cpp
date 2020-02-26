#include "PacketSourceCore.h"

#include <QDir>

#ifndef Q_CONSOLE_APP
	#include <QMessageBox>
#endif

// -------------------------------------------------------------------------------------------------------------------

PacketSourceCore::PacketSourceCore(QObject *parent)
	: QObject(parent)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PacketSourceCore::PacketSourceCore(const CmdLineParam& cmdLine, QObject *parent)
	: QObject(parent)
{
	clear();

	m_buildInfo.setBuildDirPath(cmdLine.buildDir());
	m_buildInfo.setAppDataSrvIP(cmdLine.appDataSrvIP());
	m_buildInfo.setUalTesterIP(cmdLine.ualTesterIP());
	m_buildInfo.setSourcesForRunList(cmdLine.sourcesForRunList());
}

// -------------------------------------------------------------------------------------------------------------------

PacketSourceCore::~PacketSourceCore()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::clear()
{
	m_buildInfo.clear();

	m_signalBase.clear();
	m_sourceBase.clear();
	m_signalHistory.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketSourceCore::buildInfoIsValid()
{
	// check params
	//
	QString msgTitle = tr("Loading options");

	if (m_buildInfo.buildDirPath().isEmpty() == true)
	{
		QString strError = tr("Error: Build directory is epmpty");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	if (QDir(m_buildInfo.buildDirPath()).exists() == false)
	{
		QString strError = tr("Error: Build directory is not exist");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	for (int type = 0; type < BUILD_FILE_TYPE_COUNT; type ++)
	{
		if (QFile::exists(m_buildInfo.buildFile(type).path()) == false)
		{
			QString strError = tr("File %1 is not found!").arg(m_buildInfo.buildFile(type).path());

			#ifdef Q_CONSOLE_APP
				qDebug() << strError;
			#else
				QMessageBox::information(nullptr, msgTitle, strError);
			#endif

			return false;
		}
	}

	if (m_buildInfo.appDataSrvIP().isValidIPv4(m_buildInfo.appDataSrvIP().addressStr()) == false)
	{
		QString strError = tr("Error: IP-addres (AppDataReceivingIP) for send packets to AppDataSrv is not valid");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	if (m_buildInfo.ualTesterIP().isValidIPv4(m_buildInfo.ualTesterIP().addressStr()) == false)
	{
		QString strError = tr("Error: IP-addres for listening commands from UalTester is not valid");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketSourceCore::start()
{
	// check version of RUP packets
	//
	#if RUP_VERSION != PS_SUPPORT_VERSION
		#error Current version of Rup packets is unknown
	#endif

	// check build params
	//
	if (buildInfoIsValid() == false)
	{
		return false;
	}

	// load sources
	//
	connect(&m_sourceBase, &SourceBase::sourcesLoaded, this, &PacketSourceCore::loadSignals, Qt::QueuedConnection);
	connect(&m_signalBase, &SignalBase::signalsLoaded, this, &PacketSourceCore::loadSignalsInSources, Qt::QueuedConnection);

	bool result = loadSources();
	if (result == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::stop()
{
	disconnect(&m_sourceBase, &SourceBase::sourcesLoaded, this, &PacketSourceCore::loadSignals);
	disconnect(&m_signalBase, &SignalBase::signalsLoaded, this, &PacketSourceCore::loadSignalsInSources);

	stopUpdateBuildFilesTimer();
	stopUalTesterServerThread();

	clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketSourceCore::runUalTesterServerThread()
{
	SoftwareInfo si;
	si.init(E::SoftwareType::TestClient, "TEST_SERVER_ID", 1, 0);

	m_ualTesterSever = new UalTesterServer(si, &m_sourceBase, &m_signalBase);
	if (m_ualTesterSever == nullptr)
	{
		return false;
	}

	connect(m_ualTesterSever, &UalTesterServer::connectionChanged, this, &PacketSourceCore::ualTesterSocketConnect, Qt::QueuedConnection);
	connect(m_ualTesterSever, &UalTesterServer::signalStateChanged, this, &PacketSourceCore::signalStateChanged, Qt::QueuedConnection);
	connect(m_ualTesterSever, &UalTesterServer::exitApplication, this, &PacketSourceCore::exitApplication, Qt::QueuedConnection);

	m_ualTesterServerThread = new UalTesterServerThread(m_buildInfo.ualTesterIP(), m_ualTesterSever, nullptr);
	if (m_ualTesterServerThread == nullptr)
	{
		return false;
	}

	m_ualTesterServerThread->start();

	qDebug() << "Wait for UalTester to connect and will listen:" << m_buildInfo.ualTesterIP().addressStr() << ":" << m_buildInfo.ualTesterIP().port();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::stopUalTesterServerThread()
{
	if (m_ualTesterServerThread == nullptr)
	{
		return;
	}

	disconnect(m_ualTesterSever, &UalTesterServer::connectionChanged, this, &PacketSourceCore::ualTesterSocketConnect);
	disconnect(m_ualTesterSever, &UalTesterServer::signalStateChanged, this, &PacketSourceCore::signalStateChanged);
	disconnect(m_ualTesterSever, &UalTesterServer::exitApplication, this, &PacketSourceCore::exitApplication);

	m_ualTesterServerThread->quitAndWait();
	delete m_ualTesterServerThread;
	m_ualTesterServerThread = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketSourceCore::loadSources()
{
	m_signalBase.clear();	// clear signals

	int sourceCount = m_sourceBase.readFromFile(m_buildInfo);

	qDebug() << "Loaded sources:" << sourceCount;

	return sourceCount != 0;
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::loadSignals()
{
	int signalCount = m_signalBase.readFromFile(m_buildInfo);

	qDebug() << "Loaded signals:" << signalCount;

	if (signalCount == 0)
	{
		return;
	}

	runUalTesterServerThread();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::loadSignalsInSources()
{
	int sourceCount = m_sourceBase.count();
	for(int i = 0; i < sourceCount; i++)
	{
		PS::Source* pSource = m_sourceBase.sourcePtr(i);
		if (pSource == nullptr)
		{
			continue;
		}

		pSource->loadSignals(m_signalBase);
		pSource->initSignalsState();
	}

	// if reload bases if build has been updated
	//
	m_signalBase.restoreSignalsState();							// restore states signals
	m_sourceBase.runSources(m_buildInfo.sourcesForRunList());	// restore states sources

	// run timers for update build files
	//
	startUpdateBuildFilesTimer();

	emit signalsLoaded();

	qDebug() << "Ready";
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::reloadSource()
{
	// reload build files
	//
	m_buildInfo.loadBuildFiles();						// reload new build files
	m_buildInfo.setSourcesForRunList(QStringList());	// clear list of sources id for run

	// stop timers for update build files
	//
	stopUpdateBuildFilesTimer();

	// save states sources and signals
	//
	int sourceCount = m_sourceBase.count();
	for (int s = 0; s < sourceCount; s++)
	{
		PS::Source*	pSource = m_sourceBase.sourcePtr(s);
		if (pSource == nullptr)
		{
			continue;
		}

		// save running source EquipmentIDs for restart them after reload sources
		//
		if(pSource->isRunning() == true)
		{
			if (pSource->info().equipmentID.isEmpty() == false)
			{
				m_buildInfo.appendSourcesForRunToList(pSource->info().equipmentID);
			}
		}

		// save state of signals
		//
		int signalCount = pSource->signalList().count();
		for(int i = 0; i < signalCount; i++)
		{
			PS::Signal* pSignal = &pSource->signalList()[i];
			if ( pSignal == nullptr)
			{
				continue;
			}

			if (pSignal->regValueAddr().offset() == BAD_ADDRESS || pSignal->regValueAddr().bit() == BAD_ADDRESS)
			{
				continue;
			}

			m_signalBase.saveSignalState(pSignal);
		}
	}

	// reload sources
	//
	loadSources();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::startUpdateBuildFilesTimer()
{
	if (m_buildInfo.enableReload() == false)
	{
		return;
	}

	if (m_updateBuildFilesTimer == nullptr)
	{
		m_updateBuildFilesTimer = new QTimer(this);
		connect(m_updateBuildFilesTimer, &QTimer::timeout, this, &PacketSourceCore::updateBuildFiles);
	}

	m_updateBuildFilesTimer->start(m_buildInfo.timeoutReload() * 1000);
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::stopUpdateBuildFilesTimer()
{
	if (m_updateBuildFilesTimer == nullptr)
	{
		return;
	}

	disconnect(m_updateBuildFilesTimer, &QTimer::timeout, this, &PacketSourceCore::updateBuildFiles);

	m_updateBuildFilesTimer->stop();
	delete m_updateBuildFilesTimer;
	m_updateBuildFilesTimer = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::updateBuildFiles()
{
	QString buidFileName = m_buildInfo.buildDirPath() + BUILD_FILE_SEPARATOR + "build.xml";
	if (buidFileName.isEmpty() == true)
	{
		return;
	}

	if (QFile::exists(buidFileName) == false)
	{
		return;
	}

	QFile file(buidFileName);

	if (file.size() == 0)
	{
		return;
	}

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return;
	}

	QByteArray&& bulidData = file.readAll();

	file.close();

	bool reloadBuildFiles = false;

	QXmlStreamReader m_xmlReader(bulidData);
	Builder::BuildFileInfo buildFileInfo;

	while(m_xmlReader.atEnd() == false)
	{
		if (m_xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (m_xmlReader.name() == "File")
		{
			buildFileInfo.readFromXml(m_xmlReader);

			for (int type = 0; type < BUILD_FILE_TYPE_COUNT; type ++)
			{
				if (buildFileInfo.pathFileName == m_buildInfo.buildFile(type).fileName())
				{
					if (buildFileInfo.size != m_buildInfo.buildFile(type).size())
					{
						reloadBuildFiles = true;
					}

					if (buildFileInfo.md5 != m_buildInfo.buildFile(type).md5())
					{
						reloadBuildFiles = true;
					}
				}
			}
		}
	}

	if (reloadBuildFiles == true)
	{
		qDebug() << "Build has been updated";

		emit reloadSource();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::signalStateChanged(Hash hash, double prevState, double state)
{
	if (hash == UNDEFINED_HASH)
	{
		return;
	}

	PS::Signal* pSignal = m_signalBase.signalPtr(hash);
	if (pSignal == nullptr)
	{
		return;
	}

	m_signalHistory.append( SignalForLog(pSignal, prevState, state) );
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::exitApplication()
{
	qDebug() << "Exit";

	#ifdef Q_CONSOLE_APP
		QCoreApplication::exit(0);
	#endif
}

// -------------------------------------------------------------------------------------------------------------------
