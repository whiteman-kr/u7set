#include "../lib/JsonSerializable.h"


JsonSerializable::JsonSerializable(int version) :
	m_jsonVersion(version)
{
}


void JsonSerializable::writeToJson(QByteArray& json) const
{
	QJsonObject jsonObject;

	jsonObject.insert("jsonVersion", m_jsonVersion);

	// call "toJson" serialization of derived class
	//
	toJson(jsonObject);

	json = QJsonDocument(jsonObject).toJson();
}


bool JsonSerializable::readFromJson(const QByteArray& json)
{
	QJsonDocument jsonDocument;
	QJsonParseError error;

	jsonDocument = QJsonDocument::fromJson(json, &error);

	if (error.error != QJsonParseError::ParseError::NoError)
	{
		assert(false);

		qDebug() << QString("JSON parse error: %1").arg(error.errorString());

		return false;
	}

	if (jsonDocument.isEmpty() == true ||
		jsonDocument.isNull() == true ||
		jsonDocument.isObject() == false)
	{
		qDebug() << "JSON document loading error";

		return false;
	}

	QJsonObject jsonObject = jsonDocument.object();

	int version = 0;

	JSON_READ_INT(jsonObject, "jsonVersion", version);

	if (m_jsonVersion < version)
	{
		// can't read JSON-serializable object with version grate then current version
		//
		assert(false);
		return false;
	}

	// call "fromJson" serialization of derived class
	//
	return fromJson(jsonObject, version);
}
