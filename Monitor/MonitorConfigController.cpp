#include "MonitorConfigController.h"
#include "Settings.h"
#include <QDomElement>
#include <QDomNodeList>

ConfigConnection::ConfigConnection(QString EquipmentId, QString ipAddress, int port) :
	m_equipmentId(EquipmentId),
	m_ip(ipAddress),
	m_port(port)
{
}


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

	ConfigSettings readSettings;

	// Parse XML
	//
	QDomDocument xml;
	QString parsingError;

	bool result = xml.setContent(configurationXmlData, false, &parsingError);

	if (result == false)
	{
		readSettings.errorMessage += parsingError + "\n";
	}
	else
	{
		// Get <Configuration>
		//
		QDomElement configElement = xml.documentElement();

		// Software node
		//
		QDomNodeList softwareNodes = configElement.elementsByTagName("Software");
		if (softwareNodes.size() != 1)
		{
			readSettings.errorMessage += tr("Parsing Software node error.\n");
		}
		else
		{
			result &= xmlReadSoftwareNode(softwareNodes.item(0), &readSettings);
		}

		// Settings node
		//
		QDomNodeList settingsNodes = configElement.elementsByTagName("Settings");
		if (settingsNodes.size() != 1)
		{
			readSettings.errorMessage += tr("Parsing Settings node error.\n");
		}
		else
		{
			result &= xmlReadSettingsNode(settingsNodes.item(0), &readSettings);
		}
	}

	// Error handling
	//
	if (result == false ||
		readSettings.errorMessage.isEmpty() == false)
	{
		QString completeErrorMessage = tr("Parsing configuration file error: %1").arg(readSettings.errorMessage);

		qDebug() << completeErrorMessage;
		QMessageBox::critical(nullptr, qApp->applicationName(), completeErrorMessage);
	}

	return;
}

bool MonitorConfigController::xmlReadSoftwareNode(QDomNode& softwareNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		softwareNode.nodeName() != "Software")
	{
		assert(outSetting);
		assert(softwareNode.nodeName() == "Software");
		return false;
	}

	QDomElement softwareElement = softwareNode.toElement();

	// Read StrID attribute
	//
	QString appEquipmentId = softwareElement.attribute("StrID");

	if (theSettings.instanceStrId() != appEquipmentId)
	{
		// The received file has different StrID then expected
		//
		outSetting->errorMessage += "The received file has different EquipmentID then expected.\n";
		return false;
	}

	// Read Type attribute
	//
	int softwareType = softwareElement.attribute("Type").toInt();

	if (softwareType != E::SoftwareType::Monitor)
	{
		// The received file has different type then expected,
		//
		outSetting->errorMessage += "The received file has different software type then expected.\n";
		return false;
	}

	return outSetting->errorMessage.isEmpty();
}

bool MonitorConfigController::xmlReadSettingsNode(QDomNode& settingsNode, ConfigSettings* outSetting)
{
	if (outSetting == nullptr ||
		settingsNode.nodeName() != "Settings")
	{
		assert(outSetting);
		assert(settingsNode.nodeName() == "Settings");
		return false;
	}

	QDomElement settingsElement = settingsNode.toElement();

	QDomNodeList errorNodes = settingsElement.elementsByTagName("Error");

	if (errorNodes.isEmpty() == false)
	{
		for (int i = 0; i < errorNodes.count();  i++)
		{
			outSetting->errorMessage += QString("%1\n").arg(errorNodes.at(i).toElement().text());
		}

		return false;
	}


	return outSetting->errorMessage.isEmpty();
//	if (outSetting == nullptr ||
//		xmlReader.name() != "Settings")
//	{
//		assert(outSetting);
//		//assert(xmlReader.name() == "Settings");
//		return false;
//	}

//	// Parse
//	//
//	while(xmlReader.atEnd() == false)
//	{
//		if (xmlReader.readNextStartElement() == false)
//		{
//			//qDebug() << "readNextStartElement() == false: " << xmlReader.name() << ", Text: " << xmlReader.text();
//			continue;
//		}

//		QString tagName = xmlReader.name().toString();

//		qDebug() << tagName;

//		if (xmlReader.isStartElement() == true && tagName == "Error")
//		{
//			outSetting->errorMessage += xmlReader.readElementText() + "\n";
//			xmlReader.skipCurrentElement();
//			continue;
//		}

//		if (xmlReader.isStartElement() == true && tagName == "StartSchemaID")
//		{
//			outSetting->startSchemaId = xmlReader.text().toString();
//			xmlReader.skipCurrentElement();
//			continue;
//		}

//		if (xmlReader.isStartElement() == true && tagName == "DataAquisitionService")
//		{
//			QString dasStrId1 = xmlReader.attributes().value("StrID1").toString();
//			QString dasStrId2 = xmlReader.attributes().value("StrID2").toString();

//			QString dasIp1 = xmlReader.attributes().value("ip1").toString();
//			QString dasIp2 = xmlReader.attributes().value("ip2").toString();

//			int dasPort1 = xmlReader.attributes().value("port1").toInt();
//			int dasPort2 = xmlReader.attributes().value("port2").toInt();

//			QString logString1 = QString("DataAcquisitionService1 StrID: %1, ip: %2, port: %3")
//								 .arg(dasStrId1)
//								 .arg(dasIp1)
//								 .arg(dasPort1);

//			qDebug() << logString1;

//			QString logString2 = QString("DataAcquisitionService2 StrID: %1, ip: %2, port: %3")
//								 .arg(dasStrId2)
//								 .arg(dasIp2)
//								 .arg(dasPort2);

//			qDebug() << logString2;

//			// Save data
//			//
//			outSetting->das1 = ConfigConnection(dasStrId1, dasIp1, dasPort1);
//			outSetting->das2 = ConfigConnection(dasStrId2, dasIp2, dasPort2);;

//			xmlReader.skipCurrentElement();
//			continue;
//		}

//		outSetting->errorMessage += QString("Unknown xml tag %1\n").arg(tagName);
//		xmlReader.skipCurrentElement();
//	}

//	return outSetting->errorMessage.isEmpty();
}
