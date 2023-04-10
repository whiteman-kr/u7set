#include "GatewayDescription.h"
#include "../lib/ConstStrings.h"
#include "../CommonLib/Types.h"
#include "../UtilsLib/WUtils.h"


// ---------------------------------------------------------------------------------
//
// Struct GatewayDescriptionParser::ParseLineResult implementation
//
// ---------------------------------------------------------------------------------

void GatewayDescriptionParser::ParseLineResult::setError(const QString& err)
{
	msgType = MsgType::Error;
	msg = err;
}

void GatewayDescriptionParser::ParseLineResult::setWarning(const QString& wrn)
{
	msgType = MsgType::Warning;
	msg = wrn;
}

void GatewayDescriptionParser::ParseLineResult::setMessage(const QString& msg)
{
	msgType = MsgType::Message;
	this->msg = msg;
}

void GatewayDescriptionParser::ParseLineResult::clear()
{
	*this = ParseLineResult();
}

// ---------------------------------------------------------------------------------
//
// Class GatewayDescriptionParser::SignalList implementation
//
// ---------------------------------------------------------------------------------

void GatewayDescriptionParser::SignalList::setSettingValue(const ParseLineResult& plr)
{
	settingValue.insert({ plr.setting, plr.value });
}

bool GatewayDescriptionParser::SignalList::settingIsSet(Setting st) const
{
	return settingValue.contains(st);
}

bool GatewayDescriptionParser::SignalList::prepare(int lineNo, GwParserLog* log)
{
	Q_UNUSED(lineNo);
	Q_UNUSED(log);
	return true;
}

// ---------------------------------------------------------------------------------
//
// Class GatewayDescriptionParser::Gateway implementation
//
// ---------------------------------------------------------------------------------

GatewayDescriptionParser::Gateway::Gateway() :
	gatewayType(GatewayType::Unknown)
{
}

GatewayDescriptionParser::Gateway::Gateway(GatewayType gwType) :
	gatewayType(gwType)
{

}

GatewayDescriptionParser::Gateway::~Gateway()
{
	for(SignalList* list : signalLists)
	{
		delete list;
	}
}


bool GatewayDescriptionParser::Gateway::setSettingValue(const ParseLineResult& plr)
{
	settingValue.insert({ plr.setting, plr.value });

	return true;
}

bool GatewayDescriptionParser::Gateway::settingIsSet(Setting st) const
{
	return settingValue.contains(st);
}


bool GatewayDescriptionParser::Gateway::isKnownGatewaySetting(Setting st) const
{
	return	st == Setting::GatewayType ||
			st == Setting::GatewayID ||
			st == Setting::GatewayDescription;
}

bool GatewayDescriptionParser::Gateway::isKnownSignalListSetting(Setting st) const
{
	Q_UNUSED(st);
	return false;
}

void GatewayDescriptionParser::Gateway::appendSignalList()
{
	Q_ASSERT(false);		// this function should not be called
}

bool GatewayDescriptionParser::Gateway::prepare(int lineNo, GwParserLog* log)
{
	Q_UNUSED(lineNo);
	Q_UNUSED(log);
	return true;
}

// ---------------------------------------------------------------------------------
//
// Class GatewayDescriptionParser::IVS_Impulse_SignalList implementation
//
// ---------------------------------------------------------------------------------

const std::set<GatewayDescriptionParser::Setting>
			GatewayDescriptionParser::IVS_Impulse_SignalList::requiredSettings =
{
	Setting::ListNo,
	Setting::DataType,
	Setting::SendEvents,
	Setting::IncludeAppSignalID
};


GatewayDescriptionParser::IVS_Impulse_SignalList::IVS_Impulse_SignalList()
{
}

bool GatewayDescriptionParser::IVS_Impulse_SignalList::prepare(int lineNo, GwParserLog* log)
{
	bool result = true;

	for(Setting st : requiredSettings)
	{
		auto it = settingValue.find(st);

		if (it == settingValue.end())
		{
			log->logError(lineNo, QString("required signal list setting '%1' not found").
								arg(E::valueToString<Setting>(st)));
			result = false;
			continue;
		}

		Q_ASSERT(st == it->first);

		const QVariant& value = it->second;

		switch(st)
		{
		case Setting::ListNo:
			listNo = value.toInt();
			break;

		case Setting::DataType:
			{
				QString dataTypeStr = value.toString();

				if (dataTypeStr == "A")
				{
					dataType = DataType::Analog_A;
				}
				else
				{
					if (dataTypeStr == "B")
					{
						dataType = DataType::Discrete_B;
					}
					else
					{
						log->logError(QString("unknown signal list data type '%1'").arg(dataTypeStr));
						result = false;
					}
				}
			}
			break;

		case Setting::SendEvents:
			sendEvents = value.toBool();
			break;

		case Setting::IncludeAppSignalID:
			includeAppSignalID = value.toBool();
			break;

		default:
			Q_ASSERT(false);
		}
	}

	return result;
}

// ---------------------------------------------------------------------------------
//
// Class GatewayDescriptionParser::IVS_Impulse_Gateway implementation
//
// ---------------------------------------------------------------------------------

const std::set<GatewayDescriptionParser::Setting>
			GatewayDescriptionParser::IVS_Impulse_Gateway::requiredSettings =
{
	Setting::GatewayType,
	Setting::GatewayID,
	Setting::GatewayDescription,

	Setting::GatewayIP1,
	Setting::GatewayIP2,
	Setting::SystemID,
	Setting::ListsVersion,
	Setting::Period
};


GatewayDescriptionParser::IVS_Impulse_Gateway::IVS_Impulse_Gateway() :
	Gateway(GatewayType::IVS_Impulse)
{
}

bool GatewayDescriptionParser::IVS_Impulse_Gateway::isKnownGatewaySetting(Setting st) const
{
	return IVS_Impulse_Gateway::requiredSettings.contains(st);
}

bool GatewayDescriptionParser::IVS_Impulse_Gateway::isKnownSignalListSetting(Setting st) const
{
	return IVS_Impulse_SignalList::requiredSettings.contains(st);
}

void GatewayDescriptionParser::IVS_Impulse_Gateway::appendSignalList()
{
	signalLists.push_back(new IVS_Impulse_SignalList);
}

bool GatewayDescriptionParser::IVS_Impulse_Gateway::prepare(int lineNo, GwParserLog* log)
{
	bool result = true;

	HostAddressPort addrPort;

	for(Setting st : requiredSettings)
	{
		auto it = settingValue.find(st);

		if (it == settingValue.end())
		{
			log->logError(lineNo, QString("required gateway setting '%1' not found").
								arg(E::valueToString<Setting>(st)));
			result = false;
			continue;
		}

		Q_ASSERT(st == it->first);

		const QVariant& value = it->second;

		switch(st)
		{
		case Setting::GatewayType:
			// field gatewayType already set!
			break;

		case Setting::GatewayID:
			gatewayID = value.toString();
			break;

		case Setting::GatewayDescription:
			gatewayDescription = value.toString();
			break;

		case Setting::GatewayIP1:
			addrPort.setAddressPortStr(value.toString(),  0);
			gatewayIP1 = addrPort;
			break;

		case Setting::GatewayIP2:
			addrPort.setAddressPortStr(value.toString(),  0);
			gatewayIP2 = addrPort;
			break;

		case Setting::SystemID:
			systemID = value.toInt();
			break;

		case Setting::ListsVersion:
			listsVersion = value.toInt();
			break;

		case Setting::Period:
			period = value.toInt();
			break;

		default:
			Q_ASSERT(false);
		}
	}

	return result;
}

// ---------------------------------------------------------------------------------
//
// Class GatewayDescriptionParser implementation
//
// ---------------------------------------------------------------------------------

const QString GatewayDescriptionParser::START_LINE_COMMENT("//");
const QString GatewayDescriptionParser::START_MULTILINE_COMMENT("/*");
const QString GatewayDescriptionParser::END_MULTILINE_COMMENT("*/");

const QString GatewayDescriptionParser::START_SECTION("[");
const QString GatewayDescriptionParser::END_SECTION("]");

const QString GatewayDescriptionParser::EQUAL_SIGN("=");
const QString GatewayDescriptionParser::APP_SIGNAL_ID_START_SIGN("#");

const QString GatewayDescriptionParser::ERR_SYNTAX("syntax error");

const std::map<GatewayDescriptionParser::Setting,
				GatewayDescriptionParser::SettingType>
				GatewayDescriptionParser::m_settingType =
{
	{ Setting::Unknown,				SettingType::Unknown },

	{ Setting::GatewayType,			SettingType::String },
	{ Setting::GatewayID,			SettingType::String },
	{ Setting::GatewayDescription,	SettingType::String },

	{ Setting::SystemID,			SettingType::Int	},
	{ Setting::GatewayIP1,			SettingType::IpPort	},
	{ Setting::GatewayIP2,			SettingType::IpPort	},
	{ Setting::ListsVersion,		SettingType::Int	},
	{ Setting::Period,				SettingType::Int	},
	{ Setting::SendEvents,			SettingType::Bool	},
	{ Setting::ListNo,				SettingType::Int	},
	{ Setting::DataType,			SettingType::String },
	{ Setting::IncludeAppSignalID,	SettingType::Bool	},
};

const QRegularExpression GatewayDescriptionParser::m_appSignalIdTemplate("^#[a-zA-Z0-9_]");
const QRegularExpression GatewayDescriptionParser::m_anyWhitespaceTemplate("\\s");

GatewayDescriptionParser::GatewayDescriptionParser()
{
	m_knownSections = E::enumKeyStrings<Section>();
	m_knownSettings = E::enumKeyStrings<Setting>();
}

GatewayDescriptionParser::~GatewayDescriptionParser()
{
	for(Gateway* gw : m_gateways)
	{
		delete gw;
	}
}

bool GatewayDescriptionParser::parse(const QString& desc, GwParserLog* log)
{
	TEST_PTR_RETURN_FALSE(log);

	log->clear();

	bool result = true;

	QStringList strs = desc.split(Separator::NEW_LINE, Qt::KeepEmptyParts, Qt::CaseInsensitive);

	int lineNo = 0;

	// parsing states
	//
	Section parsingSection = Section::Unknown;

	for(const QString& str : strs)
	{
		lineNo++;

		ParseLineResult plr;

		plr.lineNo = lineNo;

		bool res = parseLine(str, &plr);

		result &= res;

		if (plr.msgType != MsgType::Nothing)
		{
			log->logResult(plr);
		}

		if (plr.msgType == MsgType::Error ||
			plr.lineType == LineType::Comment)
		{
			continue;
		}

		ParseResult pr = ParseResult::Ok;

		switch(parsingSection)
		{
		case Section::Unknown:
			pr = parseUnknownSection(parsingSection, plr, log);
			break;

		case Section::Gateway:
			pr = parseGatewaySection(parsingSection, plr, log);
			break;

		case Section::SignalList:
			pr = parseSignalListSection(parsingSection, plr, log);
			break;

		default:
			Q_ASSERT(false);
			result = false;
			break;
		}

		if (pr == ParseResult::Error)
		{
			result = false;
			continue;
		}

		if (pr == ParseResult::CriticalError)
		{
			result = false;
			break;
		}
	}

	return result;
}

GatewayDescriptionParser::ParseResult GatewayDescriptionParser::parseUnknownSection(Section& parsingSection,
												const ParseLineResult& plr,
												GwParserLog* log)
{
	if (log == nullptr)
	{
		return ParseResult::CriticalError;
	}

	if (plr.lineType == LineType::Section &&
		plr.section == Section::Gateway)
	{
		m_gateways.push_back(new Gateway);
		parsingSection = Section::Gateway;
		return ParseResult::Ok;
	}

	log->logError(plr.lineNo, "section [Gateway] expected");
	return ParseResult::CriticalError;
}

GatewayDescriptionParser::ParseResult GatewayDescriptionParser::parseGatewaySection(Section& parsingSection,
												   const ParseLineResult& plr,
												   GwParserLog* log)
{
	Gateway* gw = m_gateways.back();

	bool res = true;

	switch(plr.lineType)
	{
	case LineType::Setting:

		if (gw->gatewayType == GatewayType::Unknown)
		{
			if (plr.setting == Setting::GatewayType)
			{
				QString gatewayTypeStr = plr.value.toString();

				bool res = true;

				GatewayType gatewayType = E::stringToValue<GatewayType>(gatewayTypeStr, &res);

				if (res == false ||
					gatewayType == GatewayType::Unknown)
				{
					log->logError(plr.lineNo, QString("unknown GatewayType - '%1'").
										arg(plr.value.toString()));
					return ParseResult::CriticalError;
				}

				delete m_gateways.back();		// delete base Gateway

				m_gateways.back() = createApropriateGateway(gatewayType);

				Q_ASSERT(m_gateways.back() != nullptr);

				m_gateways.back()->setSettingValue(plr);

				return ParseResult::Ok;
			}
			else
			{
				log->logError(plr.lineNo, QString("setting 'GatewayType' expected"));
				return ParseResult::CriticalError;
			}
		}

		if (gw->isKnownGatewaySetting(plr.setting) == false)
		{
			log->logError(plr.lineNo, QString("unknown gateway setting '%1'").
						arg(E::valueToString<Setting>(plr.setting)));
			return ParseResult::Error;
		}

		if (gw->settingIsSet(plr.setting) == true)
		{
			log->logWarning(plr.lineNo, QString("gateway setting '%1' already set").
					   arg(E::valueToString<Setting>(plr.setting)));
		}

		res = gw->setSettingValue(plr);

		return (res == true ? ParseResult::Ok : ParseResult::Error);

	case LineType::Section:
		switch(plr.section)
		{
		case Section::Gateway:
			gw->prepare(plr.lineNo, log);
			m_gateways.push_back(new Gateway);
			parsingSection = Section::Gateway;
			return ParseResult::Ok;

		case Section::SignalList:
			gw->prepare(plr.lineNo, log);
			m_gateways.back()->appendSignalList();
			parsingSection = Section::SignalList;
			return ParseResult::Ok;

		default:
			Q_ASSERT(false);
			break;
		}

	default:
		break;
	}

	log->logError(plr.lineNo, "unexpected  token");

	return ParseResult::Error;
}

GatewayDescriptionParser::ParseResult GatewayDescriptionParser::parseSignalListSection(Section& parsingSection, const ParseLineResult& plr, GwParserLog* log)
{
	Gateway* gw = m_gateways.back();
	SignalList* sl = gw->signalLists.back();

	switch(plr.lineType)
	{
	case LineType::Setting:
		if (gw->isKnownSignalListSetting(plr.setting) == false)
		{
			log->logError(plr.lineNo, QString("unknown signal list setting '%1'").
							arg(E::valueToString<Setting>(plr.setting)));
			return ParseResult::Error;
		}

		if (sl->settingIsSet(plr.setting) == true)
		{
			log->logWarning(plr.lineNo, QString("signal list setting '%1' already set").
					   arg(E::valueToString<Setting>(plr.setting)));
		}

		sl->setSettingValue(plr);

		return ParseResult::Ok;

	case LineType::AppSignalID:
	case LineType::CustomAppSignalID:
		sl->signalIDs.push_back(plr.value.toString());
		return ParseResult::Ok;

	case LineType::Section:
		switch(plr.section)
		{
		case Section::Gateway:
			sl->prepare(plr.lineNo, log);
			m_gateways.push_back(new Gateway);
			parsingSection = Section::Gateway;
			return ParseResult::Ok;

		case Section::SignalList:
			sl->prepare(plr.lineNo, log);
			m_gateways.back()->appendSignalList();
			parsingSection = Section::SignalList;
			return ParseResult::Ok;

		default:
			Q_ASSERT(false);
			break;
		}
	}

	return ParseResult::Error;
}

bool GatewayDescriptionParser::parseLine(const QString& str, ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	plr->lineType = LineType::Unknown;
	plr->msgType = MsgType::Nothing;
	plr->msg.clear();

	QString toParse;
	QString line = str.trimmed();

	// exclude comments first

	if (line.isEmpty() == true ||
		line.startsWith(START_LINE_COMMENT) == true)
	{
		plr->lineType = LineType::Comment;
		return true;
	}

	if (m_multilineCommentStarted == true)
	{
		qsizetype index = line.indexOf(END_MULTILINE_COMMENT);

		if (index == -1)
		{
			plr->lineType = LineType::Comment;
			return true;
		}

		m_multilineCommentStarted = false;

		toParse = line.mid(index + 2);
	}
	else
	{
		qsizetype index = line.indexOf(START_MULTILINE_COMMENT);

		if (index == -1)
		{
			index = line.indexOf(START_LINE_COMMENT);

			if (index == -1)
			{
				toParse = line;
			}
			else
			{
				toParse = line.mid(0, index);
			}
		}
		else
		{
			toParse = line.mid(0, index);
			m_multilineCommentStarted = true;
		}
	}

	// toParse contains string without comment

	toParse = toParse.trimmed();

	// check secton token

	qsizetype startSectionIndex = toParse.indexOf(START_SECTION);
	qsizetype endSectionIndex = toParse.indexOf(END_SECTION);

	if(startSectionIndex != -1 && endSectionIndex != -1)
	{
		if (startSectionIndex >= endSectionIndex ||						// [   ]
			toParse.mid(0, startSectionIndex).isEmpty() == false ||		// no symbols before [
			toParse.mid(endSectionIndex + 1).isEmpty() == false)		// no symbols after ]
		{
			plr->setError(ERR_SYNTAX);
			return false;
		}

		QString sectionID = toParse.replace(START_SECTION, "").
									replace(END_SECTION, "").
									trimmed().toLower();

		for(const QString& knownSection : m_knownSections)
		{
			if (sectionID == knownSection.toLower())
			{
				plr->lineType = LineType::Section;

				bool ok = true;
				plr->section = E::stringToValue<Section>(knownSection, &ok);
				Q_ASSERT(ok == true);

				return true;
			}
		}

		plr->msgType = MsgType::Error;
		plr->msg = QString("unknown section %1").arg(toParse.trimmed());
		return false;
	}

	// check setting token

	qsizetype equalSignIndex = toParse.indexOf(EQUAL_SIGN);

	if (equalSignIndex != -1)
	{
		QString settingID = toParse.mid(0, equalSignIndex).trimmed();
		QString settingValueStr = toParse.mid(equalSignIndex + 1).trimmed();

		if (settingID.isEmpty() == true ||
			settingValueStr.isEmpty() == true)
		{
			plr->setError(ERR_SYNTAX);
			return false;
		}

		QString lowercaseSettingID = settingID.toLower();

		Setting st = Setting::Unknown;

		for(const QString& knownSetting : m_knownSettings)
		{
			if (lowercaseSettingID == knownSetting.toLower())
			{
				bool ok = true;
				st = E::stringToValue<Setting>(knownSetting, &ok);
				Q_ASSERT(ok == true);
				break;
			}
		}

		if (st == Setting::Unknown)
		{
			plr->setError(QString("unknown setting - %1").arg(settingID));
			return false;
		}

		plr->lineType = LineType::Setting;
		plr->setting = st;

		return parseSettingValue(st, settingValueStr, plr);;
	}

	// check signalID token

	if (toParse.contains(m_appSignalIdTemplate) == true)
	{
		plr->lineType = LineType::AppSignalID;
		plr->value = QVariant(toParse);
		return true;
	}

	if (toParse.contains(m_anyWhitespaceTemplate) != true)
	{
		plr->lineType = LineType::CustomAppSignalID;
		plr->value = QVariant(toParse);
		return true;
	}

	plr->setError(ERR_SYNTAX);

	return false;
}

bool GatewayDescriptionParser::parseSettingValue(Setting setting,
												 const QString& valueStr,
												 ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	auto it = m_settingType.find(setting);

	if (it == m_settingType.end())
	{
		Q_ASSERT(false);
		plr->setError("unknown setting type");
		return false;
	}

	bool result = true;

	SettingType settingType = it->second;

	switch(settingType)
	{
	case SettingType::Int:
		result &= parseIntValueStr(valueStr, plr);
		break;

	case SettingType::String:
		plr->value = QVariant(valueStr.trimmed());
		break;

	case SettingType::Bool:
		result &= parseBoolValueStr(valueStr, plr);
		break;

	case SettingType::IpPort:
		result &= parseIpPortValueStr(valueStr, plr);
		break;

	case SettingType::Unknown:
	default:
		Q_ASSERT(false);
		plr->setError("unknown setting type");
		return false;
	}

	return result;
}

bool GatewayDescriptionParser::parseIntValueStr(const QString& valueStr,
												ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	QString valStr = valueStr.trimmed().toLower();

	bool result = true;

	int intValue = valueStr.toInt(&result);

	if (result == false)
	{
		plr->setError(QString("setting value is not a number"));
	}
	else
	{
		plr->value = QVariant(intValue);
	}

	return result;
}

bool GatewayDescriptionParser::parseBoolValueStr(const QString& valueStr,
												 ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	QString valStr = valueStr.trimmed().toLower();

	static const std::set<QString> trueStr =
	{
		QString("1"),
		QString("true"),
		QString("yes"),
		QString("on"),
	};

	static const std::set<QString> falseStr =
	{
		QString("0"),
		QString("false"),
		QString("no"),
		QString("off"),
	};

	bool result = true;

	if (trueStr.contains(valStr) == true)
	{
		plr->value = QVariant(true);
	}
	else
	{
		if (falseStr.contains(valStr) == true)
		{
			plr->value = QVariant(false);
		}
		else
		{
			plr->setError("setting value is not boolean (1/0, on/off, yes/no, true/false)");
			result = false;
		}
	}

	return result;
}

bool GatewayDescriptionParser::parseIpPortValueStr(const QString& valueStr,
												   ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	QStringList sl = valueStr.trimmed().split(":", Qt::SkipEmptyParts);

	static const QString errMsg("setting value is no valid IP:Port (for example 127.0.0.0:3500)");

	bool result = true;

	if (sl.count() == 2)
	{
		if (HostAddressPort::isValidIPv4(sl[0]) == false ||
			HostAddressPort::isValidPort(sl[1]) == false)
		{
			plr->setError(errMsg);
			result = false;
		}
		else
		{
			plr->value = QVariant(valueStr.trimmed());	// IpPort value store as string
		}
	}
	else
	{
		plr->setError(errMsg);
		result = false;
	}

	return result;
}

GatewayDescriptionParser::Gateway* GatewayDescriptionParser::createApropriateGateway(GatewayType gwType)
{
	switch(gwType)
	{
	case GatewayType::IVS_Impulse:
		return new IVS_Impulse_Gateway;

	default:
		Q_ASSERT(false);
	}

	return nullptr;
}



// ---------------------------------------------------------------------------------
//
// Class GwParserLog implementation
//
// ---------------------------------------------------------------------------------

void GwParserLog::logResult(const GatewayDescriptionParser::ParseLineResult& plr)
{
	log(plr.lineNo, plr.msgType, plr.msg);
}

void GwParserLog::logError(int lineNo,
							const QString& errMsg)
{
	log(lineNo, GatewayDescriptionParser::MsgType::Error, errMsg);
}

void GwParserLog::logError(const QString& errMsg)
{
	log(0, GatewayDescriptionParser::MsgType::Error, errMsg);
}

void GwParserLog::logWarning(int lineNo,
							  const QString& wrnMsg)
{
	log(lineNo, GatewayDescriptionParser::MsgType::Warning, wrnMsg);
}

void GwParserLog::logWarning(const QString& wrnMsg)
{
	log(0, GatewayDescriptionParser::MsgType::Warning, wrnMsg);
}

void GwParserLog::log(int lineNo, GatewayDescriptionParser::MsgType msgType, const QString& msg)
{
	push_back({lineNo, msgType, msg});
}
