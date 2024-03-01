#include "GatewayDescriptionParser.h"
#include "ModbusTcpSlaveGateway.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

namespace Gateway
{
   // ---------------------------------------------------------------------------------
   //
   // Class Gateway::ModbusSignalList implementation
   //
   // ---------------------------------------------------------------------------------

	const std::set<E::Setting> ModbusSignalList::m_requiredSettings =
	{
		E::Setting::AnalogFormat,
		E::Setting::DiscreteFormat,
	};

	ModbusSignalList::ModbusSignalList()
	{
	}

	bool ModbusSignalList::isKnownSetting(E::Setting st) const
	{
		return m_requiredSettings.contains(st);
	}

	bool ModbusSignalList::checkAndApplySettings(int lineNo, ParserLog& log)
	{
		bool result = true;

		result &= SignalList::checkAndApplySettings(lineNo, log);

		result &= Gateway::checkRequiredSettings(m_requiredSettings,
												 m_settingsValues,
												 lineNo, log);
		RETURN_IF_FALSE(result);

		for(const auto& p : m_settingsValues)
		{
			E::Setting st = p.first;
			const SettingValue& sv = p.second;

			switch(st)
			{
			case E::Setting::AnalogFormat:
				result &= checkAndApplyAnalogFormat(sv, log);
				break;

			case E::Setting::DiscreteFormat:
				result &= checkAndApplyDiscreteFormat(sv, log);
				break;

			default:
				Q_ASSERT(false);
			}
		}

		return result;
	}

	void ModbusSignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		// xml.writeIntAttribute(XmlAttribute::LIST_NO, m_listNo);
		// xml.writeEnumKeyAttribute<E::SignalListDataType>(XmlAttribute::DATA_TYPE, m_dataType);
		// xml.writeBoolAttribute(XmlAttribute::SEND_EVENTS, m_sendEvents);
		// xml.writeBoolAttribute(XmlAttribute::INCLUDE_APP_SIGNAL_ID, m_includeAppSignalID);

		xml.writeEndElement();		//	</SignalList>
	}

	bool ModbusSignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);

		// result &= xml.readIntAttribute(XmlAttribute::LIST_NO, &m_listNo);
		// result &= xml.readEnumKeyAttribute<E::SignalListDataType>(XmlAttribute::DATA_TYPE, &m_dataType);
		// result &= xml.readBoolAttribute(XmlAttribute::SEND_EVENTS, &m_sendEvents);
		// result &= xml.readBoolAttribute(XmlAttribute::INCLUDE_APP_SIGNAL_ID, &m_includeAppSignalID);

		return result;
	}

	bool ModbusSignalList::checkAndApplyAnalogFormat(const SettingValue& sv, ParserLog& log)
	{
		Q_ASSERT(sv.setting == E::Setting::AnalogFormat);

		QString str = sv.value.toString().toLower();

		str.replace(Separator::COMMA, Separator::SPACE);

		QStringList options = str.split(Separator::SPACE, Qt::SkipEmptyParts);

		m_commonAnalogFormat.byteOrder = ::E::ByteOrder::LittleEndian;

		if (options.contains("be") == true &&
			options.contains("le") == true)
		{
			log.logError(sv.lineNo, QString("Byte order is not specified in setting '%1'").arg(sv.settingName()));
			return false;
		}

		if (options.contains("be") == true)
		{
			m_commonAnalogFormat.byteOrder = ::E::ByteOrder::BigEndian;
		}
		else
		{
			m_commonAnalogFormat.byteOrder = ::E::ByteOrder::LittleEndian;
		}

		if (options.contains("float16") == true)
		{
			m_commonAnalogFormat.dataFormat = E::ModbusDataFormat::AnalogFloat16;
		}
		else
		{
			log.logError(sv.lineNo, QString("Data format is not specified in setting '%1'").arg(sv.settingName()));
			return false;
		}

		return true;
	}

	bool ModbusSignalList::checkAndApplyDiscreteFormat(const SettingValue& sv, ParserLog& log)
	{
		Q_ASSERT(sv.setting == E::Setting::DiscreteFormat);

		QString str = sv.value.toString().toLower();

		str.replace(Separator::COMMA, Separator::SPACE);

		QStringList options = str.split(Separator::SPACE, Qt::SkipEmptyParts);

		m_commonAnalogFormat.byteOrder = ::E::ByteOrder::LittleEndian;

		if (options.contains("be") == true &&
			options.contains("le") == true)
		{
			log.logError(sv.lineNo, QString("Byte order is not specified in setting '%1'").arg(sv.settingName()));
			return false;
		}

		if (options.contains("be") == true)
		{
			m_commonAnalogFormat.byteOrder = ::E::ByteOrder::BigEndian;
		}
		else
		{
			m_commonAnalogFormat.byteOrder = ::E::ByteOrder::LittleEndian;
		}

		if (options.contains("uint16") == true)
		{
			m_commonAnalogFormat.dataFormat = E::ModbusDataFormat::DiscreteUint16;
		}
		else
		{
			log.logError(sv.lineNo, QString("Data format is not specified in setting '%1'").arg(sv.settingName()));
			return false;
		}

		return true;
	}

   // ---------------------------------------------------------------------------------
   //
   // Class Gateway::ModbusTcpSlaveGateway implementation
   //
   // ---------------------------------------------------------------------------------

	const std::set<E::Setting> ModbusTcpSlaveGateway::m_requiredSettings =
	{
			E::Setting::LocalGatewayIP1,
			E::Setting::RemoteGatewayIP1,

			E::Setting::CodingMode,
			E::Setting::ModbusDeviceAddress,
	};

	const std::set<E::Setting> ModbusTcpSlaveGateway::m_optionalSettings =
		{
			E::Setting::LocalGatewayIP2,
			E::Setting::RemoteGatewayIP2
	};

	ModbusTcpSlaveGateway::ModbusTcpSlaveGateway() :
		Gateway(E::GatewayType::IVS_Impulse)
	{
	}

	ModbusTcpSlaveGateway::ModbusTcpSlaveGateway(const QString& gwID, const QString& gwDesc) :
		Gateway(E::GatewayType::IVS_Impulse, gwID, gwDesc)
	{
	}

	bool ModbusTcpSlaveGateway::isKnownSetting(E::Setting st) const
	{
		return Gateway::isKnownSetting(st) ||
			   m_requiredSettings.contains(st) ||
			   m_optionalSettings.contains(st);
	}

	bool ModbusTcpSlaveGateway::checkAndApplySettings(int lineNo, ParserLog& log)
	{
		bool result = true;

		result &= Gateway::checkAndApplySettings(lineNo, log);
		result &= Gateway::checkRequiredSettings(m_requiredSettings,
												 m_settingsValues,
												 lineNo, log);
		RETURN_IF_FALSE(result);

		HostAddressPort addrPort;

		for(const auto& p: m_settingsValues)
		{
			E::Setting st = p.first;
			const SettingValue& sv = p.second;

			switch(st)
			{
			case E::Setting::LocalGatewayIP1:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_localGatewayIP1 = addrPort;
				break;

			case E::Setting::RemoteGatewayIP1:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_remoteGatewayIP1 = addrPort;
				break;

			case E::Setting::LocalGatewayIP2:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_localGatewayIP2 = addrPort;
				break;

			case E::Setting::RemoteGatewayIP2:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_remoteGatewayIP2 = addrPort;
				break;

			default:
				;	// ok
			}
		}

		return result;
	}

	void ModbusTcpSlaveGateway::appendSignalList()
	{
		m_signalLists.push_back(std::make_shared<ModbusSignalList>());
	}

	HostAddressPort ModbusTcpSlaveGateway::localGatewayIP1() const
	{
		return m_localGatewayIP1;
	}

	HostAddressPort ModbusTcpSlaveGateway::remoteGatewayIP1() const
	{
		return m_remoteGatewayIP1;
	}

	HostAddressPort ModbusTcpSlaveGateway::localGatewayIP2() const
	{
		return m_localGatewayIP2;
	}

	HostAddressPort ModbusTcpSlaveGateway::remoteGatewayIP2() const
	{
		return m_remoteGatewayIP2;
	}

	void ModbusTcpSlaveGateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, m_localGatewayIP1);
		xml.writeHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP1, m_remoteGatewayIP1);
		xml.writeHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, m_localGatewayIP2);
		xml.writeHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP2, m_remoteGatewayIP2);
		xml.writeEndElement();		//	</Settings>
	}

	bool ModbusTcpSlaveGateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

	   //

		bool okIp1 = true;

		m_localGatewayIP1.clear();
		m_remoteGatewayIP1.clear();

		okIp1 &= xml.readHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, &m_localGatewayIP1);
		okIp1 &= xml.readHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP1, &m_remoteGatewayIP1);

	   //

		bool okIp2 = true;

		m_localGatewayIP2.clear();
		m_remoteGatewayIP2.clear();

		okIp2 &= xml.readHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, &m_localGatewayIP2);
		okIp2 &= xml.readHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP2, &m_remoteGatewayIP2);

		result &= okIp1 || okIp2;

		return result;
	}

	bool ModbusTcpSlaveGateway::generateRequiredFiles(const SignalSetAdapter& signalSetAdapter,
												  ParserLog& log)
	{
		RETURN_IF_FALSE(checkSignalListsSettings(log));
		RETURN_IF_FALSE(generateSignalListsFiles(signalSetAdapter, log));

		return true;
	}

	bool ModbusTcpSlaveGateway::checkSignalListsSettings(ParserLog& log)
	{
		bool result = true;

/*		std::map<DataType_ListID, ModbusSignalListShared> listsIDs;

		for(SignalListShared& l : m_signalLists)
		{
			ModbusSignalListShared sl =
				std::dynamic_pointer_cast<ModbusSignalList>(l);

			TEST_PTR_CONTINUE(sl);

			if (sl->listNo() < 1 || sl->listNo() > 255)
			{
				SettingValue sv = sl->getSettingValue(E::Setting::ListNo);
				log.logError(QString("ListNo should be in range from 1 to 255 (line %1)").arg(sv.lineNo));
				result = false;
				continue;
			}

			auto it = listsIDs.find({sl->dataType(), sl->listNo()});

			if (it == listsIDs.end())
			{
				listsIDs.insert({{sl->dataType(), sl->listNo()}, sl });
				continue;
			}

			SettingValue sv1 = it->second->getSettingValue(E::Setting::ListNo);
			SettingValue sv2 = sl->getSettingValue(E::Setting::ListNo);

			log.logError(QString("duplicate signal lists ListNo = %1 of data type %2 (lines %3, %4)").
						 arg(sl->listNo()).arg(::E::valueToString<E::SignalListDataType>(sl->dataType())).
						 arg(sv1.lineNo).arg(sv2.lineNo));

			result = false;
		}*/

		return result;
	}

	bool ModbusTcpSlaveGateway::generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log)
	{
		m_files.clear();
		bool result = true;

/*		for(SignalListShared& l : m_signalLists)
		{
			ModbusSignalListShared sl =
				std::dynamic_pointer_cast<ModbusSignalList>(l);

			TEST_PTR_CONTINUE(sl);

				   //

			QString fileName = QString("R%1%2%3.%4").
							   arg(m_systemID, 3, 10, Latin1Char::ZERO).
							   arg(sl->dataTypeLetter()).
							   arg(sl->listNo(), 3, 10, Latin1Char::ZERO).
							   arg(m_listsVersion, 3, 10, Latin1Char::ZERO);

			File& file = m_files.emplace_back(m_gatewayType, m_gatewayID, fileName);

			result &= generateSignalListFile(*sl, file, signalSetAdapter, log);
		}

*/

		return result;
	}

	bool ModbusTcpSlaveGateway::generateSignalListFile(const ModbusSignalList& signalList,
												   File& file,
												   const SignalSetAdapter& signalSetAdapter,
												   ParserLog& log)
	{
/*		QTextStream ts(&file.mutableFileData());

		bool result = true;

		int paramIndex = 1;

		QString str;

		for(const QString& signalID : signalList.signalIDs())
		{
			const AppSignal* s = signalSetAdapter.getAppSignal(signalID);

			if (s == nullptr)
			{
				log.logError(QString("signal '%1' not found (GatewayID = %2, ListNo = %3)").
							 arg(signalID).arg(m_gatewayID).arg(signalList.listNo()));
				result = false;
				continue;
			}

			bool res = true;

			switch(signalList.dataType())
			{
			case E::SignalListDataType::Analog_A:

				if (s->isAnalog() == false)
				{
					log.logError(QString("signal '%1' is not Analog (GatewayID = %2, ListNo = %3)").
								 arg(signalID).arg(m_gatewayID).arg(signalList.listNo()));
					res = false;
				}
				else
				{
					str = QString("|%1|%2|%3|%4|%5|%6").
						  arg(paramIndex, 3, 10, Latin1Char::ZERO).
						  arg(s->customAppSignalID().trimmed()).
						  arg(s->caption()).
						  arg(s->unit()).
						  arg(s->lowEngineeringUnits()).
						  arg(s->highEngineeringUnits());

					if (signalList.includeAppSignalID() == true)
					{
						str += QString("|%1").arg(s->appSignalID());
					}

					str += Separator::NEW_LINE;

					ts << str;

					paramIndex++;
				}
				break;

			case E::SignalListDataType::Discrete_B:
			case E::SignalListDataType::Discrete_D:

				if (s->isDiscrete() == false)
				{
					log.logError(QString("signal '%1' is not Discrete (GatewayID = %2, ListNo = %3)").
								 arg(signalID).arg(m_gatewayID).arg(signalList.listNo()));
					res = false;
				}
				else
				{
					str = QString("|%1|%2|%3").
						  arg(paramIndex, 3, 10, Latin1Char::ZERO).
						  arg(s->customAppSignalID().trimmed()).
						  arg(s->caption());

					if (signalList.includeAppSignalID() == true)
					{
						str += QString("|%1").arg(s->appSignalID());
					}

					str += Separator::NEW_LINE;

					ts << str;

					paramIndex++;
				}
				break;

			default:
				Q_ASSERT(false);
				res = false;
			}

			result &= res;

			CONTINUE_IF_FALSE(res);
		}

		return result;*/
		return true;
	}
}
