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

	m_buildOption.setСfgSrvEquipmentID(cmdLine.cfgEquipmentID());
	m_buildOption.setCfgSrvIP(cmdLine.cfgServIP());
	m_buildOption.setAppDataSrvEquipmentID(cmdLine.adsEquipmentID());
	m_buildOption.setUalTesterIP(cmdLine.ualTesterIP());
	m_buildOption.setSourcesForRunList(cmdLine.sourcesForRunList());

	// check build params
	//
	if (buildOptionIsValid() == false)
	{
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

PacketSourceCore::~PacketSourceCore()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::clear()
{
	m_buildOption.clear();

	stopUalTesterServerThread();

	clearAllBases();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::clearAllBases()
{
	// clear all bases
	//
	m_signalBase.clear();
	m_sourceBase.clear();
	m_signalHistory.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool PacketSourceCore::buildOptionIsValid()
{
	QString msgTitle = tr("Loading options");

	// check params
	//
	if (m_buildOption.cfgSrvEquipmentID().isEmpty() == true)
	{
		QString strError = tr("Error: Configuration Service EquipmentID is empty");

		#ifdef Q_CONSOLE_APP
				qDebug() << strError;
		#else
				QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	if (m_buildOption.cfgSrvIP().isValidIPv4(m_buildOption.cfgSrvIP().addressStr()) == false)
	{
		QString strError = tr("Error: IP-addres of Configuration Service is not valid");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	if (m_buildOption.appDataSrvEquipmentID().isEmpty() == true)
	{
		QString strError = tr("Error: Application Data Service EquipmentID is empty");

		#ifdef Q_CONSOLE_APP
				qDebug() << strError;
		#else
				QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return false;
	}

	if (m_buildOption.ualTesterIP().isValidIPv4(m_buildOption.ualTesterIP().addressStr()) == false)
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

bool PacketSourceCore::runUalTesterServerThread()
{
	stopUalTesterServerThread();

	if (m_signalBase.count() == 0)
	{
		return false;
	}

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

	m_ualTesterServerThread = new UalTesterServerThread(m_buildOption.ualTesterIP(), m_ualTesterSever, nullptr);
	if (m_ualTesterServerThread == nullptr)
	{
		return false;
	}

	m_ualTesterServerThread->start();

	qDebug() << "Wait for UalTester to connect and will listen:"	<< m_buildOption.ualTesterIP().addressStr()
																	<< ":" << m_buildOption.ualTesterIP().port();

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
	m_sourceBase.runSources(m_buildOption.sourcesForRunList());	// restore states sources

	emit signalsLoadedInSources();

	qDebug() << "Ready";

	runUalTesterServerThread();
}

// -------------------------------------------------------------------------------------------------------------------

void PacketSourceCore::saveSourceState()
{
	// for reload sources
	//
	m_buildOption.setSourcesForRunList(QStringList());	// clear list of sources id for run

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
				m_buildOption.appendSourcesForRunToList(pSource->info().equipmentID);
			}
		}

		// save state of signals
		//
		for(PS::Signal& signal : pSource->signalList())
		{
			if (signal.regValueAddr().offset() == BAD_ADDRESS || signal.regValueAddr().bit() == BAD_ADDRESS)
			{
				continue;
			}

			m_signalBase.saveSignalState(&signal);
		}
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
