#include "MetrologyCfgGenerator.h"

#include "../lib/MetrologySignal.h"
#include "../lib/MetrologyConnectionBase.h"
#include "../lib/SoftwareSettings.h"
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

	bool MetrologyCfgGenerator::createSettingsProfile(const QString& profile)
	{
		MetrologySettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<MetrologySettings>(profile, settingsGetter);
	}

	bool MetrologyCfgGenerator::generateConfigurationStep1()
	{
		bool result = true;

		result &= writeDatabaseInfo();
		result &= writeMetrologyItemsXml();
		result &= writeMetrologySignalSet();

		return result;
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
					xml.writeIntAttribute(XmlAttribute::COUNT, racks.count());

					for(Metrology::RackParam rack : racks)
					{
						if (rack.equipmentID().isEmpty() == true)
						{
							continue;
						}

						rack.writeToXml(xml);
					}
				}
				xml.writeEndElement(); // Metrology::RackParam


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
					xml.writeIntAttribute(XmlAttribute::COUNT, tuningSourceEquipmentID.count());

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
				xml.writeEndElement(); // TuningSourceEquipmentID


				// Creating metrology connections list from DbController
				//
				Metrology::ConnectionBase connectionBase;
				connectionBase.load(m_dbController);

				// Writing metrology connections
				//
				xml.writeStartElement("Connections");
				{
					int connectionCount = connectionBase.count();
					xml.writeIntAttribute(XmlAttribute::COUNT, connectionCount);

					for(int i = 0; i < connectionCount; i++)
					{
						Metrology::Connection connection = connectionBase.connection(i);

						bool wrongConnection = false;

						int type = connection.type();
						if (type < 0 || type >= Metrology::ConnectionTypeCount)
						{
							// Metrology connection with signals: %1 and %2, has wrong type of connection
							//
							m_log->errEQP6120(connection.appSignalID(Metrology::ConnectionIoType::Source),
											  connection.appSignalID(Metrology::ConnectionIoType::Destination));

							wrongConnection = true;
						}

						Signal* pInSignal = m_signalSet->getSignal(connection.appSignalID(Metrology::ConnectionIoType::Source));
						if (pInSignal == nullptr)
						{
							// Metrology connections contain a non-existent source signal: %1
							//
							m_log->errEQP6121(connection.appSignalID(Metrology::ConnectionIoType::Source));

							wrongConnection = true;
						}

						Signal* pOutSignal = m_signalSet->getSignal(connection.appSignalID(Metrology::ConnectionIoType::Destination));
						if (pOutSignal == nullptr)
						{
							// Metrology connections contain a non-existent destination signal: %1
							//
							m_log->errEQP6122(connection.appSignalID(Metrology::ConnectionIoType::Destination));

							wrongConnection = true;
						}

						if (wrongConnection == true)
						{
							continue;
						}

						connection.writeToXml(xml);
					}
				}
				xml.writeEndElement();  // Metrology::Connection

			}
			xml.writeEndElement();	// </MetrologyItems>
		}
		xml.writeEndDocument();


		// Create and write build file MetrologySignals.xml
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::METROLOGY_ITEMS_XML, CfgFileId::METROLOGY_ITEMS, "",  data);

		if (buildFile == nullptr)
		{
			return false;
		}

		// add link to file MetrologySignals.xml in Configuration.xml
		//
		bool result = m_cfgXml->addLinkToFile(buildFile);
		if (result == false)
		{
			// Can't link build file %1 into /%2/MetrologySignals.xml.
			//
			m_log->errCMN0018(QString("%1").arg(File::METROLOGY_ITEMS_XML), equipmentID());
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

						case E::ElectricUnit::V:

							if (testElectricLimit_Input_V(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::uA:

							if (testElectricLimit_Input_uA(signal) == false)
							{
								hasWrongField = true;
							}
							break;

						case E::ElectricUnit::Hz:

							if (testElectricLimit_Input_Hz(signal) == false)
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

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::METROLOGY_SIGNAL_SET, CfgFileId::METROLOGY_SIGNAL_SET, "",  data);
		if (buildFile == nullptr)
		{
			return false;
		}

		bool result = m_cfgXml->addLinkToFile(buildFile);
		if (result == false)
		{
			// Can't link build file %1 into /%2/MetrologySignals.set.
			//
			m_log->errCMN0018(QString("%1").arg(File::METROLOGY_SIGNAL_SET), equipmentID());
			return false;
		}

		result = m_cfgXml->addLinkToFile(Directory::COMMON, File::COMPARATORS_SET);
		if (result == false)
		{
			// Can't link build file %1 into /%2/Comparators.set.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(Directory::COMMON).arg(File::COMPARATORS_SET), equipmentID());
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

	bool MetrologyCfgGenerator::testElectricLimit_Input_uA(const Signal& signal)
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
			if (signal.electricUnit() != E::ElectricUnit::uA)
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
			if (signal.sensorType() != E::SensorType::uA_m20_p20)
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


	bool MetrologyCfgGenerator::testElectricLimit_Input_Hz(const Signal& signal)
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
			if (signal.electricUnit() != E::ElectricUnit::Hz)
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
			if (signal.sensorType() != E::SensorType::Hz_50_50000)
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
}


