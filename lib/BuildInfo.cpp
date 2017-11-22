#include "../lib/BuildInfo.h"


namespace Builder
{

	// --------------------------------------------------------------------------------------
	//
	//	BuildInfo structure implementation
	//
	// --------------------------------------------------------------------------------------

	void BuildInfo::writeToXml(QXmlStreamWriter& xmlWriter) const
	{
		xmlWriter.writeStartElement("BuildInfo");

		xmlWriter.writeAttribute("Project", project);
		xmlWriter.writeAttribute("ID", QString::number(id));
		xmlWriter.writeAttribute("Type", typeStr());
		xmlWriter.writeAttribute("Date", dateStr());
		xmlWriter.writeAttribute("Changeset", QString::number(changeset));
		xmlWriter.writeAttribute("User", user);
		xmlWriter.writeAttribute("Workstation", workstation);

		xmlWriter.writeEndElement();			// build
	}


	void BuildInfo::readFromXml(QXmlStreamReader& xmlReader)
	{
		if (xmlReader.name() != "BuildInfo")
		{
			assert(false);
			return;
		}

		project = xmlReader.attributes().value("Project").toString();
		id = xmlReader.attributes().value("ID").toInt();

		release = xmlReader.attributes().value("Type").toString() == "release" ? true : false;

		QString dateTimeStr = xmlReader.attributes().value("Date").toString();
		date = QDateTime::fromString(dateTimeStr, "dd.MM.yyyy hh:mm:ss");

		changeset = xmlReader.attributes().value("Changeset").toInt();

		user = xmlReader.attributes().value("User").toString();

		workstation = xmlReader.attributes().value("Workstation").toString();
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
		if (xmlReader.name() != "File")
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

		while (!(xmlReader.tokenType() == QXmlStreamReader::EndElement && xmlReader.name() == "File"))
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
}
