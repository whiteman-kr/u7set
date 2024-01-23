#pragma once

#include <QString>
#include <QStringList>
#include <QSettings>
#include <memory>
#include "../UtilsLib/WUtils.h"

class CircularLogger;

class CommandLineParser : public QObject
{
	Q_OBJECT

public:
	CommandLineParser() = delete;
	CommandLineParser(const QString& organization, const QString& serviceName,int argc, char** argv);
	CommandLineParser(const CommandLineParser& clp);

	void setCmdLineArgs(const QStringList& argv);
	void setCmdLineArgs(int argc, char **argv);

	qsizetype cmdLineArgCount() const;

	// cmdLineArgName should be specified without "-"
	// setting is a name of registry key where cmdLineArg value stored

	bool addSimpleNoWritableCmdLineArg(	const QString& cmdLineArgName,
										const QString& description);

	bool addSimpleCmdLineArg(const QString& cmdLineArgName,
							 const QString& settingName,
							 const QString& description);

	bool addBoolNoWritableCmdLineArg(	const QString& cmdLineArgName,
										const QString& description);

	bool addBoolCmdLineArg(const QString& cmdLineArgName,
							 const QString& settingName,
							 const QString& description);

	bool addValueNoWritebleCmdLineArg(const QString& cmdLineArgName,
									  const QString& description,
									  const QString& paramExample);

	bool addValueCmdLineArg(const QString& cmdLineArgName,
							const QString& settingName,
							const QString& description,
							const QString& paramExample);

	void readAndApplySettingsFromRegistry();
	void parseAndApplyCmdLineArgs();
	const QStringList& parsingErrors() { return m_parsingErrors; }

	void writeSettingsToRegistry(std::shared_ptr<CircularLogger> log);
	bool clearSettings();

	static bool checkSettingWriteStatus(QSettings& settings, const QString& settingName,
										std::shared_ptr<CircularLogger> logger);

	bool cmdLineArgIsSet(const QString& cmdLineArgName) const;			// use with all cmd line args types

	QString getSettingValue(const QString& settingName) const;
	QString getCmdLineArgValue(const QString& cmdLineArgName) const;

	QString helpText() const;

	void printCmdLineArgs(std::shared_ptr<CircularLogger> log) const;

private:
	enum CmdLineArgType
	{
		Simple,				//	-a, if not specified asumed as "false"
		Bool,				//  -a=yes (yes/no, on/off, true/false, 1/0)
		SingleValue,		//	-a=value, if not specified assumed as ""
	};

	struct CmdLineArg
	{
		CmdLineArgType type = CmdLineArgType::Simple;
		QString name;							// arg name in command line, ex: id
		bool saveInRegistry = false;
		QString settingName;					// respectively setting name in registry, ex: EquipmentID
		QString description;
		QString paramsExample;

		bool isSetFromCmdLine = false;
		QString valueStr;

		int order = -1;
	};

	bool addCmdLineArg(CmdLineArgType type,
				   QString cmdLineArgName,
				   bool saveInRegistry,
				   const QString& settingName,
				   const QString& description,
				   const QString& paramsExample);

private:
	QString m_organization;
	QString m_serviceName;
	QString m_appPath;
	QStringList m_argv;

	std::map<QString, CmdLineArg> m_cmdLineArgs;			// cmdLineArgName => CmdLineArg
	std::map<QString, QString> m_settingToArgName;			// settingName => cmdLineArgName

	QStringList m_parsingErrors;

	bool m_parsed = false;
};
