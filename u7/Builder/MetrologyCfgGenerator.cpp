#include "MetrologyCfgGenerator.h"
#include "../lib/MetrologySignal.h"
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
		result &= writeMetrologySignalsXml();

		return result;
	}

	bool MetrologyCfgGenerator::writeSettings()
	{
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		//
		//
		xmlWriter.writeStartElement("DatabaseInfo");
		{
			xmlWriter.writeAttribute("Version", QString::number(m_dbController->databaseVersion()));
		}
		xmlWriter.writeEndElement();

		//
		//

		Hardware::DeviceObject* pObjectSoftware = m_equipment->deviceObject(m_software->equipmentId());
		if (pObjectSoftware == nullptr || pObjectSoftware->isSoftware() == false)
		{
			// Unknown software type (Software object StrID '%1').
			//
			m_log->errEQP6100(m_software->equipmentId(), m_software->uuid());
			xmlWriter.writeTextElement("Error", tr("Software Metrology (%1) not found").arg(m_software->equipmentId()));
			return false;
		}

		//
		//

		xmlWriter.writeStartElement("Settings");
		{
			bool result = true;


			// AppDataService
			//

			QString appDataServiceId1;
			QString appDataServiceId2;

			result &= DeviceHelper::getStrProperty(pObjectSoftware, "AppDataServiceID1" , &appDataServiceId1, m_log);
			result &= DeviceHelper::getStrProperty(pObjectSoftware, "AppDataServiceID2" , &appDataServiceId2, m_log);

			if (result == false)
			{
				return false;
			}

			if (appDataServiceId1.isEmpty() == true &&
				appDataServiceId2.isEmpty() == true)
			{
				// Property '%1.%2' is empty.
				//
				m_log->errCFG3022(m_software->equipmentId(), "AppDataServiceID1");
				m_log->errCFG3022(m_software->equipmentId(), "AppDataServiceID2");

				xmlWriter.writeTextElement("Error", tr("Property AppDataServiceID1 and AppDataServiceID2 is empty"));
				return false;
			}

			bool appDataPropertyIsValid1 = false;
			bool appDataPropertyIsValid2 = false;

			AppDataServiceSettings adsSettings1;
			AppDataServiceSettings adsSettings2;

			if (appDataServiceId1.isEmpty() == false)
			{
				Hardware::Software* appDataObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId1));

				if (appDataObject1 == nullptr)
				{
					// Property '%1.%2' is linked to undefined software ID '%3'.
					//
					m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID1", appDataServiceId1);
				}
				else
				{
					adsSettings1.readFromDevice(m_equipment, appDataObject1, m_log);
					appDataPropertyIsValid1 = true;
				}
			}

			if (appDataServiceId2.isEmpty() == false)
			{
				Hardware::Software* appDataObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId2));

				if (appDataObject2 == nullptr)
				{
					// Property '%1.%2' is linked to undefined software ID '%3'.
					//
					m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID2", appDataServiceId2);
				}
				else
				{
					adsSettings2.readFromDevice(m_equipment, appDataObject2, m_log);
					appDataPropertyIsValid2 = true;
				}
			}

			xmlWriter.writeStartElement("AppDataService");
			{
				xmlWriter.writeAttribute("PropertyIsValid1", appDataPropertyIsValid1 == true ? tr("true") : tr("false"));
				xmlWriter.writeAttribute("AppDataServiceID1", appDataServiceId1);
				xmlWriter.writeAttribute("ip1", adsSettings1.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port1", QString::number(adsSettings1.clientRequestIP.port()));

				xmlWriter.writeAttribute("PropertyIsValid2", appDataPropertyIsValid2 == true ? tr("true") : tr("false"));
				xmlWriter.writeAttribute("AppDataServiceID2", appDataServiceId2);
				xmlWriter.writeAttribute("ip2", adsSettings2.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port2", QString::number(adsSettings2.clientRequestIP.port()));
			}
			xmlWriter.writeEndElement(); // </AppDataService>


			// TuningService
			//

			QString tuningServiceId;

			result &= DeviceHelper::getStrProperty(pObjectSoftware, "TuningServiceID" , &tuningServiceId, m_log);

			if (result == false)
			{
				return false;
			}

			bool tuningPropertyIsValid = false;

			TuningServiceSettings tuningSettings;

			if (tuningServiceId.isEmpty() == false)
			{
				Hardware::Software* tuningObject = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId));

				if (tuningObject == nullptr)
				{
					// Property '%1.%2' is linked to undefined software ID '%3'.
					//
					m_log->wrnCFG3015(m_software->equipmentId(), "TuningServiceID", tuningServiceId);
				}
				else
				{
					tuningSettings.readFromDevice(tuningObject, m_log);
					tuningPropertyIsValid = true;
				}
			}

			xmlWriter.writeStartElement("TuningService");
			{
				xmlWriter.writeAttribute("PropertyIsValid", tuningPropertyIsValid == true ? tr("true") : tr("false"));
				xmlWriter.writeAttribute("SoftwareMetrologyID", m_software->equipmentId());
				xmlWriter.writeAttribute("ip", tuningSettings.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port", QString::number(tuningSettings.clientRequestIP.port()));
			}
			xmlWriter.writeEndElement(); // </TuningService>
		} // </Settings>

		return true;
	}

	bool MetrologyCfgGenerator::writeMetrologySignalsXml()
	{
		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		{
			xml.writeStartElement("MetrologySignals");
			{
				xml.writeIntAttribute("buildID", m_buildResultWriter->buildInfo().id);
				xml.writeIntAttribute("Version", CFG_FILE_VER_METROLOGY_SIGNALS);			// version of MetrologySignal file


				// Creating rack list from equipment tree
				//
				QVector<Metrology::RackParam> racks;

				int systemsCount = m_equipment->root()->childrenCount();
				for (int s = 0; s < systemsCount; s++)
				{
					Hardware::DeviceObject* pSystem = m_equipment->root()->child(s);
					if (pSystem == nullptr || pSystem->isSystem() == false)
					{
						continue;
					}

					// find all racks in system
					//
					int racksCount = pSystem->childrenCount();
					for (int r = 0; r < racksCount; r++)
					{
						Hardware::DeviceObject* pRack = pSystem->child(r);
						if (pRack == nullptr || pRack->isRack() == false)
						{
							continue;
						}

						racks.append(Metrology::RackParam(racks.count() , pRack->equipmentId(), pRack->caption()));
					}
				}

				// Writing racks
				//
				xml.writeStartElement("Racks");
				{
					xml.writeIntAttribute("Count", racks.count());

					for(Metrology::RackParam rack : racks)
					{
						if (rack.equipmentID().isEmpty() == true)
						{
							continue;
						}

						rack.writeToXml(xml);
					}
				}
				xml.writeEndElement();


				// Creating tuning sources list from software property
				//
				QStringList tuningSourceEquipmentID;

				Hardware::DeviceObject* pObjectSoftware = m_equipment->deviceObject(m_software->equipmentId());
				if (pObjectSoftware != nullptr && pObjectSoftware->isSoftware() == true)
				{
					QString propertyValue;
					if (DeviceHelper::getStrProperty(pObjectSoftware, "TuningSourceEquipmentID" , &propertyValue, m_log) == true)
					{
						if (propertyValue.isEmpty() == false)
						{
							propertyValue.replace('\n', ';');
							tuningSourceEquipmentID = propertyValue.split(';');
						}
					}
				}

				// Writing tuning sources
				//
				xml.writeStartElement("TuningSources");
				{
					xml.writeIntAttribute("Count", tuningSourceEquipmentID.count());

					for(QString equipmentID : tuningSourceEquipmentID)
					{
						if (equipmentID.isEmpty() == true)
						{
							continue;
						}

						xml.writeStartElement("TuningSource");
						{
							xml.writeStringAttribute("EquipmentID", equipmentID);
						}
						xml.writeEndElement();
					}
				}
				xml.writeEndElement();

				// Creating signal list
				//
				QVector<Metrology::SignalParam> signalsToWrite;

				int signalCount = m_signalSet->count();

				for(int i = 0; i < signalCount; i++)
				{
					Signal& signal = (*m_signalSet)[i];

					bool hasWrongField = false;

					if (signal.sensorType() < 0 || signal.sensorType() >= SENSOR_TYPE_COUNT)
					{
						// Signal %1 has wrong type of sensor: %2.
						//
						m_log->errEQP6102(signal.appSignalID(), signal.sensorType());
						hasWrongField = true;
					}

					if (E::contains<E::OutputMode>(signal.outputMode()) == false)
					{
						// Signal %1 has wrong type of output range mode: %2.
						//
						m_log->errEQP6103(signal.appSignalID(), signal.outputMode());
						hasWrongField = true;
					}

					if (hasWrongField)
					{
						continue;
					}

					// find location of signal in the equipment Tree by signal equipmentID
					//
					Metrology::SignalLocation location(m_equipment->deviceObject(signal.equipmentID()));

					// append signal into list
					//
					signalsToWrite.append(Metrology::SignalParam(signal, location));
				}

				// Writing signals
				//
				xml.writeStartElement("Signals");
				{
					xml.writeIntAttribute("Count", signalsToWrite.count());

					for(Metrology::SignalParam signal : signalsToWrite)
					{
						signal.writeToXml(xml);
					}
				}
				xml.writeEndElement();	// </Signals>
			}
			xml.writeEndElement();	// </MetrologySignals>
		}
		xml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, FILE_METROLOGY_SIGNALS_XML, CFG_FILE_ID_METROLOGY_SIGNALS, "",  data);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}
}
