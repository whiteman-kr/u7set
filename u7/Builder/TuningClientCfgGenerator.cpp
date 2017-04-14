#include "TuningClientCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../VFrame30/Schema.h"

namespace Builder
{

TuningClientCfgGenerator::TuningClientCfgGenerator(DbController* db,
												   Hardware::SubsystemStorage* subsystems,
												   Hardware::Software* software,
												   SignalSet* signalSet,
												   Hardware::EquipmentSet* equipment,
												   BuildResultWriter* buildResultWriter)	:
	SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
	m_subsystems(subsystems)
{

}

bool TuningClientCfgGenerator::generateConfiguration()
{
	if (m_software == nullptr ||
			m_software->type() != E::SoftwareType::TuningClient ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr ||
			m_subsystems == nullptr)
	{
		assert(m_software);
		assert(m_software->type() == E::SoftwareType::Monitor);
		assert(m_equipment);
		assert(m_cfgXml);
		assert(m_buildResultWriter);
		assert(m_subsystems);
		return false;
	}


	IssueLogger* log = m_buildResultWriter->log();
	if (log == nullptr)
	{
		assert(log);
		return false;
	}

	bool result = true;

	result &= writeSettings();
	result &= writeObjectFilters();
	result &= writeSchemasDetails();
	result &= writeTuningSignals();
	result &= writeSchemas();
	result &= writeGlobalScript();

	return result;
}


bool TuningClientCfgGenerator::writeSettings()
{
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

		//
		// TuningServiceID1(2)
		//
		QString tuningServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID1", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (tuningServiceId1.isEmpty() == true)
		{
            m_log->errCFG3022(m_software->equipmentId(), "TuningServiceID1");

            QString errorStr = tr("TuningClient configuration error %1, property TuningServiceID1 is invalid")
					.arg(m_software->equipmentIdTemplate());

            writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		QString tuningServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID2", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		//
		// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
		//

		Hardware::Software* tunsObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId1));

        if (tunsObject1 == nullptr)
        {
            m_log->errCFG3021(m_software->equipmentIdTemplate(), "TuningServiceID1", tuningServiceId1);

            QString errorStr = tr("Object %1 is not found").arg(tuningServiceId1);

            writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
            return false;
        }

        Hardware::Software* tunsObject2 = nullptr;

        if (tuningServiceId2.isEmpty() == false)    // tuningServiceId2 is optional
        {
            tunsObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId2));

            if (tunsObject2 == nullptr)
            {
                m_log->errCFG3021(m_software->equipmentIdTemplate(), "TuningServiceID2", tuningServiceId2);

                QString errorStr = tr("Object %1 is not found").arg(tuningServiceId2);

                writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
                return false;
            }
        }

		TuningServiceSettings tunsSettings1;
		tunsSettings1.readFromDevice(tunsObject1, m_log);

		TuningServiceSettings tunsSettings2;
        if (tunsObject2 != nullptr)
        {
            tunsSettings2.readFromDevice(tunsObject2, m_log);
        }

		//
		// AutoApply
		//
		bool autoApply = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "AutoApply", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// showSignals
		//
		bool showSignals = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSignals", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// showSchemas
		//
		bool showSchemas = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSchemas", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// showSchemasList
		//
		bool showSchemasList = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "ShowSchemasList", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// filterByEquipment
		//
		bool filterByEquipment = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "FilterByEquipment", &ok);
		if (ok == false)
		{
			return false;
		}

		//
		// filterBySchema
		//
		bool filterBySchema = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "FilterBySchema", &ok);
		if (ok == false)
		{
			return false;
		}

		// Get ip addresses and ports, write them to configurations
		//
		{
			xmlWriter.writeStartElement("TuningService");
			std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
			{
				xmlWriter.writeEndElement();
			});

			// --
			//
			xmlWriter.writeAttribute("TuningServiceID1", tuningServiceId1);
			xmlWriter.writeAttribute("TuningServiceID2", tuningServiceId2);

			xmlWriter.writeAttribute("ip1", tunsSettings1.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port1", QString::number(tunsSettings1.clientRequestIP.port()));
			xmlWriter.writeAttribute("ip2", tunsSettings2.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port2", QString::number(tunsSettings2.clientRequestIP.port()));

			xmlWriter.writeAttribute("autoApply", (autoApply ? "true" : "false"));
			xmlWriter.writeAttribute("showSignals", (showSignals ? "true" : "false"));
			xmlWriter.writeAttribute("showSchemas", (showSchemas? "true" : "false"));
			xmlWriter.writeAttribute("showSchemasList", (showSchemasList ? "true" : "false"));
			xmlWriter.writeAttribute("filterByEquipment", (filterByEquipment ? "true" : "false"));
			xmlWriter.writeAttribute("filterBySchema", (filterBySchema ? "true" : "false"));


		}	// TuningService

	}


	{
		xmlWriter.writeStartElement("Schemas");
		std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
		{
			xmlWriter.writeEndElement();
		});

		// --
		//
		bool ok = true;

		//
		// Schemas
		//
		QString schemas = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Schemas", &ok);
		if (ok == false)
		{
			return false;
		}

		QStringList schemasList;
		if (schemas.isEmpty() == false)
		{
			schemas.replace('\n', ';');
			schemasList = schemas.split(';');
		}


		for (auto s : schemasList)
		{
			// Check if schema with specified ID exists
			//

			QString caption;

			bool found = false;

			for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
			{
				if (schemaFile.id == s)
				{

					VFrame30::SchemaDetails details;
					bool parsed = details.parseDetails(schemaFile.details);
					if (parsed)
					{
						caption = details.m_caption;
					}
					found = true;
					break;
				}
			}

			if (found == false)
			{
				m_log->errEQP6106(s, m_software->equipmentId());
				return false;
			}

			xmlWriter.writeStartElement("Schema");
			xmlWriter.writeAttribute("Id", s);
			xmlWriter.writeAttribute("Caption", caption);
			xmlWriter.writeEndElement();
		}
	}

	return true;
}

bool TuningClientCfgGenerator::writeObjectFilters()
{
	bool ok = true;

	//
	// Filters
	//
	QString filters = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Filters", &ok).trimmed();
	if (ok == false)
	{
		return false;
	}

	if (filters.isEmpty() == true)
	{
		m_log->errCFG3022(m_software->equipmentId(), "Filters");
		return false;
	}

	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ObjectFilters.xml", CFG_FILE_ID_TUNING_FILTERS, "", filters);

	if (buildFile == nullptr)
	{
        m_log->errCMN0012("ObjectFilters.xml");
		return false;
	}

	m_cfgXml->addLinkToFile(buildFile);

	return true;

}

bool TuningClientCfgGenerator::writeSchemas()
{
	bool ok = true;
	//
	// Schemas
	//
	QString schemas = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Schemas", &ok);
	if (ok == false)
	{
		return false;
	}

	QStringList schemasList;
	if (schemas.isEmpty() == false)
	{
		schemas.replace('\n', ';');
		schemasList = schemas.split(';');
	}


	for (const QString& s : schemasList)
	{
		bool found = false;

		// Check if schema with specified ID exists
		//
		for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
		{
			if (schemaFile.id == s)
			{
				m_cfgXml->addLinkToFile(schemaFile.subDir, schemaFile.fileName);

				found = true;
				break;
			}
		}

		if (found == false)
		{
			m_log->errEQP6106(s, m_software->equipmentId());
			return false;
		}
	}

	return true;
}


bool TuningClientCfgGenerator::writeSchemasDetails()
{
	QByteArray data;
	XmlWriteHelper xmlWriter(&data);

	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("Schemas");

	for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
	{
		xmlWriter.writeStartElement("Schema");
		std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
		{
			xmlWriter.writeEndElement();
		});

		xmlWriter.writeStringAttribute("Details", schemaFile.details);
	}

	xmlWriter.writeEndElement();

	xmlWriter.writeEndDocument();

	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "SchemasDetails.xml", CFG_FILE_ID_TUNING_SCHEMAS_DETAILS, "", data);

	if (buildFile == nullptr)
	{
        m_log->errCMN0012("SchemasDetails.xml");
		return false;
	}

	m_cfgXml->addLinkToFile(buildFile);

	return true;
}

bool TuningClientCfgGenerator::writeTuningSignals()
{
	// Parse tuningSourceEquipmentId
	//
	bool ok = false;
	QString tuningSourceEquipmentId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID", &ok).trimmed();
	if (ok == false)
	{
		return false;
	}

	QStringList tuningSourceEquipmentIdMasks;
	if (tuningSourceEquipmentId.isEmpty() == false)
	{
		tuningSourceEquipmentId.replace('\n', ';');
		tuningSourceEquipmentIdMasks = tuningSourceEquipmentId.split(';');
	}

	// Write signals
	//
	QByteArray data;
	XmlWriteHelper xmlWriter(&data);

	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();

	xmlWriter.writeStartElement("TuningSignals");

	int count = m_signalSet->count();
	for (int i = 0; i < count; i++)
	{
		const Signal& s = (*m_signalSet)[i];

		if (s.enableTuning() == false)
		{
			continue;
		}

		// Check EquipmentIdMasks
		//

		if (tuningSourceEquipmentIdMasks.empty() == false)
		{
			bool result = false;
			for (QString m : tuningSourceEquipmentIdMasks)
			{
				m = m.trimmed();

				if (m.isEmpty() == true)
				{
					continue;
				}

				QRegExp rx(m);
				rx.setPatternSyntax(QRegExp::Wildcard);
				if (rx.exactMatch(s.equipmentID()))
				{
					result = true;
					break;
				}
			}
			if (result == false)
			{
				continue;
			}
		}

		xmlWriter.writeStartElement("TuningSignal");
		std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
		{
			xmlWriter.writeEndElement();
		});

		xmlWriter.writeStringAttribute("AppSignalID", s.appSignalID());
		xmlWriter.writeStringAttribute("CustomAppSignalID", s.customAppSignalID());
		xmlWriter.writeStringAttribute("EquipmentID", s.equipmentID());
		xmlWriter.writeStringAttribute("Caption", s.caption());
		xmlWriter.writeStringAttribute("Type", s.isAnalog() ? "A" : "D");
		xmlWriter.writeDoubleAttribute("DefaultValue", s.tuningDefaultValue(), s.decimalPlaces());
		xmlWriter.writeIntAttribute("DecimalPlaces", s.decimalPlaces());
		xmlWriter.writeDoubleAttribute("LowLimit", s.lowEngeneeringUnits(), s.decimalPlaces());
		xmlWriter.writeDoubleAttribute("HighLimit", s.highEngeneeringUnits(), s.decimalPlaces());
	}

	xmlWriter.writeEndElement();

	xmlWriter.writeEndDocument();

	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.xml", CFG_FILE_ID_TUNING_SIGNALS, "", data);

	if (buildFile == nullptr)
	{
        m_log->errCMN0012("TuningSignals.xml");
		return false;
	}

	m_cfgXml->addLinkToFile(buildFile);

	return true;
}

bool TuningClientCfgGenerator::writeGlobalScript()
{
	// Writing GlobalScript
	//
	bool result = true;

	if (m_software->propertyExists("GlobalScript") == false)
	{
		m_log->errCFG3000("GlobalScript", m_software->equipmentIdTemplate());
		result = false;
	}
	else
	{
		QString globalScript = m_software->propertyValue("GlobalScript").toString();
		BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "GlobalScript.js", CFG_FILE_ID_TUNING_GLOBALSCRIPT, "", globalScript);

		m_cfgXml->addLinkToFile(globalScriptBuildFile);
	}

	return result;
}

void TuningClientCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
{
	xmlWriter.writeTextElement("Error", error);
}

}
