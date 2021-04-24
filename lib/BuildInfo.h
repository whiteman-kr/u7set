#pragma once

#include <QString>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <cassert>

#include "../CommonLib/Types.h"

namespace Builder
{

	struct BuildInfo
	{
		QString project;
		int id = -1;
		QDateTime date;
		int changeset = 0;
		QString user;
		QString workstation;

		QString dateStr() const { return date.toString("dd.MM.yyyy hh:mm:ss"); }

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		void readFromXml(QXmlStreamReader& xmlReader);
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
	};

}
