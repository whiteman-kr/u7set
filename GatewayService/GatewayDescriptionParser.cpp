#include "GatewayDescriptionParser.h"
#include "../UtilsLib/WUtils.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::SignalSetAdapter implementation
	//
	// ---------------------------------------------------------------------------------

	SignalSetAdapter::SignalSetAdapter(const AppSignalSet* appSignalSet) :
		m_appSignalSet(appSignalSet)
	{
	}

	SignalSetAdapter::SignalSetAdapter(const AppSignals& appSignals) :
		m_appSignals(&appSignals)
	{
	}

	const AppSignal* SignalSetAdapter::getAppSignal(const QString& appSignalID) const
	{
		if (m_appSignalSet != nullptr)
		{
			return m_appSignalSet->getSignal(appSignalID);
		}

		if (m_appSignals != nullptr)
		{
			return m_appSignals->getSignalByID(appSignalID);
		}

		Q_ASSERT(false);

		return nullptr;
	}

	// ---------------------------------------------------------------------------------
	//
	// Struct Gateway::Parser::ParseLineResult implementation
	//
	// ---------------------------------------------------------------------------------

	void Parser::ParseLineResult::setError(const QString& err)
	{
		msgType = LogMsgType::Error;
		msg = err;
	}

	void Parser::ParseLineResult::setWarning(const QString& wrn)
	{
		msgType = LogMsgType::Warning;
		msg = wrn;
	}

	void Parser::ParseLineResult::setMessage(const QString& msg)
	{
		msgType = LogMsgType::Message;
		this->msg = msg;
	}

	void Parser::ParseLineResult::clear()
	{
		*this = ParseLineResult();
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::ParserLog implementation
	//
	// ---------------------------------------------------------------------------------

	void ParserLog::logResult(int lineNo, LogMsgType msgType, const QString& msg)
	{
		log(lineNo, msgType, message(lineNo, msg));
	}

	void ParserLog::logError(int lineNo, const QString& errMsg)
	{
		log(lineNo, LogMsgType::Error, message(lineNo, errMsg));
	}

	void ParserLog::logError(const QString& errMsg)
	{
		log(0, LogMsgType::Error, errMsg);
	}

	void ParserLog::logWarning(int lineNo,
								  const QString& wrnMsg)
	{
		log(lineNo, LogMsgType::Warning, message(lineNo, wrnMsg));
	}

	void ParserLog::logWarning(const QString& wrnMsg)
	{
		log(0, LogMsgType::Warning, wrnMsg);
	}

	void ParserLog::logRequirtedSettingIsNotSet(int lineNo, E::Setting st)
	{
		logError(lineNo, QString("required setting '%1' is not set").
						arg(::E::valueToString<E::Setting>(st)));
	}

	int ParserLog::errorCount() const
	{
		return m_errCount;
	}

	int ParserLog::warningCount() const
	{
		return m_wrnCount;
	}

	QString ParserLog::message(int lineNo, const QString& msg)
	{
		if (lineNo == 0)
		{
			return msg;
		}

		return QString("line %1, %2").arg(lineNo).arg(msg);
	}

	void ParserLog::log(int lineNo, LogMsgType msgType, const QString& msg)
	{
		push_back({lineNo, msgType, msg});

		switch(msgType)
		{
		case LogMsgType::Error:
			m_errCount++;
			break;

		case LogMsgType::Warning:
			m_wrnCount++;
			break;

		case LogMsgType::Message:
			break;

		case LogMsgType::Nothing:
		default:
			Q_ASSERT(false);
		}
	}

	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::Parser implementation
	//
	// ---------------------------------------------------------------------------------

	const QString Parser::START_COMMENT("//");

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
		{ E::Setting::LocalGatewayIP1,		E::SettingType::IpPort	},
		{ E::Setting::RemoteGatewayIP1,		E::SettingType::IpPort	},
		{ E::Setting::LocalGatewayIP2,		E::SettingType::IpPort	},
		{ E::Setting::RemoteGatewayIP2,		E::SettingType::IpPort	},
		{ E::Setting::ListsVersion,			E::SettingType::Int		},
		{ E::Setting::Period,				E::SettingType::Int		},
		{ E::Setting::TimeType,				E::SettingType::String	},

		// IVS Impulse signal lists specific settings
		//
		{ E::Setting::SendEvents,			E::SettingType::Bool	},
		{ E::Setting::ListNo,				E::SettingType::Int		},
		{ E::Setting::DataType,				E::SettingType::String	},
		{ E::Setting::IncludeAppSignalID,	E::SettingType::Bool	},
	};

	const QRegularExpression Parser::m_anyWhitespaceSymbol("\\s");
	const QRegularExpression Parser::m_notAlphaNumericUnderlineSymbols("[^a-zA-Z0-9_]");

	Parser::Parser(const AppSignalSet* appSignalSet, GatewaysShared gateways) :
		m_signalSetAdapter(appSignalSet),
		m_gateways(gateways)
	{
		commonInitialization();
	}

	Parser::Parser(const AppSignals& appSignals, GatewaysShared gateways) :
		m_signalSetAdapter(appSignals),
		m_gateways(gateways)
	{
		commonInitialization();
	}

	Parser::~Parser()
	{
		clear();
	}

	void Parser::commonInitialization()
	{
		if (m_gateways == nullptr)
		{
			m_gateways = std::make_shared<Gateways>();
		}

		m_knownSections = ::E::enumKeyStrings<E::Section>();
		m_knownSettings = ::E::enumKeyStrings<E::Setting>();
	}

	void Parser::clear()
	{
		m_log.clear();
		m_gateways = nullptr;
	}

	bool Parser::parse(const QString& desc)
	{
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

			if (plr.msgType != LogMsgType::Nothing)
			{
				m_log.logResult(plr.lineNo, plr.msgType, plr.msg);
			}

			if (plr.lineType == LineType::Comment)
			{
				continue;
			}

			if (plr.msgType == LogMsgType::Error)
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
			m_gateways->last()->checkAndApplySettings(0, m_log);
			break;

		case E::Section::SignalList:
			m_gateways->last()->m_signalLists.back()->checkAndApplySettings(0, m_log);
			break;

		default:
			Q_ASSERT(false);
		}

		if (errCount == 0)
		{
			result &= generateGatewaysRequiredFiles(m_signalSetAdapter);
		}

		return result;
	}

	const ParserLog& Parser::log() const
	{
		return m_log;
	}

	GatewaysShared Parser::gateways()
	{
		Q_ASSERT(m_gateways != nullptr);
		return m_gateways;
	}

	bool Parser::generateGatewaysRequiredFiles(SignalSetAdapter signalSetAdapter)
	{
		bool result = true;

		for(GatewayShared gw : *m_gateways)
		{
			result &= gw->generateRequiredFiles(signalSetAdapter, m_log);
		}

		return result;
	}

	Parser::ParseResult Parser::parseUnknownSection(E::Section& parsingSection,
													const ParseLineResult& plr)
	{
		if (plr.lineType == LineType::Section &&
			plr.section == E::Section::Gateway)
		{
			m_gateways->append(std::make_shared<Gateway>());
			parsingSection = E::Section::Gateway;
			return ParseResult::Ok;
		}

		m_log.logError(plr.lineNo, "section [Gateway] expected");
		return ParseResult::CriticalError;
	}

	Parser::ParseResult Parser::parseGatewaySection(E::Section& parsingSection,
													   const ParseLineResult& plr)
	{
		GatewayShared gw = m_gateways->last();

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

					m_gateways->setLast(createTypedGateway(gatewayType));

					Q_ASSERT(m_gateways->last() != nullptr);

					bool alreadyExists = m_gateways->last()->setSettingValue(plr.lineNo, plr.setting, plr.value);

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
				m_gateways->append(std::make_shared<Gateway>());
				parsingSection = E::Section::Gateway;
				return ParseResult::Ok;

			case E::Section::SignalList:
				gw->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways->last()->appendSignalList();
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
		GatewayShared gw = m_gateways->last();
		SignalListShared sl = gw->m_signalLists.back();

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
				m_gateways->append(std::make_shared<Gateway>());
				parsingSection = E::Section::Gateway;
				return ParseResult::Ok;

			case E::Section::SignalList:
				sl->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways->last()->appendSignalList();
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
		plr->msgType = LogMsgType::Nothing;
		plr->msg.clear();

		QString toParse = str.trimmed();

		// exclude comments first

		qsizetype startCommentIndex = toParse.indexOf(START_COMMENT);

		if (startCommentIndex != -1)
		{
			toParse = toParse.mid(0, startCommentIndex).trimmed();
		}

		if (toParse.isEmpty() == true)
		{
			plr->lineType = LineType::Comment;
			return true;
		}

		// here toParse contains trimmed string without comment

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

			plr->msgType = LogMsgType::Error;
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

		bool ok = false;

		bool boolVal = stringToBool(valueStr, &ok);

		if (ok == true)
		{
			plr->value = QVariant(boolVal);
		}
		else
		{
			plr->setError("setting value is not boolean (use 1/0, on/off, yes/no, true/false)");
		}

		return ok;
	}

	bool Parser::parseIpPortValueStr(const QString& valueStr, ParseLineResult* plr)
	{
		TEST_PTR_RETURN_FALSE(plr);

		QStringList sl = valueStr.trimmed().split(":", Qt::SkipEmptyParts);

		bool ipValid = false;

		if (sl.count() >= 1)
		{
			// check IP
			ipValid = HostAddressPort::isValidIPv4(sl[0]);
		}

		bool portValid = true;		// ok!

		if (sl.count() == 2)
		{
			// check port
			portValid = HostAddressPort::isValidPort(sl[1]);
		}

		if (ipValid && portValid)
		{
			plr->value = QVariant(valueStr.trimmed());	// IpPort value store as string
		}
		else
		{
			plr->setError("setting value is no valid IP:Port (for example 127.0.0.0 or 127.0.0.0:3500)");
		}

		return ipValid && portValid;
	}

	GatewayShared Parser::createTypedGateway(E::GatewayType gwType)
	{
		switch(gwType)
		{
		case E::GatewayType::IVS_Impulse:
			return std::make_shared<IvsImpulseGateway>();

		default:
			Q_ASSERT(false);
		}

		return nullptr;
	}
}
