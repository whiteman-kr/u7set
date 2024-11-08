#include "GatewayDescriptionParser.h"
#include "ModbusTcpSlaveGateway.h"

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"
#include "../CommonLib/include/CommonLib/Types.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Struct Gateway::ModbusFormat implementation
	//
	// ---------------------------------------------------------------------------------

	bool ModbusFormat::isValid() const
	{
		return signalFormat != E::ModbusSignalFormat::Unknown &&
			   byteOrder != E::ModbusByteOrder::Unknown;
	}

	bool ModbusFormat::isDiscrete() const
	{
		return signalFormat == E::ModbusSignalFormat::DiscreteBit;
	}

	int ModbusFormat::registersCount() const
	{
		switch(signalFormat)
		{
		case E::ModbusSignalFormat::DiscreteBit:
		case E::ModbusSignalFormat::AnalogFloat16:
		case E::ModbusSignalFormat::AnalogSInt16:
			return 1;

		case E::ModbusSignalFormat::AnalogFloat32:
		case E::ModbusSignalFormat::AnalogSInt32:
			return 2;

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	QString ModbusFormat::toString() const
	{
		return QString("%1 %2").arg(::E::valueToString(signalFormat)).arg(::E::valueToString(byteOrder));
	}

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

	ParseResult ModbusSignalList::checkAndApplySetting(int lineNo, E::Setting st, const QVariant& value, ParserLog& log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(st);
		Q_UNUSED(value);
		Q_UNUSED(log);

		ParseResult pr = ParseResult::Ok;

		switch(st)
		{
		case E::Setting::SignalsFormat:
			pr = checkAndApplySignalsFormat(lineNo, value.toString(), log);
			break;

		default:
			Q_ASSERT(false);
			log.logError(lineNo, "unknown setting");
			pr = ParseResult::Error;

		}

		return pr;
	}

	ParseResult ModbusSignalList::checkSignalTypeAndFormat(int lineNo, const AppSignal* appSignal, ParserLog& log)
	{
		ParseResult pr = SignalList::checkSignalTypeAndFormat(lineNo, appSignal, log);

		if (pr != ParseResult::Ok)
		{
			return pr;
		}

		if (appSignal->isAnalog())
		{
			switch(m_modbusFormat.signalFormat)
			{
			case E::ModbusSignalFormat::AnalogFloat16:
			case E::ModbusSignalFormat::AnalogFloat32:

				if (appSignal->analogSignalFormat() != ::E::AnalogAppSignalFormat::Float32)
				{
					log.logError(lineNo, QString("uncompatible signal %1 format, expected Float32").
													arg(appSignal->appSignalID()));
					pr = ParseResult::Error;
				}
				break;

			case E::ModbusSignalFormat::AnalogSInt16:
			case E::ModbusSignalFormat::AnalogSInt32:

				if (appSignal->analogSignalFormat() != ::E::AnalogAppSignalFormat::SignedInt32)
				{
					log.logError(lineNo, QString("uncompatible signal %1 format, expected SignedInt32").
										 arg(appSignal->appSignalID()));
					pr = ParseResult::Error;
				}

				break;
			}
		}

		return pr;
	}

	ParseResult ModbusSignalList::appendAddressSignalID(int lineNo, const QString& addressStr, const QString& signalID, ParserLog& log)
	{
		Hash hash = calcHash(signalID.trimmed());

		if (m_signals.contains(hash) == true)
		{
			log.logError(lineNo, QString("signal %1 already in signal list").arg(signalID));
			return ParseResult::Error;
		}

		if (m_modbusFormat.isValid() == false)
		{
			log.logError(lineNo, "setting 'SignalsFormat' should be specified first");
			return ParseResult::Error;
		}

		QString str(addressStr);

		str.replace(Separator::COMMA, Separator::SPACE);

		QStringList addr = str.split(Separator::SPACE, Qt::SkipEmptyParts);

		if (addr.isEmpty() == true)
		{
			log.logError(lineNo, "address of signal is not specified");
			return ParseResult::Error;
		}

		QString regAddrStr = addr[0].trimmed().toLower();

		bool ok = false;

		int regAddr = regAddrStr.toInt(&ok, regAddrStr.startsWith("0x") ? 16 : 10);
		int bitNo = -1;

		if (ok == false)
		{
			log.logError(lineNo, QString("error converting register address %1 to int value").arg(regAddrStr));
			return ParseResult::Error;
		}

		if (m_modbusFormat.isDiscrete() == true)
		{
			if (addr.size() < 2)
			{
				log.logError(lineNo, "register bit number or mask should be specified for discrete signal");
				return ParseResult::Error;
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
				log.logError(lineNo, QString("error converting register bitNo or mask '%1' to int value").arg(bitNoStr));
				return ParseResult::Error;
			}

			if (isMask == true)
			{
				if (bitNoOrMask == 0)
				{
					log.logError(lineNo, "mask can't be 0");
					return ParseResult::Error;
				}

				if ((bitNoOrMask & ~0xFFFF) != 0)
				{
					log.logError(lineNo, "mask should be set in 16 bit range");
					return ParseResult::Error;
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
					log.logError(lineNo, "only one bit in mask should be set to 1");
					return ParseResult::Error;
				}

				Q_ASSERT(setBitNo != -1);

				bitNo = setBitNo;
			}
			else
			{
				if (bitNoOrMask < 0 || bitNoOrMask > 15)
				{
					log.logError(lineNo, "register bitNo should be in range 0..15");
					return ParseResult::Error;
				}

				bitNo = bitNoOrMask;
			}
		}
		else
		{
			if (addr.size() > 1)
			{
				log.logError(lineNo, "only register number should be specified for analog signal");
				return ParseResult::Error;
			}

			bitNo = 0;
		}

		Address16 addr16(regAddr, bitNo);

		appendSignalID(lineNo, signalID, log);

		m_signals.emplace(hash, addr16);

		return ParseResult::Ok;
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

	ParseResult ModbusSignalList::checkAndApplySignalsFormat(int lineNo, QString formatStr, ParserLog& log)
	{
		formatStr = formatStr.toLower();
		formatStr.replace(Separator::COMMA, Separator::SPACE);

		QStringList options = formatStr.split(Separator::SPACE, Qt::SkipEmptyParts);

		m_modbusFormat.byteOrder = E::ModbusByteOrder::Unknown;

		QStringList keys = ::E::enumKeyStrings<E::ModbusByteOrder>();

		for(const QString& key : keys)
		{
			if (options.contains(key.toLower()))
			{
				bool ok = 0;
				m_modbusFormat.byteOrder = ::E::stringToValue<E::ModbusByteOrder>(key, &ok);
				break;
			}
		}

		if (m_modbusFormat.byteOrder == E::ModbusByteOrder::Unknown)
		{
			log.logError(lineNo, QString("byte order %1 should specified").
								 arg(keys.join(", ")));
			return ParseResult::Error;
		}

		static const std::map<E::ModbusSignalFormat, ::E::SignalType> signalsFormats =
		{
			{ E::ModbusSignalFormat::DiscreteBit, ::E::SignalType::Discrete },
			{ E::ModbusSignalFormat::AnalogFloat16, ::E::SignalType::Analog },
			{ E::ModbusSignalFormat::AnalogFloat32, ::E::SignalType::Analog },
			{ E::ModbusSignalFormat::AnalogSInt16, ::E::SignalType::Analog },
			{ E::ModbusSignalFormat::AnalogSInt32, ::E::SignalType::Analog },
		};

		for(auto const& [modbusFormat, signalType] : signalsFormats)
		{
			if (options.contains(::E::valueToString(modbusFormat).toLower()))
			{
				if (m_modbusFormat.signalFormat != E::ModbusSignalFormat::Unknown)
				{
					log.logError(lineNo, QString("undefined signals format"));
					return ParseResult::Error;
				}

				m_modbusFormat.signalFormat = modbusFormat;
				setSignalType(signalType);
			}
		}

		if (m_modbusFormat.signalFormat == E::ModbusSignalFormat::Unknown)
		{
			log.logError(lineNo, QString("undefined signals format"));
			return ParseResult::Error;
		}

		return ParseResult::Ok;
	}

   // ---------------------------------------------------------------------------------
   //
   // Class Gateway::ModbusTcpSlaveGateway implementation
   //
   // ---------------------------------------------------------------------------------

	const std::set<E::Setting> ModbusTcpSlaveGateway::m_requiredSettings =
	{
			E::Setting::LocalGatewayIP1,
			E::Setting::ModbusDeviceID,
			E::Setting::ModbusMode,
	};

	const std::set<E::Setting> ModbusTcpSlaveGateway::m_optionalSettings =
	{
			E::Setting::LocalGatewayIP2,
	};

	ModbusTcpSlaveGateway::ModbusTcpSlaveGateway() :
		Gateway(E::GatewayType::ModbusTcpSlave)
	{
	}

	ModbusTcpSlaveGateway::ModbusTcpSlaveGateway(const QString& gwID, const QString& gwDesc) :
		Gateway(E::GatewayType::ModbusTcpSlave, gwID, gwDesc)
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
				addrPort.setAddressPortStr(sv.value.toString(), MODBUS_DEFAULT_PORT);
				m_localGatewayIP1 = addrPort;
				break;

			case E::Setting::LocalGatewayIP2:
				addrPort.setAddressPortStr(sv.value.toString(),  MODBUS_DEFAULT_PORT);
				m_localGatewayIP2 = addrPort;
				break;

			case E::Setting::ModbusDeviceID:
				{
					m_modbusDeviceID = sv.value.toInt();

					if (m_modbusDeviceID < 0 || m_modbusDeviceID > 255)
					{
						log.logError(sv.lineNo, "Wrong ModbusDeviceID value. Should be in range 0..255.");
						result = false;
					}
				}
				break;

			case E::Setting::ModbusMode:
				{
					bool ok = true;

					m_modbusMode = ::E::stringToValue<E::ModbusMode>(sv.value.toString(), &ok);

					if (ok == false)
					{
						QStringList values = ::E::enumKeyStrings<E::ModbusMode>();

						values.remove(0);		// remove E::ModbusMode::Unknown

						log.logError(sv.lineNo, QString("Wrong ModbusMode value. Should be one of: %1").
												arg(values.join(", ")));

						m_modbusMode = E::ModbusMode::Unknown;
						result = false;
					}
				}
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

	HostAddressPort ModbusTcpSlaveGateway::localGatewayIP2() const
	{
		return m_localGatewayIP2;
	}

	E::ModbusMode ModbusTcpSlaveGateway::modbusMode() const
	{
		return m_modbusMode;
	}

	int ModbusTcpSlaveGateway::modbusDeviceID() const
	{
		return m_modbusDeviceID;
	}

	void ModbusTcpSlaveGateway::getRequiredSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		for(const auto& [addr16, p] : m_modbusSignals)
		{
			const QString& appSignalID = p.first;
			hashes->emplace(calcHash(appSignalID));
		}
	}

	void ModbusTcpSlaveGateway::getEventSignalsHashes(std::set<Hash>* hashes) const
	{
		TEST_PTR_RETURN(hashes);

		for(const auto& [addr16, p] : m_modbusSignals)
		{
			const ModbusFormat& format = p.second;

			if (format.isDiscrete() == true)
			{
				const QString& appSignalID = p.first;
				hashes->emplace(calcHash(appSignalID));
			}
		}
	}

	const std::map<Address16, std::pair<QString, ModbusFormat>>& ModbusTcpSlaveGateway::modbusSignals() const
	{
		return m_modbusSignals;
	}

	void ModbusTcpSlaveGateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, m_localGatewayIP1);
		xml.writeIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, m_localGatewayIP2);
		xml.writeEnumKeyAttribute(XmlAttribute::MODBUS_MODE, m_modbusMode);
		xml.writeIntAttribute(XmlAttribute::MODBUS_DEVICE_ID, m_modbusDeviceID);

		xml.writeEndElement();		//	</Settings>
	}

	bool ModbusTcpSlaveGateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

	   //

		bool okIp1 = true;

		m_localGatewayIP1.clear();

		okIp1 &= xml.readIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, &m_localGatewayIP1);

	   //

		bool okIp2 = true;

		m_localGatewayIP2.clear();

		okIp2 &= xml.readIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, &m_localGatewayIP2);

		result &= okIp1 || okIp2;

		result &= xml.readEnumKeyAttribute(XmlAttribute::MODBUS_MODE, &m_modbusMode);

		result &= xml.readIntAttribute(XmlAttribute::MODBUS_DEVICE_ID, &m_modbusDeviceID);

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
		m_modbusSignals.clear();

		bool result = true;

		result &= xml.findElement(XmlElement::SIGNALS);

		RETURN_IF_FALSE(result);

		int signalCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &signalCount);

		for(int i = 0; i < signalCount; i++)
		{
			result &= xml.findElement(XmlElement::SIGNAL_ELEM);

			BREAK_IF_FALSE(result);

			int offset = 0;
			int bit = 0;
			ModbusFormat format;
			QString appSignalID;

			result &= xml.readIntAttribute(XmlAttribute::REG_ADDR, &offset);
			result &= xml.readIntAttribute(XmlAttribute::REG_BIT, &bit);
			result &= xml.readEnumKeyAttribute(XmlAttribute::FORMAT, &format.signalFormat);
			result &= xml.readEnumKeyAttribute(XmlAttribute::BYTE_ORDER_ATTR, &format.byteOrder);
			result &= xml.readStringAttribute(XmlAttribute::APP_SIGNAL_ID, &appSignalID);

			BREAK_IF_FALSE(result);

			m_modbusSignals.emplace(Address16(offset, bit),
									std::pair<QString, ModbusFormat>{appSignalID, format});
		}

		return result;
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
		std::set<int> discreteAddrs;
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
					log.logWarning(QString("signal %1 is repeated in several signal lists").arg(signalID));
				}

				existsSignals.insert(hash);

				Address16 addr16 = mbsl->getAddress(hash);

				if (addr16.isValid() == false)
				{
					log.logError(QString("invalid modbus address of signal %1"));
					result = false;
					continue;
				}

				auto it = m_modbusSignals.find(addr16);

				if (it != m_modbusSignals.end())
				{
					log.logError(QString("signal %1 address %2 is not unique (already assigned to %3)").
												arg(signalID).arg(addr16.toString()).arg(it->second.first));
					result = false;
					continue;
				}

				int regsCount = format.registersCount();

				if (regsCount == 0)
				{
					log.logError(QString("undefined register count for modbus signal %1").arg(signalID));
					result = false;
					continue;
				}

				for(int i = 0; i < regsCount; i++)
				{
					if (format.isDiscrete() == true)
					{
						if (analogRegs.contains(addr16.offset() + i))
						{
							log.logError(QString("discrete signal %1 register %2 used by analog signal").
											arg(signalID).arg(addr16.offset() + i));
							result = false;
						}
						else
						{
							if (discreteAddrs.contains(addr16.bitAddress()) == true)
							{
								log.logError(QString("duplicate address %1 of discrete signal %2").
													 arg(addr16.bitAddress()).arg(signalID));
							}
							else
							{
								discreteRegs.emplace(addr16.offset() + i);
								discreteAddrs.emplace(addr16.bitAddress());
							}
						}
					}
					else
					{
						if (discreteRegs.contains(addr16.offset() + i))
						{
							log.logError(QString("analog signal %1 register %2 used by discrete signals").
										 arg(signalID).arg(addr16.offset() + i));
							result = false;
						}
						else
						{
							if (analogRegs.contains(addr16.offset() + i) == true)
							{
								log.logError(QString("analog signal %1 register %2 used by another analog signal").
											 arg(signalID).arg(addr16.offset() + i));
								result = false;
							}
							else
							{
								analogRegs.emplace(addr16.offset() + i);
							}
						}
					}
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
