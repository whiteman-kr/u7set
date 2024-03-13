#pragma once

#include <QString>
#include <QDateTime>
#include <cassert>


class QXmlStreamReader;
class QXmlStreamWriter;


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
		int id = -1;
		QDateTime date;
		int changeset = 0;
		QString user;
		QString workstation;

		QString dateStr() const { return date.toString(dateTimeFormatStr); }

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		void readFromXml(QXmlStreamReader& xmlReader);

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
		QHash<QString, QString> metadata;		// metadata (pairs of strings)

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		void readFromXml(QXmlStreamReader& xmlReader);

		QString getMetadata(const QString& key) const;

		bool isConfigurationXml() const;
	};
}
