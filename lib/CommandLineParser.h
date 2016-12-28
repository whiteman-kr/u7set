#pragma once

#include <QString>
#include <QStringList>
#include "../lib/OrderedHash.h"


class CommandLineParser
{
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

	bool addSimpleOption(const QString& name, const QString& description);				// specify name without "-"
	bool addSingleValueOption(const QString& name, const QString& description);			// specify name without "-"
	bool addMultipleValuesOption(const QString& name, const QString& description);		// specify name without "-"

	bool addHelpOption();

	void parse();

	bool isOptionSet(const QString& name);					// use with all option types
	QString optionValue(const QString& name);				// use only with OptionType::SingleValue
	QStringList optionValues(const QString& name);			// use only with OptionType::MultipleValues

	QString helpText();

private:
	struct Option
	{
		OptionType type = OptionType::Simple;
		QString name;
		QString description;

		bool isSet = false;
		QStringList values;
	};

	bool addOption(OptionType type, const QString& name, const QString& description);

private:
	QString m_appPath;
	QVector<QString> m_cmdLineArgs;

	HashedVector<QString, Option> m_options;

	bool m_parsed = false;

	const int MIN_OPTION_LEN = 2;
	int m_maxOptionLen = 0;
};
