#include "MonitorCfgGenerator.h"
#include "../../include/ServiceSettings.h"

namespace Builder
{
	MonitorCfgGenerator::MonitorCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	MonitorCfgGenerator::~MonitorCfgGenerator()
	{
	}

	bool MonitorCfgGenerator::generateConfiguration()
	{
		if (m_software == nullptr ||
			m_software->type() != E::SoftwareType::Monitor ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(m_software);
			assert(m_software->type() == E::SoftwareType::Monitor);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			return false;
		}

		bool result = true;

		// write XML via m_cfgXml->xmlWriter()
		//
		result &= writeMonitorSettings();

		// add link to configuration files (previously written) via m_cfgXml->addLinkToFile(...)
		//

		for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
		{
			m_cfgXml->addLinkToFile(schemaFile.subDir, schemaFile.fileName);
		}

		return result;
	}

	bool MonitorCfgGenerator::writeMonitorSettings()
	{
		// write XML via m_cfgXml->xmlWriter()
		//
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		{
			xmlWriter.writeStartElement("Settings");
			std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

			// --
			//
			bool ok = true;

			// StartSchemaID
			//
			QString startSchemaId = getObjectProperty<QString>(m_software->strId(), "StartSchemaID", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (startSchemaId.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property startSchemaId is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			xmlWriter.writeTextElement("StartSchemaID", startSchemaId);

			//
			// DataAquisitionServiceID1(2)
			//
			QString dacStrID1 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID1.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceID1 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			//
			// DataAquisitionServiceID2
			//
			QString dacStrID2 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID2.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceID2 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			//
			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
			//
			Hardware::Software* dasObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(dacStrID1));
			Hardware::Software* dasObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(dacStrID2));

			if (dasObject1 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(dasObject1->strId());

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			if (dasObject2 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(dasObject2->strId());

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			DASSettings dasSettings1;
			dasSettings1.readFromDevice(dasObject1, m_log);

			DASSettings dasSettings2;
			dasSettings2.readFromDevice(dasObject2, m_log);

			// Get ip addresses and ports, write them to configurations
			//
			{
				xmlWriter.writeStartElement("DataAquisitionService");
				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
					{
						xmlWriter.writeEndElement();
					});

				// --
				//
				xmlWriter.writeAttribute("DasID1", dacStrID1);
				xmlWriter.writeAttribute("DasID2", dacStrID2);

				xmlWriter.writeAttribute("ip1", dasSettings1.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port1", QString::number(dasSettings1.clientRequestIP.port()));
				xmlWriter.writeAttribute("ip2", dasSettings2.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port2", QString::number(dasSettings2.clientRequestIP.port()));
			}	// DataAquisitionService


			//
			// Archive Service Settings
			//

//			// ArchiveServiceStrID1
//			//
//			QString asStrID1 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID1", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID1.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID1 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// ArchiveServiceStrID2
//			//
//			QString asStrID2 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID2", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID2.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID2 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP1 = getObjectProperty<QString>(dacStrID1, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort1 = getObjectProperty<QString>(dacStrID1, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP2 = getObjectProperty<QString>(dacStrID2, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort2 = getObjectProperty<QString>(dacStrID2, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// Get ip addresses and ports, write them to configurations
//			//
//			{
//				xmlWriter.writeStartElement("DataAquisitionService");
//				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
//					{
//						xmlWriter.writeEndElement();
//					});

//				// --
//				//
//				xmlWriter.writeAttribute("StrID1", dacStrID1);
//				xmlWriter.writeAttribute("StrID2", dacStrID2);

//				xmlWriter.writeAttribute("ip1", clientRequestIP1);
//				xmlWriter.writeAttribute("port1", clientRequestPort1);
//				xmlWriter.writeAttribute("ip2", clientRequestIP2);
//				xmlWriter.writeAttribute("port2", clientRequestPort2);
//			}	// DataAquisitionService


		} // Settings


		return true;
	}

	void MonitorCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}
