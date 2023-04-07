#include "GatewayDescription.h"
#include "../lib/ConstStrings.h"
#include "../CommonLib/Types.h"
#include "../UtilsLib/WUtils.h"

void GatewayDescriptionParser::ParseLineResult::setError(const QString& err)
{
	msgType = MsgType::Error;
	msg = err;
}

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
	{ Setting::Description,			SettingType::String },

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

GatewayDescriptionParser::GatewayDescriptionParser()
{
	m_knownSections = E::enumKeyStrings<Section>();
	m_knownSettings = E::enumKeyStrings<Setting>();

	m_syntaxError.setError(ERR_SYNTAX);
}

bool GatewayDescriptionParser::parse(const QString& desc,
									 std::vector<std::tuple<int, MsgType, QString>> *log)
{
	TEST_PTR_RETURN_FALSE(log);

	log->clear();

	bool result = true;

	QStringList strs = desc.split(Separator::NEW_LINE, Qt::KeepEmptyParts, Qt::CaseInsensitive);

	int lineNo = 0;

	for(const QString& str : strs)
	{
		lineNo++;

		ParseLineResult plr;

		bool res = parseLine(str, &plr);

		result &= res;

		if (plr.msgType != MsgType::Nothing)
		{
			log->push_back({lineNo, plr.msgType, plr.msg});
		}
	}

	return result;
}

bool GatewayDescriptionParser::parseLine(const QString& str, ParseLineResult* plr)
{
	TEST_PTR_RETURN_FALSE(plr);

	bool result = true;

	plr->lineType = LineType::Unknown;

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
			toParse = line;
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
			*plr = m_syntaxError;
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
			*plr = m_syntaxError;
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

		// parse setting value

		result = parseSettingValue(st, settingValueStr, plr);

		return true;
	}

	// check signalID token

	return result;
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

	SettingType settingType = it->second;

	switch(settingType)
	{
	case SettingType::Int:
		{
			bool ok = true;

			int intValue = valueStr.toInt(&ok);

			if (ok == false)
			{
				plr->setError(QString("setting value is not a number"));
				return false;
			}

			plr->value = QVariant(intValue);
		}
		break;

	case SettingType::String:
		plr->value = QVariant(valueStr);
		break;

	case SettingType::Bool:
		break;

	case SettingType::IpPort:
		break;

	case SettingType::Unknown:
	default:
		Q_ASSERT(false);
		plr->setError("unknown setting type");
		return false;
	}

	return true;
}
