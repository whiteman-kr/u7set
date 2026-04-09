#pragma once

#include <QString>
#include <QDateTime>
#include <cassert>

class QXmlStreamReader;
class QXmlStreamWriter;
class XmlReadHelper;

namespace Proto
{
	class BuildInfo;
}

namespace OnlineLib
{
	struct BuildInfo
	{
	private:
		static const QString dateTimeFormatStr;

	public:
		QString project;
		int buildNo = -1;
		QDateTime dateTime;
		int changeset = 0;
		QString user;
		QString workstation;

		void clear();

		QString dateTimeStr() const;
//		void setDateTime(const std::string& dateTimeStdString);

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		bool readFromXml(XmlReadHelper& xmlReader, bool findElement = true);

		void saveToProto(Proto::BuildInfo* proto) const;
		void loadFromProto(const Proto::BuildInfo& proto);
	};

	struct BuildFileInfo
	{
		QString pathFileName;					// path and file name from build root directory, like "/subdir/filename.xml"
		QString tag;							// file tag
		QString ID;								// file ID
		qint64 size = 0;						// size of file
		bool compressed = false;
		QString md5;							// MD5 hash of file
		std::map<QString, QString> metadata;	// metadata (pairs of strings)

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		bool readFromXml(XmlReadHelper& xmlReader, bool findElement = true);

		QString getMetadata(const QString& key) const;

		bool isConfigurationXml() const;

		void clear();
	};
}

using BuildFileInfoArray = std::vector<OnlineLib::BuildFileInfo>;
