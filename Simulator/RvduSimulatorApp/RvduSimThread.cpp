#include "RvduSimThread.h"

#include <RvduSimulatorLib.h>

RvduSimThreadWorker::RvduSimThreadWorker(unsigned int* pHdmiBuffer, const QString& buildFolder, int subsystemKey, int width, int height) :
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
		}
		else
		{
			m_rnInitOk = true;

			QThread::msleep(500);

			while (QThread::currentThread()->isInterruptionRequested() == false)
			{
				rnLibLoop();

				QThread::msleep(5);
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


	m_workerThread.start();
}

RvduSimThreadController::~RvduSimThreadController()
{
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
