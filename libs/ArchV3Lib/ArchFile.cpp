#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QDir>

#include <ArchV3Lib/ArchFile.h>
#include <ArchV3Lib/Writer.h>

#include "../../UtilsLib/WUtils.h"

#ifdef Q_OS_WIN
	#include <windows.h>
#elif defined(Q_OS_LINUX)
	#include <sys/statvfs.h>
#endif

namespace ArchV3
{
	ArchFileBase::ArchFileBase(const QString& archPath, const QString& appSignalID) :
		m_archPath(archPath),
		m_appSignalID(appSignalID)
	{
	}

	ArchFileBase::~ArchFileBase()
	{
		if (m_file.isOpen())
		{
			m_file.close();
		}
	}

	QString ArchFileBase::appSignalID() const
	{ 
		return m_appSignalID;
	}

	bool ArchFileBase::setFileName(const QString& filename)
	{ 
		m_filename = filename;
		return true;
	}

	void ArchFileBase::setActiveFile(const ArchFileInfo& afi)
	{ 
		m_archFileID = afi.archFileID;
		m_filename = afi.fileName;
		m_fileSize = afi.fileSize;
		m_recordCount = afi.recordCount;
	}

	bool ArchFileBase::hasActiveFile() const
	{ 
		return (m_archFileID > 0);
	}

	size_t ArchFileBase::recordSize(E::SignalType st)
	{
		switch (st)
		{
		case E::SignalType::Analog:
			return sizeof(ArchV3::AnalogFileRecord);

		case E::SignalType::Discrete:
			return sizeof(ArchV3::DiscreteFileRecord);

		default:
			Q_ASSERT(false);
		}

		return 0;
	}

	CheckFileResult ArchFileBase::checkFile(const QString& archPath, const ArchFileInfo& afi, ArchFileInfo* checkedAfi)
	{ 
		TEST_PTR_RETURN_VALUE(checkedAfi, CheckFileResult::CheckError);

		QString filePathName = archPath + afi.fileName;

		*checkedAfi = afi;

		// 1. Check if file exists.
		//    If not:
		//      - create empty file;
		//      - checkedAfi->recordCount = 0;
		//      - checkedAfi->fileSize = 0;
		//      - checkedAfi->timeToUtc = 0;
		//      - return false (DB must be updated).

		QFile file(filePathName);

		if (file.exists() == false)
		{
			if (file.open(QIODevice::WriteOnly) == false)
			{
				// log error
				return CheckFileResult::CheckError;
			}

			file.close();

			checkedAfi->recordCount = 0;
			checkedAfi->fileSize = 0;
			checkedAfi->timeToUTC = 0;
			return CheckFileResult::Changed;
		}

		// 2. Open file.

		if (file.open(QIODevice::ReadWrite) == false)
		{
			return CheckFileResult::CheckError;
		}

		// 3. Get actual file size.

		qint64 fileSize = file.size();

		qint64 recSize = static_cast<qint64>(recordSize(afi.signalType));

		if (recSize == 0)
		{
			return CheckFileResult::CheckError;
		}

		const qint64 alignedFileSize = (fileSize / recSize) * recSize;

		if (alignedFileSize != fileSize)
		{
			if (!file.resize(alignedFileSize))
			{
				return CheckFileResult::CheckError;
			}

			fileSize = alignedFileSize;
		}

		checkedAfi->fileSize = fileSize;
		checkedAfi->recordCount = fileSize / recSize;

		// 4. If file size differs from DB:
		//      - truncate file to whole record boundary;
		//      - update checkedAfi->fileSize;
		//      - update checkedAfi->recordCount;
		//      - read last record (if any);
		//      - update checkedAfi->timeToUtc;
		//      - return false.

		if (fileSize > 0)
		{
			switch (afi.signalType)
			{
			case E::SignalType::Analog:
				{
					AnalogFileRecord firstRecord;
					AnalogFileRecord lastRecord;

					bool res = file.seek(0);

					if (res == false)
					{
						return CheckFileResult::CheckError;
					}

					qint64 readSize = file.read(reinterpret_cast<char*>(&firstRecord), sizeof(firstRecord));

					if (readSize != sizeof(firstRecord))
					{
						return CheckFileResult::CheckError;
					}

					res = file.seek(fileSize - sizeof(lastRecord));

					if (res == false)
					{
						return CheckFileResult::CheckError;
					}

					readSize = file.read(reinterpret_cast<char*>(&lastRecord), sizeof(lastRecord));

					if (readSize != sizeof(lastRecord))
					{
						return CheckFileResult::CheckError;
					}

					checkedAfi->timeFromUTC = firstRecord.serverTimeUTC;
					checkedAfi->timeToUTC = lastRecord.serverTimeUTC;
				}
				break;

			case E::SignalType::Discrete:
				{
					DiscreteFileRecord firstRecord;
					DiscreteFileRecord lastRecord;

					bool res = file.seek(0);

					if (res == false)
					{
						return CheckFileResult::CheckError;
					}

					qint64 readSize = file.read(reinterpret_cast<char*>(&firstRecord), sizeof(firstRecord));

					if (readSize != sizeof(firstRecord))
					{
						return CheckFileResult::CheckError;
					}

					res = file.seek(fileSize - sizeof(lastRecord));

					if (res == false)
					{
						return CheckFileResult::CheckError;
					}

					readSize = file.read(reinterpret_cast<char*>(&lastRecord), sizeof(lastRecord));

					if (readSize != sizeof(lastRecord))
					{
						return CheckFileResult::CheckError;
					}

					checkedAfi->timeFromUTC = firstRecord.serverTimeUTC;
					checkedAfi->timeToUTC = lastRecord.serverTimeUTC;
				}
				break;

			default: 
				return CheckFileResult::CheckError;
			}
		}
		else
		{
			checkedAfi->recordCount = 0;
			checkedAfi->fileSize = 0;
			checkedAfi->timeFromUTC = 0;
			checkedAfi->timeToUTC = 0;	
		}
		
		return (afi == (*checkedAfi) ? CheckFileResult::Matched : CheckFileResult::Changed);
	}

	bool ArchFileBase::isOpen() const
	{
		return m_file.isOpen(); 
	}

		bool ArchFileBase::openFile()
	{
		if (m_file.isOpen() == true)
		{
			return true;
		}

		if (m_file.open(QIODevice::WriteOnly | QIODevice::Append) == false)
		{
			// log error
			return false;
		}

		return true;
	}

	void ArchFileBase::closeFile()
	{
		if (m_file.isOpen() == true)
		{
			m_file.close();
		}
	}

	bool ArchFileBase::flushBuffer(const char* data, size_t recordsCount, qint64 timeUTC)
	{
		m_lastFlushTime = timeUTC;
		return false;
	}

	bool ArchFileBase::write(qint64 timeUTC)
	{ 
		return true;
	}

	qint64 ArchFileBase::fileSize() const
	{
		return m_fileSize; 
	}

	qint64 ArchFileBase::lastWriteTime() const
	{
		return m_lastWriteTime; 
	}

	qint64 ArchFileBase::lastFlushTime() const
	{
		return m_lastFlushTime; 
	}

	bool ArchFileBase::prepareForNextState(ArchFileInfo* afi)
	{ 
		TEST_PTR_RETURN_FALSE(afi);

		Q_ASSERT(false);		// TO DO
		return true;
	}

	void ArchFileBase::readClusterSize(const QString& archDir)
	{
		QString root = QDir(archDir).rootPath();	// "D:/"

		#ifdef Q_OS_WIN
		{
			DWORD sectorsPerCluster = 0;
			DWORD bytesPerSector = 0;
			DWORD freeClusters = 0;
			DWORD totalClusters = 0;

			GetDiskFreeSpaceW(reinterpret_cast<LPCWSTR>(root.utf16()), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters);

			m_clusterSize = sectorsPerCluster * bytesPerSector;
		}
		#elif defined(Q_OS_LINUX)
		{
			struct statvfs s;

			statvfs(path, &s);

			quint64 clusterSize = s.f_frsize;
		}
		#else
		{
			m_clusterSize = 4096;
		}
		#endif
	}

	quint32 ArchFileBase::clusterSize()
	{ 
		return m_clusterSize;
	}

	bool ArchFileBase::writeRaw(const char* data, qint64 dataSize, qint64 timeUTC)
	{
		Q_ASSERT(dataSize >= 0);

		if (dataSize == 0)
		{
			return true;
		}

		if (openFile() == false)
		{
			return false;
		}

		const qint64 written = m_file.write(data, dataSize);

		if (written != dataSize)
		{
			// log error
			// truncate file to integral size
			return false;
		}

		m_fileSize += written;
		m_lastWriteTime = timeUTC;

		return true;
	}
}