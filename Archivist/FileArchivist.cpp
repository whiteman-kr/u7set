#include <QDir>
#include <QStorageInfo>
#include <QRegularExpression>
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

	if (m_reqParams.checkonly == false)
	{
		result = checkRequiredSpace();

		RETURN_IF_FALSE(result);

		result = copyFiles();
	}
	else
	{
		checkArchive();
	}

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

	QDir dr;

	dr.setNameFilters(QStringList() << "*.sta" << "*.lta");

	QDate reqBeginDate = m_reqParams.begin.date();
	QDate reqEndDate = m_reqParams.end.date();

	m_fileInfos.reserve(signalCount);

	m_expectedSize = 0;

	print.newLine();

	QRegularExpression re;

	QStringList matched;

	for(int i = 0; i < signalCount; i++)
	{
		print << QString("\rArchive scan: %1%").arg((double(i) / signalCount * 100.0), 4, 'f', 1);

		const Proto::ArchSignal& archSignal = m_archInfo.archsignal(i);

		QString appSignalID = QString::fromStdString(archSignal.appsignalid());

		//

		if (m_reqParams.signalsList.isEmpty() == false)
		{
			QString asi = appSignalID;

			asi.replace(QStringLiteral("#"), Separator::EMPTY_STR);

			bool match = false;

			for(const QString& pattern : m_reqParams.signalsList)
			{
				re.setPattern(pattern);
				QRegularExpressionMatch m = re.match(asi);

				if (m.hasMatch() == true)
				{
					match = true;
					matched.append(asi);
					break;
				}
			}

			if (match == false)
			{
				continue;
			}
		}

		//

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

			FileInfo cfi;

			cfi.pathFileName = QDir::toNativeSeparators(path + QString("/%1").arg(fileName));

			// if (cfi.pathFileName != QString("D:\\Archive\\archive_tests-archive\\SYSTEMID_RACK01_WS00_ARCHS\\66\\SYSTEMID_RACK01_FSCC02_MD00_PI_MSECOND\\2025_02_05_00_00.sta"))
			// {
			// 	DEBUG_STOP;
			// 	continue;
			// }

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
					continue;
				}

				if (fileDate == reqEndDate)
				{
					cfi.endPos = findEndPos(cfi.pathFileName, m_reqParams.end);

					if (cfi.endPos == -1)
					{
						cfi.endPos = fi.size();
					}
				}

				cfi.copyEntireFile = (cfi.startPos == 0) && (cfi.endPos == fi.size());
			}

			m_fileInfos.emplace_back(cfi);

			m_expectedSize += ROUND_TO(cfi.endPos - cfi.startPos, m_destStorageInfo.blockSize());
		}
	}

	print << QString("\rArchive scan: 100%      \n\n");

	if (matched.isEmpty() == false)
	{
		print << "Filtered signals:\n\n";

		for(const QString m : matched)
		{
			print << m;
			print.newLine();
		}

		print.newLine();
	}

	if (m_reqParams.checkonly == false)
	{
		print << QString("Expected size to copy: %1\n\n").arg(sizeStr(m_expectedSize));
	}

	m_archInfo.Clear();

	return true;
}

bool FileArchivist::checkArchive()
{
	std::thread t1(&FileArchivist::checkThreadProc, this);
	// std::thread t2(&FileArchivist::checkThreadProc, this);
	// std::thread t3(&FileArchivist::checkThreadProc, this);
	// std::thread t4(&FileArchivist::checkThreadProc, this);

	qint64 copyCount = m_fileInfos.size();

	while(m_processedCount < m_fileInfos.size())
	{
		m_processingMutex.lock();

		if (m_prevError == true)
		{
			print << QString("\n");
		}

		print << QString("\rChecked: %1%  ").arg((double(m_processedCount) / copyCount * 100.0), 4, 'f', 2);

		m_prevError = false;

		m_processingMutex.unlock();

		QThread::msleep(250);
	}

	t1.join();

	print << QString("\rChecked: 100%   ");
	// t2.join();
	// t3.join();
	// t4.join();

	return true;
}

void FileArchivist::checkThreadProc()
{
	QFile file;

	QStringList errs;

	char* buf = new char[BUF_SIZE];
	qint64 checkSize = 0;
	qint64 sizeToRead = 0;
	qint64 readSize = 0;
	qint64 offset = 0;

	qint64 prevPlantTime = -1;
	qint64 prevSystemTime = -1;
	qint64 prevLocalTime = -1;

	do
	{
		errs.clear();

		m_processingMutex.lock();

		if (m_fileInfoIndex >= m_fileInfos.size())
		{
			m_processingMutex.unlock();
			break;
		}

		FileInfo cfi = m_fileInfos[m_fileInfoIndex];

		m_fileInfoIndex++;

		m_processingMutex.unlock();

		// if (cfi.pathFileName != "D:\\Archive\\archive_tests-archive\\SYSTEMID_RACK01_WS00_ARCHS\\66\\SYSTEMID_RACK01_FSCC02_MD00_PI_MSECOND\\2025_02_05_00_00.sta")
		// {
		// 	DEBUG_STOP;
		// 	continue;
		// }

		errs << QString("File: %1").arg(cfi.pathFileName);

		if ((cfi.startPos % ArchFileRecord::SIZE) != 0)
		{
			errs << QString("StartPos %1 is non-multiple to %2").arg(cfi.startPos).arg(ArchFileRecord::SIZE);
			asyncPrintError(errs);
			m_processedCount++;
			continue;
		}

		if ((cfi.endPos % ArchFileRecord::SIZE) != 0)
		{
			errs << QString("EndPos %1 is non-multiple to %2").arg(cfi.endPos).arg(ArchFileRecord::SIZE);
			asyncPrintError(errs);
			m_processedCount++;
			continue;
		}

		QFileInfo fi(cfi.pathFileName);

		if (fi.size() < cfi.endPos)
		{
			errs << QString("EndPos %1 greate then file size %2").arg(cfi.endPos).arg(fi.size());
			asyncPrintError(errs);
			m_processedCount++;
			continue;
		}

		file.setFileName(cfi.pathFileName);

		if (file.open(QIODeviceBase::ReadOnly) == false)
		{
			asyncPrintError(QString("Error open file: %1").arg(cfi.pathFileName));
			m_processedCount++;
			continue;
		}

		if (cfi.startPos != 0)
		{
			file.seek(cfi.startPos);
		}

		checkSize = cfi.endPos - cfi.startPos;
		offset = 0;

		int corruptedCount = 0;

		prevPlantTime = -1;
		prevSystemTime = -1;
		prevLocalTime = -1;

		do
		{
			if (checkSize >= BUF_SIZE)
			{
				sizeToRead = BUF_SIZE;
				checkSize -= BUF_SIZE;
			}
			else
			{
				sizeToRead = checkSize;
				checkSize = 0;
			}

			readSize = file.read(buf, sizeToRead);

			if (readSize != sizeToRead)
			{
				asyncPrintError(QString("Error read file: %1").arg(cfi.pathFileName));
				break;
			}

			//

			for(qint64 checkedSize = 0; checkedSize < readSize;
				 checkedSize += ArchFileRecord::SIZE, offset += ArchFileRecord::SIZE)
			{
				ArchFileRecord* afr = reinterpret_cast<ArchFileRecord*>(buf + checkedSize);

				if (afr->isNotCorrupted() == false)
				{
					errs << QString("Record corrupted at %1").arg(offset);
					corruptedCount++;

					prevPlantTime = -1;
					prevSystemTime = -1;
					prevLocalTime = -1;

					if (corruptedCount >= 10)
					{
						break;
					}
				}
				else
				{
					if (prevSystemTime != -1)
					{
						if (prevPlantTime >= afr->state.plantTime)
						{
							errs << QString("Non-monotonic PlantTime  %1 (previous %2) at %3").
									arg(afr->state.plantTime).arg(prevPlantTime).arg(offset);
						}

						if (prevSystemTime >= afr->state.systemTime)
						{
							errs << QString("Non-monotonic SystemTime %1 (previous %2) at %3").
									arg(afr->state.systemTime).arg(prevSystemTime).arg(offset);
						}

/*						if (prevLocalTime >= afr->state.localTime)
						{
							errs << QString("Non-monotonic LocalTime  %1 (previous %2) at %3").
									arg(afr->state.localTime).arg(prevLocalTime).arg(offset);
						}*/
					}

					prevPlantTime = afr->state.plantTime;
					prevSystemTime = afr->state.systemTime;
					prevLocalTime = afr->state.localTime;
				}
			}
		}
		while(checkSize);

		if (errs.size() > 1)
		{
			asyncPrintError(errs);
		}

		file.close();

		m_processedCount++;
	}
	while(true);

	DELETE_ARRAY_IF_NOT_NULL(buf);
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
	QElapsedTimer timer;

	timer.start();

	//

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

	res = QFile::copy(m_reqParams.archiveLocation + Separator::DIR + File::ARCH_INFO_PROTO,
					  m_destArchivePath + Separator::DIR + File::ARCH_INFO_PROTO);

	if (res == false)
	{
		print << QString("Error copy file: %1").arg(File::ARCH_INFO_PROTO);
		return false;
	}

	res = QFile::copy(m_reqParams.archiveLocation + Separator::DIR + File::ARCHIVE_INFO,
					  m_destArchivePath + Separator::DIR + File::ARCHIVE_INFO);

	if (res == false)
	{
		print << QString("Error copy file: %1").arg(File::ARCHIVE_INFO);
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
	}

	//

	int copyCount = TO_INT(m_fileInfos.size());

	print.newLine();

	m_fileInfoIndex = 0;

	std::thread t1(&FileArchivist::copyThreadProc, this);
	std::thread t2(&FileArchivist::copyThreadProc, this);
	std::thread t3(&FileArchivist::copyThreadProc, this);

	while(m_processedCount < m_fileInfos.size())
	{
		m_processingMutex.lock();

		if (m_prevError == true)
		{
			print << QString("\n");
		}

		print << QString("\rCopied: %1%  ").arg((double(m_processedCount) / copyCount * 100.0), 4, 'f', 2);

		m_prevError = false;

		m_processingMutex.unlock();

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

	char* buf = nullptr;

	do
	{
		m_processingMutex.lock();

		if (m_fileInfoIndex >= m_fileInfos.size())
		{
			m_processingMutex.unlock();
			break;
		}

		FileInfo cfi = m_fileInfos[m_fileInfoIndex];

		m_fileInfoIndex++;

		m_processingMutex.unlock();

		QString	path = m_destArchivePath + QString("/%1/%2").arg(hexFolder(cfi.groupID)).arg(cfi.appSignalID.mid(1));

		path = dr.toNativeSeparators(path);

		if (dr.exists(path) == false)
		{
			bool res = dr.mkpath(path);

			if (res == false)
			{
				print << QString("Error create folder: %1").arg(path);
				m_processedCount++;
				continue;
			}
		}

		QString toPath = path + QString("/%1").arg(cfi.fileName);

		toPath = dr.toNativeSeparators(toPath);

		if (QFile::exists(toPath))
		{
			QFile::remove(toPath);
		}

		bool res = true;

		if (cfi.copyEntireFile)
		{
			res = QFile::copy(cfi.pathFileName, toPath);
		}
		else
		{
			if (buf == nullptr)
			{
				buf = new char[BUF_SIZE];
			}

			res = copyFile(cfi.pathFileName, toPath, cfi.startPos, cfi.endPos, buf, BUF_SIZE);
		}

		//

		// DEBUG_STOP;

		// std::random_device rd;
		// qint32 r = rd();

		// if (r < 100)
		// {
		// 	res = false;
		// }

		//

		if (res == false)
		{
			asyncPrintError(QString("Error copy file: %1").arg(cfi.pathFileName));
		}

		m_processedCount++;

	} while(true);

	DELETE_ARRAY_IF_NOT_NULL(buf);
}

bool FileArchivist::copyFile(const QString& from, const QString& to, qint64 startPos, qint64 endPos, char* buf, qint64 bufSize)
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
		if (copySize >= bufSize)
		{
			sizeToRead = bufSize;
			copySize -= bufSize;
		}
		else
		{
			sizeToRead = copySize;
			copySize = 0;
		}

		qint64 readSize = fromFile.read(buf, sizeToRead);

		Q_ASSERT((readSize % ArchFileRecord::SIZE) == 0);

		toFile.write(buf, readSize);
	}
	while(copySize);

	return true;
}

void FileArchivist::asyncPrintError(const QString& err)
{
	asyncPrintError(QStringList() << err);
}

void FileArchivist::asyncPrintError(const QStringList& errs)
{
	m_processingMutex.lock();

	if (m_prevError == false)
	{
		print << QString("\n");
	}

	print.newLine();

	for(const QString& err : errs)
	{
		print << err;
		print.newLine();
	}

	m_prevError = true;

	m_processingMutex.unlock();
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

	qint64 recordIndex = 0;

	ArchFindResult res = afp.binarySearch(E::TimeType::System,  ts.timeStamp, &recordIndex);

	if (res != ArchFindResult::Found)
	{
		return -1;
	}

	qint64 position = recordIndex * ArchFileRecord::SIZE;

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

	qint64 recordIndex = 0;

	ArchFindResult res = afp.binarySearch(E::TimeType::System,  ts.timeStamp, &recordIndex);

	if (res != ArchFindResult::Found)
	{
		return -1;
	}

	qint64 position = recordIndex * ArchFileRecord::SIZE;

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


