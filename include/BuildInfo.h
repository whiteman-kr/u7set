#pragma once

#include <QString>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <cassert>

namespace Builder
{

	struct BuildInfo
	{
		QString project;
		int id = -1;
		bool release = false;
		QDateTime date;
		int changeset = 0;
		QString user;
		QString workstation;

		QString typeStr() const { return release ? "release" : "debug"; }
		QString dateStr() const { return date.toString("dd.MM.yyyy hh:mm:ss"); }

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		void readFromXml(QXmlStreamReader& xmlReader);
	};

	struct BuildFileInfo
	{
		QString pathFileName;		// path and file name from build root directory, like "/subdir/filename.xml"
		qint64 size = 0;			// size of file
		QString md5;				// MD5 hash of file

		void writeToXml(QXmlStreamWriter& xmlWriter) const;
		void readFromXml(QXmlStreamReader& xmlReader);
	};

}
