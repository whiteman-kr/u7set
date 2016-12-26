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

	m_parsed = false;
}


bool CommandLineParser::addSimpleOption(const QString& name, const QString& description)
{
	return addOption(OptionType::Simple, name, description);
}


bool CommandLineParser::addSingleValueOption(const QString& name, const QString& description)
{
	return addOption(OptionType::SingleValue, name, description);
}


bool CommandLineParser::addMultipleValuesOption(const QString& name, const QString& description)
{
	return addOption(OptionType::MultipleValues, name, description);
}


bool CommandLineParser::addHelpOption()
{
	return addSimpleOption("h", "Print this help.");
}


void CommandLineParser::parse()
{
	QVector<QString> cmdLineArgs = m_cmdLineArgs;	// make copy

	for(int len = m_maxOptionLen; len >= MIN_OPTION_LEN; len--)
	{
		for(Option op : m_options)
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


bool CommandLineParser::isOptionSet(const QString& name)
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return false;
	}

	return m_options.value(name).isSet;
}


QString CommandLineParser::optionValue(const QString& name)
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return QString("");
	}

	return m_options.value(name).values.first();
}


QStringList CommandLineParser::optionValues(const QString& name)
{
	assert(m_parsed == true);

	if (m_options.contains(name) == false)
	{
		assert(false);				// option isn't defined
		return QStringList();
	}

	return m_options.value(name).values;
}


QString CommandLineParser::helpText()
{
	QString ht;

	ht = QString("Use: %1 [options]\n\nOptions:\n").arg(m_appPath);

	for(const Option& op : m_options)
	{
		QString opStr;

		switch(op.type)
		{
		case OptionType::Simple:
			opStr = QString("%1\t\t\t").arg(op.name);
			break;

		case OptionType::SingleValue:
			opStr = QString("%1=value\t\t").arg(op.name);
			break;

		case OptionType::MultipleValues:
			opStr = QString("%1=value1,valueN\t").arg(op.name);
			break;

		default:
			assert(false);
		}

		ht += QString("\t%1%2\n").arg(opStr).arg(op.description);
	}

	return ht;
}


bool CommandLineParser::addOption(OptionType type, const QString& name, const QString& description)
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

	m_options.insert(name, op);

	int optionLen = op.name.length();

	if (optionLen > m_maxOptionLen)
	{
		m_maxOptionLen = optionLen;
	}

	return true;
}
