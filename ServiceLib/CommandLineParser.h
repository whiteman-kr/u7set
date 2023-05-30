#pragma once

#include <QString>
#include <QStringList>
#include <QSettings>
#include <memory>
#include "../CommonLib/OrderedHash.h"
#include "../UtilsLib/WUtils.h"

class CircularLogger;

class CommandLineParser : public QObject
{
	Q_OBJECT

public:
	CommandLineParser();
	CommandLineParser(const QStringList& argv);

	void setCmdLineArgs(const QStringList& argv);
	void setCmdLineArgs(int argc, const char** argv);

	qsizetype cmdLineArgCount() const;

	// cmdLineArgName should be specified without "-"
	// setting is a cmdLineArg value stored in registry

	bool addSimpleNoWritableCmdLineArg(	const QString& cmdLineArgName,
										const QString& description);

	bool addSimpleCmdLineArg(const QString& cmdLineArgName,
							 const QString& settingName,
							 const QString& description);

	bool addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
									  const QString& description,
									  const QString& paramExample);

	bool addValueCmdLineArg(const QString& cmdLineArgName,
							const QString& settingName,
							const QString& description,
							const QString& paramExample);
	void parse();
	const QStringList& parsingErrors() { return m_parsingErrors; }

	void writeSettingsToRegistry(QSettings& settings, std::shared_ptr<CircularLogger> log);

	static bool checkSettingWriteStatus(QSettings& settings, const QString& settingName,
										std::shared_ptr<CircularLogger> logger);

	bool cmdLineArgIsSet(const QString& cmdLineArgName) const;			// use with all option types
//	QString optionValue(const QString& optionName) const;					// use only with OptionType::SingleValue

	QString settingValue(const QString& settingName) const;
//	bool settingIsSet(const QString& settingName) const;

	QString helpText() const;

private:
	enum CmdLineArgType
	{
		Simple,				//	-a, if not specified asumed as "false"
		SingleValue,		//	-a=value, if not specified assumed as ""
	};

	struct CmdLineArg
	{
		CmdLineArgType type = CmdLineArgType::Simple;
		QString name;							// option name in command line, ex: id
		bool saveInRegistry = false;
		QString settingName;					// respectively setting name in registry, ex: EquipmentID
		QString description;
		QString paramsExample;

		bool isSetFromCmdLine = false;
		QString valueStr;

		int order = -1;
	};

	bool addCmdLineArg(CmdLineArgType type,
				   QString optionName,
				   bool saveInRegistry,
				   const QString& settingName,
				   const QString& description,
				   const QString& paramsExample);

	const CmdLineArg* getOption(const QString& optionName) const;

private:
	QString m_appPath;
	QStringList m_argv;

	std::map<QString, CmdLineArg> m_cmdLineArgs;			// cmd line arg Name => CmdLineArg
	QStringList m_parsingErrors;

	bool m_parsed = false;
};
