#include "GatewayDescription.h"
#include "../lib/ConstStrings.h"
#include "../CommonLib/Types.h"
#include "../UtilsLib/WUtils.h"

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
	// Struct Gateway::Parser::ParseLineResult implementation
	//
	// ---------------------------------------------------------------------------------

	void Parser::ParseLineResult::setError(const QString& err)
	{
		msgType = MsgType::Error;
		msg = err;
	}

	void Parser::ParseLineResult::setWarning(const QString& wrn)
	{
		msgType = MsgType::Warning;
		msg = wrn;
	}

	void Parser::ParseLineResult::setMessage(const QString& msg)
	{
		msgType = MsgType::Message;
		this->msg = msg;
	}

	void Parser::ParseLineResult::clear()
	{
		*this = ParseLineResult();
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Parser::Log implementation
	//
	// ---------------------------------------------------------------------------------

	void Parser::Log::logResult(const Parser::ParseLineResult& plr)
	{
		log(plr.lineNo, plr.msgType, message(plr.lineNo, plr.msg));
	}

	void Parser::Log::logError(int lineNo,
								const QString& errMsg)
	{
		log(lineNo, Parser::MsgType::Error, message(lineNo, errMsg));
	}

	void Parser::Log::logError(const QString& errMsg)
	{
		log(0, Parser::MsgType::Error, errMsg);
	}

	void Parser::Log::logWarning(int lineNo,
								  const QString& wrnMsg)
	{
		log(lineNo, Parser::MsgType::Warning, message(lineNo, wrnMsg));
	}

	void Parser::Log::logWarning(const QString& wrnMsg)
	{
		log(0, Parser::MsgType::Warning, wrnMsg);
	}

	void Parser::Log::logRequirtedSettingIsNotSet(int lineNo, E::Setting st)
	{
		logError(lineNo, QString("required setting '%1' is not set").
						arg(::E::valueToString<E::Setting>(st)));
	}

	QString Parser::Log::message(int lineNo, const QString& msg)
	{
		if (lineNo == 0)
		{
			return msg;
		}

		return QString("line %1, %2").arg(lineNo).arg(msg);
	}

	void Parser::Log::log(int lineNo, Parser::MsgType msgType, const QString& msg)
	{
		push_back({lineNo, msgType, msg});
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Parser implementation
	//
	// ---------------------------------------------------------------------------------

	const QString Parser::START_LINE_COMMENT("//");
	const QString Parser::START_MULTILINE_COMMENT("/*");
	const QString Parser::END_MULTILINE_COMMENT("*/");

	const QString Parser::START_SECTION("[");
	const QString Parser::END_SECTION("]");

	const QString Parser::EQUAL_SIGN("=");
	const QString Parser::APP_SIGNAL_ID_START_SIGN("#");

	const QString Parser::ERR_SYNTAX("syntax error");

	const std::map<E::Setting,
				   E::SettingType>
				   Parser::m_settingType =
	{
		{ E::Setting::Unknown,				E::SettingType::Unknown	},

		// Common gateways settings
		//
		{ E::Setting::GatewayType,			E::SettingType::AlphaNumericUnderlineString	},
		{ E::Setting::GatewayID,			E::SettingType::AlphaNumericUnderlineString	},
		{ E::Setting::GatewayDescription,	E::SettingType::String	},

		// IVS Impulse gateway specific settings
		//
		{ E::Setting::SystemID,				E::SettingType::Int		},
		{ E::Setting::GatewayIP1,			E::SettingType::IpPort	},
		{ E::Setting::GatewayIP2,			E::SettingType::IpPort	},
		{ E::Setting::ListsVersion,			E::SettingType::Int		},
		{ E::Setting::Period,				E::SettingType::Int		},

		// IVS Impulse signal lists specific settings
		//
		{ E::Setting::SendEvents,			E::SettingType::Bool	},
		{ E::Setting::ListNo,				E::SettingType::Int		},
		{ E::Setting::DataType,				E::SettingType::String	},
		{ E::Setting::IncludeAppSignalID,	E::SettingType::Bool	},
	};

	const QRegularExpression Parser::m_anyWhitespaceSymbol("\\s");
	const QRegularExpression Parser::m_notAlphaNumericUnderlineSymbols("[^a-zA-Z0-9_]");

	Parser::Parser()
	{
		m_knownSections = ::E::enumKeyStrings<E::Section>();
		m_knownSettings = ::E::enumKeyStrings<E::Setting>();
	}

	Parser::~Parser()
	{
		clear();
	}

	bool Parser::parse(const QString& desc)
	{
		clear();

		bool result = true;

		QStringList strs = desc.split(Separator::NEW_LINE, Qt::KeepEmptyParts, Qt::CaseInsensitive);

		int errCount = 0;
		int lineNo = 0;

		// parsing states
		//
		E::Section parsingSection = E::Section::Unknown;

		for(const QString& str : strs)
		{
			lineNo++;

			ParseLineResult plr;

			plr.lineNo = lineNo;

			bool res = parseLine(str, &plr);

			result &= res;

			if (plr.msgType != MsgType::Nothing)
			{
				m_log.logResult(plr);
			}

			if (plr.lineType == LineType::Comment)
			{
				continue;
			}

			if (plr.msgType == MsgType::Error)
			{
				errCount++;
				continue;
			}

			ParseResult pr = ParseResult::Ok;

			switch(parsingSection)
			{
			case E::Section::Unknown:
				pr = parseUnknownSection(parsingSection, plr);
				break;

			case E::Section::Gateway:
				pr = parseGatewaySection(parsingSection, plr);
				break;

			case E::Section::SignalList:
				pr = parseSignalListSection(parsingSection, plr);
				break;

			default:
				Q_ASSERT(false);
				result = false;
				break;
			}

			if (pr == ParseResult::Error)
			{
				errCount++;
				result = false;
				continue;
			}

			if (pr == ParseResult::CriticalError)
			{
				errCount++;
				result = false;
				break;
			}
		}

		// finalize parsing
		switch(parsingSection)
		{
		case E::Section::Unknown:
			break;

		case E::Section::Gateway:
			m_gateways.back()->checkAndApplySettings(0, m_log);
			break;

		case E::Section::SignalList:
			m_gateways.back()->m_signalLists.back()->checkAndApplySettings(0, m_log);
			break;

		default:
			Q_ASSERT(false);
		}

		if (errCount == 0)
		{
			result &= generateGatewaysRequiredFiles();
		}

		return result;
	}

	const Parser::Log& Parser::log() const
	{
		return m_log;
	}

	bool Parser::generateGatewaysRequiredFiles()
	{
		m_log.clear();

		bool result = true;

		for(Gateway* gw : m_gateways)
		{
			result &= gw->generateRequiredFiles(m_log);
		}

		return result;
	}

	Parser::ParseResult Parser::parseUnknownSection(E::Section& parsingSection,
													const ParseLineResult& plr)
	{
		if (plr.lineType == LineType::Section &&
			plr.section == E::Section::Gateway)
		{
			m_gateways.push_back(new Gateway);
			parsingSection = E::Section::Gateway;
			return ParseResult::Ok;
		}

		m_log.logError(plr.lineNo, "section [Gateway] expected");
		return ParseResult::CriticalError;
	}

	Parser::ParseResult Parser::parseGatewaySection(E::Section& parsingSection,
													   const ParseLineResult& plr)
	{
		Gateway* gw = m_gateways.back();

		bool res = true;

		switch(plr.lineType)
		{
		case LineType::Setting:

			if (gw->m_gatewayType == E::GatewayType::Unknown)
			{
				if (plr.setting == E::Setting::GatewayType)
				{
					QString gatewayTypeStr = plr.value.toString();

					res = true;

					E::GatewayType gatewayType = ::E::stringToValue<E::GatewayType>(gatewayTypeStr, &res);

					if (res == false ||
						gatewayType == E::GatewayType::Unknown)
					{
						m_log.logError(plr.lineNo, QString("unknown GatewayType - '%1'").
											arg(plr.value.toString()));
						return ParseResult::CriticalError;
					}

					delete m_gateways.back();		// delete base Gateway

					m_gateways.back() = createApropriateGateway(gatewayType);

					Q_ASSERT(m_gateways.back() != nullptr);

					bool alreadyExists = m_gateways.back()->setSettingValue(plr.lineNo, plr.setting, plr.value);

					Q_ASSERT(alreadyExists == false);

					return ParseResult::Ok;
				}
				else
				{
					m_log.logError(plr.lineNo, QString("setting 'GatewayType' expected"));
					return ParseResult::CriticalError;
				}
			}

			if (gw->isKnownSetting(plr.setting) == false)
			{
				m_log.logError(plr.lineNo, QString("unknown gateway setting '%1'").
							arg(::E::valueToString<E::Setting>(plr.setting)));
				return ParseResult::Error;
			}

			if (gw->settingIsSet(plr.setting) == true)
			{
				m_log.logWarning(plr.lineNo, QString("gateway setting '%1' already set").
						   arg(::E::valueToString<E::Setting>(plr.setting)));
			}

			gw->setSettingValue(plr.lineNo, plr.setting, plr.value);

			return (res == true ? ParseResult::Ok : ParseResult::Error);

		case LineType::Section:
			switch(plr.section)
			{
			case E::Section::Gateway:
				gw->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways.push_back(new Gateway);
				parsingSection = E::Section::Gateway;
				return ParseResult::Ok;

			case E::Section::SignalList:
				gw->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways.back()->appendSignalList();
				parsingSection = E::Section::SignalList;
				return ParseResult::Ok;

			default:
				Q_ASSERT(false);
				break;
			}

		default:
			break;
		}

		m_log.logError(plr.lineNo, "unexpected  token");

		return ParseResult::Error;
	}

	Parser::ParseResult Parser::parseSignalListSection(E::Section& parsingSection,
													   const ParseLineResult& plr)
	{
		Gateway* gw = m_gateways.back();
		SignalList* sl = gw->m_signalLists.back();

		switch(plr.lineType)
		{
		case LineType::Setting:
			if (sl->isKnownSetting(plr.setting) == false)
			{
				m_log.logError(plr.lineNo, QString("unknown signal list setting '%1'").
								arg(::E::valueToString<E::Setting>(plr.setting)));
				return ParseResult::Error;
			}

			if (sl->settingIsSet(plr.setting) == true)
			{
				m_log.logWarning(plr.lineNo, QString("signal list setting '%1' already set").
						   arg(::E::valueToString<E::Setting>(plr.setting)));
			}

			sl->setSettingValue(plr.lineNo, plr.setting, plr.value);

			return ParseResult::Ok;

		case LineType::SignalID:
			sl->m_signalIDs.push_back(plr.value.toString());
			return ParseResult::Ok;

		case LineType::Section:
			switch(plr.section)
			{
			case E::Section::Gateway:
				sl->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways.push_back(new Gateway);
				parsingSection = E::Section::Gateway;
				return ParseResult::Ok;

			case E::Section::SignalList:
				sl->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways.back()->appendSignalList();
				parsingSection = E::Section::SignalList;
				return ParseResult::Ok;

			default:
				Q_ASSERT(false);
				break;
			}
		}

		return ParseResult::Error;
	}

	bool Parser::parseLine(const QString& str, ParseLineResult* plr)
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

			plr->lineType = LineType::Section;

			QString sectionID = toParse.replace(START_SECTION, "").
										replace(END_SECTION, "").
										trimmed().toLower();

			for(const QString& knownSection : m_knownSections)
			{
				if (sectionID == knownSection.toLower())
				{
					bool ok = true;
					plr->section = ::E::stringToValue<E::Section>(knownSection, &ok);
					Q_ASSERT(ok == true);

					return true;
				}
			}

			plr->msgType = MsgType::Error;
			plr->msg = QString("unknown section - %1").arg(toParse.trimmed());
			return false;
		}

		// check setting token

		qsizetype equalSignIndex = toParse.indexOf(EQUAL_SIGN);

		if (equalSignIndex != -1)
		{
			plr->lineType = LineType::Setting;

			QString settingID = toParse.mid(0, equalSignIndex).trimmed();
			QString settingValueStr = toParse.mid(equalSignIndex + 1).trimmed();

			if (settingID.isEmpty() == true ||
				settingValueStr.isEmpty() == true)
			{
				plr->setError(ERR_SYNTAX);
				return false;
			}

			QString lowercaseSettingID = settingID.toLower();

			E::Setting st = E::Setting::Unknown;

			for(const QString& knownSetting : m_knownSettings)
			{
				if (lowercaseSettingID == knownSetting.toLower())
				{
					bool ok = true;
					st = ::E::stringToValue<E::Setting>(knownSetting, &ok);
					Q_ASSERT(ok == true);
					break;
				}
			}

			if (st == E::Setting::Unknown)
			{
				plr->setError(QString("unknown setting - %1").arg(settingID));
				return false;
			}

			plr->setting = st;

			return parseSettingValue(st, settingValueStr, plr);;
		}

		// check signalID token

		if (toParse.contains(m_anyWhitespaceSymbol) == true)
		{
			plr->setError("signal identifier should not contain any whitespace symbols");
			return false;
		}

		plr->lineType = LineType::SignalID;
		plr->value = QVariant(toParse);

		return true;
	}

	bool Parser::parseSettingValue(E::Setting setting,
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

		E::SettingType settingType = it->second;

		switch(settingType)
		{
		case E::SettingType::Int:
			result &= parseIntValueStr(valueStr, plr);
			break;

		case E::SettingType::String:
			plr->value = QVariant(valueStr.trimmed());
			break;

		case E::SettingType::AlphaNumericUnderlineString:
			result &= parseAlphsNumericUnderlineStr(valueStr, plr);
			break;

		case E::SettingType::Bool:
			result &= parseBoolValueStr(valueStr, plr);
			break;

		case E::SettingType::IpPort:
			result &= parseIpPortValueStr(valueStr, plr);
			break;

		case E::SettingType::Unknown:
		default:
			Q_ASSERT(false);
			plr->setError("unknown setting type");
			return false;
		}

		return result;
	}

	bool Parser::parseIntValueStr(const QString& valueStr,
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

	bool Parser::parseAlphsNumericUnderlineStr(const QString& valueStr, ParseLineResult* plr)
	{
		if (valueStr.contains(m_notAlphaNumericUnderlineSymbols) == true)
		{
			plr->setError("setting value should contains only english letters, numbers and underline sign");
			return false;
		}

		plr->value = QVariant(valueStr);

		return true;
	}

	bool Parser::parseBoolValueStr(const QString& valueStr,
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
				plr->setError("setting value is not boolean (use 1/0, on/off, yes/no, true/false)");
				result = false;
			}
		}

		return result;
	}

	bool Parser::parseIpPortValueStr(const QString& valueStr,
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

	Gateway* Parser::createApropriateGateway(E::GatewayType gwType)
	{
		switch(gwType)
		{
		case E::GatewayType::IVS_Impulse:
			return new IVS_Impulse_Gateway;

		default:
			Q_ASSERT(false);
		}

		return nullptr;
	}

	void Parser::clear()
	{
		m_log.clear();

		for(Gateway* gw : m_gateways)
		{
			delete gw;
		}

		m_gateways.clear();
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

	bool SignalList::checkAndApplySettings(int lineNo, Parser::Log& log)
	{
		Q_UNUSED(lineNo);
		Q_UNUSED(log);
		return true;
	}

	SettingValue SignalList::getSettingValue(E::Setting st) const
	{
		return m_settingsValues.getSettingVaue(st);
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

	Gateway::~Gateway()
	{
		for(SignalList* list : m_signalLists)
		{
			delete list;
		}
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

	bool Gateway::checkAndApplySettings(int lineNo, Parser::Log& log)
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

	bool Gateway::checkRequiredSettings(const std::set<E::Setting> reqSettings,
									  const SettingsValues& settingsValues,
									  int lineNo, Parser::Log& log)
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

	bool Gateway::generateRequiredFiles(Parser::Log& log)
	{
		Q_UNUSED(log);
		return true;
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

	bool IVS_Impulse_SignalList::checkAndApplySettings(int lineNo, Parser::Log& log)
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
						m_dataType = DataType::Analog_A;
					}
					else
					{
						if (dataTypeStr == "B")
						{
							m_dataType = DataType::Discrete_B;
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

	bool IVS_Impulse_Gateway::checkAndApplySettings(int lineNo, Parser::Log& log)
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
		m_signalLists.push_back(new IVS_Impulse_SignalList);
	}

	bool IVS_Impulse_Gateway::generateRequiredFiles(Parser::Log& log)
	{
		bool result = true;

		result = checkSignalListsSettings(log);

		RETURN_IF_FALSE(result);

		return result;
	}

	bool IVS_Impulse_Gateway::checkSignalListsSettings(Parser::Log& log)
	{
		bool result = true;

		std::map<int, IVS_Impulse_SignalList*> listsIDs;

		for(SignalList* l : m_signalLists)
		{
			IVS_Impulse_SignalList* sl = dynamic_cast<IVS_Impulse_SignalList*>(l);

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


}
