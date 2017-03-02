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
		result &= writeMetrologySignalsXml();

		return result;
	}

	bool MetrologyCfgGenerator::writeSettings()
	{
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		std::vector<DbProject> projects;
		m_dbController->getProjectList(&projects, nullptr);

		DbProject project;

		for (DbProject projectI : projects)
		{
			if (projectI.projectName() == m_buildResultWriter->buildInfo().project)
			{
				project = projectI;
				break;
			}
		}

		if (project.projectName().isEmpty() == true)
		{
			QString errorStr = tr("Metrology configuration error, not found ProjectName");

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		xmlWriter.writeStartElement("DatabaseInfo");
		{
			xmlWriter.writeAttribute("DatabaseName", project.databaseName());
			xmlWriter.writeAttribute("Description", project.description());
			xmlWriter.writeAttribute("Version", QString::number( project.version() ));
		}
		xmlWriter.writeEndElement();

		xmlWriter.writeStartElement("Settings");
		{
			bool ok = true;


			// AppDataServiceID1 ----------------------------------------------
			//
			QString appDataServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (appDataServiceId1.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property AppDataServiceID1 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// AppDataServiceID2
			//
			QString appDataServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (appDataServiceId2.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property AppDataServiceID2 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// appDataServiceStrID -- objects
			//
			Hardware::Software* appDataObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId1));
			Hardware::Software* appDataObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId2));

			if (appDataObject1 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(appDataServiceId1);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			if (appDataObject2 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(appDataServiceId2);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			AppDataServiceSettings adsSettings1;
			adsSettings1.readFromDevice(appDataObject1, m_log);

			AppDataServiceSettings adsSettings2;
			adsSettings2.readFromDevice(appDataObject2, m_log);

			xmlWriter.writeStartElement("AppDataService");
			{
				xmlWriter.writeAttribute("AppDataServiceID1", appDataServiceId1);
				xmlWriter.writeAttribute("ip1", adsSettings1.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port1", QString::number(adsSettings1.clientRequestIP.port()));

				xmlWriter.writeAttribute("AppDataServiceID2", appDataServiceId2);
				xmlWriter.writeAttribute("ip2", adsSettings2.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port2", QString::number(adsSettings2.clientRequestIP.port()));
			}
			xmlWriter.writeEndElement();


			// TuningClientID1  ----------------------------------------------
			//
			QString tuningClientId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningClientID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (tuningClientId1.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property TuningClientID1 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// TuningClientID2
			//
			QString tuningClientId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningClientID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (tuningClientId2.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property DataAquisitionClientID2 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// tuningClientStrID -- objects
			//
			Hardware::Software* tuningClientObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningClientId1));
			Hardware::Software* tuningClientObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningClientId2));

			if (tuningClientObject1 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(tuningClientId1);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			if (tuningClientObject2 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(tuningClientId2);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			xmlWriter.writeStartElement("TuningClient");
			{
				xmlWriter.writeAttribute("TuningClientID1", tuningClientId1);
				xmlWriter.writeAttribute("TuningClientID2", tuningClientId2);
			}
			xmlWriter.writeEndElement();


			// TuningServiceID1  ----------------------------------------------
			//
			QString tuningServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (tuningServiceId1.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property TuningServiceID1 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// TuningServiceID2
			//
			QString tuningServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (tuningServiceId2.isEmpty() == true)
			{
				QString errorStr = tr("Metrology configuration error %1, property DataAquisitionServiceID2 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// tuningServiceStrID -- objects
			//
			Hardware::Software* tuningServiceObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId1));
			Hardware::Software* tuningServiceObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId2));

			if (tuningServiceObject1 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId1);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			if (tuningServiceObject2 == nullptr)
			{
				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId2);

				m_log->writeError(errorStr);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
				return false;
			}

			TuningServiceSettings tunsSettings1;
			tunsSettings1.readFromDevice(tuningServiceObject1, m_log);

			TuningServiceSettings tunsSettings2;
			tunsSettings2.readFromDevice(tuningServiceObject2, m_log);

			xmlWriter.writeStartElement("TuningService");
			{
				xmlWriter.writeAttribute("TuningServiceID1", tuningServiceId1);
				xmlWriter.writeAttribute("ip1", tunsSettings1.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port1", QString::number(tunsSettings1.clientRequestIP.port()));

				xmlWriter.writeAttribute("TuningServiceID2", tuningServiceId2);
				xmlWriter.writeAttribute("ip2", tunsSettings2.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port2", QString::number(tunsSettings2.clientRequestIP.port()));
			}
			xmlWriter.writeEndElement();
		}
		xmlWriter.writeEndElement();

		return true;
	}

	bool MetrologyCfgGenerator::writeMetrologySignalsXml()
	{
		UnitList unitInfo;
		m_dbController->getUnits(&unitInfo, nullptr);

		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("MetrologySignals");
		{
			xml.writeIntAttribute("buildID", m_buildResultWriter->buildInfo().id);
			xml.writeIntAttribute("Version", CFG_FILE_VER_METROLOGY_SIGNALS);

			// Writing units
			//
			xml.writeStartElement("Units");
			{
				xml.writeIntAttribute("Count", unitInfo.count());

				int unitsCount = unitInfo.count();

				for (int i = 0; i < unitsCount; i++)
				{
					xml.writeStartElement("Unit");

					xml.writeIntAttribute("ID", unitInfo.keyAt(i));
					xml.writeStringAttribute("Caption", unitInfo[i]);

					xml.writeEndElement();
				}
			}
			xml.writeEndElement();

			QVector<SignalParam> signalsToWrite;

			int signalCount = m_signalSet->count();

			for(int i = 0; i < signalCount; i++)
			{
				Signal& signal = (*m_signalSet)[i];

				bool hasWrongField = false;

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

				SignalLocation location( m_equipment->deviceObject(signal.equipmentID() ) );

				signalsToWrite.append( SignalParam( signal, location) );
			}

			// Writing signals
			//
			xml.writeStartElement("Signals");
			{
				xml.writeIntAttribute("Count", signalsToWrite.count());

				for(SignalParam signal : signalsToWrite)
				{
					signal.writeToXml(xml);
				}
			}
			xml.writeEndElement();	// </Signals>
		}
		xml.writeEndElement();	// </MetrologySignals>
		xml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(m_subDir, "MetrologySignals.xml", CFG_FILE_ID_METROLOGY_SIGNALS, "",  data);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}

	void MetrologyCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}

	SignalLocation::SignalLocation(Hardware::DeviceObject* pDeviceObject)
	{
		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return;
		}

		setEquipmentID(pDeviceObject->equipmentId());

		getParentObject(pDeviceObject);
	}

	void SignalLocation::getParentObject(Hardware::DeviceObject* pDeviceObject)
	{
		if (pDeviceObject == nullptr || pDeviceObject->isRoot() == true)
		{
			return;
		}

		switch(pDeviceObject->deviceType())
		{
			case Hardware::DeviceType::Rack:    setRackCaption(pDeviceObject->caption());   break;
			case Hardware::DeviceType::Chassis: setChassis(pDeviceObject->place());         break;
			case Hardware::DeviceType::Module:  setModule(pDeviceObject->place());          break;
			case Hardware::DeviceType::Signal:  setPlace(pDeviceObject->place());
												setContact( pDeviceObject->equipmentId().remove( pDeviceObject->parent()->equipmentId() ) );    break;
		}

		getParentObject(pDeviceObject->parent());
	}

	void SignalParam::setParam(const Signal& signal, const SignalLocation& location)
	{
		setAppSignalID(signal.appSignalID());
		setCustomAppSignalID(signal.customAppSignalID());
		setCaption(signal.caption());

		setSignalType(signal.signalType());
		setInOutType(signal.inOutType());

		setLocation(location);

		setLowADC(signal.lowADC());
		setHighADC(signal.highADC());

		setInputElectricLowLimit(signal.inputLowLimit());
		setInputElectricHighLimit(signal.inputHighLimit());
		setInputElectricUnitID(signal.inputUnitID());
		setInputElectricSensorType(signal.inputSensorType());
		setInputElectricPrecision(3);

		setInputPhysicalLowLimit(signal.lowEngeneeringUnits());
		setInputPhysicalHighLimit(signal.highEngeneeringUnits());
		setInputPhysicalUnitID(signal.unitID());
		setInputPhysicalPrecision(signal.decimalPlaces());

		switch(signal.outputMode())
		{
			case E::OutputMode::Plus0_Plus5_V:     setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::V);   break;
			case E::OutputMode::Plus4_Plus20_mA:   setOutputElectricLowLimit(4);    setOutputElectricHighLimit(20); setOutputElectricUnitID(E::InputUnit::mA);  break;
			case E::OutputMode::Minus10_Plus10_V:  setOutputElectricLowLimit(-10);  setOutputElectricHighLimit(10); setOutputElectricUnitID(E::InputUnit::V);   break;
			case E::OutputMode::Plus0_Plus5_mA:    setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::mA);  break;
		}
		setOutputElectricSensorType(signal.outputSensorType());
		setOutputElectricPrecision(3);

		setOutputPhysicalLowLimit(signal.outputLowLimit());
		setOutputPhysicalHighLimit(signal.outputHighLimit());
		setOutputPhysicalUnitID(signal.outputUnitID());
		setOutputPhysicalPrecision(signal.decimalPlaces());

		setEnableTuning(signal.enableTuning());
		setTuningDefaultValue(signal.tuningDefaultValue());
	}


	void SignalParam::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Signal");    // <Signal>
		{
			xml.writeStringAttribute("AppSignalID", appSignalID());
			xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
			xml.writeStringAttribute("Caption", caption());

			xml.writeIntAttribute("SignalType", signalType());
			xml.writeIntAttribute("InOutType", TO_INT(inOutType()));

			xml.writeStringAttribute("EquipmentID", location().equipmentID());
			xml.writeStringAttribute("Rack", location().rackCaption());
			xml.writeIntAttribute("Chassis", location().chassis());
			xml.writeIntAttribute("Module", location().module());
			xml.writeIntAttribute("Place", location().place());
			xml.writeStringAttribute("Contact", location().contact());

			xml.writeIntAttribute("LowADC", lowADC());
			xml.writeIntAttribute("HighADC", highADC());

			xml.writeDoubleAttribute("InputElectricLowLimit", inputElectricLowLimit());
			xml.writeDoubleAttribute("InputElectricHighLimit", inputElectricHighLimit());
			xml.writeIntAttribute("InputElectricUnitID", inputElectricUnitID());
			xml.writeIntAttribute("InputElectricSensorType", inputElectricSensorType());
			xml.writeIntAttribute("InputElectricPrecision", inputElectricPrecision());

			xml.writeDoubleAttribute("InputPhysicalLowLimit", inputPhysicalLowLimit());
			xml.writeDoubleAttribute("InputPhysicalHighLimit", inputPhysicalHighLimit());
			xml.writeIntAttribute("InputPhysicalUnitID", inputPhysicalUnitID());
			xml.writeIntAttribute("InputPhysicalPrecision", inputPhysicalPrecision());

			xml.writeDoubleAttribute("OutputElectricLowLimit", outputElectricLowLimit());
			xml.writeDoubleAttribute("OutputElectricHighLimit", outputElectricHighLimit());
			xml.writeIntAttribute("OutputElectricUnitID", outputElectricUnitID());
			xml.writeIntAttribute("OutputElectricSensorType", outputElectricSensorType());
			xml.writeIntAttribute("OutputElectricPrecision", outputElectricPrecision());

			xml.writeDoubleAttribute("OutputPhysicalLowLimit", outputPhysicalLowLimit());
			xml.writeDoubleAttribute("OutputPhysicalHighLimit", outputPhysicalHighLimit());
			xml.writeIntAttribute("OutputPhysicalUnitID", outputPhysicalUnitID());
			xml.writeIntAttribute("OutputPhysicalPrecision", outputPhysicalPrecision());

			xml.writeBoolAttribute("EnableTuning", enableTuning());
			xml.writeDoubleAttribute("TuningDefaultValue", tuningDefaultValue());
		}
		xml.writeEndElement();  // </Signal>
	}
}
