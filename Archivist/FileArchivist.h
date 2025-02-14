#pragma once

#include <QMutex>
#include "Archivist.h"
#include <ArchSignal.pb.h>
#include "../ArchivingService/ArchFileRecord.h"

class FileArchivist : public Archivist
{
	struct FileInfo
	{
		QString pathFileName;
		QString fileName;
		QString appSignalID;
		int groupID = 0;

		//

		qint64 startPos = 0;
		qint64 endPos = 0;
		bool copyEntireFile = true;
	};

public:
	FileArchivist(const RequestParams& rp);
	~FileArchivist();

	bool copyArchive() override;

private:
	bool readArchInfoProto();
	bool scanArchive();

	bool checkArchive();
	void checkThreadProc();

	bool checkRequiredSpace();
	bool copyFiles();
	void copyThreadProc();
	bool copyFile(const QString& from, const QString& to, qint64 startPos, qint64 endPos, char* buf, qint64 bufSize);

	void asyncPrintError(const QString& err);
	void asyncPrintError(const QStringList& errs);

	qint64 findBeginPos(const QString& pathFileName, QDateTime beginDate);
	qint64 findEndPos(const QString& pathFileName, QDateTime endDate);
	QString sizeStr(qint64 size);
	QString timeStr(qint64 time);
	QString hexFolder(int g);

private:
	Proto::ArchInfo m_archInfo;
	QString m_project;
	QString m_archServiceID;
	QString m_destArchivePath;

	QStorageInfo m_destStorageInfo;

//	QString m_destFile;

	QMutex m_processingMutex;
	std::vector<FileInfo> m_fileInfos;
	bool m_prevError = false;
	int m_fileInfoIndex = 0;

	std::atomic<int> m_processedCount = 0;

	qint64 m_expectedSize = 0;

	inline static const qint64 BUF_SIZE = 10000 * sizeof(ArchFileRecord);
};
