#include <CommonLib/ConstStrings.h>
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"

#include "GatewayDescription.h"
#include "GatewayDescriptionParser.h"
#include "IvsImpulseGateway.h"
#include "ModbusTcpSlaveGateway.h"

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

		auto [it, inserted] = m_settingsValues.insert({ st, sv });

		return !inserted;		// if inserted == true - setting value already exists
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

	SignalList::SignalList()
	{
		m_signalIDs.reserve(10000);
	}

	bool SignalList::setSettingValue(int lineNo, E::Setting st, const QVariant& value, ParserLog& log)
	{
		ParseResult pr = checkAndApplySetting(lineNo, st, value, log);

		if (pr != ParseResult::Ok)
		{
			return false;
		}

		Q_ASSERT(m_settingsValues.contains(st) == false);

		m_settingsValues.insert(lineNo, st, value);

		return true;
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

	ParseResult SignalList::checkAndApplySetting(int lineNo, E::Setting st, const QVariant& value, ParserLog& log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(st);
		Q_UNUSED(value);
		Q_UNUSED(log);
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

		return ParseResult::Ok;
	}

	ParseResult SignalList::appendAddressSignalID(int lineNo, const QString& addressStr, const QString& appSignalID, ParserLog& log)
	{
		Q_UNUSED(addressStr);

		return appendSignalID(lineNo, appSignalID, log);
	}

	std::optional<::E::SignalType> SignalList::signalType() const
	{
		return m_signalType;
	}

	void SignalList::setSignalType(::E::SignalType st)
	{
		m_signalType = st;
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

	const std::set<E::Setting> Gateway::m_gatewayOptionalSettings =
	{
		E::Setting::Enable,
	};

	Gateway::Gateway() :
		m_gatewayType(E::GatewayType::Unknown)
	{
	}

	Gateway::Gateway(E::GatewayType gwType) :
		m_gatewayType(gwType)
	{
	}

	Gateway::Gateway(E::GatewayType gwType, const QString& gwID, const QString& gwDesc, bool enable) :
		m_gatewayType(gwType),
		m_gatewayID(gwID),
		m_gatewayDescription(gwDesc),
		m_enable(enable)
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

	bool Gateway::enable() const
	{
		return m_enable;
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
		return m_gatewayRequiredSettings.contains(st) ||
			   m_gatewayOptionalSettings.contains(st);
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

			case E::Setting::Enable:
				m_enable = sv.value.toBool();

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

	GatewayShared Gateways::createTypedGateway(E::GatewayType gwType, const QString& gwID, const QString& gwDesc, bool enable)
	{
		GatewayShared gw;

		switch(gwType)
		{
		case E::GatewayType::IVS_Impulse:
			gw = std::make_shared<IvsImpulseGateway>(gwID, gwDesc, enable);
			break;

		case E::GatewayType::ModbusTcpSlave:
			gw = std::make_shared<ModbusTcpSlaveGateway>(gwID, gwDesc, enable);
			break;

		case E::GatewayType::Unknown:
		default:
			Q_ASSERT(false);
			gw = std::make_shared<Gateway>(gwType, gwID, gwDesc, enable);
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
			xml.writeBoolAttribute(XmlAttribute::ENABLE, gw->enable());

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
			bool enable = true;

			result &= xml.readEnumKeyAttribute<E::GatewayType>(XmlAttribute::GATEWAY_TYPE, &gatewayType);
			result &= xml.readStringAttribute(XmlAttribute::GATEWAY_ID, &gatewayID);
			result &= xml.readStringAttribute(XmlAttribute::GATEWAY_DESCRIPTION, &datewayDescription);
			result &= xml.readBoolAttribute(XmlAttribute::ENABLE, &enable);

			BREAK_IF_FALSE(result);

			GatewayShared gw = createTypedGateway(gatewayType,
												  gatewayID,
												  datewayDescription,
												  enable);
			result &= gw->readFromXml(xml);

			BREAK_IF_FALSE(result);

			m_gateways.push_back(gw);
		}

		return result;
	}
}
