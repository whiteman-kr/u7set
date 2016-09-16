#include "TuningClientCfgGenerator.h"
#include "../lib/ServiceSettings.h"

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

	bool result = true;

	result &= writeSettings();
	result &= writeObjectFilters();
	result &= writeTuningSignals();

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
			QString errorStr = tr("TuningClient configuration error %1, property TuningServiceID1 is invalid")
					.arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		QString tuningServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID2", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (tuningServiceId2.isEmpty() == true)
		{
			QString errorStr = tr("TuningClient configuration error %1, property TuningServiceID2 is invalid")
					.arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		//
		// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* tunsObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId1));
		Hardware::Software* tunsObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId2));

		if (tunsObject1 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(tuningServiceId1);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		if (tunsObject2 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(tuningServiceId2);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		TuningServiceSettings tunsSettings1;
		tunsSettings1.readFromDevice(tunsObject1, m_log);

		TuningServiceSettings tunsSettings2;
		tunsSettings2.readFromDevice(tunsObject2, m_log);

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
		}	// TuningService

		return true;
	}
}

bool TuningClientCfgGenerator::writeObjectFilters()
{
	bool ok = true;

	//
	// ObjectFilters
	//
	QString objectFilters = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "ObjectFilters", &ok).trimmed();
	if (ok == false)
	{
		return false;
	}

	if (objectFilters.isEmpty() == true)
	{
		QString errorStr = tr("TuningClient configuration error %1, property ObjectFilters is invalid")
				.arg(m_software->equipmentIdTemplate());

		m_log->writeError(errorStr);
		return false;
	}

	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ObjectFilters.xml", CFG_FILE_ID_TUNING_FILTERS, "", objectFilters);

	if (buildFile == nullptr)
	{
		QString errorStr = tr("TuningClient configuration error %1, can't add file ObjectFilters.xml")
				.arg(m_software->equipmentIdTemplate());

		m_log->writeError(errorStr);
		return false;
	}

	m_cfgXml->addLinkToFile(buildFile);

	return true;

}

bool TuningClientCfgGenerator::writeTuningSignals()
{
	QByteArray data;
	XmlWriteHelper xmlWriter(&data);

	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("TuningSignals");

	int count = m_signalSet->count();
	for (int i = 0; i < count; i++)
	{
		xmlWriter.writeStartElement("TuningSignal");
		std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
		{
			xmlWriter.writeEndElement();
		});

		const Signal& s = (*m_signalSet)[i];

		if (s.enableTuning() == false)
		{
			continue;
		}

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
		QString errorStr = tr("TuningClient configuration error %1, can't add file TuningSignals.xml")
				.arg(m_software->equipmentIdTemplate());

		m_log->writeError(errorStr);
		return false;
	}

	m_cfgXml->addLinkToFile(buildFile);

	return true;
}

void TuningClientCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
{
	xmlWriter.writeTextElement("Error", error);
}

}
