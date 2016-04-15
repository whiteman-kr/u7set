#include "MonitorConfigController.h"
#include "Settings.h"

MonitorConfigController::MonitorConfigController(HostAddressPort address1, HostAddressPort address2)
{
	// Communication instance no
	//
	m_appInstanceSharedMemory.setKey("MonitorInstanceNo");
	int maxInstanceCount = 512;

	bool ok = m_appInstanceSharedMemory.create(maxInstanceCount * sizeof(qint64));

	if (ok == true)
	{
		// Shared memory created, initialize it
		//
		m_appInstanceSharedMemory.lock();

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());

		for (int i = 0; i < maxInstanceCount; i++)
		{
			sharedData[i] = 0;
		}

		sharedData[0] = qApp->applicationPid();
		m_appInstanceNo = 0;

		m_appInstanceSharedMemory.unlock();
	}
	else
	{
		if (m_appInstanceSharedMemory.error() == QSharedMemory::SharedMemoryError::AlreadyExists)
		{
			ok = m_appInstanceSharedMemory.attach();
		}

		if (ok == false)
		{
			QMessageBox::critical(nullptr,
								  qApp->applicationName(),
								  QString("Cannot create or attach to shared memory to determine software instance no. Error: %1")
								  .arg(m_appInstanceSharedMemory.errorString()));

			// Set "Some" Application Instance No
			//
			m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
		}
		else
		{
			// Get empty slot from shared memory
			//
			assert(m_appInstanceSharedMemory.isAttached() == true);

			m_appInstanceSharedMemory.lock();

			qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
			m_appInstanceNo = -1;

			for (int i = 0; i < maxInstanceCount; i++)
			{
				if (sharedData[i] == 0)
				{
					// This is an empty slot, use it
					//
					sharedData[i] = qApp->applicationPid();	// 1 means
					m_appInstanceNo = i;

					break;
				}
			}

			if (m_appInstanceNo == -1)
			{
				assert(m_appInstanceNo > 0);

				QMessageBox::critical(nullptr,
									  qApp->applicationName(),
									  tr("Cannot determine software instance no. It seems all slots are occupied"));

				// Set "Some" Application Instance No
				//
				m_appInstanceNo = static_cast<int>(QDateTime::currentMSecsSinceEpoch());		// cut the highest bytes
			}

			m_appInstanceSharedMemory.unlock();
		}
	}

	qDebug() << "MonitorInstanceNo: " << m_appInstanceNo;

	// --
	//
	m_cfgLoaderThread = new CfgLoaderThread(theSettings.instanceStrId(), m_appInstanceNo, address1,  address2);

	connect(m_cfgLoaderThread, &CfgLoaderThread::signal_configurationReady, this, &MonitorConfigController::slot_configurationReady);

	m_cfgLoaderThread->start();

	m_cfgLoaderThread->enableDownloadConfiguration();

	return;
}

MonitorConfigController::~MonitorConfigController()
{
	// Release application instance slot
	//
	if (m_appInstanceNo != -1)
	{
		assert(m_appInstanceSharedMemory.isAttached() == true);

		m_appInstanceSharedMemory.lock();

		qint64* sharedData = static_cast<qint64*>(m_appInstanceSharedMemory.data());
		sharedData[m_appInstanceNo] = 0;

		m_appInstanceSharedMemory.unlock();
	}

	// Stop communication
	//
	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
}

void MonitorConfigController::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray /*buildFileInfoArray*/)
{
	qDebug() << "MonitorConfigThread::slot_configurationReady";

	QString errorMessage;

	QXmlStreamReader xmlReader(configurationXmlData);

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == "Software")
		{
			bool result = xmlReadSoftwareSection(xmlReader);
			if (result == false)
			{
				errorMessage = tr("Wrong file format or parameters, configuration.xml, section Software.");
			}
			continue;
		}

		if (xmlReader.name() == "Settings")
		{
			bool result = xmlReadSettingsSection(xmlReader);
			if (result == false)
			{
				errorMessage = tr("Wrong file format or parameters, configuration.xml, section Settings.");
			}
			continue;
		}
	}

	if (errorMessage.isEmpty() == false)
	{
		QMessageBox::critical(nullptr, qApp->applicationName(), errorMessage);
	}

	return;
}

bool MonitorConfigController::xmlReadSoftwareSection(QXmlStreamReader& xmlReader)
{
	if (xmlReader.name() != "Software")
	{
		assert(false);
		return false;
	}

	QString appStrId = xmlReader.attributes().value("StrID").toString();

	if (theSettings.instanceStrId() != appStrId)
	{
		// The received file has different StrID then expected
		//
		return false;
	}

	int softwareType = xmlReader.attributes().value("Type").toInt();

	if (softwareType != E::SoftwareType::Monitor)
	{
		// The received file has different type then expected,
		//
		return false;
	}

	return true;
}

bool MonitorConfigController::xmlReadSettingsSection(QXmlStreamReader& xmlReader)
{
	if (xmlReader.name() != "Settings")
	{
		assert(false);
		return false;
	}

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		if (xmlReader.name() == "DataAquisitionService")
		{
			QString dasStrId1 = xmlReader.attributes().value("StrID1").toString();
			QString dasStrId2 = xmlReader.attributes().value("StrID2").toString();

			QString dasIp1 = xmlReader.attributes().value("ip1").toString();
			QString dasIp2 = xmlReader.attributes().value("ip2").toString();

			int dasPort1 = xmlReader.attributes().value("port1").toInt();
			int dasPort2 = xmlReader.attributes().value("port2").toInt();

			QString logString1 = QString("DataAcquisitionService1 StrID: %1, ip: %2, port: %3")
								 .arg(dasStrId1)
								 .arg(dasIp1)
								 .arg(dasPort1);
			qDebug() << logString1;

			QString logString2 = QString("DataAcquisitionService2 StrID: %1, ip: %2, port: %3")
								 .arg(dasStrId2)
								 .arg(dasIp2)
								 .arg(dasPort2);
			qDebug() << logString2;

			// Send new settings to DAS thread etc
			//

			continue;
		}
	}

	return true;
}
