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

		if (m_signals.contains(hash) == true)
		{
			*errMsg = QString("signal %1 already in signal list").arg(signalID);
			return false;
		}

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

		if (m_modbusFormat.isDiscrete() == true)
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

		appendSignalID(signalID, errMsg);
		m_signals.emplace(hash, addr16);

		return true;
	}

	ModbusFormat ModbusSignalList::modbusFormat() const
	{
		return m_modbusFormat;
	}

	Address16 ModbusSignalList::getAddress(Hash hash) const
	{
		return getValueOrDefault(m_signals, hash, Address16());
	}

	void ModbusSignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeEnumKeyAttribute<E::ModbusSignalFormat>(XmlAttribute::SIGNAL_FORMAT, m_modbusFormat.signalFormat);
		xml.writeEnumKeyAttribute<E::ModbusByteOrder>(XmlAttribute::BYTE_ORDER_ATTR, m_modbusFormat.byteOrder);

		xml.writeEndElement();		//	</SignalList>
	}

	bool ModbusSignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);

		result &= xml.readEnumKeyAttribute<E::ModbusSignalFormat>(XmlAttribute::SIGNAL_FORMAT, &m_modbusFormat.signalFormat);
		result &= xml.readEnumKeyAttribute<E::ModbusByteOrder>(XmlAttribute::BYTE_ORDER_ATTR, &m_modbusFormat.byteOrder);

		return result;
	}

	void ModbusSignalList::writeSignalsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNALS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_signalIDs.size()));

		for(const QString& id : m_signalIDs)
		{
			xml.writeStartElement(XmlElement::SIGNAL_ELEM);

			//xml.writeStringAttribute(XmlAttribute::APP_SIGNAL_ID, );

			xml.writeEndElement();		// </Signal>
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

		static const std::map<E::ModbusSignalFormat, ::E::SignalType> signalsFormats =
		{
			{ E::ModbusSignalFormat::DiscreteUint16, ::E::SignalType::Discrete },
			{ E::ModbusSignalFormat::AnalogFloat16, ::E::SignalType::Analog }
		};

		for(auto const& [modbusFormat, signalType] : signalsFormats)
		{
			if (options.contains(::E::valueToString(modbusFormat).toLower()))
			{
				if (m_modbusFormat.signalFormat != E::ModbusSignalFormat::Unknown)
				{
					log.logError(lineNo, QString("undefined signals format"));
					return false;
				}

				m_modbusFormat.signalFormat = modbusFormat;
				setSignalType(signalType);
			}
		}

		if (m_modbusFormat.signalFormat == E::ModbusSignalFormat::Unknown)
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
		xml.writeEnumKeyAttribute(XmlAttribute::MODBUS_CODING, m_coding);

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

		result &= xml.readEnumKeyAttribute(XmlAttribute::MODBUS_CODING, &m_coding);

		return result;
	}

	void ModbusTcpSlaveGateway::writeSignalListsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNALS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_modbusSignals.size()));

		for(const auto& [addr16, p] : m_modbusSignals)
		{
			const QString& signalID = p.first;
			const ModbusFormat& format = p.second;

			xml.writeStartElement(XmlElement::SIGNAL_ELEM);

			xml.writeIntAttribute(XmlAttribute::REG_ADDR, addr16.offset());
			xml.writeIntAttribute(XmlAttribute::REG_BIT, addr16.bit());
			xml.writeEnumKeyAttribute(XmlAttribute::FORMAT, format.signalFormat);
			xml.writeEnumKeyAttribute(XmlAttribute::BYTE_ORDER_ATTR, format.byteOrder);
			xml.writeStringAttribute(XmlAttribute::APP_SIGNAL_ID, signalID);

			xml.writeEndElement();		//	</Signal>
		}

		xml.writeEndElement();		//	</Signals>
	}

	bool ModbusTcpSlaveGateway::readSignalListsFromXml(XmlReadHelper& xml)
	{
		return true;
	}

	bool ModbusTcpSlaveGateway::generateRequiredFiles(const SignalSetAdapter& signalSetAdapter,
												  ParserLog& log)
	{
		Q_UNUSED(signalSetAdapter);

		m_files.clear();

		bool result = true;

		result = buildModbusSignalsList(log);

		RETURN_IF_FALSE(result);

		result = generateModbusSignalsFile();

		return result;
	}

	bool ModbusTcpSlaveGateway::buildModbusSignalsList(ParserLog& log)
	{
		m_modbusSignals.clear();

		bool result = true;

		const SignalLists& lists = signalLists();

		std::set<Hash> existsSignals;
		std::set<int> discreteRegs;
		std::set<int> analogRegs;

		for(const SignalListShared& sl : lists)
		{
			std::shared_ptr<ModbusSignalList> mbsl = std::dynamic_pointer_cast<ModbusSignalList>(sl);

			TEST_PTR_CONTINUE(mbsl);

			ModbusFormat format = mbsl->modbusFormat();

			Q_ASSERT(format.isValid() == true);

			const std::vector<QString>& signalIDs = mbsl->signalIDs();

			for(const QString& signalID : signalIDs)
			{
				Hash hash = calcHash(signalID);

				if (existsSignals.contains(hash))
				{
					log.logError(QString("signal '%1' is repeated in several signal lists").arg(signalID));
					result = false;
					continue;
				}

				existsSignals.insert(hash);

				Address16 addr16 = mbsl->getAddress(hash);

				if (addr16.isValid() == false)
				{
					log.logError(QString("invalid modbus address of signal'%1'"));
					result = false;
					continue;
				}

				auto it = m_modbusSignals.find(addr16);

				if (it != m_modbusSignals.end())
				{
					log.logError(QString("signal '%1' address %2 is not unique (already assigned to '%3')").
												arg(signalID).arg(addr16.toString()).arg(it->second.first));
					result = false;
					continue;
				}

				if (format.isDiscrete() == true)
				{
					int regsCount = 0;

					switch(format.signalFormat)
					{
					case E::ModbusSignalFormat::DiscreteUint16:
						regsCount = 1;
						break;

					default:
						Q_ASSERT(false);
					}

					if (regsCount == 0)
					{
						log.logError(QString("undefined register count for signal '%1'").arg(signalID));
						result = false;
						continue;
					}

					bool res = true;

					for(int i = 0; i < regsCount; i++)
					{
						if (analogRegs.contains(addr16.offset() + i))
						{
							log.logError(QString("discrete signal '%1' register %2 used by analog signal").
											arg(signalID).arg(addr16.offset() + i));
							res = false;
							break;
						}

						discreteRegs.emplace(addr16.offset() + i);
					}

					result &= res;

					CONTINUE_IF_FALSE(res)
				}
				else
				{
					int regsCount = 0;

					switch(format.signalFormat)
					{
					case E::ModbusSignalFormat::AnalogFloat16:
						regsCount = 1;
						break;

					default:
						Q_ASSERT(false);
					}

					if (regsCount == 0)
					{
						log.logError(QString("undefined register count for signal '%1'").arg(signalID));
						result = false;
						continue;
					}

					bool res = true;

					for(int i = 0; i < regsCount; i++)
					{
						if (discreteRegs.contains(addr16.offset() + i))
						{
							log.logError(QString("analog signal '%1' register %2 used by discrete signals").
										 arg(signalID).arg(addr16.offset() + i));
							res = false;
							break;
						}

						analogRegs.emplace(addr16.offset() + i);
					}

					result &= res;

					CONTINUE_IF_FALSE(res)
				}

				m_modbusSignals.emplace(addr16, std::pair<QString, ModbusFormat>{signalID, format});
			}
		}

		return result;
	}

	bool ModbusTcpSlaveGateway::generateModbusSignalsFile()
	{
		bool result = true;

		QStringList fd;

		fd.append("ModbusTcpSlave gateway signals list");
		fd.append("");
		fd.append(QString("GatewayID:   %1").arg(m_gatewayID));
		fd.append(QString("Description: %1 ").arg(m_gatewayDescription));
		fd.append("");

		static const QString line("----------------------------------------------------------------------------------------------------");

		fd.append(line);
		fd.append(" RegAddr | BitNo |  Mask  |       Format      | AppSignalID");
		fd.append(line);

		QString bitStr;
		QString maskStr;

		for(const auto& [addr16, p] : m_modbusSignals)
		{
			const QString& signalID = p.first;
			const ModbusFormat& format = p.second;

			if (format.isDiscrete() == true)
			{
				bitStr = QString("%1").arg(addr16.bit(), 2, 10, Latin1Char::ZERO);
				maskStr = QString("0x%1").arg(1 << addr16.bit(), 4, 16, Latin1Char::ZERO).toUpper();
			}
			else
			{
				bitStr = QStringLiteral("  ");
				maskStr = QStringLiteral("      ");
			}

			fd.append(QString("  %1  |   %2  | %3 | %4 | %5").
						arg(addr16.offset(), 5, 10, Latin1Char::ZERO).arg(bitStr).arg(maskStr).
						arg(format.toString(), -17, Latin1Char::SPACE).arg(signalID));
		}

		fd.append(line);

		File& file = m_files.emplace_back(m_gatewayType, m_gatewayID, "ModbusSignals.txt");

		QByteArray fileData = fd.join("\n").toUtf8();

		file.mutableFileData().swap(fileData);

		return result;
	}
}
