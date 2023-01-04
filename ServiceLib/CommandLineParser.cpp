#ifndef SERVICE_LIB_DOMAIN
#error Don't include this file in the project! Link ServiceLib instead.
#endif

#include "../OnlineLib/CircularLogger.h"
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
		assert(false);		// argc must be >= 1
		return;
	}

	if (argv == nullptr)
	{
		assert(false);
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

		op.isSet = false;
		op.values.clear();
	}

	m_cmdLineArgsIsSet = true;
	m_parsed = false;
}

qsizetype CommandLineParser::argCount() const
{
	return m_cmdLineArgs.count() + 1;	// 1 == application path arg
}

bool CommandLineParser::addSimpleOption(const QString& optionName, const QString& description)
{
	return addOption(OptionType::Simple, optionName, QStringList(), description, QString(""));
}

bool CommandLineParser::addSingleValueOption(const QString& optionName,
											 const QString& settingName,
											 const QString& description,
											 const QString& paramExample)
{
	QStringList settingsNames;

	settingsNames.append(settingName);

	return addOption(OptionType::SingleValue, optionName, settingsNames, description, paramExample);
}

bool CommandLineParser::addMultipleValuesOption(const QString& optionName,
												const QStringList& settingsNames,
												const QString& description,
												const QString& paramsExample)
{
	return addOption(OptionType::MultipleValues, optionName, settingsNames, description, paramsExample);
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
			op.isSet = true;
			break;

		case OptionType::SingleValue:
			{
				op.isSet = true;

				QString cmdLineArgValue;

				if (cmdLineArgAndValue.size() > 1)
				{
					cmdLineArgValue = cmdLineArgAndValue[1];
				}

				op.values.append(cmdLineArgValue);

				if (op.settingsNames.count() > 0)
				{
					Q_ASSERT(op.settingsNames.count() == 1);

					m_settingsValues.insert(op.settingsNames.first(), cmdLineArgValue);
				}
			}
			break;

		case OptionType::MultipleValues:
			{
				op.isSet = true;

				QString cmdLineArgValue;

				if (cmdLineArgAndValue.size() > 1)
				{
					cmdLineArgValue = cmdLineArgAndValue[1];
				}

				op.values = cmdLineArgValue.split(",", Qt::SkipEmptyParts);

				int valueIndex = 0;
				int settingsCount = static_cast<int>(op.settingsNames.count());

				for(QString& opValue : op.values)
				{
					opValue = opValue.trimmed();

					if (valueIndex < settingsCount)
					{
						m_settingsValues.insert(op.settingsNames[valueIndex], opValue);
					}

					valueIndex++;
				}
			}
			break;

		default:
			assert(false);
		}
	}

	m_parsed = true;
}

void CommandLineParser::processSettings(QSettings& settings, std::shared_ptr<CircularLogger> log)
{
	Q_ASSERT(m_parsed == true);

	QList<QString> settingNames = m_settingsValues.keys();

	for(const QString& settingName : settingNames)
	{
		if (settingName.isEmpty() == true)
		{
			assert(false);
			continue;
		}

		QString settingValue = m_settingsValues.value(settingName, QString());

		settings.setValue(settingName, QVariant(settingValue));

		settings.sync();

		checkSettingWriteStatus(settings, settingName, log);
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
		assert(false);		// wtf?
	}

	return false;
}

bool CommandLineParser::optionIsSet(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	auto it = m_options.find(optionName);

	if (it == m_options.end())
	{
		return false;
	}

	return it->second.isSet;
}

QString CommandLineParser::optionValue(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	std::optional<Option> oop = getOption(optionName);

	if (oop.has_value() == false)
	{
		return QString("");
	}

	const Option& op = oop.value();

	Q_ASSERT(op.type == OptionType::SingleValue);

	return op.values.first();
}

QStringList CommandLineParser::optionValues(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	std::optional<Option> oop = getOption(optionName);

	if (oop.has_value() == false)
	{
		return QStringList();
	}

	const Option& op = oop.value();

	Q_ASSERT(op.type == OptionType::MultipleValues);

	return op.values;
}

QString CommandLineParser::settingValue(const QString& settingName) const
{
	Q_ASSERT(m_parsed == true);

	if (m_settingsValues.contains(settingName) == false)
	{
		return QString();
	}

	QString settingValue = m_settingsValues.value(settingName, QString());

	return settingValue;
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
				opStr = op.name;
				break;

			case OptionType::SingleValue:
				if (op.paramsExample.isEmpty() == true)
				{
					opStr = op.name + "=value";
				}
				else
				{
					opStr = op.name + "=" + op.paramsExample;
				}
				break;

			case OptionType::MultipleValues:
				if (op.paramsExample.isEmpty() == true)
				{
					opStr = op.name + "=value1,...,valueN";
				}
				else
				{
					opStr =  op.name + "=" + op.paramsExample;
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
								  QString name,
								  const QStringList& settingsNames,
								  const QString& description,
								  const QString& paramsExample)
{
	name = name.trimmed().toLower();

	if (name.isEmpty() == true)
	{
		Q_ASSERT(false);			// option name can't be empty
		return false;
	}

	if (m_options.contains(name))
	{
		Q_ASSERT(false);			// option with same name already exists
		return false;
	}

	Option op;

	op.type = type;
	op.name = QString("-") + name;
	op.settingsNames = settingsNames;
	op.description = description;
	op.paramsExample = paramsExample;

	op.order = static_cast<int>(m_options.size());

	m_options.insert({name, op});

	return true;
}

std::optional<CommandLineParser::Option> CommandLineParser::getOption(const QString& optionName) const
{
	std::optional<Option> result;

	Q_ASSERT(m_parsed == true);

	auto it = m_options.find(optionName);

	if (it == m_options.end())
	{
		return result;
	}

	result = it->second;

	return result;
}

