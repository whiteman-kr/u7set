#include "MonitorCfgGenerator.h"

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

			// StartSchemaStrID
			//
			QString startSchemaStrId = getObjectProperty<QString>(m_software->strId(), "StartSchemaStrID", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (startSchemaStrId.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID1 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			xmlWriter.writeTextElement("StartSchemaStrID", startSchemaStrId);

			// DataAquisitionServiceStrID1
			//
			QString dacStrID1 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceStrID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID1.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID1 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// DataAquisitionServiceStrID2
			//
			QString dacStrID2 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceStrID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID2.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID2 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
			//
			QString clientRequestIP1 = getObjectProperty<QString>(dacStrID1, "ClientRequestIP", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			QString clientRequestPort1 = getObjectProperty<QString>(dacStrID1, "ClientRequestPort", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
			//
			QString clientRequestIP2 = getObjectProperty<QString>(dacStrID2, "ClientRequestIP", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			QString clientRequestPort2 = getObjectProperty<QString>(dacStrID2, "ClientRequestPort", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

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
				xmlWriter.writeAttribute("StrID1", dacStrID1);
				xmlWriter.writeAttribute("StrID2", dacStrID2);

				xmlWriter.writeAttribute("ip1", clientRequestIP1);
				xmlWriter.writeAttribute("port1", clientRequestPort1);
				xmlWriter.writeAttribute("ip2", clientRequestIP2);
				xmlWriter.writeAttribute("port2", clientRequestPort2);
			}	// DataAquisitionService


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
