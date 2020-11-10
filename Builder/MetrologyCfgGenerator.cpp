#include "MetrologyCfgGenerator.h"
#include "../lib/MetrologySignal.h"
#include "../lib/ServiceSettings.h"
#include "../lib/DeviceObject.h"
#include "../lib/SignalProperties.h"

namespace Builder
{
	MetrologyCfgGenerator::MetrologyCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software),
		m_subsystems(context->m_subsystems.get()),
		m_analogSignalsOnSchemas(context->m_analogSignalsOnSchemas)
	{
	}


	MetrologyCfgGenerator::~MetrologyCfgGenerator()
	{
	}


	bool MetrologyCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeDatabaseInfo();
		result &= writeSettings();
		result &= writeMetrologyItemsXml();
		result &= writeMetrologySignalSet();

		return result;
	}

	bool MetrologyCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		xmlWriter.writeStartElement("Settings");

		{
			bool result = true;

			// AppDataService
			//

			QString appDataServiceId1;
			QString appDataServiceId2;

			result &= DeviceHelper::getStrProperty(m_software, "AppDataServiceID1" , &appDataServiceId1, m_log);
			result &= DeviceHelper::getStrProperty(m_software, "AppDataServiceID2" , &appDataServiceId2, m_log);

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

			result &= DeviceHelper::getStrProperty(m_software, "TuningServiceID" , &tuningServiceId, m_log);

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

	bool MetrologyCfgGenerator::writeDatabaseInfo()
	{
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		xmlWriter.writeStartElement("DatabaseInfo");
		{
			xmlWriter.writeAttribute("Version", QString::number(m_dbController->databaseVersion()));
		}
		xmlWriter.writeEndElement();

		return true;
	}

	bool MetrologyCfgGenerator::writeSettings()
	{
		return getSettingsXml(m_cfgXml->xmlWriter());
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
						case E::ElectricUnit::V:

							if (testElectricLimit_Input_V(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::mA:

							if (testElectricLimit_Input_mA(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::mV:

							if (testElectricLimit_Input_mV(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::Ohm:

							if (testElectricLimit_Input_Ohm(signal) == false)
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

			// signal is shown in the schemas - only analog signals
			//
			bool showOnSchemas = false;

			if (signal.isAnalog() == true)
			{
				showOnSchemas = m_analogSignalsOnSchemas.find(signal.appSignalID()) != m_analogSignalsOnSchemas.end();
			}

			// find location of signal in the equipment Tree by signal equipmentID
			//
			Metrology::SignalLocation location(m_equipment->deviceObject(signal.equipmentID()), showOnSchemas);

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

	bool MetrologyCfgGenerator::testElectricLimit(const Signal& signal, double lowLimit, double highLimit)
	{
		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return false;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return false;
		}

		QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();

		if (signal.electricLowLimit() < lowLimit || signal.electricLowLimit() > highLimit)
		{
			//  Signal %1 has wrong low electric limit: %2 %5. Electric limit: %3 .. %4 %5.
			//
			m_log->errEQP6116(signal.appSignalID(), signal.electricLowLimit(), lowLimit, highLimit, meu.key(signal.electricUnit()), 4);

			return false;
		}

		if (signal.electricHighLimit() < lowLimit || signal.electricHighLimit() > highLimit)
		{
			//  Signal %1 has wrong high electric limit: %2 %5. Electric limit: %3 .. %4 %5.
			//
			m_log->errEQP6117(signal.appSignalID(), signal.electricHighLimit(), lowLimit, highLimit, meu.key(signal.electricUnit()), 4);

			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testEngineeringLimit(const Signal& signal, double lowLimit, double highLimit)
	{
		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return false;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return false;
		}

		QMetaEnum meu = QMetaEnum::fromType<E::ElectricUnit>();

		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return false;
		}

		UnitsConvertor uc;

		double lowEngineeringLimit = uc.conversion(lowLimit, UnitsConvertType::ElectricToPhysical, signal);
		double highEngineeringLimit = uc.conversion(highLimit, UnitsConvertType::ElectricToPhysical, signal);

		if (signal.lowEngineeringUnits() < lowEngineeringLimit || signal.lowEngineeringUnits() > highEngineeringLimit)
		{
			//  Signal %1 has wrong low engineering limit: %2 %5. Engineering limit: %3 .. %4 %5.
			//
			m_log->errEQP6118(signal.appSignalID(), signal.lowEngineeringUnits(), lowEngineeringLimit, highEngineeringLimit, signal.unit(), signal.decimalPlaces());

			return false;
		}

		if (signal.highEngineeringUnits() < lowEngineeringLimit || signal.highEngineeringUnits() > highEngineeringLimit)
		{
			//  Signal %1 has wrong high engineering limit: %2 %5. Engineering limit: %3 .. %4 %5.
			//
			m_log->errEQP6119(signal.appSignalID(), signal.highEngineeringUnits(), lowEngineeringLimit, highEngineeringLimit, signal.unit(), signal.decimalPlaces());

			return false;
		}

		// get current electric limit from current engineeringUnits and round to 4 digit after point
		//
		double lowElectricVal = floor(uc.conversion(signal.lowEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal) * 10000 + 0.5) / 10000;
		double highElectricVal = floor(uc.conversion(signal.highEngineeringUnits(), UnitsConvertType::PhysicalToElectric, signal) * 10000 + 0.5) / 10000;

		if ((std::nextafter(lowElectricVal, std::numeric_limits<double>::lowest()) <= signal.electricLowLimit() && std::nextafter(lowElectricVal, std::numeric_limits<double>::max()) >= signal.electricLowLimit()) == false)
		{
			// Signal %1 - low engineering limit mismatch low electrical limit: %2 %4, set low electrical Limit: %3 %4.
			//
			m_log->errEQP6112(signal.appSignalID(), signal.electricLowLimit(), lowElectricVal, meu.key(signal.electricUnit()), 4);
			return false;
		}

		if ((std::nextafter(highElectricVal, std::numeric_limits<double>::lowest()) <= signal.electricHighLimit() && std::nextafter(highElectricVal, std::numeric_limits<double>::max()) >= signal.electricHighLimit()) == false)
		{
			// Signal %1 - high engineering limit mismatch high electrical limit: %2 %4, set high electrical Limit: %3 %4.
			//
			m_log->errEQP6113(signal.appSignalID(), signal.electricHighLimit(), highElectricVal, meu.key(signal.electricUnit()), 4);
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_V(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.electricUnit() != E::ElectricUnit::V)
			{
				return false;
			}
		}

		if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.sensorType() != E::SensorType::V_0_5 && signal.sensorType() != E::SensorType::V_m10_p10)
			{
				return false;
			}
		}

		UnitsConvertor uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_mA(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.electricUnit() != E::ElectricUnit::mA)
			{
				return false;
			}
		}

		if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.sensorType() != E::SensorType::V_0_5)
			{
				return false;
			}
		}

		if (signal.isSpecPropExists(SignalProperties::rload_OhmCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.rload_Ohm() < RLOAD_OHM_LOW_LIMIT || signal.rload_Ohm() > RLOAD_OHM_HIGH_LIMIT)
			{
				// Signal %1 has wrong RLoad (mA).
				//
				m_log->errEQP6115(signal.appSignalID());
				return false;
			}
		}

		UnitsConvertor uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			return false;
		}

		double lowLimit = electricLimit.lowLimit  / signal.rload_Ohm() * RLOAD_OHM_HIGH_LIMIT;
		double highLimit = electricLimit.highLimit / signal.rload_Ohm() * RLOAD_OHM_HIGH_LIMIT;

		if (testElectricLimit(signal, lowLimit, highLimit) == false)
		{
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_mV(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.electricUnit() != E::ElectricUnit::mV)
			{
				return false;
			}
		}

		if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}

		UnitsConvertor uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			return false;
		}

		if (testEngineeringLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_Ohm(const Signal& signal)
	{
		if (signal.isSpecPropExists(SignalProperties::lowEngineeringUnitsCaption) == false || signal.isSpecPropExists(SignalProperties::highEngineeringUnitsCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return true;
		}
		else
		{
			if (signal.electricUnit() != E::ElectricUnit::Ohm)
			{
				return false;
			}
		}

		if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return true;
		}

		E::SensorType sensorType = signal.sensorType();


		UnitsConvertor uc;

		double r0 = uc.r0_from_signal(signal);

		if (uc.r0_is_use(sensorType) == true && r0 == 0.0)
		{
			// Signal %1 has wrong R0 (ThermoResistor)
			//
			m_log->errEQP6114(signal.appSignalID());
			return false;
		}

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			return false;
		}

		double lowLimit = electricLimit.lowLimit;
		double highLimit = electricLimit.highLimit;

		if (uc.r0_is_use(sensorType) == true)
		{
			lowLimit = lowLimit * r0 / 100;
			highLimit = highLimit * r0 / 100;
		}

		if (testElectricLimit(signal, lowLimit, highLimit) == false)
		{
			return false;
		}

		if (testEngineeringLimit(signal, lowLimit, highLimit) == false)
		{
			return false;
		}

		return true;
	}
}


