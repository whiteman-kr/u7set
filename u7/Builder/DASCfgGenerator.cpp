#include "DASCfgGenerator.h"
#include "../include/ServiceSettings.h"


namespace Builder
{
	DASCfgGenerator::DASCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	DASCfgGenerator::~DASCfgGenerator()
	{
	}


	bool DASCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result &= writeSettings();
		result &= writeAppSignalsXml();
		result &= writeEquipmentXml();
		result &= writeDataSourcesXml();

		return result;
	}


	bool DASCfgGenerator::writeSettings()
	{
		DASSettings dasSettings;

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

	bool DASCfgGenerator::writeAppSignalsXml()
	{
		if (m_signalSet->isEmpty())
		{
			LOG_MESSAGE(m_log, tr("Signals not found!"));
			return true;
		}

		UnitList unitInfo;
		m_dbController->getUnits(&unitInfo, nullptr);

		DataFormatList dataFormatInfo;

		QByteArray data;
		QXmlStreamWriter appSignalsXml(&data);

		appSignalsXml.setAutoFormatting(true);
		appSignalsXml.writeStartDocument();
		appSignalsXml.writeStartElement("AppSignals");

		// Writing units
		appSignalsXml.writeStartElement("Units");
		appSignalsXml.writeAttribute("Count", QString::number(unitInfo.count()));

		int unitsCount = unitInfo.count();

		for (int i = 0; i < unitsCount; i++)
		{
			appSignalsXml.writeStartElement("Unit");

			appSignalsXml.writeAttribute("ID", QString::number(unitInfo.keyAt(i)));
			appSignalsXml.writeAttribute("Name", unitInfo[i]);

			appSignalsXml.writeEndElement();
		}

		appSignalsXml.writeEndElement();

		// Writing signals
		appSignalsXml.writeStartElement("Signals");

		for (int i = 0; i < m_signalSet->count(); i++)
		{
			Signal& signal = (*m_signalSet)[i];

			bool hasWrongField = false;

			if (!dataFormatInfo.contains(signal.dataFormatInt()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong dataFormat field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.unitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong unitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.inputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputUnitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.outputUnitID()))
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputUnitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.inputSensorID() < 0 || signal.inputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inputSensorID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.outputSensorID() < 0 || signal.outputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputSensorID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.outputRangeMode() < 0 || signal.outputRangeMode() >= OUTPUT_RANGE_MODE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong outputRangeMode field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.inOutType() < 0 || signal.inOutType() >= IN_OUT_TYPE_COUNT)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong inOutType field").arg(signal.strID()));
				hasWrongField = true;
			}

			switch (static_cast<E::ByteOrder>(signal.byteOrderInt()))
			{
				case E::ByteOrder::LittleEndian:
				case E::ByteOrder::BigEndian:
					break;
				default:
					LOG_WARNING_OBSOLETE(m_log, IssuePrexif::NotDefined, QString("Signal %1 has wrong byteOrder field").arg(signal.strID()));
					hasWrongField = true;
			}

			if (hasWrongField)
			{
				continue;
			}

			appSignalsXml.writeStartElement("Signal");

			appSignalsXml.writeAttribute("ID", QString::number(signal.ID()));
			appSignalsXml.writeAttribute("SignalGroupID", QString::number(signal.signalGroupID()));
			appSignalsXml.writeAttribute("SignalInstanceID", QString::number(signal.signalInstanceID()));
			appSignalsXml.writeAttribute("Channel", QString::number(signal.channel()));
			appSignalsXml.writeAttribute("Type", signal.type() == E::SignalType::Analog ? "Analog" : "Discrete");
			appSignalsXml.writeAttribute("StrID", signal.strID());
			appSignalsXml.writeAttribute("ExtStrID", signal.extStrID());
			appSignalsXml.writeAttribute("Name", signal.caption());
			appSignalsXml.writeAttribute("DataFormat", dataFormatInfo.value(signal.dataFormatInt()));
			appSignalsXml.writeAttribute("DataSize", QString::number(signal.dataSize()));
			appSignalsXml.writeAttribute("LowADC", QString::number(signal.lowADC()));
			appSignalsXml.writeAttribute("HighADC", QString::number(signal.highADC()));
			appSignalsXml.writeAttribute("LowLimit", QString::number(signal.lowLimit()));
			appSignalsXml.writeAttribute("HighLimit", QString::number(signal.highLimit()));
			appSignalsXml.writeAttribute("UnitID", unitInfo.value(signal.unitID()));
			appSignalsXml.writeAttribute("Adjustment", QString::number(signal.adjustment()));
			appSignalsXml.writeAttribute("DropLimit", QString::number(signal.dropLimit()));
			appSignalsXml.writeAttribute("ExcessLimit", QString::number(signal.excessLimit()));
			appSignalsXml.writeAttribute("UnbalanceLimit", QString::number(signal.unbalanceLimit()));
			appSignalsXml.writeAttribute("InputLowLimit", QString::number(signal.inputLowLimit()));
			appSignalsXml.writeAttribute("InputHighLimit", QString::number(signal.inputHighLimit()));
			appSignalsXml.writeAttribute("InputUnitID", unitInfo.value(signal.inputUnitID()));
			appSignalsXml.writeAttribute("InputSensorID", SensorTypeStr[signal.inputSensorID()]);
			appSignalsXml.writeAttribute("OutputLowLimit", QString::number(signal.outputLowLimit()));
			appSignalsXml.writeAttribute("OutputHighLimit", QString::number(signal.outputHighLimit()));
			appSignalsXml.writeAttribute("OutputUnitID", unitInfo.value(signal.outputUnitID()));
			appSignalsXml.writeAttribute("OutputRangeMode", OutputRangeModeStr[signal.outputRangeMode()]);
			appSignalsXml.writeAttribute("OutputSensorID", SensorTypeStr[signal.outputSensorID()]);
			appSignalsXml.writeAttribute("Acquire", signal.acquire() ? "true" : "false");
			appSignalsXml.writeAttribute("Calculated", signal.calculated() ? "true" : "false");
			appSignalsXml.writeAttribute("NormalState", QString::number(signal.normalState()));
			appSignalsXml.writeAttribute("DecimalPlaces", QString::number(signal.decimalPlaces()));
			appSignalsXml.writeAttribute("Aperture", QString::number(signal.aperture()));
			appSignalsXml.writeAttribute("InOutType", InOutTypeStr[signal.inOutType()]);
			appSignalsXml.writeAttribute("DeviceStrID", signal.deviceStrID());
			appSignalsXml.writeAttribute("FilteringTime", QString::number(signal.filteringTime()));
			appSignalsXml.writeAttribute("MaxDifference", QString::number(signal.maxDifference()));
			appSignalsXml.writeAttribute("ByteOrder", E::valueToString<E::ByteOrder>(signal.byteOrderInt()));
			appSignalsXml.writeAttribute("RamAddr", signal.ramAddr().toString());
			appSignalsXml.writeAttribute("RegAddr", signal.regAddr().toString());

			appSignalsXml.writeEndElement();	// signal
		}

		appSignalsXml.writeEndElement();	// </Signals>
		appSignalsXml.writeEndElement();	// </AppSignals>
		appSignalsXml.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "appSignals.xml", data);

		m_cfgXml->addLinkToFile(m_subDir, "appSignals.xml");

		return true;
	}


	bool DASCfgGenerator::writeEquipmentXml()
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


	bool DASCfgGenerator::writeDataSourcesXml()
	{
		bool result = true;

		QByteArray data;
		QXmlStreamWriter xml(&data);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();
		xml.writeStartElement("DataSources");

		DataSource ds;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				result = false;
				continue;
			}

			for(int channel = 1; channel <= 2; channel++)
			{
				LmEthernetAdapterNetworkProperties lmNetProperties;

				int adapter = LM_ETHERNET_ADAPTER2;

				if (channel == 2)
				{
					adapter = LM_ETHERNET_ADAPTER3;
				}

				result &= lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, adapter, m_log);

				if (result == false)
				{
					break;
				}

				if (lmNetProperties.appDataServiceStrID == m_software->strId())
				{
					ds.setEthernetChannel(channel);
					ds.setDataType(DataSource::DataType::App);
					ds.setLmStrID(lm->strId());
					ds.setLmCaption(lm->caption());
					ds.setLmAdapterStrID(lmNetProperties.adapterStrID);
					ds.setLmDataEnable(lmNetProperties.appDataEnable);
					ds.setLmAddressStr(lmNetProperties.appDataIP);
					ds.setLmPort(lmNetProperties.appDataPort);

					ds.serializeToXml(xml);
				}

				if (lmNetProperties.diagDataServiceStrID == m_software->strId())
				{
					ds.setEthernetChannel(channel);
					ds.setDataType(DataSource::DataType::Diag);
					ds.setLmStrID(lm->strId());
					ds.setLmCaption(lm->caption());
					ds.setLmAdapterStrID(lmNetProperties.adapterStrID);
					ds.setLmDataEnable(lmNetProperties.diagDataEnable);
					ds.setLmAddressStr(lmNetProperties.diagDataIP);
					ds.setLmPort(lmNetProperties.diagDataPort);

					ds.serializeToXml(xml);
				}
			}

			if (result == false)
			{
				break;
			}
		}

		xml.writeEndElement();	// </DataSources>
		xml.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "dataSources.xml", data);
		m_cfgXml->addLinkToFile(m_subDir, "dataSources.xml");

		return result;
	}


	bool DASCfgGenerator::LmEthernetAdapterNetworkProperties::getLmEthernetAdapterNetworkProperties(Hardware::DeviceModule* lm, int adptrNo, IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		if (adptrNo < LM_ETHERNET_ADAPTER1 ||
			adptrNo > LM_ETHERNET_ADAPTER3)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		adapterNo = adptrNo;

		QString suffix = QString("_ETHERNET0%1").arg(adapterNo);

		Hardware::DeviceController* adapter = DeviceHelper::getChildControllerBySuffix(lm, suffix);

		if (adapter == nullptr)
		{
			LOG_ERROR_OBSOLETE(log, IssuePrexif::NotDefined,
							   QString("Can't find child object by suffix '%1' in object '%2'").
							   arg(suffix).arg(lm->strId()));
			return false;
		}

		adapterStrID = adapter->strId();

		bool result = true;

		if (adptrNo == LM_ETHERNET_ADAPTER1)
		{
			// tunig adapter
			//
			result &= DeviceHelper::getBoolProperty(adapter, "TuningEnable", &tuningEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "TuningIP", &tuningIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "TuningPort", &tuningPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "TuningServiceStrID", &tuningServiceStrID, log);
			return result;
		}

		if (adptrNo == LM_ETHERNET_ADAPTER2 ||
			adptrNo == LM_ETHERNET_ADAPTER3)
		{
			// application and diagnostics data adapter
			//
			result &= DeviceHelper::getBoolProperty(adapter, "AppDataEnable", &appDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "AppDataIP", &appDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "AppDataPort", &appDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "AppDataServiceStrID", &appDataServiceStrID, log);

			result &= DeviceHelper::getBoolProperty(adapter, "DiagDataEnable", &diagDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "DiagDataIP", &diagDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "DiagDataPort", &diagDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "DiagDataServiceStrID", &diagDataServiceStrID, log);

			return result;
		}

		assert(false);
		return false;
	}


}
