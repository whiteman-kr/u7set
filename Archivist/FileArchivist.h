#pragma once

#include <QMutex>
#include "Archivist.h"
#include <ArchSignal.pb.h>

class FileArchivist : public Archivist
{
	struct CopyFileInfo
	{
		QString fileName;
		QString shortFileName;
		QString appSignalID;
		int groupID = 0;
		qint64 startPos = 0;
		qint64 endPos = 0;
		bool fullFile = true;
	};

public:
	FileArchivist(const RequestParams& rp);

	bool copyArchive() override;

private:
	bool readArchInfoProto();
	bool scanArchive();
	bool checkRequiredSpace();
	bool copyFiles();
	void copyThreadProc();

	qint64 findBeginPos(const QString& fileName, const QDateTime& beginDate);
	qint64 findEndPos(const QString& fileName, const QDateTime& endDate);
	QString sizeStr(qint64 size);
	QString timeStr(qint64 time);
	QString hexFolder(int g);

private:
	Proto::ArchInfo m_archInfo;
	QString m_project;
	QString m_archServiceID;
	QString m_destArchivePath;

	QMutex m_copyMutex;
	std::vector<CopyFileInfo> m_copyFileInfos;
	int m_copyInfoIndex = 0;

	std::atomic<int> m_copiedCount = 0;

	qint64 m_expectedSize = 0;
};
