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

	lmIP.clear();
	appDataSrvIP.clear();
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
		qDebug() << "Run source" << m_si.equipmentID << "-" << m_si.lmIP.addressStr();
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

	qDebug() << "Stop source" << m_si.equipmentID << "-" << m_si.lmIP.addressStr();

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

	if (m_si.appDataSrvIP.isEmpty() == true)
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
			m_signalList.push_back(signal);
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

			m_signalList.push_back(*pSignal);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void PS::Source::initSignalsState()
{
	for(PS::Signal& signal : m_signalList)
	{
		if (signal.isDiscrete() == true)
		{
			if (signal.equipmentID().endsWith("VALID") == true)
			{
				signal.setState(true);
			}
			else
			{
				signal.setState(false);
			}
		}

		if (signal.isAnalog() == true)
		{
			signal.setState(signal.lowEngineeringUnits());
		}

		if (signal.enableTuning() == true)
		{
			signal.setState(signal.tuningDefaultValue().toDouble());
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

	QMutexLocker l(&m_sourceMutex);

	m_sourceList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::count() const
{
	QMutexLocker l(&m_sourceMutex);

	return TO_INT(m_sourceList.size());
}

// -------------------------------------------------------------------------------------------------------------------

int SourceBase::append(const PS::Source& source)
{
	QMutexLocker l(&m_sourceMutex);

	m_sourceList.push_back(source);

	return TO_INT(m_sourceList.size() - 1);
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::remove(int index)
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return;
	}

	m_sourceList.erase(m_sourceList.cbegin() + index);
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source SourceBase::source(int index) const
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return PS::Source();
	}

	return m_sourceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceBase::sourcePtr(int index)
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return nullptr;
	}

	return &m_sourceList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

PS::Source* SourceBase::sourcePtr(const QString& equipmentID)
{
	QMutexLocker l(&m_sourceMutex);

	PS::Source* pSource = nullptr;

	for (PS::Source& source : m_sourceList)
	{
		if (source.info().equipmentID == equipmentID)
		{
			pSource = &source;
			break;
		}
	}

	return pSource;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::setSource(int index, const PS::Source &source)
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return;
	}

	m_sourceList[static_cast<quint64>(index)] = source;
}

// -------------------------------------------------------------------------------------------------------------------

SourceBase& SourceBase::operator=(const SourceBase& from)
{
	QMutexLocker l(&m_sourceMutex);

	m_sourceList = from.m_sourceList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runSourece(int index)
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return;
	}

	m_sourceList[static_cast<quint64>(index)].run();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopSourece(int index)
{
	QMutexLocker l(&m_sourceMutex);

	if (index < 0 || index >= TO_INT(m_sourceList.size()))
	{
		return;
	}

	m_sourceList[static_cast<quint64>(index)].stop();
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runAllSoureces()
{
	QMutexLocker l(&m_sourceMutex);

	for (PS::Source& source : m_sourceList)
	{
		source.run();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::stopAllSoureces()
{
	QMutexLocker l(&m_sourceMutex);

	for (PS::Source& source : m_sourceList)
	{
		source.stop();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SourceBase::runSources(const QStringList& sourceIDList)
{
	QMutexLocker l(&m_sourceMutex);

	for (PS::Source& source : m_sourceList)
	{
		foreach (const QString& sourceID, sourceIDList)
		{
			if(sourceID == source.info().equipmentID)
			{
				source.run();
				break;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
