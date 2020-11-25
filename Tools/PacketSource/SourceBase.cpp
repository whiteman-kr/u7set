#include "SourceBase.h"

#include <assert.h>
#include <QFile>

#include "../../lib/XmlHelper.h"
#include "../../lib/DataSource.h"
#include "../../lib/ConstStrings.h"

#ifndef Q_CONSOLE_APP
	#include <QMessageBox>
#endif

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::SourceInfo::SourceInfo()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

void PS::SourceInfo::clear()
{
	index = -1;

	caption.clear();
	equipmentID.clear();

	moduleType = 0;
	subSystem.clear();
	frameCount = 0;
	dataID = 0;

	lmAddress.clear();
	serverAddress.clear();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source() :
	m_pThread(nullptr),
	m_pWorker(nullptr)
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source(const Source& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::Source(const PS::SourceInfo& si)
{
	m_si = si;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source::~Source()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::clear()
{
	m_sourceMutex.lock();

		m_si.clear();
		m_associatedSignalList.clear();
		m_signalList.clear();
		m_frameBase.clear();

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::run()
{
	if (m_pWorker != nullptr)
	{
		return false;
	}

	bool result = createWorker();
	if (result == true)
	{
		qDebug() << "Run source" << m_si.equipmentID << "-" << m_si.lmAddress.addressStr();
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::stop()
{
	if (m_pWorker == nullptr)
	{
		return false;
	}

	m_pWorker->finish();

	deleteWorker();

	qDebug() << "Stop source" << m_si.equipmentID << "-" << m_si.lmAddress.addressStr();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::isRunning()
{
	if (m_pWorker == nullptr)
	{
		return false;
	}

	return m_pWorker->isRunnig();
}

// -------------------------------------------------------------------------------------------------------------------

int PS::Source::sentFrames()
{
	if (m_pWorker == nullptr)
	{
		return 0;
	}

	return m_pWorker->sentFrames();
}

// -------------------------------------------------------------------------------------------------------------------

bool PS::Source::createWorker()
{
	if (m_pWorker != nullptr)
	{
		return false;
	}

	if (m_si.serverAddress.isEmpty() == true)
	{
		return false;
	}

	m_pWorker = new SourceWorker(this);
	if (m_pWorker == nullptr)
	{
		return false;
	}

	if (m_pThread != nullptr)
	{
		return false;
	}

	m_pThread = new QThread;
	if (m_pThread == nullptr)
	{
		delete m_pWorker;
		return false;
	}

	m_pWorker->moveToThread(m_pThread);

	connect(m_pThread, &QThread::started, m_pWorker, &SourceWorker::process);	// on start thread run method process()

	m_pThread->start();															// run thread that runs process()

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::deleteWorker()
{
	if (m_pThread != nullptr)
	{
		m_pThread->quit();
		m_pThread->wait(10000);
		delete m_pThread;
		m_pThread = nullptr;
	}

	if (m_pWorker != nullptr)
	{
		m_pWorker->wait();
		delete m_pWorker;
		m_pWorker = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::loadSignals(const SignalBase& signalBase)
{
	for (int i = 0; i < m_si.signalCount; i++)
	{
		PS::Signal* pSignal = signalBase.signalPtr(m_associatedSignalList[i]);
		if (pSignal == nullptr)
		{
			PS::Signal signal;
			signal.setAppSignalID(m_associatedSignalList[i]);
			qDebug() << "Signal:" << m_associatedSignalList[i] << "has not been found";
			m_signalList.append(signal);
		}
		else
		{
			pSignal->calcOffset();

			int frameIndex = pSignal->frameIndex();
			if (frameIndex >= 0 && frameIndex < m_si.frameCount)
			{
				PS::FrameData* pFrameData = m_frameBase.frameDataPtr(frameIndex);
				if (pFrameData != nullptr)
				{
					if (pSignal->frameOffset() >= 0 && pSignal->frameOffset() < Rup::FRAME_DATA_SIZE)
					{
						pSignal->setValueData(&pFrameData->data()[pSignal->frameOffset()]);
					}
				}
			}

			m_signalList.append(*pSignal);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::initSignalsState()
{
	int sigalCount = m_signalList.count();
	for(int i = 0; i < sigalCount; i++)
	{
		PS::Signal* pSignal = &m_signalList[i];
		if ( pSignal == nullptr)
		{
			continue;
		}

		if (pSignal->isDiscrete() == true)
		{
			if (pSignal->equipmentID().endsWith("VALID") == true)
			{
				pSignal->setState(true);
			}
			else
			{
				pSignal->setState(false);
			}
		}

		if (pSignal->isAnalog() == true)
		{
			pSignal->setState(pSignal->lowEngineeringUnits());
		}

		if (pSignal->enableTuning() == true)
		{
			pSignal->setState(pSignal->tuningDefaultValue().toDouble());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source& PS::Source::operator=(const PS::Source& from)
{
	m_sourceMutex.lock();

		m_pThread = from.m_pThread;
		m_pWorker = from.m_pWorker;

		m_si = from.m_si;
		m_associatedSignalList = from.m_associatedSignalList;
		m_signalList = from.m_signalList;
		m_frameBase = from.m_frameBase;

	m_sourceMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceBase::SourceBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

SourceBase::~SourceBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::clear()
{
	stopAllSoureces();

	m_sourceMutex.lock();

		m_sourceList.clear();

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::count() const
{
	int count = 0;

	m_sourceMutex.lock();

		count = m_sourceList.count();

	m_sourceMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::readFromFile(const BuildInfo& buildInfo)
{
	clear();

	//
	//
	QString msgTitle = tr("Loading sources");

	if (buildInfo.buildDirPath().isEmpty() == true)
	{
		QString strError = tr("Please, input path to build directory!");

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	// read Server IP and Server Port
	//
	QString fileCfg = buildInfo.buildFile(BUILD_FILE_TYPE_SOURCE_CFG).path();
	if (fileCfg.isEmpty() == true)
	{
		QString strError = tr("Sources configuration file %1 - path is empty!").arg(File::CONFIGURATION_XML);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QFile cfgFileXml(fileCfg);
	if (cfgFileXml.exists() == false)
	{
		QString strError = tr("File %1 is not found!").arg(fileCfg);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	if (cfgFileXml.open(QIODevice::ReadOnly) == false)
	{
		QString strError = tr("File %1 is not opened!").arg(fileCfg);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QByteArray&& cfgData = cfgFileXml.readAll();

	cfgFileXml.close();

	XmlReadHelper xmlCfg(cfgData);

	HostAddressPort serverAddress;

	if (xmlCfg.readHostAddressPort("AppDataReceivingIP", "AppDataReceivingPort", &serverAddress) == false)
	{
		QString strError = tr("IP-address of AppDataSrv is not found in file %1!").arg(File::CONFIGURATION_XML);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	// read Data Sources
	//
	QString sourcesFile = buildInfo.buildFile(BUILD_FILE_TYPE_SOURCES).path();
	if (sourcesFile.isEmpty() == true)
	{
		QString strError =  tr("Sources file %1 - path is empty!").arg(File::APP_DATA_SOURCES_XML);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QFile sourcesFileXml(sourcesFile);
	if (sourcesFileXml.exists() == false)
	{
		QString strError = tr("File %1 is not found!").arg(sourcesFile);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	if (sourcesFileXml.open(QIODevice::ReadOnly) == false)
	{
		QString strError = tr("File %1 is not opened!").arg(sourcesFile);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	QByteArray&& sourceData = sourcesFileXml.readAll();

	sourcesFileXml.close();

	//
	//
	QVector<DataSource> dataSources;

	bool result = DataSourcesXML<DataSource>::readFromXml(sourceData, &dataSources);
	if (result == false)
	{
		QString strError = tr("Error reading sources from XML-file %1 - Invalid file format!").arg(sourcesFile);

		#ifdef Q_CONSOLE_APP
			qDebug() << strError;
		#else
			QMessageBox::information(nullptr, msgTitle, strError);
		#endif

		return 0;
	}

	int dataSourcesCount = dataSources.count();
	for(int i = 0; i < dataSourcesCount; i++)
	{
		const DataSource& ds = dataSources[i];

		// Source Info
		//
		PS::SourceInfo si;

		si.index = i;

		si.caption = ds.lmCaption();
		si.equipmentID = ds.lmEquipmentID();

		si.moduleType = ds.lmModuleType();
		si.subSystem = ds.lmSubsystemID();
		si.frameCount = ds.lmRupFramesQuantity();
		si.dataID = ds.lmDataID();

		si.lmAddress = ds.lmAddressPort();
		si.serverAddress = buildInfo.appDataSrvIP();

		si.signalCount = ds.associatedSignals().count();

		// Source
		//
		PS::Source source;

		source.info() = si;
		source.associatedSignalList() = ds.associatedSignals();
		source.frameBase().setFrameCount(si.frameCount);

		append(source);
	}

	emit sourcesLoaded();

	return count();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::append(const PS::Source& source)
{
	int index = -1;

	m_sourceMutex.lock();

		m_sourceList.append(source);
		index = m_sourceList.count() - 1;

	m_sourceMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::remove(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList.remove(index);
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source SourceBase::source(int index) const
{
	PS::Source source;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			source = m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return source;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceBase::sourcePtr(int index)
{
	PS::Source* pSource = nullptr;

	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			pSource = &m_sourceList[index];
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceBase::sourcePtr(const QString& equipmentID)
{
	PS::Source* pSource = nullptr;

	m_sourceMutex.lock();

		int sourceCount = m_sourceList.count();
		for (int index = 0; index < sourceCount; index++)
		{
			if (m_sourceList[index].info().equipmentID == equipmentID)
			{
				pSource = &m_sourceList[index];
				break;
			}
		}

	m_sourceMutex.unlock();

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::setSource(int index, const PS::Source &source)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index] = source;
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

SourceBase& SourceBase::operator=(const SourceBase& from)
{
	m_sourceMutex.lock();

		m_sourceList = from.m_sourceList;

	m_sourceMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runSourece(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index].run();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopSourece(int index)
{
	m_sourceMutex.lock();

		if (index >= 0 && index < m_sourceList.count())
		{
			m_sourceList[index].stop();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runAllSoureces()
{
	m_sourceMutex.lock();

		int count = m_sourceList.count();
		for(int i = 0; i < count; i++)
		{
			m_sourceList[i].run();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopAllSoureces()
{
	m_sourceMutex.lock();

		int count = m_sourceList.count();
		for(int i = 0; i < count; i++)
		{
			m_sourceList[i].stop();
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runSources(const QStringList& sourceIDList)
{
	m_sourceMutex.lock();

		int count = m_sourceList.count();
		for(int s = 0; s < count; s++)
		{
			foreach (const QString& sourceID, sourceIDList)
			{
				if(sourceID == m_sourceList[s].info().equipmentID)
				{
					m_sourceList[s].run();
					break;
				}
			}
		}

	m_sourceMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
