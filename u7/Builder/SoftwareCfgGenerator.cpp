#include "../Builder/SoftwareCfgGenerator.h"

namespace Builder
{
	SoftwareCfgGenerator::SoftwareCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		m_dbController(db),
		m_software(software),
		m_signalSet(signalSet),
		m_equipment(equipment),
		m_buildResultWriter(buildResultWriter)
	{
	}


	bool SoftwareCfgGenerator::run()
	{
		if (m_dbController == nullptr ||
			m_software == nullptr ||
			m_signalSet == nullptr ||
			m_equipment == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(false);
			return false;
		}

		m_log = m_buildResultWriter->log();

		if (m_log == nullptr)
		{
			assert(false);
			return false;
		}

		m_subDir = m_software->strId();

		m_cfgXml = m_buildResultWriter->createConfigurationXmlFile(m_subDir);

		if (m_cfgXml == nullptr)
		{
			LOG_ERROR(m_log, QString(tr("Can't create 'configuration.xml' file for software %1")).
					  arg(m_software->strId()));
			return false;
		}

		LOG_MESSAGE(m_log, QString(tr("Generate configuration for: %1")).
					arg(m_software->strId()));

		m_cfgXml->xmlWriter().writeStartElement("Software");

		m_cfgXml->xmlWriter().writeAttribute("Caption", m_software->caption());
		m_cfgXml->xmlWriter().writeAttribute("StrID", m_software->strId());
		m_cfgXml->xmlWriter().writeAttribute("Type", QString("%1").arg(static_cast<int>(m_software->type())));

		m_cfgXml->xmlWriter().writeEndElement();	// </Software>

		bool result = true;

		switch(static_cast<E::SoftwareType>(m_software->type()))
		{
		case E::SoftwareType::Monitor:
			result = generateMonitorCfg();
			break;

		case E::SoftwareType::DataAcquisitionService:
			result = generateDataAcqisitionServiceCfg();
			break;

		default:
			assert(false);
			LOG_ERROR(m_log, QString(tr("Unknown type of software %1")).
					  arg(m_software->strId()));
			result = false;
		}

		return result;
	}


	bool SoftwareCfgGenerator::generateDataAcqisitionServiceCfg()
	{
		bool result = true;

		result &= writeAppSignalsXml();
		result &= writeEquipmentXml();

		return result;
	}


	bool SoftwareCfgGenerator::writeAppSignalsXml()
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
		appSignalsXml.writeStartElement("configuration");

		// Writing units
		appSignalsXml.writeStartElement("units");
		appSignalsXml.writeAttribute("count", QString::number(unitInfo.count()));

		int unitsCount = unitInfo.count();

		for (int i = 0; i < unitsCount; i++)
		{
			appSignalsXml.writeStartElement("unit");

			appSignalsXml.writeAttribute("ID", QString::number(unitInfo.keyAt(i)));
			appSignalsXml.writeAttribute("name", unitInfo[i]);

			appSignalsXml.writeEndElement();
		}

		appSignalsXml.writeEndElement();

		// Writing signals
		appSignalsXml.writeStartElement("applicationSignals");

		for (int i = 0; i < m_signalSet->count(); i++)
		{
			Signal& signal = (*m_signalSet)[i];

			bool hasWrongField = false;

			if (!dataFormatInfo.contains(signal.dataFormatInt()))
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong dataFormat field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.unitID()))
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong unitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.inputUnitID()))
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong inputUnitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (!unitInfo.contains(signal.outputUnitID()))
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong outputUnitID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.inputSensorID() < 0 || signal.inputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong inputSensorID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.outputSensorID() < 0 || signal.outputSensorID() >= SENSOR_TYPE_COUNT)
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong outputSensorID field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.outputRangeMode() < 0 || signal.outputRangeMode() >= OUTPUT_RANGE_MODE_COUNT)
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong outputRangeMode field").arg(signal.strID()));
				hasWrongField = true;
			}
			if (signal.inOutType() < 0 || signal.inOutType() >= IN_OUT_TYPE_COUNT)
			{
				LOG_WARNING(m_log, QString("Signal %1 has wrong inOutType field").arg(signal.strID()));
				hasWrongField = true;
			}

			switch (static_cast<E::ByteOrder>(signal.byteOrderInt()))
			{
				case E::ByteOrder::LittleEndian:
				case E::ByteOrder::BigEndian:
					break;
				default:
					LOG_WARNING(m_log, QString("Signal %1 has wrong byteOrder field").arg(signal.strID()));
					hasWrongField = true;
			}

			if (hasWrongField)
			{
				continue;
			}

			appSignalsXml.writeStartElement("signal");

			appSignalsXml.writeAttribute("ID", QString::number(signal.ID()));
			appSignalsXml.writeAttribute("signalGroupID", QString::number(signal.signalGroupID()));
			appSignalsXml.writeAttribute("signalInstanceID", QString::number(signal.signalInstanceID()));
			appSignalsXml.writeAttribute("channel", QString::number(signal.channel()));
			appSignalsXml.writeAttribute("type", signal.type() == E::SignalType::Analog ? "Analog" : "Discrete");
			appSignalsXml.writeAttribute("strID", signal.strID());
			appSignalsXml.writeAttribute("extStrID", signal.extStrID());
			appSignalsXml.writeAttribute("name", signal.name());
			appSignalsXml.writeAttribute("dataFormat", dataFormatInfo.value(signal.dataFormatInt()));
			appSignalsXml.writeAttribute("dataSize", QString::number(signal.dataSize()));
			appSignalsXml.writeAttribute("lowADC", QString::number(signal.lowADC()));
			appSignalsXml.writeAttribute("highADC", QString::number(signal.highADC()));
			appSignalsXml.writeAttribute("lowLimit", QString::number(signal.lowLimit()));
			appSignalsXml.writeAttribute("highLimit", QString::number(signal.highLimit()));
			appSignalsXml.writeAttribute("unitID", unitInfo.value(signal.unitID()));
			appSignalsXml.writeAttribute("adjustment", QString::number(signal.adjustment()));
			appSignalsXml.writeAttribute("dropLimit", QString::number(signal.dropLimit()));
			appSignalsXml.writeAttribute("excessLimit", QString::number(signal.excessLimit()));
			appSignalsXml.writeAttribute("unbalanceLimit", QString::number(signal.unbalanceLimit()));
			appSignalsXml.writeAttribute("inputLowLimit", QString::number(signal.inputLowLimit()));
			appSignalsXml.writeAttribute("inputHighLimit", QString::number(signal.inputHighLimit()));
			appSignalsXml.writeAttribute("inputUnitID", unitInfo.value(signal.inputUnitID()));
			appSignalsXml.writeAttribute("inputSensorID", SensorTypeStr[signal.inputSensorID()]);
			appSignalsXml.writeAttribute("outputLowLimit", QString::number(signal.outputLowLimit()));
			appSignalsXml.writeAttribute("outputHighLimit", QString::number(signal.outputHighLimit()));
			appSignalsXml.writeAttribute("outputUnitID", unitInfo.value(signal.outputUnitID()));
			appSignalsXml.writeAttribute("outputRangeMode", OutputRangeModeStr[signal.outputRangeMode()]);
			appSignalsXml.writeAttribute("outputSensorID", SensorTypeStr[signal.outputSensorID()]);
			appSignalsXml.writeAttribute("acquire", signal.acquire() ? "true" : "false");
			appSignalsXml.writeAttribute("calculated", signal.calculated() ? "true" : "false");
			appSignalsXml.writeAttribute("normalState", QString::number(signal.normalState()));
			appSignalsXml.writeAttribute("decimalPlaces", QString::number(signal.decimalPlaces()));
			appSignalsXml.writeAttribute("aperture", QString::number(signal.aperture()));
			appSignalsXml.writeAttribute("inOutType", InOutTypeStr[signal.inOutType()]);
			appSignalsXml.writeAttribute("deviceStrID", signal.deviceStrID());
			appSignalsXml.writeAttribute("filteringTime", QString::number(signal.filteringTime()));
			appSignalsXml.writeAttribute("maxDifference", QString::number(signal.maxDifference()));
			appSignalsXml.writeAttribute("byteOrder", E::valueToString<E::ByteOrder>(signal.byteOrderInt()));
			appSignalsXml.writeAttribute("ramAddr", signal.ramAddr().toString());
			appSignalsXml.writeAttribute("regAddr", signal.regAddr().toString());

			appSignalsXml.writeEndElement();	// signal
		}

		appSignalsXml.writeEndElement();	// applicationSignals
		appSignalsXml.writeEndElement();	// configuration
		appSignalsXml.writeEndDocument();

		m_buildResultWriter->addFile(m_subDir, "appSignals.xml", data);

		m_cfgXml->addLinkToFile(m_subDir, "appSignals.xml");

		return true;
	}

	bool SoftwareCfgGenerator::writeEquipmentXml()
	{
		Hardware::DeviceRoot* deviceRoot;

		Hardware::DeviceObject* currentDevice = m_software->parent();
		while (currentDevice != nullptr)
		{
			if (typeid(*currentDevice) == typeid(Hardware::DeviceRoot))
			{
				deviceRoot = dynamic_cast<Hardware::DeviceRoot*>(currentDevice);
				break;
			}
			currentDevice = currentDevice->parent();
		}

		if (deviceRoot != nullptr)
		{
			QByteArray data;
			QXmlStreamWriter equipmentWriter(&data);

			equipmentWriter.setAutoFormatting(true);
			equipmentWriter.writeStartDocument();

			equipmentWalker(deviceRoot, [&equipmentWriter](Hardware::DeviceObject* currentDevice)
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
		}
		return true;
	}


	bool SoftwareCfgGenerator::generateMonitorCfg()
	{
		bool result = true;

		if (m_software == nullptr ||
			m_software->type() != E::SoftwareType::Monitor ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(m_software);
			assert(m_software->type() == E::SoftwareType::Monitor);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			return false;
		}

		// write XML via m_cfgXml->xmlWriter()
		//
		result &= writeMonitorSettings();

		// write any files via m_buildResultWriter->addFile(...)
		//

		// add link to configuration files (previously written) via m_cfgXml->addLinkToFile(...)
		//

		return result;
	}

	bool SoftwareCfgGenerator::writeMonitorSettings()
	{
		// write XML via m_cfgXml->xmlWriter()
		//
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		{
			xmlWriter.writeStartElement("Settings");
			std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

			// --
			//
			QVariant vDataAquisistionService = m_software->propertyValue("DataAquisitionServiceStrID");

			if (vDataAquisistionService.isValid() == false ||
				vDataAquisistionService.type() != QVariant::String)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID is invalid").arg(m_software->strId());
				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			QString dasStrId = vDataAquisistionService.toString().trimmed();

			if (dasStrId.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID is empty").arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			Hardware::DeviceObject* dasObject = m_equipment->deviceObject(dasStrId);
			Hardware::Software* dasSoftwareObject = dynamic_cast<Hardware::Software*>(dasObject);

			if (dasObject == nullptr)
			{
				QString errorStr = tr("Monitor configuration error %1, DataAquisitionServiceStrID %2 is not found")
								   .arg(m_software->strId())
								   .arg(dasStrId);

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			if (dasObject->isSoftware() == false ||
				dasSoftwareObject == nullptr ||
				dasSoftwareObject->type() != E::SoftwareType::DataAcquisitionService)
			{
				QString errorStr = tr("Monitor configuration error %1, DataAquisitionServiceStrID %2 is not software")
								  .arg(m_software->strId())
								  .arg(dasStrId);

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// Get ip addresses and ports, write them to configurations
			//
			{
				xmlWriter.writeStartElement("DataAquisitionService");
				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
					{
						xmlWriter.writeEndElement();
					});

				// --
				//
				xmlWriter.writeAttribute("StrID", dasSoftwareObject->strId());

				xmlWriter.writeStartElement("Connection1");
				xmlWriter.writeAttribute("ip", "127.0.0.1");
				xmlWriter.writeAttribute("port", QString::number(PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));
				xmlWriter.writeEndElement();		// Connection1"

				xmlWriter.writeStartElement("Connection2");
				xmlWriter.writeAttribute("ip", "127.0.0.1");
				xmlWriter.writeAttribute("port", QString::number(PORT_DATA_AQUISITION_SERVICE_CLIENT_REQUEST));
				xmlWriter.writeEndElement();		// Connection2"
			}	// DataAquisitionService


		} // Settings


		return true;
	}

	void SoftwareCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}

