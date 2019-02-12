#pragma once

#include <QtGlobal>
#include <QString>
#include <QDateTime>
#include <QDir>
#include <iostream>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>

#include "../../ArchivingService/ArchFileRecord.h"

class ArchUtils
{
public:
	ArchUtils(const QString& workDir);

	void dump(const QString& archFile, bool lt, bool st, bool pt);

private:
	bool getFileStartTime(const QString& fileName, QDateTime* dt);

	QString getTimeStr(const QDateTime& pt);
	QString getTimeStr(qint64 ms);

	QString getLocalTimeStr(const ArchFileRecord& ar);
	QString getSystemTimeStr(const ArchFileRecord& ar);
	QString getPlantTimeStr(const ArchFileRecord& ar);
	QString getFlagsStr(const ArchFileRecord& ar);
	QString getValueStr(const ArchFileRecord& ar);

private:
	QString m_workDir;
};
