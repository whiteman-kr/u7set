#include <CommonLib/ConstStrings.h>
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

#include "GatewayDescription.h"
#include "IvsImpulseGateway.h"
#include "ModbusSlaveGateway.h"

namespace Gateway
{
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
	// Class Gateway::SettingsSet implementation
	//
	// ---------------------------------------------------------------------------------

	SettingsSet::SettingsSet()
	{
	}

	SettingsSet::~SettingsSet()
	{
	}

	void SettingsSet::appendRequiredSetting(E::Setting reqSetting)
	{
		m_requiredSettings.insert(reqSetting);
	}

	void SettingsSet::appendRequiredSettings(const std::vector<E::Setting>& reqSettings)
	{
		for(E::Setting s : reqSettings)
		{
			m_requiredSettings.insert(s);
		}
	}

	void SettingsSet::appendOptionalSetting(E::Setting optSetting)
	{
		m_optionalSettings.insert(optSetting);
	}

	void SettingsSet::appendOptionalSettings(const std::vector<E::Setting>& optSettings)
	{
		for(E::Setting s : optSettings)
		{
			m_optionalSettings.insert(s);
		}
	}

	bool SettingsSet::isKnownSetting(E::Setting st) const
	{
		return m_requiredSettings.contains(st) ||
			   m_optionalSettings.contains(st);
	}

	bool SettingsSet::settingIsSet(E::Setting st) const
	{
		return m_settingsValues.contains(st);
	}

	const std::map<E::Setting, SettingValue>& SettingsSet::settingsValues() const
	{
		return m_settingsValues;
	}

	ParseResult SettingsSet::setSettingValue(int lineNo, E::Setting st, const QVariant& value, ParserLog& log)
	{
		if (isKnownSetting(st) == false)
		{
			log.logError(lineNo, QString("unknown setting '%1'").arg(settingName(st)));
			return ParseResult::Error;
		}

		if (m_settingsValues.contains(st))
		{
			log.logError(lineNo, QString("setting '%1' already set").arg(settingName(st)));
			return ParseResult::Error;
		}

		SettingValue sv;

		sv.lineNo = lineNo;
		sv.setting = st;
		sv.value = value;

		m_settingsValues.emplace(st, sv);

		return ParseResult::Ok;
	}

	bool SettingsSet::setSettingValue(E::Setting st, const QVariant& value)
	{
		ParserLog log;
		ParseResult pr = setSettingValue(0, st, value, log);

		return (pr == ParseResult::Ok);
	}

	const SettingValue& SettingsSet::getSettingValue(E::Setting st) const
	{
		auto it = m_settingsValues.find(st);

		if (it == m_settingsValues.end())
		{
			return m_invalidSettingValue;
		}

		return it->second;
	}

	QString SettingsSet::settingName(E::Setting st) const
	{
		return ::E::valueToString<E::Setting>(st);
	}

	bool SettingsSet::isSettingsChecked() const
	{
		return m_settingsChecked;
	}

	ParseResult SettingsSet::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		log.logError(QString("checkAndApplySetting is not implemented for setting '%1'").
							arg(settingName(sv.setting)));

		return ParseResult::Error;
	}

	ParseResult SettingsSet::checkRequiredSettings(int lineNo, ParserLog& log)
	{
		ParseResult pr = ParseResult::Ok;

		for(E::Setting st : m_requiredSettings)
		{
			if (m_settingsValues.contains(st) == false)
			{
				log.logError(lineNo, QString("required setting '%1' is not set").
									 arg(::E::valueToString<E::Setting>(st)));
				pr = ParseResult::Error;
			}
		}

		return pr;
	}

	ParseResult SettingsSet::checkAndApplySettings(int lineNo, ParserLog& log)
	{
		m_settingsChecked = true;

		ParseResult pr = checkRequiredSettings(lineNo, log);

		if (pr != ParseResult::Ok)
		{
			return pr;
		}

		for(const auto& [st, settingValue] : m_settingsValues)
		{
			Q_ASSERT(st == settingValue.setting);

			if (isKnownSetting(st) == false)
			{
				log.logError(lineNo, QString("unknown setting '%1'").arg(settingName(st)));
				pr = ParseResult::Error;
				continue;
			}

			ParseResult pr2 = checkAndApplySetting(settingValue, log);

			if (pr2 != ParseResult::Ok)
			{
				pr = ParseResult::Error;
			}
		}

		return pr;
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::SignalList implementation
	//
	// ---------------------------------------------------------------------------------

	SignalList::SignalList()
	{
		appendOptionalSetting(E::Setting::UniqSignalsInList);

		m_signalIDs.reserve(1000);
	}

	ParseResult SignalList::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		Q_UNUSED(log);

		switch(sv.setting)
		{
		default:
			return SettingsSet::checkAndApplySetting(sv, log);

		case E::Setting::UniqSignalsInList:
			m_uniqSignalsInList = sv.value.toBool();
			break;
		}

		return ParseResult::Ok;
	}

	ParseResult SignalList::checkSignalTypeAndFormat(int lineNo, const AppSignal* appSignal, ParserLog& log)
	{
		TEST_PTR_RETURN_VALUE(appSignal, ParseResult::CriticalError);

		if (m_signalType.has_value() == false)
		{
			log.logError(lineNo, QString("required signal type of list is undefined, set list signal type (format) first"));
			return ParseResult::CriticalError;
		}

		if (appSignal->signalType() != m_signalType.value())
		{
			log.logError(lineNo, QString("signal type of '%1' isn't corresponds to list signal type '%2'").
									   arg(appSignal->appSignalID(), ::E::valueToString(m_signalType.value())));
			return ParseResult::Error;
		}

		return ParseResult::Ok;
	}

	ParseResult SignalList::appendSignalID(int lineNo, const QString& appSignalID, ParserLog& log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(log);

		m_signalIDs.emplace_back(appSignalID);

		Hash h = calcHash(appSignalID);

		if (m_uniqSignalsInList && m_existSignals.contains(h))
		{
			log.logError(lineNo, QString("signal '%1' duplicated in list").arg(appSignalID));
		}

		m_existSignals.insert(h);

		return ParseResult::Ok;
	}

	ParseResult SignalList::parseAddressStr(int lineNo, const QString& addStr, Address16* addr, ParserLog& log)
	{
		Q_UNUSED(addStr);
		Q_UNUSED(addr);

		log.logError(lineNo, "parseAddressStr is not implemented for this gateway type");
		return 	ParseResult::Error;
	}

	ParseResult SignalList::appendAddressSignalID(int lineNo, const Address16& addr16, const QString& appSignalID, ParserLog& log)
	{
		Q_UNUSED(addr16);
		Q_UNUSED(appSignalID);

		log.logError(lineNo, "appendAddressSignalID is not implemented for this gateway type");
		return 	ParseResult::Error;
	}

	ParseResult SignalList::appendAddressConstValue(int lineNo, const Address16& addr16, const QString& desc, double constValue, ParserLog& log)
	{
		Q_UNUSED(addr16);
		Q_UNUSED(desc);
		Q_UNUSED(constValue);

		log.logError(lineNo, "appendAddressConstValue is not implemented for this gateway type");
		return 	ParseResult::Error;
	}

	std::optional<::E::SignalType> SignalList::signalType() const
	{
		return m_signalType;
	}

	void SignalList::setSignalType(::E::SignalType st)
	{
		m_signalType = st;
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

	Gateway::Gateway() :
		m_gatewayType(E::GatewayType::Unknown)
	{
	}

	Gateway::Gateway(E::GatewayType gwType) :
		m_gatewayType(gwType)
	{
		appendRequiredSettings({	E::Setting::GatewayType,
									E::Setting::GatewayID,
									E::Setting::GatewayDescription	});

		appendOptionalSettings({	E::Setting::Enable,
									E::Setting::UniqSignalsInAllLists	});
	}

	std::shared_ptr<Gateway> Gateway::createTypedGateway(E::GatewayType gwType)
	{
		switch(gwType)
		{
		case E::GatewayType::IVS_Impulse:
			return std::make_shared<IvsImpulseGateway>();

		case E::GatewayType::ModbusSlave:
			return std::make_shared<ModbusSlaveGateway>();

		default:
			Q_ASSERT(false);
		}

		return nullptr;
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

	bool Gateway::enable() const
	{
		return m_enable;
	}

	bool Gateway::uniqSignalsInAllLists() const
	{
		return m_uniqSignalsInAllLists;
	}

	int Gateway::signalListsCount() const
	{
		return TO_INT(m_signalLists.size());
	}

	ParseResult Gateway::checkAndApplySetting(const SettingValue& sv, ParserLog& log)
	{
		Q_UNUSED(log);

		ParseResult pr = ParseResult::Ok;

		switch(sv.setting)
		{
		default:
			return SettingsSet::checkAndApplySetting(sv, log);

		case E::Setting::GatewayType:
			{
				// m_gatewayType should be set during typedGateway creation in Parser
				// here only check gatewayType
				//
				bool ok = false	;

				E::GatewayType gwType = ::E::stringToValue<E::GatewayType>(sv.value.toString(), &ok);

				if (!ok || m_gatewayType != gwType)
				{
					Q_ASSERT(false);
					log.logError(sv.lineNo, "check m_gatewayType ERROR!");
					pr = ParseResult::CriticalError;
				}
			}
			break;

		case E::Setting::GatewayID:
			m_gatewayID = sv.value.toString();
			break;

		case E::Setting::GatewayDescription:
			m_gatewayDescription = sv.value.toString();
			break;

		case E::Setting::Enable:
			m_enable = sv.value.toBool();
			break;

		case E::Setting::UniqSignalsInAllLists:
			m_uniqSignalsInAllLists = sv.value.toBool();
			break;
		}

		return pr;
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
		xml.writeStartElement(XmlElement::GATEWAY);

		xml.writeEnumKeyAttribute<E::GatewayType>(XmlAttribute::GATEWAY_TYPE, m_gatewayType);
		xml.writeStringAttribute(XmlAttribute::GATEWAY_ID, m_gatewayID);
		xml.writeStringAttribute(XmlAttribute::GATEWAY_DESCRIPTION, m_gatewayDescription);
		xml.writeBoolAttribute(XmlAttribute::ENABLE, m_enable);
		xml.writeBoolAttribute(XmlAttribute::UNIQ_SIGNALS_IN_ALL_LISTS, m_uniqSignalsInAllLists);

		writeSettingsToXml(xml);
		writeSignalListsToXml(xml);

		xml.writeEndElement();			// </Gateway>
	}

	std::shared_ptr<Gateway> Gateway::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::GATEWAY);

		RETURN_VALUE_IF_FALSE(result, nullptr);

		E::GatewayType gatewayType;

		result &= xml.readEnumKeyAttribute<E::GatewayType>(XmlAttribute::GATEWAY_TYPE, &gatewayType);

		RETURN_VALUE_IF_FALSE(result, nullptr);

		GatewayShared gw = Gateway::createTypedGateway(gatewayType);

		if (gw == nullptr)
		{
			Q_ASSERT(false);
			return nullptr;
		}

		QString gatewayID;
		QString datewayDescription;
		bool enable = true;
		bool uniqSignalsInAllLists = false;

		result &= xml.readStringAttribute(XmlAttribute::GATEWAY_ID, &gatewayID);
		result &= xml.readStringAttribute(XmlAttribute::GATEWAY_DESCRIPTION, &datewayDescription);
		result &= xml.readBoolAttribute(XmlAttribute::ENABLE, &enable);
		result &= xml.readBoolAttribute(XmlAttribute::UNIQ_SIGNALS_IN_ALL_LISTS, &uniqSignalsInAllLists);

		RETURN_VALUE_IF_FALSE(result, nullptr);

		//

		result &= gw->setSettingValue(E::Setting::GatewayID, gatewayID);
		result &= gw->setSettingValue(E::Setting::GatewayDescription, datewayDescription);
		result &= gw->setSettingValue(E::Setting::Enable, enable);
		result &= gw->setSettingValue(E::Setting::UniqSignalsInAllLists, uniqSignalsInAllLists);

		//

		result &= gw->readSettingsFromXml(xml);
		result &= gw->readSignalListsFromXml(xml);

		return gw;
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

		for(const SignalListShared& sl : m_signalLists)
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

	bool Gateway::generateRequiredFiles(const AppSignalSet* signalSet, ParserLog& log)
	{
		Q_UNUSED(signalSet);
		Q_UNUSED(log);
		return true;
	}

	void Gateway::initSettingsSet()
	{
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

	void Gateways::replaceLast(GatewayShared gw)
	{
		if (m_gateways.empty())
		{
			Q_ASSERT(false);
			return;
		}

		m_gateways.back() = gw;
	}

	GatewayShared Gateways::last()
	{
		if (m_gateways.empty())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return m_gateways.back();
	}

	bool Gateways::isUniqGatewayID(const QString& gwID) const
	{
		for(const GatewayShared& gw : m_gateways)
		{
			if (gw->gatewayID() == gwID)
			{
				return false;
			}
		}

		return true;
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

	void Gateways::fillAcquiredSignalsSet(std::set<Hash>* acquiredSignals) const
	{
		TEST_PTR_RETURN(acquiredSignals);

		acquiredSignals->clear();

		for(auto& gw : m_gateways)
		{
			gw->fillAcquiredSignalsSet(acquiredSignals);
		}
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

			gw->writeToXml(xml);
		}

		xml.writeEndElement();		// </Gateways>

		xml.writeEndDocument();
	}

	bool Gateways::readFromXml(XmlReadHelper& xml, bool skipDisabledGateways, QStringList* disabledGateways)
	{
		m_gateways.clear();

		bool result = true;

		result &= xml.findElement(XmlElement::GATEWAYS);

		int gatewaysCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &gatewaysCount);

		RETURN_IF_FALSE(result);

		for(int i = 0; i < gatewaysCount; i++)
		{
			GatewayShared gw = Gateway::readFromXml(xml);		// returns typedGateway

			if (gw == nullptr)
			{
				result = false;
				continue;
			}

			if (gw->enable() == false)
			{
				if (disabledGateways != nullptr)
				{
					disabledGateways->append(gw->gatewayID());
				}

				if (skipDisabledGateways)
				{
					continue;
				}
			}

			m_gateways.push_back(gw);
		}

		return result;
	}
}
