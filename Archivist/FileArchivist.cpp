#include <QDir>
#include <QStorageInfo>
#include <CommonLib/Times.h>
#include "FileArchivist.h"
#include "../UtilsLib/WUtils.h"
#include "../ArchivingService/ArchFilePartition.h"

FileArchivist::FileArchivist(const RequestParams& rp) :
	Archivist(rp)
{
}

FileArchivist::~FileArchivist()
{
}

bool FileArchivist::copyArchive()
{
	printRequestParams();

	// convert request times to UTC

	m_reqParams.begin = m_reqParams.begin.toUTC();
	m_reqParams.end = m_reqParams.end.toUTC();

	//

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
	m_destStorageInfo.setPath(m_reqParams.destLocation);

	if (m_destStorageInfo.isValid() == false)
	{
		print << "Wrong destination location!\n";
		return false;
	}

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
			Q_ASSERT((fi.size() % ArchFileRecord::SIZE) == 0);

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

			CopyFileInfo cfi;

			cfi.pathFileName = path + QString("/%1").arg(fileName);
			cfi.fileName = fileName;
			cfi.appSignalID = appSignalID;
			cfi.groupID = groupID;

			//

			cfi.startPos = 0;
			cfi.endPos = fi.size();

			if (fileDate > reqBeginDate && fileDate < reqEndDate)
			{
				cfi.copyEntireFile = true;
			}
			else
			{
				cfi.startPos = findBeginPos(cfi.pathFileName, m_reqParams.begin);

				if (cfi.startPos == -1)
				{
					DEBUG_STOP;
					continue;
				}

				if (fileDate == reqEndDate)
				{
					cfi.endPos = findEndPos(cfi.pathFileName, m_reqParams.end);

					if (cfi.endPos == -1)
					{
						DEBUG_STOP;
						continue;
					}
				}

				cfi.copyEntireFile = (cfi.startPos == 0) && (cfi.endPos == fi.size());
			}

			m_copyFileInfos.emplace_back(cfi);

			m_expectedSize += ROUND_TO(cfi.endPos - cfi.startPos, m_destStorageInfo.blockSize());
		}
	}

	print << QString("\rArchive scan: 100%      \n\n");

	print << QString("Expected size to copy: %1\n\n").arg(sizeStr(m_expectedSize));

	m_archInfo.Clear();

	return true;
}

bool FileArchivist::checkRequiredSpace()
{
	if (m_destStorageInfo.bytesAvailable() < m_expectedSize)
	{
		print << "No space available in destination location\n\n";
		return false;
	}

	print << QString("Space available in destination location: %1\n\n").arg(sizeStr(m_destStorageInfo.bytesAvailable()));

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

	//

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

		QString	path = m_destArchivePath + QString("/%1/%2").arg(hexFolder(cfi.groupID)).arg(cfi.appSignalID.mid(1));

		path = dr.toNativeSeparators(path);

		if (dr.exists(path) == false)
		{
			bool res = dr.mkpath(path);

			if (res == false)
			{
				print << QString("Error create folder: %1").arg(path);
				m_copiedCount++;
				continue;
			}
		}

//		QString	path3 = m_destFile + QString(":%1-%2").arg(hexFolder(cfi.groupID)).arg(cfi.appSignalID.mid(1));

		//			print << QString("Folder created: %1\n").arg(path3);

		QString toPath = path + QString("/%1").arg(cfi.fileName);

		toPath = dr.toNativeSeparators(toPath);

		if (QFile::exists(toPath))
		{
			QFile::remove(toPath);
		}

		if (cfi.copyEntireFile)
		{
			bool res = QFile::copy(cfi.pathFileName, toPath);

			if (res == false)
			{
				DEBUG_STOP;
			}

/*			std::filesystem::path from(cfi.fileName.toStdString());
			std::filesystem::path to(toPath.toStdString());

			std::filesystem::copy(from, to, err);

			if (err.value() != 0)
			{
				DEBUG_STOP;
			}*/

/*			bool res = copyFile(cfi.fileName, toPath);

			//			qDebug() << toPath;

			if (res == false)
			{
				DEBUG_STOP;
			}*/
		}
		else
		{
			copyFile(cfi.pathFileName, toPath, cfi.startPos, cfi.endPos);
		}

		m_copiedCount++;

	} while(true);
}

bool FileArchivist::copyFile(const QString& from, const QString& to, qint64 startPos, qint64 endPos)
{
	Q_ASSERT((startPos % ArchFileRecord::SIZE) == 0);
	Q_ASSERT((endPos % ArchFileRecord::SIZE) == 0);
	Q_ASSERT(startPos < endPos);

	QFile fromFile(from);
	QFile toFile(to);

	if (fromFile.open(QIODeviceBase::ReadOnly) == false ||
		toFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate) == false)
	{
		return false;
	}

	if (startPos > 0)
	{
		fromFile.seek(startPos);
	}

	qint64 copySize = endPos - startPos;

	Q_ASSERT((copySize % ArchFileRecord::SIZE) == 0);

	qint64 sizeToRead = 0;

	do
	{
		if (copySize >= BUF_SIZE)
		{
			sizeToRead = BUF_SIZE;
			copySize -= BUF_SIZE;
		}
		else
		{
			sizeToRead = copySize;
			copySize = 0;
		}

		qint64 readSize = fromFile.read(m_buffer, sizeToRead);

		Q_ASSERT((readSize % ArchFileRecord::SIZE) == 0);

		toFile.write(m_buffer, readSize);
	}
	while(copySize);

	return true;
}


qint64 FileArchivist::findBeginPos(const QString& pathFileName, QDateTime beginDate)
{
	ArchFilePartition afp;

	bool result = afp.openForReading(pathFileName);

	if (result == false)
	{
		return -1;
	}

	beginDate.setTimeZone(TIME_ZONE_UTC);

	TimeStamp ts(beginDate);

	qint64 position = 0;

	ArchFindResult res = afp.binarySearch(E::TimeType::System,  ts.timeStamp, &position);

	if (res != ArchFindResult::Found)
	{
		return -1;
	}

	if (position > ArchFileRecord::SIZE)
	{
		position -= ArchFileRecord::SIZE;
	}

	Q_ASSERT((position % ArchFileRecord::SIZE) == 0);

	return position;
}

qint64 FileArchivist::findEndPos(const QString& pathFileName, QDateTime endDate)
{
	ArchFilePartition afp;

	bool result = afp.openForReading(pathFileName);

	if (result == false)
	{
		return -1;
	}

	endDate.setTimeZone(TIME_ZONE_UTC);

	TimeStamp ts(endDate);

	qint64 position = 0;

	ArchFindResult res = afp.binarySearch(E::TimeType::System,  ts.timeStamp, &position);

	if (res != ArchFindResult::Found)
	{
		return -1;
	}

	if (position + ArchFileRecord::SIZE <= afp.size())
	{
		position += ArchFileRecord::SIZE;
	}

	Q_ASSERT((position % ArchFileRecord::SIZE) == 0);

	return position;
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


