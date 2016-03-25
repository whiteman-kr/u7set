#include "../Builder/SoftwareCfgGenerator.h"
#include "IssueLogger.h"

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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Can't create 'configuration.xml' file for software %1")).
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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Unknown type of software %1")).arg(m_software->strId()));
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
			appSignalsXml.writeAttribute("Name", signal.name());
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
			bool ok = true;

			// DataAquisitionServiceStrID1
			//
			QString dacStrID1 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceStrID1", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID1.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID1 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// DataAquisitionServiceStrID2
			//
			QString dacStrID2 = getObjectProperty<QString>(m_software->strId(), "DataAquisitionServiceStrID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (dacStrID2.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceStrID2 is invalid")
								   .arg(m_software->strId());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
			//
			QString clientRequestIP1 = getObjectProperty<QString>(dacStrID1, "ClientRequestIP", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			QString clientRequestPort1 = getObjectProperty<QString>(dacStrID1, "ClientRequestPort", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
			//
			QString clientRequestIP2 = getObjectProperty<QString>(dacStrID2, "ClientRequestIP", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			QString clientRequestPort2 = getObjectProperty<QString>(dacStrID2, "ClientRequestPort", &ok).trimmed();
			if (ok == false)
			{
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
				xmlWriter.writeAttribute("StrID1", dacStrID1);
				xmlWriter.writeAttribute("StrID2", dacStrID2);

				xmlWriter.writeAttribute("ip1", clientRequestIP1);
				xmlWriter.writeAttribute("port1", clientRequestPort1);
				xmlWriter.writeAttribute("ip2", clientRequestIP2);
				xmlWriter.writeAttribute("port2", clientRequestPort2);
			}	// DataAquisitionService


			// Archive Service Settings
			//

//			// ArchiveServiceStrID1
//			//
//			QString asStrID1 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID1", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID1.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID1 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// ArchiveServiceStrID2
//			//
//			QString asStrID2 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID2", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID2.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID2 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP1 = getObjectProperty<QString>(dacStrID1, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort1 = getObjectProperty<QString>(dacStrID1, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP2 = getObjectProperty<QString>(dacStrID2, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort2 = getObjectProperty<QString>(dacStrID2, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// Get ip addresses and ports, write them to configurations
//			//
//			{
//				xmlWriter.writeStartElement("DataAquisitionService");
//				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
//					{
//						xmlWriter.writeEndElement();
//					});

//				// --
//				//
//				xmlWriter.writeAttribute("StrID1", dacStrID1);
//				xmlWriter.writeAttribute("StrID2", dacStrID2);

//				xmlWriter.writeAttribute("ip1", clientRequestIP1);
//				xmlWriter.writeAttribute("port1", clientRequestPort1);
//				xmlWriter.writeAttribute("ip2", clientRequestIP2);
//				xmlWriter.writeAttribute("port2", clientRequestPort2);
//			}	// DataAquisitionService


		} // Settings


		return true;
	}

	void SoftwareCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}

