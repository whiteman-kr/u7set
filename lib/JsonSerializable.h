#pragma once

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <cassert>


const char* const JSON_DATE_TIME_FORMAT_STR = "dd.MM.yyy HH:mm:ss.zzz";


#define JSON_TEST_VALUE(jsonValue, valueName) \
	if (jsonValue.isUndefined()) \
	{ \
		qDebug() << QString("JSON parse error: undefined value '%1'").arg(valueName); \
		assert(false); \
		return false; \
	}


#define	JSON_READ_INT(jsonObject, valueName, variable) \
	{  \
		QJsonValue jsonValue = jsonObject.value(valueName); \
		JSON_TEST_VALUE(jsonValue, valueName) \
		variable = 	jsonValue.toInt(); \
	}


#define	JSON_READ_BOOL(jsonObject, valueName, variable) \
	{  \
		QJsonValue jsonValue = jsonObject.value(valueName); \
		JSON_TEST_VALUE(jsonValue, valueName) \
		variable = 	jsonValue.toBool(); \
	}


#define	JSON_READ_STRING(jsonObject, valueName, variable) \
	{  \
		QJsonValue jsonValue = jsonObject.value(valueName); \
		JSON_TEST_VALUE(jsonValue, valueName) \
		variable = 	jsonValue.toString(); \
	}


#define	JSON_READ_DATETIME(jsonObject, valueName, variable) \
	{  \
		QJsonValue jsonValue = jsonObject.value(valueName); \
		JSON_TEST_VALUE(jsonValue, valueName) \
		variable = 	QDateTime::fromString(jsonValue.toString(), JSON_DATE_TIME_FORMAT_STR); \
	}


#define	JSON_WRITE_DATETIME(jsonObject, valueName, variable) \
	jsonObject.insert(valueName, variable.toString(JSON_DATE_TIME_FORMAT_STR)); \


//
// Base class for JSON-serializable classes
//

class JsonSerializable
{
private:
	int m_jsonVersion = 1;

protected:
	virtual void toJson(QJsonObject& jsonObject) const = 0;
	virtual bool fromJson(const QJsonObject& jsonObject, int version) = 0;

public:
	JsonSerializable() {}
	JsonSerializable(int version);

	int jsonVersion() { return m_jsonVersion; }

	void writeToJson(QByteArray& json) const;
	bool readFromJson(const QByteArray& json);
};


