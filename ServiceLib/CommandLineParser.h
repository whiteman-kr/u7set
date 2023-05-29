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
	enum OptionType
	{
		Simple,				//	-a
		SingleValue,		//	-a=value
	};

public:
	CommandLineParser();
	CommandLineParser(int argc, char** argv);

	void setCmdLineArgs(int argc, char** argv);

	qsizetype argCount() const;

	// option is a command line argument like -id
	// optionName should be specified without "-"
	//
	// setting is an option value stored in registry

	bool addSimpleNoWritableOption(const QString& optionName,
									const QString& description);

	bool addSimpleOption(const QString& optionName,
						 const QString& settingName,
						 const QString& description);

	bool addValueNoWritebleOption(const QString& optionName,
								  const QString& description,
								  const QString& paramExample);

	bool addValueOption(const QString& optionName,
						const QString& settingName,
						const QString& description,
						const QString& paramExample);
	void parse();
	const QStringList& parsingErrors() { return m_parsingErrors; }

	void writeSettingsToRegistry(QSettings& settings, std::shared_ptr<CircularLogger> log);

	static bool checkSettingWriteStatus(QSettings& settings, const QString& settingName,
										std::shared_ptr<CircularLogger> logger);

	bool optionIsSetFromCmdLine(const QString& optionName) const;			// use with all option types
	QString optionValue(const QString& optionName) const;					// use only with OptionType::SingleValue

	QString settingValue(const QString& settingName) const;
	bool settingIsSet(const QString& settingName) const;

	QString helpText() const;

	static OptionalBool strToBool(QString str);

private:
	struct Option
	{
		OptionType type = OptionType::Simple;
		QString name;							// option name in command line, ex: id
		bool saveInRegistry = false;
		QString settingName;					// respectively setting name in registry, ex: EquipmentID
		QString description;
		QString paramsExample;

		bool isSetFromCmdLine = false;
		QVariant value;

		int order = -1;
	};

	bool addOption(OptionType type,
				   QString optionName,
				   bool saveInRegistry,
				   const QString& settingName,
				   const QString& description,
				   const QString& paramsExample);

	const Option* getOption(const QString& optionName) const;

private:
	QString m_appPath;
	QVector<QString> m_cmdLineArgs;

	std::map<QString, Option> m_options;			// opName => Option
	QStringList m_parsingErrors;

	bool m_parsed = false;
	bool m_cmdLineArgsIsSet = false;
};
