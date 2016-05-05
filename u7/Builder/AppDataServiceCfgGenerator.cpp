#include "AppDataServiceCfgGenerator.h"
#include "../include/ServiceSettings.h"


namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	AppDataServiceCfgGenerator::~AppDataServiceCfgGenerator()
	{
	}


	bool AppDataServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeAppDataSourcesXml();
		result &= writeAppSignalsXml();
//		result &= writeEquipmentXml();

		return result;
	}


	bool AppDataServiceCfgGenerator::writeSettings()
	{
		AppDataServiceSettings dasSettings;

		bool result = true;

		result = dasSettings.readFromDevice(m_software, m_log);

		if (result == false)
		{
			return false;
		}

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		dasSettings.writeToXml(xml);

		return true;
	}

	bool AppDataServiceCfgGenerator::writeAppSignalsXml()
	{
		UnitList unitInfo;
		m_dbController->getUnits(&unitInfo, nullptr);

		DataFormatList dataFormatInfo;

		QByteArray data;
		XmlWriteHelper xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("AppSignals");

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

			if (m_associatedAppSignals.contains(signal.appSignalID()) == false)
			{
				continue;
			}

			bool hasWrongField = false;

			if (!dataFormatInfo.contains(signal.dataFormatInt()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong dataFormat field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

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

			if (signal.inputSensorID() < 0 || signal.inputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (signal.outputSensorID() < 0 || signal.outputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputSensorID field").arg(signal.appSignalID()));
				hasWrongField = true;
			}

			if (signal.outputRangeMode() < 0 || signal.outputRangeMode() >= OUTPUT_RANGE_MODE_COUNT)
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

		m_buildResultWriter->addFile(m_subDir, "appSignals.xml", CFG_FILE_ID_APP_SIGNALS, "",  data);

		m_cfgXml->addLinkToFile(m_subDir, "appSignals.xml");

		return true;
	}


	bool AppDataServiceCfgGenerator::writeEquipmentXml()
	{
		QByteArray data;
		QXmlStreamWriter equipmentWriter(&data);

		equipmentWriter.setAutoFormatting(true);
		equipmentWriter.writeStartDocument();

		equipmentWalker(m_deviceRoot, [&equipmentWriter](Hardware::DeviceObject* currentDevice)
		{
			if (currentDevice == nullptr)
			{
				return;
			}
			const QMetaObject* metaObject = currentDevice->metaObject();
			QString name = metaObject->className();
			int position = name.lastIndexOf(QChar(':'));
			if (position == -1)
			{
				equipmentWriter.writeStartElement(name);
			}
			else
			{
				equipmentWriter.writeStartElement(name.mid(position + 1));
			}

			const std::string& className = metaObject->className();
			equipmentWriter.writeAttribute("classNameHash", QString::number(CUtils::GetClassHashCode(className), 16));

			for (auto p : currentDevice->properties())
			{
				if (p->readOnly())
				{
					continue;
				}
				QVariant tmp = p->value();
				assert(tmp.convert(QMetaType::QString));
				equipmentWriter.writeAttribute(p->caption(), tmp.toString());
			}
		}, [&equipmentWriter](Hardware::DeviceObject*)
		{
			equipmentWriter.writeEndElement();
		});

		//equipmentWriter.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "equipment.xml", data);

		m_cfgXml->addLinkToFile(m_subDir, "equipment.xml");

		return true;
	}


	bool AppDataServiceCfgGenerator::writeAppDataSourcesXml()
	{
		bool result = true;

		QByteArray data;
		QXmlStreamWriter xmlWriter(&data);

		XmlWriteHelper xml(xmlWriter);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("AppDataSources");

		m_associatedAppSignals.clear();

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				result = false;
				continue;
			}

			for(int channel = 0; channel < AppDataServiceSettings::DATA_CHANNEL_COUNT; channel++)
			{
				LmEthernetAdapterNetworkProperties lmNetProperties;

				int adapter = LM_ETHERNET_ADAPTER2;

				if (channel == 1)
				{
					adapter = LM_ETHERNET_ADAPTER3;
				}

				result &= lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, adapter, m_log);

				if (result == false)
				{
					break;
				}

				if (lmNetProperties.appDataServiceID == m_software->equipmentIdTemplate())
				{
					DataSource ds;

					ds.setChannel(channel);
					ds.setDataType(DataSource::DataType::App);
					ds.setLmStrID(lm->equipmentIdTemplate());
					ds.setLmCaption(lm->caption());
					ds.setLmAdapterStrID(lmNetProperties.adapterID);
					ds.setLmDataEnable(lmNetProperties.appDataEnable);
					ds.setLmAddressStr(lmNetProperties.appDataIP);
					ds.setLmPort(lmNetProperties.appDataPort);

					result &= findAppDataSourceAssociatedSignals(ds);	// inside fills m_associatedAppSignals also

					ds.writeToXml(xml);
				}
			}

			if (result == false)
			{
				break;
			}
		}

		xml.writeEndElement();	// </AppDataSources>
		xml.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "AppDataSources.xml", CFG_FILE_ID_DATA_SOURCES, "", data);
		m_cfgXml->addLinkToFile(m_subDir, "AppDataSources.xml");

		return result;
	}


	bool AppDataServiceCfgGenerator::findAppDataSourceAssociatedSignals(DataSource& appDataSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(appDataSource.lmStrID());

		if (lm == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Not found LM with ID '%1'").arg(appDataSource.lmStrID()));
			return false;
		}

		const Hardware::DeviceChassis* dataSourceChassis = lm->getParentChassis();

		int signalCount = m_signalSet->count();

		for(int i = 0; i < signalCount; i++)
		{
			const Signal& appSignal =  (*m_signalSet)[i];

			QString appSignalEquipmentID = appSignal.equipmentID();

			if (appSignalEquipmentID.isEmpty())
			{
				continue;
			}

			Hardware::DeviceObject* device = m_equipment->deviceObject(appSignalEquipmentID);

			if (device == nullptr)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal '%1' bound with an unknown device '%2'").
					arg(appSignal.appSignalID()).arg(appSignalEquipmentID));
				continue;
			}

			const Hardware::DeviceChassis* chassis = device->getParentChassis();

			if (chassis == dataSourceChassis)
			{
				appDataSource.addAssociatedSignal(appSignal.appSignalID());

				m_associatedAppSignals.insert(appSignal.appSignalID(), true);
			}
		}

		return true;
	}
}
