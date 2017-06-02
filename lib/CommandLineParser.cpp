#include "CircularLogger.h"
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

	for(Option& op : m_options)
	{
		op.isSet = false;
		op.values.clear();
	}

	m_cmdLineArgsIsSet = true;
	m_parsed = false;
}


int CommandLineParser::argCount() const
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
	assert(m_cmdLineArgsIsSet == true);

	QVector<QString> cmdLineArgs = m_cmdLineArgs;	// make copy

	for(int len = m_maxOptionLen; len >= MIN_OPTION_LEN; len--)
	{
		for(Option& op : m_options)
		{
			if (op.name.length() != len)
			{
				continue;
			}

			for(int i = 0; i < cmdLineArgs.count(); i++)
			{
				const QString cmdLineArg = cmdLineArgs[i];

				if (cmdLineArg.mid(0, len) != op.name)
				{
					continue;
				}

				switch(op.type)
				{
				case OptionType::Simple:
					op.isSet = true;
					break;

				case OptionType::SingleValue:
					{
						op.isSet = true;

						int pos = cmdLineArg.indexOf("=");

						if (pos != -1)
						{
							op.values.append(cmdLineArg.mid(pos + 1));

							if (op.settingsNames.count() > 0)
							{
								assert(op.settingsNames.count() == 1);

								m_settingsValues.insert(op.settingsNames.first(), op.values.first());
							}
						}
					}
					break;

				case OptionType::MultipleValues:
					{
						op.isSet = true;

						int pos = cmdLineArg.indexOf("=");

						if (pos != -1)
						{
							QString value = cmdLineArg.mid(pos + 1);

							op.values = value.split(",", QString::KeepEmptyParts);

							int valueIndex = 0;
							int settingsCount = op.settingsNames.count();

							for(QString opValue : op.values)
							{
								if (valueIndex >= settingsCount)
								{
									break;
								}

								m_settingsValues.insert(op.settingsNames.at(valueIndex), opValue);

								valueIndex++;
							}
						}
					}
					break;

				default:
					assert(false);
				}

				cmdLineArgs.removeAt(i);			// Argument is processed. Remove it from list.
				break;
			}
		}
	}

	m_parsed = true;
}

void CommandLineParser::processSettings(QSettings& settings, std::shared_ptr<CircularLogger> log)
{
	assert(m_parsed == true);

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
	assert(m_parsed == true);

	if (m_options.contains(optionName) == false)
	{
//		assert(false);				// option isn't defined
		return false;
	}

	return m_options.value(optionName).isSet;
}


QString CommandLineParser::optionValue(const QString& optionName) const
{
	assert(m_parsed == true);

	if (m_options.contains(optionName) == false)
	{
//		assert(false);				// option isn't defined
		return QString("");
	}

	Option op = m_options.value(optionName);

	assert(op.type == OptionType::SingleValue);

	return op.values.first();
}

QStringList CommandLineParser::optionValues(const QString& optionName) const
{
	assert(m_parsed == true);

	if (m_options.contains(optionName) == false)
	{
//		assert(false);				// option isn't defined
		return QStringList();
	}

	Option op = m_options.value(optionName);

	assert(op.type == OptionType::MultipleValues);

	return op.values;
}

QString CommandLineParser::settingValue(const QString& settingName) const
{
	assert(m_parsed == true);

	if (m_settingsValues.contains(settingName) == false)
	{
		return QString();
	}

	QString settingValue = m_settingsValues.value(settingName, QString());

	return settingValue;
}

QString CommandLineParser::helpText() const
{
	QStringList opStrs;

	int opMaxLen = 0;

	for(const Option& op : m_options)
	{
		switch(op.type)
		{
		case OptionType::Simple:
			opStrs << op.name;
			break;

		case OptionType::SingleValue:
			if (op.paramsExample.isEmpty() == true)
			{
				opStrs << (op.name + "=value");
			}
			else
			{
				opStrs << (op.name + "=" + op.paramsExample);
			}
			break;

		case OptionType::MultipleValues:
			if (op.paramsExample.isEmpty() == true)
			{
				opStrs << (op.name + "=value1,...,valueN");
			}
			else
			{
				opStrs << (op.name + "=" + op.paramsExample);
			}
			break;

		default:
			assert(false);
		}

		int len = opStrs.last().length();

		if (len > opMaxLen)
		{
			opMaxLen = len;
		}
	}

	opMaxLen += 4;			// indent between option and description

	QString helpText;

	helpText = QString("\nUse: %1 [options]\n\nOptions:\n").arg(m_appPath);

	int index = 0;

	for(QString opStr : opStrs)
	{
		opStr = QString("    ") + opStr.leftJustified(opMaxLen, ' ');

		opStr += m_options[index].description;

		helpText += opStr + "\n";

		index++;
	}

	helpText += "\n";

	return helpText;
}


bool CommandLineParser::addOption(OptionType type,
								  const QString& name,
								  const QStringList& settingsNames,
								  const QString& description, const QString& paramsExample)
{
	if (name.isEmpty() == true)
	{
		assert(false);				// option name can't be empty
		return false;
	}

	if (m_options.contains(name))
	{
		assert(false);				// option with same name already exists
		return false;
	}

	Option op;

	op.type = type;
	op.name = QString("-") + name;
	op.settingsNames = settingsNames;
	op.description = description;
	op.paramsExample = paramsExample;

	m_options.insert(name, op);

	int optionLen = op.name.length();

	if (optionLen > m_maxOptionLen)
	{
		m_maxOptionLen = optionLen;
	}

	return true;
}
