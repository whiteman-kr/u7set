#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "BuildInfo.h"
#include "../UtilsLib/WUtils.h"
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

	void BuildInfo::writeToXml(QXmlStreamWriter& xmlWriter) const
	{
		xmlWriter.writeStartElement("BuildInfo");

		xmlWriter.writeAttribute("Project", project);
		xmlWriter.writeAttribute("ID", QString::number(id));
		xmlWriter.writeAttribute("Date", dateStr());
		xmlWriter.writeAttribute("Changeset", QString::number(changeset));
		xmlWriter.writeAttribute("User", user);
		xmlWriter.writeAttribute("Workstation", workstation);

		xmlWriter.writeEndElement();			// build
	}


	void BuildInfo::readFromXml(QXmlStreamReader& xmlReader)
	{
		if (xmlReader.name() != QLatin1String("BuildInfo"))
		{
			assert(false);
			return;
		}

		project = xmlReader.attributes().value("Project").toString();
		id = xmlReader.attributes().value("ID").toInt();

		QString dateTimeStr = xmlReader.attributes().value("Date").toString();
		date = QDateTime::fromString(dateTimeStr, dateTimeFormatStr);

		changeset = xmlReader.attributes().value("Changeset").toInt();

		user = xmlReader.attributes().value("User").toString();

		workstation = xmlReader.attributes().value("Workstation").toString();
	}

	void BuildInfo::saveToProto(Proto::BuildInfo* proto) const
	{
		TEST_PTR_RETURN(proto);

		proto->set_project(project.toStdString());
		proto->set_buildno(id);
		proto->set_datetime(dateStr().toStdString());
		proto->set_changeset(changeset);
		proto->set_user(user.toStdString());
		proto->set_workstation(workstation.toStdString());
	}

	void BuildInfo::loadFromProto(const Proto::BuildInfo& proto)
	{
		project = QString::fromStdString(proto.project());
		id = proto.buildno();
		date = QDateTime::fromString(QString::fromStdString(proto.datetime()), dateTimeFormatStr);
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
		xmlWriter.writeStartElement("File");

		xmlWriter.writeAttribute("Name", pathFileName);
		xmlWriter.writeAttribute("ID", ID);
		xmlWriter.writeAttribute("Tag", tag);
		xmlWriter.writeAttribute("Compressed", compressed == true ? "Yes" : "No" );
		xmlWriter.writeAttribute("Size", QString::number(size));
		xmlWriter.writeAttribute("MD5", md5);

		for(auto it = metadata.begin(); it != metadata.end(); ++it)
		{
			xmlWriter.writeTextElement(it.key(), *it);
		}

		xmlWriter.writeEndElement();		// file
	}


	void BuildFileInfo::readFromXml(QXmlStreamReader& xmlReader)
	{
		if (xmlReader.name() != QLatin1String("File"))
		{
			assert(false);
			return;
		}

		pathFileName = xmlReader.attributes().value("Name").toString();
		ID = xmlReader.attributes().value("ID").toString();
		tag = xmlReader.attributes().value("Tag").toString();

		QString compressedStr = xmlReader.attributes().value("Compressed").toString();
		compressed = compressedStr == "Yes" ? true : false;

		size = xmlReader.attributes().value("Size").toInt();
		md5 = xmlReader.attributes().value("MD5").toString();

		metadata.clear();

		xmlReader.readNext();

		while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == QLatin1String("File")))
		{
			if (xmlReader.tokenType() == QXmlStreamReader::StartElement)
			{
				QString key = xmlReader.name().toString();

				xmlReader.readNext();

				if (xmlReader.tokenType() == QXmlStreamReader::Characters)
				{
					metadata.insert(key, xmlReader.text().toString());
				}
			}

			xmlReader.readNext();
		}
	}


	QString BuildFileInfo::getMetadata(const QString& key) const
	{
		if (metadata.contains(key))
		{
			return metadata[key];
		}

		return QString();
	}

	bool BuildFileInfo::isConfigurationXml() const
	{
		return ID == CfgFileId::CONFIGURATION_XML;
	}
}
