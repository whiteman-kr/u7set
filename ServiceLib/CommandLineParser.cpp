#ifndef SERVICE_LIB_DOMAIN
#error Don't include this file in the project! Link ServiceLib instead.
#endif

#include "../OnlineLib/CircularLogger.h"
#include "../Lib/ConstStrings.h"
#include "CommandLineParser.h"

CommandLineParser::CommandLineParser()
{
}

CommandLineParser::CommandLineParser(const QStringList& argv)
{
	setCmdLineArgs(argv);
}

void CommandLineParser::setCmdLineArgs(const QStringList& argv)
{
	if (argv.size() < 1)
	{
		Q_ASSERT(false);		// argc must be >= 1
		return;
	}

	m_argv = argv;
	m_appPath = m_argv[0];

	for(auto& p : m_cmdLineArgs)
	{
		CmdLineArg& op = p.second;

		op.isSetFromCmdLine = false;
		op.valueStr.clear();
	}

	m_parsed = false;
}

void CommandLineParser::setCmdLineArgs(int argc, const char** argv)
{
	Q_ASSERT(argc >= 1);
	TEST_PTR_RETURN(argv);

	QStringList cmdLineArgs;

	for(int i = 0; i < argc; i++)
	{
		TEST_PTR_CONTINUE(argv[i]);

		cmdLineArgs.append(QString(argv[i]).trimmed());
	}

	setCmdLineArgs(cmdLineArgs);
}

qsizetype CommandLineParser::cmdLineArgCount() const
{
	return m_argv.count();
}

bool CommandLineParser::addSimpleNoWritableCmdLineArg(const QString& cmdLineArgName,
												 const QString& description)
{
	return addCmdLineArg(CmdLineArgType::Simple, cmdLineArgName, false, QString(), description, QString());
}

bool CommandLineParser::addSimpleCmdLineArg(const QString& cmdLineArgName,
										const QString& settingName,
										const QString& description)
{
	return addCmdLineArg(CmdLineArgType::Simple, cmdLineArgName, true, settingName, description, QString());
}

bool CommandLineParser::addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
										const QString& description,
										const QString& paramExample)
{
	return addCmdLineArg(CmdLineArgType::SingleValue, cmdLineArgName, false, QString(), description, paramExample);
}

bool CommandLineParser::addValueCmdLineArg(const QString& cmdLineArgName,
										const QString& settingName,
										const QString& description,
										const QString& paramExample)
{
	return addCmdLineArg(CmdLineArgType::SingleValue, cmdLineArgName, true, settingName, description, paramExample);
}

void CommandLineParser::parse()
{
	Q_ASSERT(m_argv.size() >= 1);

	m_parsingErrors.clear();

	bool first = true;

	for(const QString& cmdLineArg : m_argv)
	{
		if (first == true)
		{
			first = false;
			continue;
		}

		QStringList cmdLineArgAndValue = cmdLineArg.split("=", Qt::SkipEmptyParts);

		if (cmdLineArgAndValue.isEmpty() == true)
		{
			continue;
		}

		QString argName = cmdLineArgAndValue.first().trimmed().toLower();

		while(argName.isEmpty() == false && argName[0].isLetterOrNumber() == false)
		{
			argName.remove(0, 1);
		}

		auto it = m_cmdLineArgs.find(argName);

		if (it == m_cmdLineArgs.end())
		{
			m_parsingErrors.append(QString("Unknown command line argument: %1").arg(cmdLineArg));
			continue;
		}

		CmdLineArg& arg = it->second;

		switch(arg.type)
		{
		case CmdLineArgType::Simple:
			arg.isSetFromCmdLine = true;
			arg.valueStr = boolToString(true);
			break;

		case CmdLineArgType::SingleValue:
			{
				arg.isSetFromCmdLine = true;

				QString cmdLineArgValue;

				if (cmdLineArgAndValue.size() > 1)
				{
					cmdLineArgValue = cmdLineArgAndValue[1];
				}

				arg.valueStr = cmdLineArgValue;
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

	for(const auto& p : m_cmdLineArgs)
	{
		const CmdLineArg& arg = p.second;

		if (arg.settingName == SoftwareSetting::EQUIPMENT_ID)
		{
			DEBUG_STOP;
		}

		if (arg.saveInRegistry == false ||
			arg.isSetFromCmdLine == false)
		{
			continue;
		}

		if (arg.settingName.isEmpty() == true)
		{
			Q_ASSERT(false);
			continue;
		}

		settings.setValue(arg.settingName, QVariant(arg.valueStr));
		qDebug() << settings.value(SoftwareSetting::EQUIPMENT_ID);
	}

	qDebug() << settings.value(SoftwareSetting::EQUIPMENT_ID);

	settings.sync();
	checkSettingWriteStatus(settings, QString(), log);

	qDebug() << settings.value(SoftwareSetting::EQUIPMENT_ID);
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

bool CommandLineParser::cmdLineArgIsSet(const QString& cmdLineArgName) const
{
	Q_ASSERT(m_parsed == true);

	QString name = cmdLineArgName;

	auto it = m_cmdLineArgs.find(name.trimmed().toLower());

	if (it == m_cmdLineArgs.end())
	{
		Q_ASSERT(false);		// unknown cmdLineArg name
		return false;
	}

	return it->second.isSetFromCmdLine;
}

/*QString CommandLineParser::optionValue(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	const Option* op = getOption(optionName);

	if (op == nullptr)
	{
		Q_ASSERT(false);				// unknown optionName
		return QString("");
	}

	if (op->type != CmdLineArgType::SingleValue)
	{
		Q_ASSERT(false);				// wrong option type
		return QString("");
	}

	return op->value.toString();
}*/

QString CommandLineParser::settingValue(const QString& settingName) const
{
	Q_ASSERT(m_parsed == true);

	for(const auto& p : m_cmdLineArgs)
	{
		const CmdLineArg& arg = p.second;

		if (arg.settingName != settingName)
		{
			continue;
		}

		return arg.valueStr;
	}

	Q_ASSERT(false);				// setting not found

	return QString();
}

/*
bool CommandLineParser::settingIsSet(const QString& settingName) const
{
	Q_ASSERT(m_parsed == true);

	for(const auto& p : m_options)
	{
		const Option& op = p.second;

		if (op.settingName != settingName)
		{
			continue;
		}

		if (op.type != CmdLineArgType::Simple)
		{
			Q_ASSERT(false);				// wrong option type
			return false;
		}

		return op.value.toBool();
	}

	Q_ASSERT(false);				// setting not found

	return false;
}*/

QString CommandLineParser::helpText() const
{
	std::list<std::pair<QString, QString>> opStrs;

	int opCount = static_cast<int>(m_cmdLineArgs.size());

	int opMaxLen = 0;

	for(int i = 0; i < opCount; i++)
	{
		for(const auto& p : m_cmdLineArgs)
		{
			const CmdLineArg& op = p.second;

			if (op.order != i)
			{
				continue;
			}

			QString opStr;

			switch(op.type)
			{
			case CmdLineArgType::Simple:
				opStr = Separator::MINUS + op.name;
				break;

			case CmdLineArgType::SingleValue:
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

bool CommandLineParser::addCmdLineArg(CmdLineArgType type,
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

	if (m_cmdLineArgs.contains(optionName))
	{
		Q_ASSERT(false);			// option with same name already exists
		return false;
	}

	CmdLineArg op;

	op.type = type;
	op.name = optionName;
	op.saveInRegistry = saveToRegistry;
	op.settingName = settingName.isEmpty() == true ? optionName : settingName;
	op.description = description;
	op.paramsExample = paramsExample;

	switch(op.type)
	{
	case CmdLineArgType::Simple:
		op.valueStr = boolToString(false);
		break;

	case CmdLineArgType::SingleValue:
		op.valueStr = QString();
		break;

	default:
		Q_ASSERT(false);
	}

	op.order = static_cast<int>(m_cmdLineArgs.size());

	m_cmdLineArgs.insert({optionName, op});

	return true;
}

const CommandLineParser::CmdLineArg* CommandLineParser::getOption(const QString& optionName) const
{
	Q_ASSERT(m_parsed == true);

	auto it = m_cmdLineArgs.find(optionName);

	if (it == m_cmdLineArgs.end())
	{
		return nullptr;
	}

	return &it->second;
}

