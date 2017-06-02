#pragma once

#include <QString>
#include <QStringList>
#include <QSettings>
#include <memory>
#include "../lib/OrderedHash.h"

class CircularLogger;

class CommandLineParser : public QObject
{
	Q_OBJECT

public:
	enum OptionType
	{
		Simple,				//	-a
		SingleValue,		//	-a=value
		MultipleValues		//	-a=value1,value2,valueN
	};

public:
	CommandLineParser();
	CommandLineParser(int argc, char** argv);

	void setCmdLineArgs(int argc, char** argv);

	int argCount() const;

	// optionName should be specified without "-"
	//
	bool addSimpleOption(const QString& optionName,
						 const QString& description);

	bool addSingleValueOption(const QString& optionName,
							  const QString& settingName,
							  const QString& description,
							  const QString& paramExample);

	bool addMultipleValuesOption(const QString& optionName,
								 const QStringList& settingsNames,
								 const QString& description,
								 const QString& paramsExample);

	void parse();

	void processSettings(QSettings& settings, std::shared_ptr<CircularLogger> log);

	static bool checkSettingWriteStatus(QSettings& settings, const QString& settingName, std::shared_ptr<CircularLogger> logger);

	bool optionIsSet(const QString& optionName) const;					// use with all option types
	QString optionValue(const QString& optionName) const;			// use only with OptionType::SingleValue
	QStringList optionValues(const QString& optionName) const;			// use only with OptionType::MultipleValues

	QString settingValue(const QString& settingName) const;

	QString helpText() const;

private:
	struct Option
	{
		OptionType type = OptionType::Simple;
		QString name;
		QStringList settingsNames;
		QString description;
		QString paramsExample;

		bool isSet = false;
		QStringList values;
	};

	bool addOption(OptionType type,
				   const QString& name,
				   const QStringList &settingsNames,
				   const QString& description,
				   const QString& paramsExample);

private:
	QString m_appPath;
	QVector<QString> m_cmdLineArgs;

	HashedVector<QString, Option> m_options;
	QHash<QString, QString> m_settingsValues;

	bool m_parsed = false;
	bool m_cmdLineArgsIsSet = false;

	const int MIN_OPTION_LEN = 2;
	int m_maxOptionLen = 0;
};
