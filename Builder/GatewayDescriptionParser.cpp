#include "GatewayDescriptionParser.h"
#include "../UtilsLib/WUtils.h"
#include "../GatewayService/IvsImpulseGateway.h"
#include "../GatewayService/ModbusSlaveGateway.h"
#include "ModuleLogicCompiler.h"

namespace Gateway
{
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
	// Class Gateway::Parser implementation
	//
	// ---------------------------------------------------------------------------------

	const QString Parser::START_COMMENT("//");

	const QString Parser::START_SECTION("[");
	const QString Parser::END_SECTION("]");

	const QString Parser::EQUAL_SIGN("=");
	const QString Parser::LEFT_POINTER_SIGN("<-");
	const QString Parser::APP_SIGNAL_ID_START_SIGN("#");

	const QString Parser::ERR_SYNTAX("syntax error");

	const std::map<E::Setting,
				   E::SettingType>
				   Parser::m_settingType =
	{
		{ E::Setting::Unknown,					E::SettingType::Unknown	},

		// Common gateways settings
		//
		{ E::Setting::GatewayType,				E::SettingType::AlphaNumericUnderlineString	},
		{ E::Setting::GatewayID,				E::SettingType::AlphaNumericUnderlineString	},
		{ E::Setting::GatewayDescription,		E::SettingType::String	},
		{ E::Setting::Enable,					E::SettingType::Bool	},
		{ E::Setting::UniqSignalsInAllLists,	E::SettingType::Bool	},

		// Common signal lists settings
		//
		{ E::Setting::UniqSignalsInList,		E::SettingType::Bool	},

		// IVS Impulse gateway specific settings
		//
		{ E::Setting::SystemID,					E::SettingType::Int		},
		{ E::Setting::LocalGatewayIP1,			E::SettingType::IpPort	},
		{ E::Setting::RemoteGatewayIP1,			E::SettingType::IpPort	},
		{ E::Setting::LocalGatewayIP2,			E::SettingType::IpPort	},
		{ E::Setting::RemoteGatewayIP2,			E::SettingType::IpPort	},
		{ E::Setting::ListsVersion,				E::SettingType::Int		},
		{ E::Setting::Period,					E::SettingType::Int		},
		{ E::Setting::TimeType,					E::SettingType::String	},

		// IVS Impulse signal lists specific settings
		//
		{ E::Setting::SendEvents,				E::SettingType::Bool	},
		{ E::Setting::ListNo,					E::SettingType::Int		},
		{ E::Setting::DataType,					E::SettingType::String	},
		{ E::Setting::IncludeAppSignalID,		E::SettingType::Bool	},

		// ModbusTcpSlave gateway specific settings
		//
		{ E::Setting::ModbusDeviceID,			E::SettingType::Int		},
		{ E::Setting::ModbusMode,				E::SettingType::String	},

		// ModbusTcpSlave signal lists specific settings
		//
		{ E::Setting::SignalsFormat,			E::SettingType::String	},
	};

	const QRegularExpression Parser::m_anyWhitespaceSymbol("\\s");
	const QRegularExpression Parser::m_notAlphaNumericUnderlineSymbols("[^a-zA-Z0-9_]");

	Parser::Parser(const Builder::Context* context, GatewaysShared gateways) :
		m_context(context),
		m_gateways(gateways)
	{
		if (m_context == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_appSignalSet = context->m_signalSet->appSignalSet();

		if (m_gateways == nullptr)
		{
			m_gateways = std::make_shared<Gateways>();
		}

		m_knownSections = ::E::enumKeyStrings<E::Section>();
		m_knownSettings = ::E::enumKeyStrings<E::Setting>();

		//

		const std::map<Hash, Builder::ModuleLogicCompilerShared>& mlCompilers = m_context->m_moduleLogicCompilers;

		for(const auto& [mlHash, mlComp] : mlCompilers)
		{
			m_mlNotFoundIn.insert(mlHash);
		}
	}

	Parser::~Parser()
	{
		clear();
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
				return false;		// break parsing
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

	const ParserLog& Parser::log() const
	{
		return m_log;
	}

	GatewaysShared Parser::gateways()
	{
		Q_ASSERT(m_gateways != nullptr);
		return m_gateways;
	}

	bool Parser::generateGatewaysRequiredFiles()
	{
		bool result = true;

		for(GatewayShared gw : *m_gateways)
		{
			result &= gw->generateRequiredFiles(m_appSignalSet, m_log);
		}

		return result;
	}

	ParseResult Parser::parseUnknownSection(E::Section& parsingSection,
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

	ParseResult Parser::parseGatewaySection(E::Section& parsingSection,
											const ParseLineResult& plr)
	{
		GatewayShared gw = m_gateways->last();

		if (gw == nullptr)
		{
			m_log.logError(plr.lineNo, QString("last Gateway not exists"));
			return ParseResult::CriticalError;
		}

		ParseResult pr = ParseResult::Ok;

		switch(plr.lineType)
		{
		case LineType::Setting:

			if (gw->m_gatewayType == E::GatewayType::Unknown)
			{
				if (plr.setting == E::Setting::GatewayType)
				{
					QString gwTypeStr = plr.value.toString();

					E::GatewayType gatewayType = getGatewayType(gwTypeStr);

					if (gatewayType == E::GatewayType::Unknown)
					{
						m_log.logError(plr.lineNo, QString("unknown GatewayType '%1', use: %2").
												   arg(plr.value.toString(), knownGatewayTypes().join(", ")));
						return ParseResult::CriticalError;
					}

					GatewayShared typedGateway = Gateway::createTypedGateway(gatewayType);

					if (typedGateway == nullptr)
					{
						Q_ASSERT(false);
						m_log.logError(plr.lineNo, QString("createTypedGateway ERROR!"));
						return ParseResult::CriticalError;
					}

					m_gateways->replaceLast(typedGateway);

					return typedGateway->setSettingValue(plr.lineNo, plr.setting, plr.value, m_log);
				}
				else
				{
					m_log.logError(plr.lineNo, QString("setting 'GatewayType' should be specified first"));
					return ParseResult::CriticalError;
				}
			}
			else
			{
				if (plr.setting == E::Setting::GatewayID &&
					m_gateways->isUniqGatewayID(plr.value.toString()) == false)
				{
					m_log.logError(plr.lineNo, QString("GatewayID should be unique"));
					return ParseResult::CriticalError;
				}
			}

			return gw->setSettingValue(plr.lineNo, plr.setting, plr.value, m_log);

		case LineType::Section:
			switch(plr.section)
			{
			case E::Section::Gateway:
				pr = gw->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways->append(std::make_shared<Gateway>());
				parsingSection = E::Section::Gateway;
				return pr;

			case E::Section::SignalList:
				pr = gw->checkAndApplySettings(plr.lineNo, m_log);
				m_gateways->last()->appendSignalList();
				parsingSection = E::Section::SignalList;
				return pr;

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

	ParseResult Parser::parseSignalListSection(E::Section& parsingSection,
												const ParseLineResult& plr)
	{
		GatewayShared gw = m_gateways->last();
		SignalListShared sl = gw->m_signalLists.back();

		switch(plr.lineType)
		{
		case LineType::Setting:
			sl->setSettingValue(plr.lineNo, plr.setting, plr.value, m_log);
			return ParseResult::Ok;

		case LineType::SignalID:
			if (sl->isSettingsChecked() == false)
			{
				sl->checkAndApplySettings(plr.lineNo, m_log);
			}
			return appendAddressSignalID(sl, plr, false);

		case LineType::AddressSignalID:
			if (sl->isSettingsChecked() == false)
			{
				sl->checkAndApplySettings(plr.lineNo, m_log);
			}
			return appendAddressSignalID(sl, plr, true);

		case LineType::Section:
			switch(plr.section)
			{
			case E::Section::Gateway:
				m_gateways->append(std::make_shared<Gateway>());
				parsingSection = E::Section::Gateway;
				return ParseResult::Ok;

			case E::Section::SignalList:
				m_gateways->last()->appendSignalList();
				parsingSection = E::Section::SignalList;
				return ParseResult::Ok;

			default:
				Q_ASSERT(false);
				break;
			}

		default:
			Q_ASSERT(false);
		}

		return ParseResult::Error;
	}

	ParseResult Parser::appendAddressSignalID(SignalListShared signalList,
											  const ParseLineResult& plr, bool appendAddr)
	{
		TEST_PTR_RETURN_VALUE(m_appSignalSet, ParseResult::CriticalError);

		QString plrValue = plr.value.toString().trimmed();

		//

		bool isPropValue = false;
		double propValue = 0;

		ParseResult pr = parsePropValue(plr.lineNo, plrValue, &isPropValue, &propValue);

		if (isPropValue)
		{
			Address16 addr16;

			pr = signalList->parseAddressStr(plr.lineNo, plr.addressStr, &addr16, m_log);

			if (pr != ParseResult::Ok)
			{
				return pr;
			}

			pr = signalList->appendAddressConstValue(plr.lineNo, addr16, plrValue, propValue, m_log);

			return pr;
		}

		if (plrValue.startsWith("#") == false)
		{
			m_log.logError(plr.lineNo, "signal ID should starts with '#' symbol");
			return ParseResult::Error;
		}

		//

		const AppSignal* s = m_appSignalSet->getSignal(plrValue);

		if (s == nullptr)
		{
			m_log.logError(plr.lineNo, QString("signal '%1' not found").arg(plrValue));
			return ParseResult::Error;
		}

		pr = signalList->checkSignalTypeAndFormat(plr.lineNo, s, m_log);

		if (pr != ParseResult::Ok)
		{
			return pr;
		}

		if (appendAddr == false)
		{
			pr = signalList->appendSignalID(plr.lineNo, plrValue, m_log);
		}
		else
		{
			Address16 addr16;

			pr = signalList->parseAddressStr(plr.lineNo, plr.addressStr, &addr16, m_log);

			if (pr != ParseResult::Ok)
			{
				return pr;
			}

			pr = signalList->appendAddressSignalID(plr.lineNo, addr16, plrValue, m_log);
		}

		return pr;
	}

	ParseResult Parser::parsePropValue(int lineNo, const QString& plrValue, bool* isPropValue, double* propValue)
	{
		TEST_PTR_RETURN_VALUE(isPropValue, ParseResult::CriticalError);
		TEST_PTR_RETURN_VALUE(propValue, ParseResult::CriticalError);

		// plrValue already trimmed!

		static const QString PROP_VALUE("propvalue(");
		static const int PROP_VALUE_LEN = PROP_VALUE.length();
		static const QString PROP_VALUE_END(")");
		static const QString PROP_VALUE_SYNTAX_ERROR("property value syntax error, use: propValue(item_label.propName)");

		*isPropValue = false;
		*propValue = 0;

		if (plrValue.length() < PROP_VALUE_LEN)
		{
			return ParseResult::Ok;
		}

		if (plrValue.mid(0, PROP_VALUE_LEN).toLower() != PROP_VALUE)
		{
			return ParseResult::Ok;
		}

		if (plrValue.endsWith(PROP_VALUE_END) == false)
		{
			m_log.logError(lineNo, PROP_VALUE_SYNTAX_ERROR);
			return ParseResult::Error;
		}

		QString lPropName = plrValue.mid(PROP_VALUE_LEN, plrValue.length() - PROP_VALUE_LEN - 1);

		QStringList sl = lPropName.split(Separator::DOT, Qt::SkipEmptyParts);

		if (sl.size() != 2)
		{
			m_log.logError(lineNo, PROP_VALUE_SYNTAX_ERROR);
			return ParseResult::Error;
		}

		*isPropValue = true;

		QString label = sl[0];
		QString propName = sl[1];

		ParseResult res = findPropertyValue(lineNo, label, propName, propValue);

		return res;
	}

	ParseResult Parser::findPropertyValue(int lineNo, const QString& itemLabel, const QString& propName, double* propValue)
	{
		const std::map<Hash, Builder::ModuleLogicCompilerShared>& mlCompilers = m_context->m_moduleLogicCompilers;

		// first search in already found LMs
		//
		for(Hash mlHash : m_mlFoundIn)
		{
			auto it = mlCompilers.find(mlHash);

			if (it == mlCompilers.end())
			{
				Q_ASSERT(false);
				continue;
			}

			Builder::ModuleLogicCompilerShared mlCompiler = it->second;

			TEST_PTR_CONTINUE(mlCompiler);

			const auto [itemFound, propFound, prValue] = mlCompiler->getUalAfbParamValue(itemLabel, propName);

			if (itemFound)
			{
				if (propFound)
				{
					*propValue = prValue;
					return ParseResult::Ok;
				}
				else
				{
					m_log.logError(lineNo, QString("Property '%1' is not found in item '%2'").
												arg(propName, itemLabel));
					return ParseResult::Error;
				}
			}
		}

		// search in other LMs
		//
		for(Hash mlHash : m_mlNotFoundIn)
		{
			auto it = mlCompilers.find(mlHash);

			if (it == mlCompilers.end())
			{
				Q_ASSERT(false);
				continue;
			}

			Builder::ModuleLogicCompilerShared mlCompiler = it->second;

			TEST_PTR_CONTINUE(mlCompiler);

			const auto [itemFound, paramFound, paramValue] = mlCompiler->getUalAfbParamValue(itemLabel, propName);

			if (itemFound)
			{
				m_mlFoundIn.insert(mlHash);
				m_mlNotFoundIn.erase(mlHash);

				//

				if (paramFound)
				{
					*propValue = paramValue;
					return ParseResult::Ok;
				}
				else
				{
					m_log.logError(lineNo, QString("property '%1' is not found in item '%2'").
										   arg(propName, itemLabel));
					return ParseResult::Error;
				}
			}
		}

		m_log.logError(lineNo, QString("item label '%1' is not found").
									   arg(itemLabel));

		return ParseResult::Error;		// prop value is not found
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
				plr->setError(QString("unknown setting '%1'").arg(settingID));
				return false;
			}

			plr->setting = st;

			return parseSettingValue(st, settingValueStr, plr);
		}

		// check addressSignalID token like:  address -> SignalID

		qsizetype pointerSignIndex = toParse.indexOf(LEFT_POINTER_SIGN);

		if (pointerSignIndex != -1)
		{
			plr->lineType = LineType::AddressSignalID;

			QString addrStr = toParse.mid(0, pointerSignIndex).trimmed();
			QString signalID = toParse.mid(pointerSignIndex + LEFT_POINTER_SIGN.length()).trimmed();

			if (addrStr.isEmpty() == true ||
				signalID.isEmpty() == true)
			{
				plr->setError(ERR_SYNTAX);
				return false;
			}

			if (signalID.contains(m_anyWhitespaceSymbol) == true)
			{
				plr->setError("signal identifier should not contain any whitespace symbols");
				return false;
			}

			plr->addressStr = addrStr;
			plr->value = QVariant(signalID);

			return true;
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

	QStringList Parser::knownGatewayTypes() const
	{
		QStringList kgt = ::E::enumKeyStrings<E::GatewayType>();

		kgt.remove(0);

		return kgt;
	}

	E::GatewayType Parser::getGatewayType(const QString& gwTypeStr) const
	{
		bool ok = false;

		E::GatewayType gwType = ::E::stringToValue<E::GatewayType>(gwTypeStr, &ok);

		if (ok == false)
		{
			gwType = E::GatewayType::Unknown;
		}

		return gwType;
	}

}
