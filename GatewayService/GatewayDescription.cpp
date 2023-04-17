#include "../lib/ConstStrings.h"
#include "../CommonLib/Types.h"
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

		RETURN_IF_FALSE(signalListsCount);

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

			GatewayShared gw = std::make_shared<Gateway>(gatewayType,
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

	const std::set<E::Setting> IVS_Impulse_SignalList::m_requiredSettings =
	{
		E::Setting::ListNo,
		E::Setting::DataType,
		E::Setting::SendEvents,
		E::Setting::IncludeAppSignalID
	};

	IVS_Impulse_SignalList::IVS_Impulse_SignalList()
	{
	}

	bool IVS_Impulse_SignalList::isKnownSetting(E::Setting st) const
	{
		return m_requiredSettings.contains(st);
	}

	bool IVS_Impulse_SignalList::checkAndApplySettings(int lineNo, ParserLog& log)
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
							log.logError(sv.lineNo, QString("unknown signal list data type '%1' use 'A' or 'B' instead").
														arg(dataTypeStr));
							result = false;
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

	int IVS_Impulse_SignalList::listNo() const
	{
		return m_listNo;
	}

	E::SignalListDataType IVS_Impulse_SignalList::dataType() const
	{
		return m_dataType;
	}

	char IVS_Impulse_SignalList::dataTypeLetter() const
	{
		switch(m_dataType)
		{
		case E::SignalListDataType::Analog_A:
			return 'A';

		case E::SignalListDataType::Discrete_B:
			return 'B';

		default:
			Q_ASSERT(false);
		}

		return '_';
	}

	bool IVS_Impulse_SignalList::sendEvents() const
	{
		return m_sendEvents;
	}

	bool IVS_Impulse_SignalList::includeAppSignalID() const
	{
		return m_includeAppSignalID;
	}

	void IVS_Impulse_SignalList::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SIGNAL_LIST);

		xml.writeIntAttribute(XmlAttribute::LIST_NO, m_listNo);
		xml.writeEnumKeyAttribute<E::SignalListDataType>(XmlAttribute::DATA_TYPE, m_dataType);
		xml.writeBoolAttribute(XmlAttribute::SEND_EVENTS, m_sendEvents);
		xml.writeBoolAttribute(XmlAttribute::INCLUDE_APP_SIGNAL_ID, m_includeAppSignalID);

		xml.writeEndElement();		//	</SignalList>
	}

	bool IVS_Impulse_SignalList::readSettingsFromXml(XmlReadHelper& xml)
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

	const std::set<E::Setting>	IVS_Impulse_Gateway::m_requiredSettings =
	{
		E::Setting::GatewayIP1,
		E::Setting::GatewayIP2,
		E::Setting::SystemID,
		E::Setting::ListsVersion,
		E::Setting::Period
	};

	IVS_Impulse_Gateway::IVS_Impulse_Gateway() :
		Gateway(E::GatewayType::IVS_Impulse)
	{
	}

	bool IVS_Impulse_Gateway::isKnownSetting(E::Setting st) const
	{
		return Gateway::isKnownSetting(st) ||
				m_requiredSettings.contains(st);
	}

	bool IVS_Impulse_Gateway::checkAndApplySettings(int lineNo, ParserLog& log)
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
			case E::Setting::GatewayIP1:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_gatewayIP1 = addrPort;
				break;

			case E::Setting::GatewayIP2:
				addrPort.setAddressPortStr(sv.value.toString(),  0);
				m_gatewayIP2 = addrPort;
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

			default:
				;	// ok
			}
		}

		return result;
	}

	void IVS_Impulse_Gateway::appendSignalList()
	{
		m_signalLists.push_back(std::make_shared<IVS_Impulse_SignalList>());
	}

	void IVS_Impulse_Gateway::writeSettingsToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::SETTINGS);

		xml.writeIntAttribute(XmlAttribute::SYSTEM_ID, m_systemID);
		xml.writeHostAddressPortAttribute(XmlAttribute::GATEWAY_IP1, m_gatewayIP1);
		xml.writeHostAddressPortAttribute(XmlAttribute::GATEWAY_IP2, m_gatewayIP2);
		xml.writeIntAttribute(XmlAttribute::LISTS_VERSION, m_listsVersion);
		xml.writeIntAttribute(XmlAttribute::PERIOD, m_period);
		xml.writeEndElement();		//	</Settings>
	}

	bool IVS_Impulse_Gateway::readSettingsFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::SETTINGS);

		result &= xml.readIntAttribute(XmlAttribute::SYSTEM_ID, &m_systemID);
		result &= xml.readHostAddressPortAttribute(XmlAttribute::GATEWAY_IP1, &m_gatewayIP1);
		result &= xml.readHostAddressPortAttribute(XmlAttribute::GATEWAY_IP2, &m_gatewayIP2);
		result &= xml.readIntAttribute(XmlAttribute::LISTS_VERSION, &m_listsVersion);
		result &= xml.readIntAttribute(XmlAttribute::PERIOD, &m_period);

		return result;
	}

	bool IVS_Impulse_Gateway::generateRequiredFiles(const SignalSetAdapter& signalSetAdapter,
													ParserLog& log)
	{
		RETURN_IF_FALSE(checkSignalListsSettings(log));
		RETURN_IF_FALSE(generateSignalListsFiles(signalSetAdapter, log));

		return true;
	}

	bool IVS_Impulse_Gateway::checkSignalListsSettings(ParserLog& log)
	{
		bool result = true;

		std::map<int, IVS_Impulse_SignalList_Shared> listsIDs;

		for(SignalListShared l : m_signalLists)
		{
			IVS_Impulse_SignalList_Shared sl =
					std::dynamic_pointer_cast<IVS_Impulse_SignalList>(l);

			TEST_PTR_CONTINUE(sl);

			auto it = listsIDs.find(sl->listNo());

			if (it == listsIDs.end())
			{
				listsIDs.insert({ sl->listNo(), sl });
				continue;
			}

			SettingValue sv1 = it->second->getSettingValue(E::Setting::ListNo);
			SettingValue sv2 = sl->getSettingValue(E::Setting::ListNo);

			log.logError(QString("duplicate signal lists ListNo = %1 (lines %2, %3)").
						 arg(sl->listNo()).arg(sv1.lineNo).arg(sv2.lineNo));

			result = false;
		}

		return result;
	}

	bool IVS_Impulse_Gateway::generateSignalListsFiles(const SignalSetAdapter& signalSetAdapter, ParserLog& log)
	{
		m_files.clear();
		bool result = true;

		for(SignalListShared l : m_signalLists)
		{
			IVS_Impulse_SignalList_Shared sl =
					std::dynamic_pointer_cast<IVS_Impulse_SignalList>(l);

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

	bool IVS_Impulse_Gateway::generateSignalListFile(const IVS_Impulse_SignalList& signalList,
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
}
