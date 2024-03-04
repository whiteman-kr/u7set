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
		E::Setting::SignalsFormat,
	};

	ModbusSignalList::ModbusSignalList()
	{
	}

	bool ModbusSignalList::isKnownSetting(E::Setting st) const
	{
		return m_requiredSettings.contains(st);
	}

	bool ModbusSignalList::checkAndApplySetting(int lineNo, E::Setting st, const QVariant& value, ParserLog& log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(st);
		Q_UNUSED(value);
		Q_UNUSED(log);

		bool result = true;

		switch(st)
		{
		case E::Setting::SignalsFormat:
			result &= checkAndApplySignalsFormat(lineNo, value.toString(), log);
			break;

		default:
			Q_ASSERT(false);
			result = false;
			log.logError(lineNo, "unknown setting");
		}

		return result;
	}

	bool ModbusSignalList::appendAddressSignalID(const QString& addressStr, const QString& signalID, QString* errMsg)
	{
		Hash hash = calcHash(signalID.trimmed());

		if (m_existsSignals.contains(hash) == true)
		{
			*errMsg = QString("signal %1 already in signal list").arg(signalID);
			return false;
		}

		m_existsSignals.insert(hash);

		bool res = SignalList::appendSignalID(signalID, errMsg);

		RETURN_IF_FALSE(res);

		if (m_modbusFormat.isValid() == false)
		{
			*errMsg = "setting 'SignalsFormat' should be specified first";
			return false;
		}

		QString str(addressStr);

		str.replace(Separator::COMMA, Separator::SPACE);

		QStringList addr = str.split(Separator::SPACE, Qt::SkipEmptyParts);

		if (addr.isEmpty() == true)
		{
			*errMsg = "address of signal is not specified";
			return false;
		}

		QString regAddrStr = addr[0].trimmed().toLower();

		bool ok = false;

		int regAddr = regAddrStr.toInt(&ok, regAddrStr.startsWith("0x") ? 16 : 10);
		int bitNo = -1;

		if (ok == false)
		{
			*errMsg = QString("error converting register address '%1' to int value").arg(regAddrStr);
			return false;
		}

		if (m_modbusFormat.isDiscretes() == true)
		{
			if (addr.size() < 2)
			{
				*errMsg = "register bit number or mask should be specified for discrete signal";
				return false;
			}

			QString bitNoStr = addr[1];
			bool isMask = false;

			if (bitNoStr.startsWith("(") && bitNoStr.endsWith(")"))
			{
				bitNoStr = bitNoStr.mid(1, bitNoStr.length() - 2);
				isMask = true;
			}

			int bitNoOrMask = bitNoStr.toInt(&ok, bitNoStr.startsWith("0x") ? 16 : 10);

			if (ok == false)
			{
				*errMsg = QString("error converting register bitNo or mask '%1' to int value").arg(bitNoStr);
				return false;
			}

			if (isMask == true)
			{
				if (bitNoOrMask == 0)
				{
					*errMsg = "mask can't be 0";
					return false;
				}

				if ((bitNoOrMask & ~0xFFFF) != 0)
				{
					*errMsg = "mask should be set in 16 bit range";
					return false;
				}

				int setBitsCount = 0;
				int setBitNo = -1;

				for(int i = 0; i < 16; i++)
				{
					if ((bitNoOrMask & 0x0001) == 1)
					{
						setBitsCount++;
						setBitNo = i;
					}

					bitNoOrMask >>= 1;
				}

				if (setBitsCount > 1)
				{
					*errMsg = "only one bit in mask should be set to 1";
					return false;
				}

				Q_ASSERT(setBitNo != -1);

				bitNo = setBitNo;
			}
			else
			{
				if (bitNoOrMask < 0 || bitNoOrMask > 15)
				{
					*errMsg = "register bitNo should be in range 0..15";
					return false;
				}

				bitNo = bitNoOrMask;
			}
		}
		else
		{
			if (addr.size() > 1)
			{
				*errMsg = "only register number should be specified for analog signal";
				return false;
			}

			bitNo = 0;
		}

		Address16 addr16(regAddr, bitNo);

		auto it = m_signals.find(addr16);

		if (it != m_signals.end())
		{
			*errMsg = QString("signal %1 address %2 is not unique (already assigned to %3)").
					  arg(signalID).arg(addr16.toString()).arg(it->second);
			return false;
		}

		m_signals.emplace(addr16, signalID);

		return true;
	}

	void ModbusSignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeEnumKeyAttribute<E::ModbusSignalFormat>(XmlAttribute::SIGNAL_FORMAT, m_modbusFormat.signalsFormat);
		xml.writeEnumKeyAttribute<E::ModbusByteOrder>(XmlAttribute::BYTE_ORDER, m_modbusFormat.byteOrder);

		xml.writeEndElement();		//	</SignalList>
	}

	bool ModbusSignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);

		result &= xml.readEnumKeyAttribute<E::ModbusSignalFormat>(XmlAttribute::SIGNAL_FORMAT, &m_modbusFormat.signalsFormat);
		result &= xml.readEnumKeyAttribute<E::ModbusByteOrder>(XmlAttribute::BYTE_ORDER, &m_modbusFormat.byteOrder);

		return result;
	}

	void ModbusSignalList::writeSignalsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNALS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_signalIDs.size()));

		for(const QString& id : m_signalIDs)
		{
			xml.writeStringElement(XmlElement::ID, id);
		}

		xml.writeEndElement();		// </Signals>

	}

	bool ModbusSignalList::readSignalsFromXml(XmlReadHelper& xml)
	{
		return true;
	}

	bool ModbusSignalList::checkAndApplySignalsFormat(int lineNo, QString formatStr, ParserLog& log)
	{
		formatStr = formatStr.toLower();
		formatStr.replace(Separator::COMMA, Separator::SPACE);

		QStringList options = formatStr.split(Separator::SPACE, Qt::SkipEmptyParts);

		m_modbusFormat.byteOrder = E::ModbusByteOrder::Unknown;

		if (options.contains(::E::valueToString(E::ModbusByteOrder::BE).toLower()) == true)
		{
			m_modbusFormat.byteOrder = E::ModbusByteOrder::BE;
		}

		if (options.contains(::E::valueToString(E::ModbusByteOrder::LE).toLower()) == true)
		{
			if (m_modbusFormat.byteOrder != E::ModbusByteOrder::Unknown)
			{
				log.logError(lineNo, "undefined byte order");
				return false;
			}

			m_modbusFormat.byteOrder = E::ModbusByteOrder::LE;
		}

		if (m_modbusFormat.byteOrder == E::ModbusByteOrder::Unknown)
		{
			log.logError(lineNo, "byte order 'BE' or 'LE' is not specified");
			return false;
		}

		static const std::set<E::ModbusSignalFormat> signalsFormats =
		{
			E::ModbusSignalFormat::DiscreteUint16,
			E::ModbusSignalFormat::AnalogFloat16,
		};

		for(E::ModbusSignalFormat format : signalsFormats)
		{
			if (options.contains(::E::valueToString(format).toLower()))
			{
				if (m_modbusFormat.signalsFormat != E::ModbusSignalFormat::Unknown)
				{
					log.logError(lineNo, QString("undefined signals format"));
					return false;
				}

				m_modbusFormat.signalsFormat = format;
			}
		}

		if (m_modbusFormat.signalsFormat == E::ModbusSignalFormat::Unknown)
		{
			log.logError(lineNo, QString("undefined signals format"));
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
		Gateway(E::GatewayType::ModbusTcpSlave)
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
