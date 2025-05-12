#ifndef GATEWAY_LIB_DOMAIN
#error Do not include this file in the project! Link GatewayLib instead.
#endif

#include "IvsImpulseGateway.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

namespace Gateway
{

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::IvsImpulseSignalList implementation
	//
	// ---------------------------------------------------------------------------------

	IvsImpulseSignalList::IvsImpulseSignalList()
	{
		appendRequiredSettings({	E::Setting::ListNo,
									E::Setting::DataType,
									E::Setting::SendEvents,
									E::Setting::IncludeAppSignalID	});
	}

	ParseResult IvsImpulseSignalList::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		ParseResult pr = ParseResult::Ok;

		switch(sv.setting)
		{
		default:
			pr = SignalList::checkAndApplySetting(sv, log);
			break;

		case E::Setting::ListNo:
			m_listNo = sv.value.toInt();
			break;

		case E::Setting::DataType:
			{
				QString dataTypeStr =sv.value.toString();

				if (dataTypeStr == "A")
				{
					m_dataType = E::SignalListDataType::Analog_A;
					setSignalType(::E::SignalType::Analog);
				}
				else
				{
					if (dataTypeStr == "B")
					{
						m_dataType = E::SignalListDataType::Discrete_B;
						setSignalType(::E::SignalType::Discrete);
					}
					else
					{
						if (dataTypeStr == "D")
						{
							m_dataType = E::SignalListDataType::Discrete_D;
							setSignalType(::E::SignalType::Discrete);
						}
						else
						{
							log.logError(sv.lineNo, QString("unknown signal list data type '%1' use 'A', 'B' or 'D'").
													arg(dataTypeStr));
							pr = ParseResult::Error;
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
		}

		return pr;
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
   // Class Gateway::IvsImpulseGateway implementation
   //
   // ---------------------------------------------------------------------------------

	IvsImpulseGateway::IvsImpulseGateway() :
		Gateway(E::GatewayType::IVS_Impulse)
	{
		appendRequiredSettings({	E::Setting::LocalGatewayIP1,
									E::Setting::RemoteGatewayIP1,
									E::Setting::SystemID,
									E::Setting::ListsVersion,
									E::Setting::Period,
									E::Setting::TimeType });

		appendOptionalSettings({	E::Setting::LocalGatewayIP2,
									E::Setting::RemoteGatewayIP2 });
	}

	ParseResult IvsImpulseGateway::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		ParseResult pr = ParseResult::Ok;

		HostAddressPort addrPort;

		switch(sv.setting)
		{
		default:
			pr = Gateway::checkAndApplySetting(sv, log);
			break;

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
					pr = ParseResult::Error;
				}
			}
			break;
		}

		return pr;
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
		xml.writeIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, m_localGatewayIP1);
		xml.writeIPv4PortAttribute(XmlAttribute::REMOTE_GATEWAY_IP1, m_remoteGatewayIP1);
		xml.writeIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, m_localGatewayIP2);
		xml.writeIPv4PortAttribute(XmlAttribute::REMOTE_GATEWAY_IP2, m_remoteGatewayIP2);
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

		okIp1 &= xml.readIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP1, &m_localGatewayIP1);
		okIp1 &= xml.readIPv4PortAttribute(XmlAttribute::REMOTE_GATEWAY_IP1, &m_remoteGatewayIP1);

			   //

		bool okIp2 = true;

		m_localGatewayIP2.clear();
		m_remoteGatewayIP2.clear();

		okIp2 &= xml.readIPv4PortAttribute(XmlAttribute::LOCAL_GATEWAY_IP2, &m_localGatewayIP2);
		okIp2 &= xml.readIPv4PortAttribute(XmlAttribute::REMOTE_GATEWAY_IP2, &m_remoteGatewayIP2);

		result &= okIp1 || okIp2;

			   //

		result &= xml.readIntAttribute(XmlAttribute::LISTS_VERSION, &m_listsVersion);
		result &= xml.readEnumKeyAttribute<::E::TimeType>(XmlAttribute::TIME_TYPE, &m_timeType);
		result &= xml.readIntAttribute(XmlAttribute::PERIOD, &m_period);

		return result;
	}

	bool IvsImpulseGateway::generateRequiredFiles(const AppSignalSet* signalSet,
												  ParserLog& log)
	{
		RETURN_IF_FALSE(checkSignalListsSettings(log));
		RETURN_IF_FALSE(generateSignalListsFiles(signalSet, log));

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

	bool IvsImpulseGateway::generateSignalListsFiles(const AppSignalSet* signalSet, ParserLog& log)
	{
		TEST_PTR_RETURN_FALSE(signalSet);

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

			File& file = m_files.emplace_back(m_gatewayType, gatewayID(), fileName);

			result &= generateSignalListFile(*sl, file, signalSet, log);
		}

		return result;
	}

	bool IvsImpulseGateway::generateSignalListFile(const IvsImpulseSignalList& signalList,
												   File& file,
												   const AppSignalSet* signalSet,
												   ParserLog& log)
	{
		TEST_PTR_RETURN_FALSE(signalSet);

		QTextStream ts(&file.mutableFileData());

		bool result = true;

		int paramIndex = 1;

		QString str;

		for(const QString& signalID : signalList.signalIDs())
		{
			const AppSignal* s = signalSet->getSignal(signalID);

			TEST_PTR_CONTINUE(s);

			bool res = true;

			switch(signalList.dataType())
			{
			case E::SignalListDataType::Analog_A:

				if (s->isAnalog() == false)
				{
					log.logError(QString("signal '%1' is not Analog (GatewayID = %2, ListNo = %3)").
								 arg(signalID).arg(gatewayID()).arg(signalList.listNo()));
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
								 arg(signalID).arg(gatewayID()).arg(signalList.listNo()));
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
