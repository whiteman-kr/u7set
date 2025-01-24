#include <QDir>
#include <QStorageInfo>
#include "FileArchivist.h"
#include "../UtilsLib/WUtils.h"

FileArchivist::FileArchivist(const RequestParams& rp) :
	Archivist(rp)
{
}

bool FileArchivist::copyArchive()
{
	printRequestParams();

	bool result = true;

	result = readArchInfoProto();

	RETURN_IF_FALSE(result);

	result = scanArchive();

	RETURN_IF_FALSE(result);

	result = checkRequiredSpace();

	RETURN_IF_FALSE(result);

	result = copyFiles();

	return true;
}

bool FileArchivist::readArchInfoProto()
{
	QString fileName = m_reqParams.archiveLocation + "/ArchInfo.proto";

	QFile f(fileName);

	if (f.open(QIODevice::ReadOnly) == false)
	{
		print << QString("Read file error: %1\n").arg(fileName);
		return false;
	}

	QByteArray data = f.readAll();

	bool result = m_archInfo.ParseFromArray(data, data.size());

	if (result == false)
	{
		print << QString("File %1 parsing error!\n").arg(fileName);
		return false;
	}

	print << QString("File %1 parsed Ok\n").arg(fileName);

	return true;
}

bool FileArchivist::scanArchive()
{
	m_project = QString::fromStdString(m_archInfo.buildinfo().project());
	m_archServiceID = QString::fromStdString(m_archInfo.archiveserviceid());

	int signalCount = m_archInfo.archsignal_size();

	QChar ZERO('0');

	QDir dr;

	dr.setNameFilters(QStringList() << "*.sta" << "*.lta");

	QDate reqBeginDate = m_reqParams.begin.date();
	QDate reqEndDate = m_reqParams.end.date();

	m_copyFileInfos.reserve(signalCount);

	m_expectedSize = 0;

	print.newLine();

	for(int i = 0; i < signalCount; i++)
	{
		print << QString("\rArchive scan: %1%").arg((double(i) / signalCount * 100.0), 4, 'f', 1);

		const Proto::ArchSignal& archSignal = m_archInfo.archsignal(i);

		QString appSignalID = QString::fromStdString(archSignal.appsignalid());

		Hash h = calcHash(appSignalID);

		int groupID = static_cast<int>(h & 0xFF);

		QString path = m_reqParams.archiveLocation +
					   QString("/%1/%2").arg(hexFolder(groupID)).arg(appSignalID.mid(1));

		path = dr.toNativeSeparators(path);

		dr.setPath(path);

		if (dr.exists() == false)
		{
			continue;
		}

		QFileInfoList fileInfos = dr.entryInfoList(QDir::Files, QDir::Name);

		for(const QFileInfo& fi : fileInfos)
		{
			QString fileName = fi.fileName();

			bool ok = true;

			int year = fileName.mid(0, 4).toInt(&ok);

			CONTINUE_IF_FALSE(ok);

			int month = fileName.mid(5, 2).toInt(&ok);

			CONTINUE_IF_FALSE(ok);

			int day = fileName.mid(8, 2).toInt(&ok);

			CONTINUE_IF_FALSE(ok);

			QDate fileDate(year, month, day);

			if (fileDate < reqBeginDate ||
				fileDate > reqEndDate)
			{
				continue;
			}

			CopyFileInfo& cfi = m_copyFileInfos.emplace_back(CopyFileInfo{});

			cfi.fileName = path + QString("/%1").arg(fileName);
			cfi.shortFileName = fileName;
			cfi.appSignalID = appSignalID;
			cfi.groupID = groupID;

			if (fileDate == reqBeginDate)
			{
				cfi.startPos = 0;	//	findBeginPos(cfi.fileName, m_reqParams.begin);
				cfi.endPos = fi.size();
				cfi.fullFile = true;
			}
			else
			{
				if (fileDate == reqEndDate)
				{
					cfi.startPos = 0;
					cfi.endPos = fi.size();	//	findEndPos(cfi.fileName, m_reqParams.end);
					cfi.fullFile = true;
				}
				else
				{
					cfi.startPos = 0;
					cfi.endPos = fi.size();
					cfi.fullFile = true;
				}
			}

			m_expectedSize += cfi.endPos - cfi.startPos;
		}
	}

	print << QString("\rArchive scan: 100%      \n\n");

	print << QString("Expected size to copy: %1\n\n").arg(sizeStr(m_expectedSize));

	m_archInfo.Clear();

	return true;
}

bool FileArchivist::checkRequiredSpace()
{
	QStorageInfo si(m_reqParams.destLocation);

	if (si.bytesAvailable() < m_expectedSize)
	{
		print << "No space available in destionation location\n\n";
		return false;
	}

	print << QString("Space available in destionation location: %1\n\n").arg(sizeStr(si.bytesAvailable()));

	print << "Copy archive? (y/n) ";

	char v;

	std::cin >> v;

	if (v != 'y' && v != 'Y')
	{
		print.newLine();
		return false;
	}

	return true;
}

bool FileArchivist::copyFiles()
{
	QDir dr;

	m_destArchivePath = m_reqParams.destLocation + QString("/%1-archive/%2").
											  arg(m_project.toLower()).arg(m_archServiceID);

	m_destArchivePath = dr.toNativeSeparators(m_destArchivePath);

	dr.setPath(m_destArchivePath);
	dr.removeRecursively();

	bool res = dr.mkpath(m_destArchivePath);

	if (res == false)
	{
		print << QString("Error create folder: %1").arg(m_destArchivePath);
		return false;
	}

//	print << QString("Folder created: %1\n").arg(path);

	//

	const QChar ZERO('0');

	for(int i = 0; i < 256; i++)
	{
		QString path2 = m_destArchivePath + QString("/%1").arg(hexFolder(i));

		path2 = dr.toNativeSeparators(path2);

		res = dr.mkpath(path2);

		if (res == false)
		{
			print << QString("Error create folder: %1").arg(path2);
			return false;
		}

		//print << QString("Folder created: %1\n").arg(path2);
	}

	QElapsedTimer timer;

	timer.start();

	int copyCount = TO_INT(m_copyFileInfos.size());

	QString prevAppSignalID;
	QString path3;

	print.newLine();

	m_copyInfoIndex = 0;

	std::thread t1(&FileArchivist::copyThreadProc, this);
	std::thread t2(&FileArchivist::copyThreadProc, this);
	std::thread t3(&FileArchivist::copyThreadProc, this);

	while(m_copiedCount < m_copyFileInfos.size())
	{
		print << QString("\rCopied: %1%  ").arg((double(m_copiedCount) / copyCount * 100.0), 4, 'f', 2);
		QThread::msleep(250);
	}

	t1.join();
	t2.join();
	t3.join();

/*	for(int i = 0; i < copyCount; i++)
	{
		const CopyFileInfo& cfi = m_copyFileInfos[i];

		if (prevAppSignalID != cfi.appSignalID)
		{
			path3 = m_destArchivePath + QString("/%1/%2").arg(hexFolder(cfi.groupID)).arg(cfi.appSignalID.mid(1));

			path3 = dr.toNativeSeparators(path3);

			res = dr.mkpath(path3);

			if (res == false)
			{
				print << QString("Error create folder: %1").arg(path3);
				return false;
			}

//			print << QString("Folder created: %1\n").arg(path3);

			prevAppSignalID = cfi.appSignalID;
		}

		if (cfi.fullFile)
		{
			//std::filesystem::path from(cfi.fileName.toStdString());

			QString toPath = path3 + QString("/%1").arg(cfi.shortFileName);

			toPath = dr.toNativeSeparators(toPath);

			//std::filesystem::path to(toPath.toStdString());

			if (QFile::exists(toPath))
			{
				QFile::remove(toPath);
			}

			bool res = QFile::copy(cfi.fileName, toPath);

//			qDebug() << toPath;

			if (res == false)
			{
				DEBUG_STOP;
			}
		}

		print << QString("\rCopied: %1%  ").arg((double(i) / copyCount * 100.0), 4, 'f', 2);
	}*/

	print << QString("\rCopied: 100%    \n\n");

	//

	QString roFileName = m_destArchivePath + Separator::DIR + File::READONLY;

	QFile f(roFileName);

	if (f.open(QIODevice::Truncate | QIODevice::WriteOnly) == false)
	{
		print << QString("Error write file: %1\n\n").arg(roFileName);
	}

	char c = 0;
	f.write(&c, 1);

	f.close();

	print << QString("Archive copied to %1\n\n").arg(m_destArchivePath);

	print << QString("Time elapsed: %1\n\n").arg(timeStr(timer.elapsed()));

	return true;
}

void FileArchivist::copyThreadProc()
{
	QDir dr;

	do
	{
		m_copyMutex.lock();

		if (m_copyInfoIndex >= m_copyFileInfos.size())
		{
			m_copyMutex.unlock();
			break;
		}

		CopyFileInfo cfi = m_copyFileInfos[m_copyInfoIndex];

		m_copyInfoIndex++;

		m_copyMutex.unlock();

		QString	path3 = m_destArchivePath + QString("/%1/%2").arg(hexFolder(cfi.groupID)).arg(cfi.appSignalID.mid(1));

		path3 = dr.toNativeSeparators(path3);

		if (dr.exists(path3) == false)
		{
			bool res = dr.mkpath(path3);

			if (res == false)
			{
				print << QString("Error create folder: %1").arg(path3);
				m_copiedCount++;
				continue;
			}
		}

		//			print << QString("Folder created: %1\n").arg(path3);

		if (cfi.fullFile)
		{
			//std::filesystem::path from(cfi.fileName.toStdString());

			QString toPath = path3 + QString("/%1").arg(cfi.shortFileName);

			toPath = dr.toNativeSeparators(toPath);

			//std::filesystem::path to(toPath.toStdString());

			if (QFile::exists(toPath))
			{
				QFile::remove(toPath);
			}

			bool res = QFile::copy(cfi.fileName, toPath);

			//			qDebug() << toPath;

			if (res == false)
			{
				DEBUG_STOP;
			}
		}

		m_copiedCount++;

	} while(true);
}

qint64 FileArchivist::findBeginPos(const QString& fileName, const QDateTime& beginDate)
{
	return 0;
}

qint64 FileArchivist::findEndPos(const QString& fileName, const QDateTime& endDate)
{
	return 0;
}

QString FileArchivist::sizeStr(qint64 size)
{
	size /= 1024;

	if (size < 1024)
	{
		return QString("%1 Kb").arg(size);
	}

	size /= 1024;

	if (size < 1024)
	{
		return QString("%1 Mb").arg(size);
	}

	return QString("%1 Gb").arg((size / 1024.0), 4, 'f', 2);
}

QString FileArchivist::timeStr(qint64 time)
{
	if (time < 1000)
	{
		return QString("%1 ms").arg(time);
	}

	time /= 1000;

	if (time < 60)
	{
		return QString("%1 sec").arg(time);
	}

	if (time < 60 * 60)
	{
		return QString("%1 min %2 sec").arg(time / 60).arg(time %60);
	}

	return QString("%1 h %2 min %3 sec").
					arg(time / (60 * 60)).
					arg((time % (60 * 60)) / 60).
					arg((time % (60 * 60)) % 60);
}

QString FileArchivist::hexFolder(int g)
{
	return QString("%1").arg(g, 2, 16, QChar('0')).toUpper();
}


