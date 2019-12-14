#include "MetrologyCfgGenerator.h"
#include "../lib/MetrologySignal.h"
#include "../lib/ServiceSettings.h"
#include "../lib/DeviceObject.h"
#include "../lib/SignalProperties.h"

namespace Builder
{
	MetrologyCfgGenerator::MetrologyCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software),
		m_subsystems(context->m_subsystems.get())
	{
	}


	MetrologyCfgGenerator::~MetrologyCfgGenerator()
	{
	}


	bool MetrologyCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeMetrologyItemsXml();
		result &= writeMetrologySignalSet();

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

	bool MetrologyCfgGenerator::writeMetrologyItemsXml()
	{
		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		{
			xml.writeStartElement("MetrologyItems");
			{
				xml.writeIntAttribute("buildID", m_buildResultWriter->buildInfo().id);
				xml.writeIntAttribute("Version", CFG_FILE_VER_METROLOGY_ITEMS_XML);			// version of MetrologyItems file


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
			}
			xml.writeEndElement();	// </MetrologyItems>

		}
		xml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, FILE_METROLOGY_ITEMS_XML, CFG_FILE_ID_METROLOGY_ITEMS, "",  data);

		if (buildFile == nullptr)
		{
			return false;
		}

		bool result = m_cfgXml->addLinkToFile(buildFile);
		if (result == false)
		{
			// Can't link build file %1 into /%2/MetrologySignals.xml.
			//
			m_log->errCMN0018(QString("%1").arg(FILE_METROLOGY_ITEMS_XML), equipmentID());
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::writeMetrologySignalSet()
	{
		// Creating signal list
		//
		QVector<Metrology::SignalParam> signalsToWrite;

		int signalCount = m_signalSet->count();
		for(int i = 0; i < signalCount; i++)
		{
			Signal& signal = (*m_signalSet)[i];

			if (signal.isAcquired() == false)
			{
				continue;
			}

			bool hasWrongField = false;

			if (signal.isAnalog() == true && signal.isInput() == true)
			{
				if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == true)
				{
					switch (signal.electricUnit())
					{
						case E::ElectricUnit::mV:

							if (testElectricRange_ThermoCouple(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::Ohm:

							if (testElectricRange_ThermoResistor(signal) == false)
							{
								hasWrongField = true;
							}
							break;
					}
				}
			}

			if (hasWrongField == true)
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
		::Proto::MetrologySignalSet protoMetrologySignalSet;

		for(Metrology::SignalParam signal : signalsToWrite)
		{
			::Proto::MetrologySignal* protoMetrologySignal = protoMetrologySignalSet.add_metrologysignal();
			signal.serializeTo(protoMetrologySignal);
		}

		int dataSize = protoMetrologySignalSet.ByteSize();

		QByteArray data;

		data.resize(dataSize);

		protoMetrologySignalSet.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(data.data()));

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, FILE_METROLOGY_SIGNAL_SET, CFG_FILE_ID_METROLOGY_SIGNAL_SET, "",  data);
		if (buildFile == nullptr)
		{
			return false;
		}

		bool result = m_cfgXml->addLinkToFile(buildFile);
		if (result == false)
		{
			// Can't link build file %1 into /%2/MetrologySignals.set.
			//
			m_log->errCMN0018(QString("%1").arg(FILE_METROLOGY_SIGNAL_SET), equipmentID());
			return false;
		}

		result = m_cfgXml->addLinkToFile(DIR_COMMON, FILE_COMPARATORS_SET);
		if (result == false)
		{
			// Can't link build file %1 into /%2/Comparators.set.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(DIR_COMMON).arg(FILE_COMPARATORS_SET), equipmentID());
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricRange_ThermoCouple(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false || signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.electricUnit() != E::ElectricUnit::mV)
		{
			return false;
		}

		UnitsConvertor uc;

		switch (signal.sensorType())
		{
			case E::SensorType::mV_K_TXA:
			case E::SensorType::mV_L_TXK:
			case E::SensorType::mV_N_THH:

			case E::SensorType::mV_Type_B:
			case E::SensorType::mV_Type_E:
			case E::SensorType::mV_Type_J:
			case E::SensorType::mV_Type_K:
			case E::SensorType::mV_Type_N:
			case E::SensorType::mV_Type_R:
			case E::SensorType::mV_Type_S:
			case E::SensorType::mV_Type_T:
				{
					UnitsConvertResult physicalLowLimit = uc.electricToPhysical_ThermoCouple(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), signal.electricUnit(), signal.sensorType());
					UnitsConvertResult physicalHighLimit = uc.electricToPhysical_ThermoCouple(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), signal.electricUnit(), signal.sensorType());

					if (physicalLowLimit.ok() == false)
					{
						// Signal %1 has wrong physical low Limit
						//
						m_log->errEQP6110(signal.customAppSignalID());
						return false;
					}

					if (physicalHighLimit.ok() == false)
					{
						// Signal %1 has wrong physical high Limit
						//
						m_log->errEQP6111(signal.customAppSignalID());
						return false;
					}


					if (physicalLowLimit.isEqual(signal.lowEngineeringUnits()) == false)
					{
						QString nowElValStr, newElValStr;
						double elVal = uc.conversion(signal.lowEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal.electricUnit(), signal.sensorType());
						QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
						nowElValStr = newElValStr.sprintf("%0.4f ", signal.electricLowLimit()) +  meu.key(signal.electricUnit());
						newElValStr = newElValStr.sprintf("%0.4f ", elVal) +  meu.key(signal.electricUnit());

						// Signal %1 has wrong engineering low Limit
						//
						m_log->errEQP6112(signal.customAppSignalID(), nowElValStr, newElValStr);
						return false;
					}

					if (physicalHighLimit.isEqual(signal.highEngineeringUnits()) == false)
					{
						QString nowElValStr, newElValStr;
						double elVal = uc.conversion(signal.highEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal.electricUnit(), signal.sensorType());
						QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
						nowElValStr = newElValStr.sprintf("%0.4f ", signal.electricHighLimit()) +  meu.key(signal.electricUnit());
						newElValStr = newElValStr.sprintf("%0.4f ", elVal) +  meu.key(signal.electricUnit());

						// Signal %1 has wrong engineering high Limit
						//
						m_log->errEQP6113(signal.customAppSignalID(), nowElValStr, newElValStr);
						return false;
					}
				}
				break;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricRange_ThermoResistor(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false || signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.electricUnit() != E::ElectricUnit::Ohm)
		{
			return false;
		}

		UnitsConvertor uc;

		switch (signal.sensorType())
		{
			case E::SensorType::Ohm_Pt_a_391:
			case E::SensorType::Ohm_Pt_a_385:
			case E::SensorType::Ohm_Cu_a_428:
			case E::SensorType::Ohm_Cu_a_426:
			case E::SensorType::Ohm_Ni_a_617:
				{
					if (signal.r0_Ohm() == 0.0)
					{
						// Signal %1 has wrong R0 (ThermoResistor)
						//
						m_log->errEQP6114(signal.customAppSignalID());
						return false;
					}

					UnitsConvertResult physicalLowLimit = uc.electricToPhysical_ThermoResistor(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), signal.electricUnit(), signal.sensorType(), signal.r0_Ohm());
					UnitsConvertResult physicalHighLimit = uc.electricToPhysical_ThermoResistor(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), signal.electricUnit(), signal.sensorType(), signal.r0_Ohm());

					if (physicalLowLimit.ok() == false)
					{
						// Signal %1 has wrong physical low Limit
						//
						m_log->errEQP6110(signal.customAppSignalID());
						return false;
					}

					if (physicalHighLimit.ok() == false)
					{
						// Signal %1 has wrong physical high Limit
						//
						m_log->errEQP6111(signal.customAppSignalID());
						return false;
					}

					if (physicalLowLimit.isEqual(signal.lowEngineeringUnits()) == false)
					{
						QString nowElValStr, newElValStr;
						double elVal = uc.conversion(signal.lowEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal.electricUnit(), signal.sensorType(), signal.r0_Ohm());
						QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
						nowElValStr = newElValStr.sprintf("%0.4f ", signal.electricLowLimit()) +  meu.key(signal.electricUnit());
						newElValStr = newElValStr.sprintf("%0.4f ", elVal) +  meu.key(signal.electricUnit());

						// Signal %1 - engineering low Limit mismatch electrical low Limit
						//
						m_log->errEQP6112(signal.customAppSignalID(), nowElValStr, newElValStr);
						return false;
					}

					if (physicalHighLimit.isEqual(signal.highEngineeringUnits()) == false)
					{
						QString nowElValStr, newElValStr;
						double elVal = uc.conversion(signal.highEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal.electricUnit(), signal.sensorType(), signal.r0_Ohm());
						QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();
						nowElValStr = newElValStr.sprintf("%0.4f ", signal.electricHighLimit()) +  meu.key(signal.electricUnit());
						newElValStr = newElValStr.sprintf("%0.4f ", elVal) +  meu.key(signal.electricUnit());

						// SSignal %1 - engineering high Limit mismatch electrical high Limit
						//
						m_log->errEQP6113(signal.customAppSignalID(), nowElValStr, newElValStr);
						return false;
					}

				}
				break;
		}

		return true;
	}

}


