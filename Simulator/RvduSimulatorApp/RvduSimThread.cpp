#include "RvduSimThread.h"

#include <RvduSimulatorLib.h>

RvduSimThreadWorker::RvduSimThreadWorker(unsigned int* pHdmiBuffer,
										 const QString& buildFolder,
										 int subsystemKey,
										 int width,
										 int height) :
	m_pHdmiBuffer(pHdmiBuffer),
	m_buildFolder(buildFolder),
	m_subsystemKey(subsystemKey),
	m_width(width),
	m_height(height)
{
}

bool RvduSimThreadWorker::isRunning() const
{ 
	return m_rnInitOk; 
}

bool RvduSimThreadWorker::isError() const
{ 
	return m_rnInitError; 
}

void RvduSimThreadWorker::threadProc()
{
	unsigned int* pVideoBuffer = new unsigned int[m_width * m_height]; // 4 bytes per pixel

	bool ok = rnLibInit();
	assert(ok);

	if (ok == true)
	{
		ok = rnLibStartSim(m_buildFolder.toStdString().c_str(),
						   m_subsystemKey,
						   (char*)pVideoBuffer,
						   (char*)m_pHdmiBuffer,
						   m_width,
						   m_height);
		if (ok == false)
		{
			m_rnInitError = true;
			emit updateProjectInfo(tr("Initialization error"));
		}
		else
		{
			m_rnInitOk = true;
			emit updateProjectInfo(QString::fromUtf8(rnLibEqipmentId()));
			emit rnInitComplete();

			QThread::msleep(500);

			while (QThread::currentThread()->isInterruptionRequested() == false)
			{
				rnLibLoop();

				QThread::msleep(50);
			}
		}

		rnLibStopSim();
	}

	rnLibShutdown();

	delete[] pVideoBuffer;
}

RvduSimThreadController::RvduSimThreadController(QObject* parent,
												 unsigned int* pHdmiBuffer,
												 const QString& buildFolder,
												 int subsystemKey,
												 int width,
												 int height) :
	QObject(parent)
{
	m_worker = new RvduSimThreadWorker(pHdmiBuffer, buildFolder, subsystemKey, width, height);
	connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
	connect(&m_workerThread, &QThread::started, m_worker, &RvduSimThreadWorker::threadProc);
	m_worker->moveToThread(&m_workerThread);

	connect(m_worker, &RvduSimThreadWorker::logError, this, &RvduSimThreadController::logError);
	connect(m_worker, &RvduSimThreadWorker::logWarning, this, &RvduSimThreadController::logWarning);
	connect(m_worker, &RvduSimThreadWorker::logMessage, this, &RvduSimThreadController::logMessage);
	connect(m_worker, &RvduSimThreadWorker::rnInitComplete, this, &RvduSimThreadController::rnInitComplete);

	connect(m_worker, &RvduSimThreadWorker::updateProjectInfo, this, &RvduSimThreadController::updateProjectInfo);

	m_workerThread.start();
}

RvduSimThreadController::~RvduSimThreadController()
{
	rnLibLoopShutdown();

	m_workerThread.requestInterruption();
	m_workerThread.quit();
	m_workerThread.wait();
}

bool RvduSimThreadController::isRunning() const
{ 
	return m_worker->isRunning(); 
}

bool RvduSimThreadController::isError() const
{ 
	return m_worker->isError(); 
}
