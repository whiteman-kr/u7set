#include "../lib/ConstStrings.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

#include "GatewayDescription.h"
#include "GatewayDescriptionParser.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::SettingsValues implementation
	//
	// ---------------------------------------------------------------------------------

	bool SettingsValues::contains(E::Setting st) const
	{
		return m_settingsValues.contains(st);
	}

	bool SettingsValues::insert(int lineNo, E::Setting st, const QVariant& value)
	{
		SettingValue sv =
		{
			.lineNo = lineNo,
			.setting = st,
			.value = value
		};

		auto p = m_settingsValues.insert({ st, sv });

		return !p.second;		// if true - setting value already exists
	}

	std::map<E::Setting, SettingValue>::const_iterator SettingsValues::begin() const
	{
		return m_settingsValues.begin();
	}

	std::map<E::Setting, SettingValue>::iterator SettingsValues::begin()
	{
		return m_settingsValues.begin();
	}

	std::map<E::Setting, SettingValue>::const_iterator SettingsValues::end() const
	{
		return m_settingsValues.end();
	}

	std::map<E::Setting, SettingValue>::iterator SettingsValues::end()
	{
		return m_settingsValues.end();
	}

	SettingValue SettingsValues::getSettingVaue(E::Setting st) const
	{
		auto it = m_settingsValues.find(st);

		if (it == m_settingsValues.end())
		{
			return SettingValue();
		}

		return it->second;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::File implementation
	//
	// ---------------------------------------------------------------------------------

	File::File(E::GatewayType gatewayType, const QString& gatewayID, const QString& fileName) :
		m_gatewayType(gatewayType),
		m_gatewayID(gatewayID),
		m_fileName(fileName)
	{
	}

	const QByteArray& File::fileData() const
	{
		return m_fileData;
	}

	QByteArray& File::mutableFileData()
	{
		return m_fileData;
	}

	QString File::gatewayID() const
	{
		return m_gatewayID;
	}

	QString File::fileName() const
	{
		return m_fileName;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::SignalList implementation
	//
	// ---------------------------------------------------------------------------------

	bool SignalList::setSettingValue(int lineNo, E::Setting st, const QVariant& value)
	{
		return m_settingsValues.insert(lineNo, st, value);
	}

	bool SignalList::settingIsSet(E::Setting st) const
	{
		return m_settingsValues.contains(st);
	}

	bool SignalList::isKnownSetting(E::Setting st) const
	{
		Q_UNUSED(st);
		return false;
	}

	bool SignalList::checkAndApplySettings(int lineNo, ParserLog &log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(log);
		return true;
	}

	SettingValue SignalList::getSettingValue(E::Setting st) const
	{
		return m_settingsValues.getSettingVaue(st);
	}

	const std::vector<QString>& SignalList::signalIDs() const
	{
		return m_signalIDs;
	}

	int SignalList::signalsCount() const
	{
		return TO_INT(m_signalIDs.size());
	}

	void SignalList::fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const
	{
		TEST_PTR_RETURN(acquiredSignals);

		for(const QString& id : m_signalIDs)
		{
			acquiredSignals->insert(calcHash(id));
		}
	}

	void SignalList::writeToXml(XmlWriteHelper& xml) const
	{
		writeSettingsToXml(xml);
		writeSignalsToXml(xml);
	}

	bool SignalList::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= readSettingsFromXml(xml);
		result &= readSignalsFromXml(xml);

		return result;
	}

	void SignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		Q_UNUSED(xml);
		Q_ASSERT(false);		// this function should be overrided in derived class
	}

	bool SignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		Q_UNUSED(xml);
		Q_ASSERT(false);		// this function should be overrided in derived class
		return false;
	}

	void SignalList::writeSignalsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNALS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_signalIDs.size()));

		for(const QString& id : m_signalIDs)
		{
			xml.writeStringElement(XmlElement::ID, id);
		}

		xml.writeEndElement();		// </Signals>
	}

	bool SignalList::readSignalsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNALS);

		int signalsCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &signalsCount);

		RETURN_IF_FALSE(result);

		for(int i = 0; i < signalsCount; i++)
		{
			QString signalID;

			result &= xml.readStringElement(XmlElement::ID, &signalID, true);

			BREAK_IF_FALSE(result);

			m_signalIDs.push_back(signalID);
		}

		return result;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Gateway implementation
	//
	// ---------------------------------------------------------------------------------

	const std::set<E::Setting> Gateway::m_gatewayRequiredSettings =
	{
		E::Setting::GatewayType,
		E::Setting::GatewayID,
		E::Setting::GatewayDescription,
	};

	Gateway::Gateway() :
		m_gatewayType(E::GatewayType::Unknown)
	{
	}

	Gateway::Gateway(E::GatewayType gwType) :
		m_gatewayType(gwType)
	{
	}

	Gateway::Gateway(E::GatewayType gwType, const QString& gwID, const QString& gwDesc) :
		m_gatewayType(gwType),
		m_gatewayID(gwID),
		m_gatewayDescription(gwDesc)
	{
	}

	Gateway::~Gateway()
	{
	}

	E::GatewayType Gateway::gatewayType() const
	{
		return m_gatewayType;
	}

	QString Gateway::gatewayID() const
	{
		return m_gatewayID;
	}

	QString Gateway::gatewayDescription() const
	{
		return m_gatewayDescription;
	}

	int Gateway::signalListsCount() const
	{
		return TO_INT(m_signalLists.size());
	}

	bool Gateway::setSettingValue(int lineNo, E::Setting st, const QVariant& value)
	{
		return m_settingsValues.insert(lineNo, st, value);
	}

	bool Gateway::settingIsSet(E::Setting st) const
	{
		return m_settingsValues.contains(st);
	}

	bool Gateway::isKnownSetting(E::Setting st) const
	{
		return m_gatewayRequiredSettings.contains(st);
	}

	bool Gateway::checkAndApplySettings(int lineNo, ParserLog& log)
	{
		bool result = true;

		result &= checkRequiredSettings(m_gatewayRequiredSettings, m_settingsValues, lineNo, log);

		RETURN_IF_FALSE(result);

		for(const auto& p : m_settingsValues)
		{
			E::Setting st = p.first;
			const SettingValue& sv = p.second;

			switch(st)
			{
			case E::Setting::GatewayType:
				// setting GatewayType was checked and applied early
				break;

			case E::Setting::GatewayID:
				m_gatewayID = sv.value.toString();
				break;

			case E::Setting::GatewayDescription:
				m_gatewayDescription = sv.value.toString();
				break;

			default:
				;		// ok
			}
		}

		return result;
	}

	void Gateway::appendSignalList()
	{
		Q_ASSERT(false);		// this function should be called in derived classes only!
	}

	const SignalLists& Gateway::signalLists() const
	{
		return m_signalLists;
	}

	int Gateway::signalsCount() const
	{
		int signalsCount = 0;

		for(const auto& sl : m_signalLists)
		{
			TEST_PTR_CONTINUE(sl);

			signalsCount += sl->signalsCount();
		}

		return signalsCount;
	}

	const std::vector<File>& Gateway::files() const
	{
		return m_files;
	}

	bool Gateway::checkRequiredSettings(const std::set<E::Setting> reqSettings,
									  const SettingsValues& settingsValues,
									  int lineNo, ParserLog& log)
	{
		bool result = true;

		for(E::Setting st : reqSettings)
		{
			if (settingsValues.contains(st) == false)
			{
				log.logRequirtedSettingIsNotSet(lineNo, st);
				result = false;
			}
		}

		return result;
	}

	void Gateway::fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const
	{
		TEST_PTR_RETURN(acquiredSignals);

		for(auto& sl : m_signalLists)
		{
			sl->fillAcquiredSignalsSet(acquiredSignals);
		}
	}

	void Gateway::writeToXml(XmlWriteHelper& xml) const
	{
		writeSettingsToXml(xml);
		writeSignalListsToXml(xml);
	}

	bool Gateway::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= readSettingsFromXml(xml);
		result &= readSignalListsFromXml(xml);

		return result;
	}

	void Gateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		Q_UNUSED(xml);
		Q_ASSERT(false);		// this function should be overrided in derived class
	}

	bool Gateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		Q_UNUSED(xml);
		Q_ASSERT(false);		// this function should be overrided in derived class
		return false;
	}

	void Gateway::writeSignalListsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LISTS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_signalLists.size()));

		for(SignalListShared sl : m_signalLists)
		{
			sl->writeToXml(xml);
		}

		xml.writeEndElement();		//	</SignalLists>
	}

	bool Gateway::readSignalListsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LISTS);

		int signalListsCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &signalListsCount);

		RETURN_IF_FALSE(result);

		for(int i = 0; i < signalListsCount; i++)
		{
			appendSignalList();

			SignalListShared sl = m_signalLists.back();

			result &= sl->readFromXml(xml);

			if (result == false)
			{
				m_signalLists.pop_back();
				break;
			}
		}

		return result;
	}

	bool Gateway::generateRequiredFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log)
	{
		Q_UNUSED(signalSetAdapter);
		Q_UNUSED(log);
		return true;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Gateways implementation
	//
	// ---------------------------------------------------------------------------------

	void Gateways::append(GatewayShared gw)
	{
		m_gateways.push_back(gw);
	}

	void Gateways::setLast(GatewayShared gw)
	{
		m_gateways.back() = gw;
	}

	GatewayShared Gateways::last()
	{
		return m_gateways.back();
	}

	std::vector<GatewayShared>::iterator Gateways::begin()
	{
		return m_gateways.begin();
	}

	std::vector<GatewayShared>::iterator Gateways::end()
	{
		return m_gateways.end();
	}

	std::vector<GatewayShared>::const_iterator Gateways::begin() const
	{
		return m_gateways.begin();
	}

	std::vector<GatewayShared>::const_iterator Gateways::end() const
	{
		return m_gateways.end();
	}

	void Gateways::clear()
	{
		m_gateways.clear();
	}

	GatewayShared Gateways::createTypedGateway(E::GatewayType gwType, const QString& gwID, const QString& gwDesc)
	{
		GatewayShared gw;

		switch(gwType)
		{
		case E::GatewayType::IVS_Impulse:
			gw = std::make_shared<IvsImpulseGateway>(gwID, gwDesc);
			break;

		case E::GatewayType::Unknown:
		default:
			Q_ASSERT(false);
			gw = std::make_shared<Gateway>(gwType, gwID, gwDesc);
		};

		return gw;
	}

	void Gateways::writeToXml(XmlWriteHelper& xml) const
	{
		xml.setAutoFormatting(true);
		xml.writeStartDocument();

		xml.writeStartElement(XmlElement::GATEWAYS);
		xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_gateways.size()));

		for(GatewayShared gw : m_gateways)
		{
			TEST_PTR_CONTINUE(gw);

			xml.writeStartElement(XmlElement::GATEWAY);

			xml.writeEnumKeyAttribute<E::GatewayType>(XmlAttribute::GATEWAY_TYPE, gw->gatewayType());
			xml.writeStringAttribute(XmlAttribute::GATEWAY_ID, gw->gatewayID());
			xml.writeStringAttribute(XmlAttribute::GATEWAY_DESCRIPTION, gw->gatewayDescription());

			gw->writeToXml(xml);

			xml.writeEndElement();			// </Gateway>
		}

		xml.writeEndElement();		// </Gateways>

		xml.writeEndDocument();
	}

	void Gateways::fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const
	{
		TEST_PTR_RETURN(acquiredSignals);

		acquiredSignals->clear();

		for(auto& gw : m_gateways)
		{
			gw->fillAcquiredSignalsSet(acquiredSignals);
		}
	}

	bool Gateways::readFromXml(XmlReadHelper& xml)
	{
		m_gateways.clear();

		bool result = true;

		result &= xml.findElement(XmlElement::GATEWAYS);

		int gatewaysCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &gatewaysCount);

		RETURN_IF_FALSE(result);

		for(int i = 0; i < gatewaysCount; i++)
		{
			result &= xml.findElement(XmlElement::GATEWAY);

			BREAK_IF_FALSE(result);

			E::GatewayType gatewayType;
			QString gatewayID;
			QString datewayDescription;

			xml.readEnumKeyAttribute<E::GatewayType>(XmlAttribute::GATEWAY_TYPE, &gatewayType);
			xml.readStringAttribute(XmlAttribute::GATEWAY_ID, &gatewayID);
			xml.readStringAttribute(XmlAttribute::GATEWAY_DESCRIPTION, &datewayDescription);

			GatewayShared gw = createTypedGateway(gatewayType,
												  gatewayID,
												  datewayDescription);
			result &= gw->readFromXml(xml);

			BREAK_IF_FALSE(result);

			m_gateways.push_back(gw);
		}

		return result;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::IVS_Impulse_SignalList implementation
	//
	// ---------------------------------------------------------------------------------

	const std::set<E::Setting> IvsImpulseSignalList::m_requiredSettings =
	{
		E::Setting::ListNo,
		E::Setting::DataType,
		E::Setting::SendEvents,
		E::Setting::IncludeAppSignalID
	};

	IvsImpulseSignalList::IvsImpulseSignalList()
	{
	}

	bool IvsImpulseSignalList::isKnownSetting(E::Setting st) const
	{
		return m_requiredSettings.contains(st);
	}

	bool IvsImpulseSignalList::checkAndApplySettings(int lineNo, ParserLog& log)
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
			case E::Setting::ListNo:
				m_listNo = sv.value.toInt();
				break;

			case E::Setting::DataType:
				{
					QString dataTypeStr = sv.value.toString();

					if (dataTypeStr == "A")
					{
						m_dataType = E::SignalListDataType::Analog_A;
					}
					else
					{
						if (dataTypeStr == "B")
						{
							m_dataType = E::SignalListDataType::Discrete_B;
						}
						else
						{
							if (dataTypeStr == "D")
							{
								m_dataType = E::SignalListDataType::Discrete_D;
							}
							else
							{
								log.logError(sv.lineNo, QString("unknown signal list data type '%1' use 'A', 'B' or 'D'").
															arg(dataTypeStr));
								result = false;
							}
						}
					}
				}
				break;

			case E::Setting::SendEvents:
				m_sendEvents = sv.value.toBool();
				break;

			case E::Setting::IncludeAppSignalID:
				m_includeAppSignalID = sv.value.toBool();
				break;

			default:
				Q_ASSERT(false);
			}
		}

		return result;
	}

	int IvsImpulseSignalList::listNo() const
	{
		return m_listNo;
	}

	E::SignalListDataType IvsImpulseSignalList::dataType() const
	{
		return m_dataType;
	}

	char IvsImpulseSignalList::dataTypeLetter() const
	{
		switch(m_dataType)
		{
		case E::SignalListDataType::Analog_A:
			return 'A';

		case E::SignalListDataType::Discrete_B:
			return 'B';

		case E::SignalListDataType::Discrete_D:
			return 'D';

		default:
			Q_ASSERT(false);
		}

		return '_';
	}

	bool IvsImpulseSignalList::sendEvents() const
	{
		return m_sendEvents;
	}

	bool IvsImpulseSignalList::includeAppSignalID() const
	{
		return m_includeAppSignalID;
	}

	void IvsImpulseSignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeIntAttribute(XmlAttribute::LIST_NO, m_listNo);
		xml.writeEnumKeyAttribute<E::SignalListDataType>(XmlAttribute::DATA_TYPE, m_dataType);
		xml.writeBoolAttribute(XmlAttribute::SEND_EVENTS, m_sendEvents);
		xml.writeBoolAttribute(XmlAttribute::INCLUDE_APP_SIGNAL_ID, m_includeAppSignalID);

		xml.writeEndElement();		//	</SignalList>
	}

	bool IvsImpulseSignalList::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SIGNAL_LIST);

		result &= xml.readIntAttribute(XmlAttribute::LIST_NO, &m_listNo);
		result &= xml.readEnumKeyAttribute<E::SignalListDataType>(XmlAttribute::DATA_TYPE, &m_dataType);
		result &= xml.readBoolAttribute(XmlAttribute::SEND_EVENTS, &m_sendEvents);
		result &= xml.readBoolAttribute(XmlAttribute::INCLUDE_APP_SIGNAL_ID, &m_includeAppSignalID);

		return result;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::IVS_Impulse_Gateway implementation
	//
	// ---------------------------------------------------------------------------------

	const std::set<E::Setting> IvsImpulseGateway::m_requiredSettings =
	{
		E::Setting::LocalGatewayIP1,
		E::Setting::RemoteGatewayIP1,
		E::Setting::SystemID,
		E::Setting::ListsVersion,
		E::Setting::Period,
		E::Setting::TimeType
	};

	const std::set<E::Setting> IvsImpulseGateway::m_optionalSettings =
	{
		E::Setting::LocalGatewayIP2,
		E::Setting::RemoteGatewayIP2
	};

	IvsImpulseGateway::IvsImpulseGateway() :
		Gateway(E::GatewayType::IVS_Impulse)
	{
	}

	IvsImpulseGateway::IvsImpulseGateway(const QString& gwID, const QString& gwDesc) :
		Gateway(E::GatewayType::IVS_Impulse, gwID, gwDesc)
	{
	}

	bool IvsImpulseGateway::isKnownSetting(E::Setting st) const
	{
		return Gateway::isKnownSetting(st) ||
				m_requiredSettings.contains(st) ||
				m_optionalSettings.contains(st);
	}

	bool IvsImpulseGateway::checkAndApplySettings(int lineNo, ParserLog& log)
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

			case E::Setting::SystemID:
				m_systemID = sv.value.toInt();
				break;

			case E::Setting::ListsVersion:
				m_listsVersion = sv.value.toInt();
				break;

			case E::Setting::Period:
				m_period = sv.value.toInt();
				break;

			case E::Setting::TimeType:
				{
					QString timeTypeStr = sv.value.toString();

					bool ok = true;

					m_timeType = ::E::stringToValue<::E::TimeType>(timeTypeStr, &ok);

					if (ok == false)
					{
						log.logError(sv.lineNo, QString("unknown gateway time type '%1' use 'Plant', 'System' or 'Local'").
													arg(timeTypeStr));
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

	void IvsImpulseGateway::appendSignalList()
	{
		m_signalLists.push_back(std::make_shared<IvsImpulseSignalList>());
	}

	int IvsImpulseGateway::systemID() const
	{
		return m_systemID;
	}

	HostAddressPort IvsImpulseGateway::localGatewayIP1() const
	{
		return m_localGatewayIP1;
	}

	HostAddressPort IvsImpulseGateway::remoteGatewayIP1() const
	{
		return m_remoteGatewayIP1;
	}

	HostAddressPort IvsImpulseGateway::localGatewayIP2() const
	{
		return m_localGatewayIP2;
	}

	HostAddressPort IvsImpulseGateway::remoteGatewayIP2() const
	{
		return m_remoteGatewayIP2;
	}

	int IvsImpulseGateway::listsVersion() const
	{
		return m_listsVersion;
	}

	::E::TimeType IvsImpulseGateway::timeType() const
	{
		return m_timeType;
	}

	int IvsImpulseGateway::period() const
	{
		return m_period;
	}

	void IvsImpulseGateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeIntAttribute(XmlAttribute::SYSTEM_ID, m_systemID);
		xml.writeHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, m_localGatewayIP1);
		xml.writeHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP1, m_remoteGatewayIP1);
		xml.writeHostAddressPortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, m_localGatewayIP2);
		xml.writeHostAddressPortAttribute(XmlAttribute::REMOTE_GATEWAY_IP2, m_remoteGatewayIP2);
		xml.writeIntAttribute(XmlAttribute::LISTS_VERSION, m_listsVersion);
		xml.writeEnumKeyAttribute<::E::TimeType>(XmlAttribute::TIME_TYPE, m_timeType);
		xml.writeIntAttribute(XmlAttribute::PERIOD, m_period);
		xml.writeEndElement();		//	</Settings>
	}

	bool IvsImpulseGateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

		result &= xml.readIntAttribute(XmlAttribute::SYSTEM_ID, &m_systemID);

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

		//

		result &= xml.readIntAttribute(XmlAttribute::LISTS_VERSION, &m_listsVersion);
		result &= xml.readEnumKeyAttribute<::E::TimeType>(XmlAttribute::TIME_TYPE, &m_timeType);
		result &= xml.readIntAttribute(XmlAttribute::PERIOD, &m_period);

		return result;
	}

	bool IvsImpulseGateway::generateRequiredFiles(const SignalSetAdapter& signalSetAdapter,
													ParserLog& log)
	{
		RETURN_IF_FALSE(checkSignalListsSettings(log));
		RETURN_IF_FALSE(generateSignalListsFiles(signalSetAdapter, log));

		return true;
	}

	bool IvsImpulseGateway::checkSignalListsSettings(ParserLog& log)
	{
		bool result = true;

		std::map<DataType_ListID, IvsImpulseSignalListShared> listsIDs;

		for(SignalListShared& l : m_signalLists)
		{
			IvsImpulseSignalListShared sl =
					std::dynamic_pointer_cast<IvsImpulseSignalList>(l);

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
		}

		return result;
	}

	bool IvsImpulseGateway::generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log)
	{
		m_files.clear();
		bool result = true;

		for(SignalListShared& l : m_signalLists)
		{
			IvsImpulseSignalListShared sl =
					std::dynamic_pointer_cast<IvsImpulseSignalList>(l);

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

		return result;
	}

	bool IvsImpulseGateway::generateSignalListFile(const IvsImpulseSignalList& signalList,
													 File& file,
													 const SignalSetAdapter& signalSetAdapter,
													 ParserLog& log)
	{
		QTextStream ts(&file.mutableFileData());

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

		return result;
	}

	bool operator < (const IvsImpulseGateway::DataType_ListID& s1,
					 const IvsImpulseGateway::DataType_ListID& s2)
	{
		return (TO_INT(s1.dataType) * 1000 + s1.listID) < (TO_INT(s2.dataType) * 1000 + s2.listID);
	}

}
