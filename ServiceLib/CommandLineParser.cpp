#ifndef SERVICE_LIB_DOMAIN
#error Don't include this file in the project! Link ServiceLib instead.
#endif

#include "../OnlineLib/CircularLogger.h"
#include "../Lib/ConstStrings.h"
#include "CommandLineParser.h"

CommandLineParser::CommandLineParser()
{
}

CommandLineParser::CommandLineParser(int argc, char** argv)
{
	setCmdLineArgs(argc, argv);
}

void CommandLineParser::setCmdLineArgs(int argc, char** argv)
{
	m_cmdLineArgsIsSet = false;

	if (argc < 1)
	{
		Q_ASSERT(false);		// argc must be >= 1
		return;
	}

	if (argv == nullptr)
	{
		Q_ASSERT(false);
		return;
	}

	m_cmdLineArgs.clear();
	m_appPath.clear();

	for(int i = 0; i < argc; i++)
	{
		QString arg(argv[i]);

		arg = arg.trimmed();

		if (i == 0)
		{
			m_appPath = arg;
			continue;
		}

		m_cmdLineArgs.append(arg);
	}

	for(auto& p : m_options)
	{
		Option& op = p.second;

		op.isSetFromCmdLine = false;
		op.value.clear();
	}

	m_cmdLineArgsIsSet = true;
	m_parsed = false;
}

qsizetype CommandLineParser::argCount() const
{
	return m_cmdLineArgs.count() + 1;	// 1 == application path arg
}


bool CommandLineParser::addSimpleNoWritableOption(const QString& optionName,
												 const QString& description)
{
	return addOption(OptionType::Simple, optionName, false, QString(), description, QString());
}

bool CommandLineParser::addSimpleOption(const QString& optionName,
										const QString& settingName,
										const QString& description)
{
	return addOption(OptionType::Simple, optionName, true, settingName, description, QString());
}

bool CommandLineParser::addValueNoWritebleOption(const QString& optionName,
										const QString& description,
										const QString& paramExample)
{
	return addOption(OptionType::SingleValue, optionName, false, QString(), description, paramExample);
}

bool CommandLineParser::addValueOption(const QString& optionName,
										const QString& settingName,
										const QString& description,
										const QString& paramExample)
{
	return addOption(OptionType::SingleValue, optionName, true, settingName, description, paramExample);
}

void CommandLineParser::parse()
{
	Q_ASSERT(m_cmdLineArgsIsSet == true);

	m_parsingErrors.clear();

	for(QString cmdLineArg : m_cmdLineArgs)
	{
		QStringList cmdLineArgAndValue = cmdLineArg.split("=", Qt::SkipEmptyParts);

		if (cmdLineArgAndValue.isEmpty() == true)
		{
			continue;
		}

		QString optionName = cmdLineArgAndValue.first().trimmed().toLower();

		while(optionName.isEmpty() == false && optionName[0].isLetterOrNumber() == false)
		{
			optionName.remove(0, 1);
		}

		auto it = m_options.find(optionName);

		if (it == m_options.end())
		{
			m_parsingErrors.append(QString("Unknown command line argument: %1").arg(cmdLineArg));
			continue;
		}

		Option& op = it->second;

		switch(op.type)
		{
		case OptionType::Simple:
			op.isSetFromCmdLine = true;
			break;

		case OptionType::SingleValue:
			{
				op.isSetFromCmdLine = true;

				QString cmdLineArgValue;

				if (cmdLineArgAndValue.size() > 1)
				{
					cmdLineArgValue = cmdLineArgAndValue[1];
				}

				op.value = cmdLineArgValue;

//				m_settingsValues.insert(op.settingName, cmdLineArgValue);
			}
			break;

		default:
			Q_ASSERT(false);
		}
	}

	m_parsed = true;
}

void CommandLineParser::writeSettingsToRegistry(QSettings& settings, std::shared_ptr<CircularLogger> log)
{
	Q_ASSERT(m_parsed == true);


	for(const auto& p : m_options)
	{
		const Option& op = p.second;

		if (op.saveInRegistry == false ||
			op.isSetFromCmdLine == false)
		{
			continue;
		}

		if (op.settingName.isEmpty() == true)
		{
			Q_ASSERT(false);
			continue;
		}

		settings.setValue(op.settingName, QVariant(op.value));
		settings.sync();
		checkSettingWriteStatus(settings, op.settingName, log);
	}
}

bool CommandLineParser::checkSettingWriteStatus(QSettings& settings, const QString& settingName, std::shared_ptr<CircularLogger> logger)
{
	QSettings::Status s = settings.status();

	if (s == QSettings::Status::NoError)
	{
		return true;
	}

	if (logger == nullptr)
	{
		return false;
	}

	switch(s)
	{
	case QSettings::Status::AccessError:
		if (settingName.isEmpty() == true)
		{
			DEBUG_LOG_ERR(logger, QString(tr("Settings write error: QSettings::Status::AccessError.")))
		}
		else
		{
			DEBUG_LOG_ERR(logger, QString(tr("Setting '%1' write error: QSettings::Status::AccessError.")).arg(settingName))
		}
		break;

	case QSettings::Status::FormatError:
		if (settingName.isEmpty() == true)
		{
			DEBUG_LOG_ERR(logger, QString(tr("Settings write error: QSettings::Status::FormatError.")))
		}
		else
		{
			DEBUG_LOG_ERR(logger, QString(tr("Setting '%1' write error: QSettings::Status::FormatError.")).arg(settingName))
		}
		break;

	default:
		Q_ASSERT(false);		// wtf?
	}

	return false;
}

bool CommandLineParser::optionIsSetFromCmdLine(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	auto it = m_options.find(optionName.trimmed().toLower());

	if (it == m_options.end())
	{
		Q_ASSERT(false);		// unknown option name
		return false;
	}

	return it->second.isSetFromCmdLine;
}

QString CommandLineParser::optionValue(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	const Option* op = getOption(optionName);

	if (op == nullptr)
	{
		Q_ASSERT(false);				// unknown optionName
		return QString("");
	}

	if (op->type != OptionType::SingleValue)
	{
		Q_ASSERT(false);				// wrong option type
		return QString("");
	}

	return op->value.toString();
}

QString CommandLineParser::settingValue(const QString& settingName) const
{
	Q_ASSERT(m_parsed == true);

	for(const auto& p : m_options)
	{
		const Option& op = p.second;

		if (op.settingName != settingName)
		{
			continue;
		}

		if (op.type != OptionType::SingleValue)
		{
			Q_ASSERT(false);				// wrong option type
			return QString("");
		}

		return op.value.toString();
	}

	Q_ASSERT(false);				// setting not found

	return QString();
}

QString CommandLineParser::helpText() const
{
	std::list<std::pair<QString, QString>> opStrs;

	int opCount = static_cast<int>(m_options.size());

	int opMaxLen = 0;

	for(int i = 0; i < opCount; i++)
	{
		for(const auto& p : m_options)
		{
			const Option& op = p.second;

			if (op.order != i)
			{
				continue;
			}

			QString opStr;

			switch(op.type)
			{
			case OptionType::Simple:
				opStr = Separator::MINUS + op.name;
				break;

			case OptionType::SingleValue:
				if (op.paramsExample.isEmpty() == true)
				{
					opStr = Separator::MINUS + op.name + "=value";
				}
				else
				{
					opStr = Separator::MINUS + op.name + "=" + op.paramsExample;
				}
				break;

			default:
				Q_ASSERT(false);
				continue;
			}

			opStrs.push_back({opStr, op.description});

			int len = static_cast<int>(opStr.length());

			if (len > opMaxLen)
			{
				opMaxLen = len;
			}
		}
	}

	opMaxLen += 4;			// indent between option and description

	QString helpText;

	helpText = QString("\nUse: %1 [options]\n\nOptions:\n").arg(m_appPath);

	int index = 0;

	for(const auto& p : opStrs)
	{
		QString opStr = p.first;

		opStr = QString("    ") + opStr.leftJustified(opMaxLen, ' ');

		opStr += p.second;

		helpText += opStr + "\n";

		index++;
	}

	helpText += "\n";

	return helpText;
}

OptionalBool CommandLineParser::strToBool(QString str)
{
	str = str.trimmed().toLower();

	OptionalBool result;

	if (str == "on" ||
		str == "1" ||
		str == "true" ||
		str == "yes")
	{
		result = true;
	}
	else
	{
		if (str == "off" ||
			str == "0" ||
			str == "false" ||
			str == "no")
		{
			result = false;
		}
	}

	return result;
}

bool CommandLineParser::addOption(OptionType type,
								  QString optionName,
								  bool saveToRegistry,
								  const QString& settingName,
								  const QString& description,
								  const QString& paramsExample)
{
	optionName = optionName.trimmed().toLower();

	if (optionName.isEmpty() == true)
	{
		Q_ASSERT(false);			// option name can't be empty
		return false;
	}

	if (m_options.contains(optionName))
	{
		Q_ASSERT(false);			// option with same name already exists
		return false;
	}

	Option op;

	op.type = type;
	op.name = optionName;
	op.saveInRegistry = saveToRegistry;
	op.settingName = settingName.isEmpty() == true ? optionName : settingName;
	op.description = description;
	op.paramsExample = paramsExample;

	switch(op.type)
	{
	case OptionType::Simple:
		op.value = false;
		break;

	case OptionType::SingleValue:
		op.value = QString();
		break;

	default:
		Q_ASSERT(false);
	}

	op.order = static_cast<int>(m_options.size());

	m_options.insert({optionName, op});

	return true;
}

const CommandLineParser::Option* CommandLineParser::getOption(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	auto it = m_options.find(optionName);

	if (it == m_options.end())
	{
		return nullptr;
	}

	return &it->second;
}

