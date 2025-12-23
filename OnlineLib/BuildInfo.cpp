#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "BuildInfo.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/XmlHelper.h"
#include <CommonLib/ConstStrings.h>
#include <BuildInfo.pb.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace OnlineLib
{
	// --------------------------------------------------------------------------------------
	//
	//	BuildInfo structure implementation
	//
	// --------------------------------------------------------------------------------------

	const QString BuildInfo::dateTimeFormatStr("dd.MM.yyyy hh:mm:ss");

	void BuildInfo::clear()
	{
		project.clear();
		buildNo = -1;
		dateTime = QDateTime();
		changeset = 0;
		user.clear();
		workstation.clear();
	}

	QString BuildInfo::dateTimeStr() const
	{
		return dateTime.toString(dateTimeFormatStr);
	}

	// void BuildInfo::setDateTime(const std::string& dateTimeStdString)
	// {
	// 	dateTime = QDateTime::fromString(QString::fromStdString(dateTimeStdString), dateTimeFormatStr);
	// }

	void BuildInfo::writeToXml(QXmlStreamWriter& xmlWriter) const
	{
		xmlWriter.writeStartElement(XmlElement::BUILD_INFO);

		xmlWriter.writeAttribute(XmlAttribute::PROJECT, project);
		xmlWriter.writeAttribute(XmlAttribute::ID, QString::number(buildNo));
		xmlWriter.writeAttribute(XmlAttribute::DATE, dateTimeStr());
		xmlWriter.writeAttribute(XmlAttribute::CHANGESET, QString::number(changeset));
		xmlWriter.writeAttribute(XmlAttribute::USER, user);
		xmlWriter.writeAttribute(XmlAttribute::WORKSTATION, workstation);

		xmlWriter.writeEndElement();			// BuildInfo
	}

	bool BuildInfo::readFromXml(XmlReadHelper& xmlReader, bool findElement)
	{
		bool res = true;

		if (findElement == true)
		{
			res = xmlReader.findElement(XmlElement::BUILD_INFO);
		}
		else
		{
			res = xmlReader.name() == XmlElement::BUILD_INFO;
		}

		RETURN_IF_FALSE(res);

		res &= xmlReader.readStringAttribute(XmlAttribute::PROJECT, &project);
		res &= xmlReader.readIntAttribute(XmlAttribute::ID, &buildNo);

		QString dateTimeStr;

		res &= xmlReader.readStringAttribute(XmlAttribute::DATE, &dateTimeStr);
		dateTime = QDateTime::fromString(dateTimeStr, dateTimeFormatStr);

		res &= xmlReader.readIntAttribute(XmlAttribute::CHANGESET, &changeset);
		res &= xmlReader.readStringAttribute(XmlAttribute::USER, &user);
		res &= xmlReader.readStringAttribute(XmlAttribute::WORKSTATION, &workstation);

		return res;
	}

	void BuildInfo::saveToProto(Proto::BuildInfo* proto) const
	{
		TEST_PTR_RETURN(proto);

		proto->set_project(project.toStdString());
		proto->set_buildno(buildNo);
		proto->set_datetime(dateTimeStr().toStdString());
		proto->set_changeset(changeset);
		proto->set_user(user.toStdString());
		proto->set_workstation(workstation.toStdString());
	}

	void BuildInfo::loadFromProto(const Proto::BuildInfo& proto)
	{
		project = QString::fromStdString(proto.project());
		buildNo = proto.buildno();
		dateTime = QDateTime::fromString(QString::fromStdString(proto.datetime()), dateTimeFormatStr);
		changeset = proto.changeset();
		user = QString::fromStdString(proto.user());
		workstation = QString::fromStdString(proto.workstation());
	}

	// --------------------------------------------------------------------------------------
	//
	//	BuildFileInfo structure implementation
	//
	// --------------------------------------------------------------------------------------

	void BuildFileInfo::writeToXml(QXmlStreamWriter& xmlWriter) const
	{
		xmlWriter.writeStartElement(XmlElement::FILE);

		xmlWriter.writeAttribute(XmlAttribute::NAME, pathFileName);
		xmlWriter.writeAttribute(XmlAttribute::ID, ID);
		xmlWriter.writeAttribute(XmlAttribute::TAG, tag);
		xmlWriter.writeAttribute(XmlAttribute::COMPRESSED, boolToString(compressed));
		xmlWriter.writeAttribute(XmlAttribute::SIZE, QString::number(size));
		xmlWriter.writeAttribute(XmlAttribute::MD5, md5);

		for(auto const& [key, str] : metadata)
		{
			xmlWriter.writeTextElement(key, str);
		}

		xmlWriter.writeEndElement();		// File
	}

	bool BuildFileInfo::readFromXml(XmlReadHelper& xmlReader, bool findElement)
	{
		bool res = true;

		if (findElement == true)
		{
			res = xmlReader.findElement(XmlElement::FILE);
		}
		else
		{
			res = xmlReader.name() == XmlElement::FILE;
		}

		RETURN_IF_FALSE(res);

		res &= xmlReader.readStringAttribute(XmlAttribute::NAME, &pathFileName);
		res &= xmlReader.readStringAttribute(XmlAttribute::ID, &ID);
		res &= xmlReader.readStringAttribute(XmlAttribute::TAG, &tag);
		res &= xmlReader.readBoolAttribute(XmlAttribute::COMPRESSED, &compressed);
		res &= xmlReader.readInt64Attribute(XmlAttribute::SIZE, &size);
		res &= xmlReader.readStringAttribute(XmlAttribute::MD5, &md5);

		RETURN_IF_FALSE(res);

		metadata.clear();

		xmlReader.readNext();

		while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == XmlElement::FILE))
		{
			if (xmlReader.tokenType() == QXmlStreamReader::StartElement)
			{
				QString key = xmlReader.name();

				QString value;

				xmlReader.readStringElement(key, &value, false);

				metadata.emplace(key, value);

				xmlReader.readNext();
			}

			xmlReader.readNext();
		}

		return res;
	}

	QString BuildFileInfo::getMetadata(const QString& key) const
	{
		return getValueOrDefault(metadata, key, QString());
	}

	bool BuildFileInfo::isConfigurationXml() const
	{
		return ID == CfgFileId::CONFIGURATION_XML;
	}

	void BuildFileInfo::clear()
	{
		pathFileName.clear();
		tag.clear();
		ID.clear();
		size = 0;
		compressed = false;
		md5.clear();
		metadata.clear();
	}
}
