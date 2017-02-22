#include "MetrologyCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/ProtobufHelper.h"
#include "../lib/DeviceObject.h"

namespace Builder
{
    MetrologyCfgGenerator::MetrologyCfgGenerator(	DbController* db,
                                                    Hardware::SubsystemStorage *subsystems,
                                                    Hardware::Software* software,
                                                    SignalSet* signalSet,
                                                    Hardware::EquipmentSet* equipment,
                                                    BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter),
		m_subsystems(subsystems)
	{
	}


    MetrologyCfgGenerator::~MetrologyCfgGenerator()
	{
	}


    bool MetrologyCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeAppSignalsXml();


		return result;
	}


    bool MetrologyCfgGenerator::writeSettings()
	{
        XmlWriteHelper xml(m_cfgXml->xmlWriter());

        return true;
    }

    bool MetrologyCfgGenerator::writeAppSignalsXml()
	{
		UnitList unitInfo;
		m_dbController->getUnits(&unitInfo, nullptr);

		DataFormatList dataFormatInfo;

		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("AppSignals");
		xml.writeIntAttribute("buildID", m_buildResultWriter->buildInfo().id);

		// Writing units
		xml.writeStartElement("Units");
		xml.writeIntAttribute("Count", unitInfo.count());

		int unitsCount = unitInfo.count();

		for (int i = 0; i < unitsCount; i++)
		{
			xml.writeStartElement("Unit");

			xml.writeIntAttribute("ID", unitInfo.keyAt(i));
			xml.writeStringAttribute("Caption", unitInfo[i]);

			xml.writeEndElement();
		}

		xml.writeEndElement();

		QVector<Signal*> signalsToWrite;

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			Signal& signal = (*m_signalSet)[i];

			bool hasWrongField = false;

/*			if (!dataFormatInfo.contains(signal.dataFormatInt()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong dataFormat field").arg(signal.appSignalID()));
				hasWrongField = true;
			}*/

			if (!unitInfo.contains(signal.unitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong unitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (!unitInfo.contains(signal.inputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputUnitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (!unitInfo.contains(signal.outputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputUnitID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

            if (signal.inputSensorType() < 0 || signal.inputSensorType() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

            if (signal.outputSensorType() < 0 || signal.outputSensorType() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (signal.outputMode() < 0 || signal.outputMode() >= OUTPUT_MODE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputRangeMode field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (TO_INT(signal.inOutType()) < 0 || TO_INT(signal.inOutType()) >= IN_OUT_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inOutType field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			switch (static_cast<E::ByteOrder>(signal.byteOrderInt()))
			{
				case E::ByteOrder::LittleEndian:
				case E::ByteOrder::BigEndian:
					break;
				default:
					LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong byteOrder field").arg(signal.appSignalID()));
					hasWrongField = true;
			}

			if (hasWrongField)
			{
				continue;
			}

			signalsToWrite.append(&signal);
		}

		// Writing signals
		//
		xml.writeStartElement("Signals");
		xml.writeIntAttribute("Count", signalsToWrite.count());

		for(Signal* signal : signalsToWrite)
		{
			signal->writeToXml(xml);
		}

		signalsToWrite.clear();

		xml.writeEndElement();	// </Signals>
		xml.writeEndElement();	// </AppSignals>
		xml.writeEndDocument();

        BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, "appSignals.xml", CFG_FILE_ID_METROLOGY_SIGNALS, "",  data);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}
}
