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


bool CommandLineParser::addSimpleOption(const QString& name, const QString& description)
{
	return addOption(OptionType::Simple, name, description, QString(""));
}


bool CommandLineParser::addSingleValueOption(const QString& name, const QString& description, const QString& paramExample)
{
	return addOption(OptionType::SingleValue, name, description, paramExample);
}


bool CommandLineParser::addMultipleValuesOption(const QString& name, const QString& description, const QString& paramsExample)
{
	return addOption(OptionType::MultipleValues, name, description, paramsExample);
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


bool CommandLineParser::optionIsSet(const QString& name) const
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return false;
	}

	return m_options.value(name).isSet;
}


QString CommandLineParser::optionValue(const QString& name) const
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return QString("");
	}

	Option op = m_options.value(name);

	assert(op.type == OptionType::SingleValue);

	return op.values.first();
}


QStringList CommandLineParser::optionValues(const QString& name) const
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return QStringList();
	}

	Option op = m_options.value(name);

	assert(op.type == OptionType::MultipleValues);

	return op.values;
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


bool CommandLineParser::addOption(OptionType type, const QString& name, const QString& description, const QString& paramsExample)
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
