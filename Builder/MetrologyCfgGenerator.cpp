#include "MetrologyCfgGenerator.h"
#include "DbMetrologyConnection.h"
#include "SoftwareSettingsGetter.h"
#include "DeviceHelper.h"

#include <HardwareLib/DeviceObject.h>

#include "../UtilsLib/XmlHelper.h"
#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{
	const QString MetrologyCfgGenerator::ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION("wrong combination of ElectricUnit and SensorType");
	const QString MetrologyCfgGenerator::ERR_WRONG_ELECTRIC_LIMITS("wrong electric limits");
	const QString MetrologyCfgGenerator::ERR_WRONG_ENGINEERING_LIMITS("wrong engineering limits");

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

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
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
		result &= linkComparatorsSet();

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
					Hardware::DeviceObject* pSystem = m_equipment->root()->child(s).get();
					TEST_PTR_CONTINUE(pSystem);

					if (pSystem->isSystem() == false)
					{
						continue;
					}

					// find all racks in system
					//
					int racksCount = pSystem->childrenCount();
					for (int r = 0; r < racksCount; r++)
					{
						Hardware::DeviceObject* pRack = pSystem->child(r).get();
						TEST_PTR_CONTINUE(pRack);

						if (pRack->isRack() == false)
						{
							continue;
						}

						racks.append(Metrology::RackParam(TO_INT(racks.count()) , pRack->equipmentId(), pRack->caption()));
					}
				}

				// Writing racks
				//
				xml.writeStartElement("Racks");
				{
					xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(racks.count()));

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

				Hardware::DeviceObject* pObjectSoftware = m_equipment->deviceObject(m_software->equipmentId()).get();
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
					xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(tuningSourceEquipmentID.count()));

					for(const QString& equipmentID : tuningSourceEquipmentID)
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
				Metrology::DbConnectionBase connectionBase(nullptr);

				connectionBase.setDbController(&m_context->m_db);

				connectionBase.load();

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

						if (ERR_METROLOGY_CONNECTION_TYPE(connection.type()) == true)
						{
							// Metrology connection with signals: %1 and %2, has wrong type of connection
							//
							m_log->errEQP6120(connection.appSignalID(Metrology::ConnectionIoType::Source),
											  connection.appSignalID(Metrology::ConnectionIoType::Destination));

							wrongConnection = true;
						}

						AppSignal* pInSignal = m_signalSet->getSignal(connection.appSignalID(Metrology::ConnectionIoType::Source));
						if (pInSignal == nullptr)
						{
							// Metrology connections contain a non-existent source signal: %1
							//
							m_log->errEQP6121(connection.appSignalID(Metrology::ConnectionIoType::Source));

							wrongConnection = true;
						}

						AppSignal* pOutSignal = m_signalSet->getSignal(connection.appSignalID(Metrology::ConnectionIoType::Destination));
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


		// Create and write build file MetrologyItems.xml
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::METROLOGY_ITEMS_XML,
															CfgFileId::METROLOGY_ITEMS, "",  data);
		TEST_PTR_RETURN_FALSE(buildFile);

		// add link to file MetrologyItems.xml in Configuration.xml
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
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_signalSet, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_equipment, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_buildResultWriter, m_log);

		bool result = true;

		if (m_buildResultWriter->isBuildFileByIDExists(Directory::COMMON, CfgFileId::METROLOGY_SIGNAL_SET) == true)
		{
			BuildFile* metrologySignalSetFile = m_buildResultWriter->getBuildFileByID(Directory::COMMON, CfgFileId::METROLOGY_SIGNAL_SET);

			if (metrologySignalSetFile == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			result &= m_cfgXml->addLinkToFile(metrologySignalSetFile);

			if (result == false)
			{
				// Can't link build file %1 into /%2/Configuration.xml.
				//
				m_log->errCMN0018(File::METROLOGY_SIGNAL_SET, equipmentID());
				return false;
			}

			return result;
		}

		// Creating signal list
		//
		std::vector<Metrology::SignalParam> signalsToWrite;

		signalsToWrite.reserve(m_signalSet->count());

		for(const AppSignal* s : *m_signalSet)
		{
			const AppSignal& signal = *s;

			if (signal.isAcquired() == false)
			{
				continue;
			}

			if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == true)
			{
				bool testResult = true;
				QString err;

				E::ElectricUnit electricUnit = signal.electricUnit();

				switch (electricUnit)
				{
					case E::ElectricUnit::NoUnit:
						break;

					case E::ElectricUnit::mA:
						testResult = testElectricLimit_Input_mA(signal, &err);
						break;

					case E::ElectricUnit::mV:
						testResult = testElectricLimit_Input_mV(signal, &err);
						break;

					case E::ElectricUnit::Ohm:
						testResult = testElectricLimit_Input_Ohm(signal, &err);
						break;

					case E::ElectricUnit::V:
						testResult = testElectricLimit_Input_V(signal, &err);
						break;

					case E::ElectricUnit::uA:
						testResult = testElectricLimit_Input_uA(signal, &err);
						break;

					case E::ElectricUnit::Hz:
						testResult = testElectricLimit_Input_Hz(signal, &err);
						break;

					default:
						testResult = false;
						LOG_INTERNAL_ERROR_MSG(m_log, QString("Unknown value of property ElectricUnit in signal %1").
														arg(signal.appSignalID()));
				}

				if (testResult == false && err.isEmpty() == false)
				{
					//  Metrology parameters checking error of signal %1: %2
					//
					m_log->errEQP6123(signal.appSignalID(), err);

					result = false;
					continue;
				}
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
			Metrology::SignalLocation location(showOnSchemas);
			getSignalLocation(m_equipment->deviceObject(signal.equipmentID()).get(), location);

			// append signal into list
			//
			signalsToWrite.emplace_back(signal, location);
		}

		RETURN_IF_FALSE(result);

		// Writing signals
		//
		::Proto::MetrologySignalSet protoMetrologySignalSet;

		for(const Metrology::SignalParam& signal : signalsToWrite)
		{
			::Proto::MetrologySignal* protoMetrologySignal = protoMetrologySignalSet.add_metrologysignal();
			signal.saveToProto(protoMetrologySignal);
		}

		int dataSize = static_cast<int>(protoMetrologySignalSet.ByteSizeLong());

		QByteArray data;

		data.resize(dataSize);

		protoMetrologySignalSet.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(data.data()));

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::COMMON, File::METROLOGY_SIGNAL_SET, CfgFileId::METROLOGY_SIGNAL_SET, "",  data);
		TEST_PTR_RETURN_FALSE(buildFile);

		result &= m_cfgXml->addLinkToFile(buildFile);

		if (result == false)
		{
			// Can't link build file %1 into /%2/Configuration.xml.
			//
			m_log->errCMN0018(File::METROLOGY_SIGNAL_SET, equipmentID());
			return false;
		}

		return result;
	}

	bool MetrologyCfgGenerator::linkComparatorsSet()
	{
		bool result = m_cfgXml->addLinkToFile(Directory::COMMON, File::COMPARATORS_SET);

		if (result == false)
		{
			// Can't link build file %1 into /%2/Comparators.set.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(Directory::COMMON).arg(File::COMPARATORS_SET), equipmentID());
			return false;
		}

		return result;
	}

	void MetrologyCfgGenerator::getSignalLocation(Hardware::DeviceObject* pDeviceObject, Metrology::SignalLocation& l)
	{
		TEST_PTR_RETURN(pDeviceObject);

		if (pDeviceObject->isRoot() == true)
		{
			return;
		}

		switch(pDeviceObject->deviceType())
		{
			case Hardware::DeviceType::Rack:
				l.rack().setEquipmentID(pDeviceObject->equipmentId());
				break;

			case Hardware::DeviceType::Chassis:
				l.setChassisID(pDeviceObject->equipmentId());
				l.setChassis(pDeviceObject->place());
				break;

			case Hardware::DeviceType::Module:
				l.setModuleID(pDeviceObject->equipmentId());
				l.setModuleCaption(pDeviceObject->caption());
				l.setModule(pDeviceObject->place());
				break;

			case Hardware::DeviceType::AppSignal:
				l.setPlace(pDeviceObject->place());
				l.setContact(pDeviceObject->equipmentId().remove(pDeviceObject->parent()->equipmentId()));
				break;
		}

		getSignalLocation(pDeviceObject->parent().get(), l);
	}

	bool MetrologyCfgGenerator::testElectricLimit(const AppSignal& signal, double lowLimit, double highLimit)
	{
		static const QStringList requiredProps =
		{
			AppSignalPropNames::ELECTRIC_LOW_LIMIT,
			AppSignalPropNames::ELECTRIC_HIGH_LIMIT,
			AppSignalPropNames::ELECTRIC_UNIT
		};

		RETURN_IF_FALSE(checkRequiredProperties(signal, requiredProps));

		if (signal.electricLowLimit() < lowLimit || signal.electricLowLimit() > highLimit)
		{
			//  Signal %1 has wrong low electric limit: %2 %5. Electric limit: %3 .. %4 %5.
			//
			m_log->errEQP6116(signal.appSignalID(), signal.electricLowLimit(), lowLimit, highLimit,
							  E::valueToString(signal.electricUnit()), 4);

			return false;
		}

		if (signal.electricHighLimit() < lowLimit || signal.electricHighLimit() > highLimit)
		{
			//  Signal %1 has wrong high electric limit: %2 %5. Electric limit: %3 .. %4 %5.
			//
			m_log->errEQP6117(signal.appSignalID(), signal.electricHighLimit(), lowLimit, highLimit,
							  E::valueToString(signal.electricUnit()), 4);

			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testEngineeringLimit(const AppSignal& signal, double lowLimit, double highLimit)
	{
		static const QStringList requiredProps =
		{
			AppSignalPropNames::ELECTRIC_LOW_LIMIT,
			AppSignalPropNames::ELECTRIC_HIGH_LIMIT,
			AppSignalPropNames::ELECTRIC_UNIT,
			AppSignalPropNames::LOW_ENGINEERING_UNITS,
			AppSignalPropNames::HIGH_ENGINEERING_UNITS
		};

		RETURN_IF_FALSE(checkRequiredProperties(signal, requiredProps));

		UnitsConverter uc;

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

		double lowEngUnits = signal.lowEngineeringUnits();
		double highEngUnits = signal.highEngineeringUnits();

		if (lowEngUnits > highEngUnits)
		{
			std::swap(lowEngUnits, highEngUnits);
		}

		double lowElectricVal = floor(uc.conversion(lowEngUnits, UnitsConvertType::PhysicalToElectric, signal) * 10000 + 0.5) / 10000;
		double highElectricVal = floor(uc.conversion(highEngUnits, UnitsConvertType::PhysicalToElectric, signal) * 10000 + 0.5) / 10000;

		if ((std::nextafter(lowElectricVal, std::numeric_limits<double>::lowest()) <= signal.electricLowLimit() &&
			 std::nextafter(lowElectricVal, std::numeric_limits<double>::max()) >= signal.electricLowLimit()) == false)
		{
			// Signal %1 - low engineering limit mismatch low electrical limit: %2 %4, set low electrical Limit: %3 %4.
			//
			m_log->errEQP6112(signal.appSignalID(), signal.electricLowLimit(), lowElectricVal,
							  E::valueToString(signal.electricUnit()), 4);
			return false;
		}

		if ((std::nextafter(highElectricVal, std::numeric_limits<double>::lowest()) <= signal.electricHighLimit() &&
			 std::nextafter(highElectricVal, std::numeric_limits<double>::max()) >= signal.electricHighLimit()) == false)
		{
			// Signal %1 - high engineering limit mismatch high electrical limit: %2 %4, set high electrical Limit: %3 %4.
			//
			m_log->errEQP6113(signal.appSignalID(), signal.electricHighLimit(), highElectricVal,
							  E::valueToString(signal.electricUnit()), 4);
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_mA(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::mA);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}

		if (signal.sensorType() == E::SensorType::NoSensor)
        {
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::RLOAD_OHM) == false)
		{
			return true;
		}
		else
		{
			if (UnitsConverter::rloadIsValid(signal.rloadOhm()) == false)
			{
				// Signal %1 has wrong RLoad (mA).
				//
				m_log->errEQP6115(signal.appSignalID());
				return false;
			}
		}

		SignalElectricLimit electricLimit = UnitsConverter::getElectricLimit(signal.electricUnit(), signal.sensorType());

		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		double lowLimit = electricLimit.lowLimit / signal.rloadOhm() * UnitsConverter::RLOAD_OHM_HIGH_LIMIT;
		double highLimit = electricLimit.highLimit / signal.rloadOhm() * UnitsConverter::RLOAD_OHM_HIGH_LIMIT;

		if (testElectricLimit(signal, lowLimit, highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_mV(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::mV);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}

		UnitsConverter uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		if (testEngineeringLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			*err = ERR_WRONG_ENGINEERING_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_Ohm(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::Ohm);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}

		E::SensorType sensorType = signal.sensorType();

		double r0 = UnitsConverter::r0_from_signal(signal);

		if (sensorType != E::SensorType::NoSensor && sensorType != E::SensorType::Ohm_Raw)
		{
			if (UnitsConverter::r0_OhmIsValid(r0) == false)
			{
				// Signal %1 has wrong R0 (ThermoResistor)
				//
				m_log->errEQP6114(signal.appSignalID());
				return false;
			}
		}

		SignalElectricLimit electricLimit = UnitsConverter::getElectricLimit(signal.electricUnit(), signal.sensorType());

		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		double lowLimit = electricLimit.lowLimit;
		double highLimit = electricLimit.highLimit;

		if (sensorType != E::SensorType::NoSensor && sensorType != E::SensorType::Ohm_Raw)
		{
			lowLimit = lowLimit * r0 / 100;
			highLimit = highLimit * r0 / 100;
		}

		if (testElectricLimit(signal, lowLimit, highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		if (testEngineeringLimit(signal, lowLimit, highLimit) == false)
		{
			*err = ERR_WRONG_ENGINEERING_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_V(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::V);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}
		else
		{
			if (signal.sensorType() != E::SensorType::V_0_5 &&
				signal.sensorType() != E::SensorType::V_m10_p10)
			{
				// Signal %1 has wrong SensorType %2.
				//
				m_log->errEQP6102(signal.appSignalID(), signal.sensorType());
				return false;
			}
		}

		UnitsConverter uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_uA(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::uA);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_UNIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}
		else
		{
			if (signal.sensorType() != E::SensorType::uA_m20_p20)
			{
				// Signal %1 has wrong SensorType %2.
				//
				m_log->errEQP6102(signal.appSignalID(), signal.sensorType());
				return false;
			}
		}

		UnitsConverter uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::testElectricLimit_Input_Hz(const AppSignal& signal, QString* err)
	{
		TEST_PTR_RETURN_FALSE(err);
		Q_ASSERT(signal.electricUnit() == E::ElectricUnit::Hz);

		if (signal.isSpecPropExists(AppSignalPropNames::LOW_ENGINEERING_UNITS) == false ||
			signal.isSpecPropExists(AppSignalPropNames::HIGH_ENGINEERING_UNITS) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_LOW_LIMIT) == false ||
			signal.isSpecPropExists(AppSignalPropNames::ELECTRIC_HIGH_LIMIT) == false)
		{
			return true;
		}

		if (signal.isSpecPropExists(AppSignalPropNames::SENSOR_TYPE) == false)
		{
			return true;
		}

		if (UnitsConverter::isSensorValid(E::ElectricUnit::Hz, signal.sensorType()) == false)
		{
			// Signal %1 has wrong SensorType %2.
			//
			m_log->errEQP6102(signal.appSignalID(), signal.sensorType());
			return false;
		}

		UnitsConverter uc;

		SignalElectricLimit electricLimit = uc.getElectricLimit(signal.electricUnit(), signal.sensorType());
		if(electricLimit.isValid() == false)
		{
			*err = ERR_WRONG_ELECTRIC_UNIT_SENSOR_TYPE_COMBINATION;
			return false;
		}

		if (testElectricLimit(signal, electricLimit.lowLimit, electricLimit.highLimit) == false)
		{
			*err = ERR_WRONG_ELECTRIC_LIMITS;
			return false;
		}

		return true;
	}

	bool MetrologyCfgGenerator::checkRequiredProperties(const AppSignal& signal, const QStringList& propNames) const
	{
		bool result = true;

		for(const QString& propName : propNames)
		{
			if (signal.isSpecPropExists(propName) == false)
			{
				// Specific property %1 is not exists in signal %2
				//
				m_log->errALC5176(signal.appSignalID(), propName);

				result = false;
			}
		}

		return result;
	}

}


