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
	ArchFileBase::ArchFileBase()
	{
	}

	ArchFileBase::~ArchFileBase()
	{
		if (m_file.isOpen())
		{
			m_file.close();
		}
	}

	bool ArchFileBase::setFilePath(const QString& path)
	{
		m_path = path;
		
		//bool result = QDir().mkpath(m_path);

		//if (result == false)
		//{
		//	m_archWriter.logErr(QString("Failed to create directory: %1").arg(m_path));
		//}

		return true;
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

	bool ArchFileBase::isChecked() const
	{ 
		return m_checked;
	}

	bool ArchFileBase::checkFile(const ArchFileInfo& afi, ArchFileInfo* checkedAfi)
	{ 
		TEST_PTR_RETURN_FALSE(checkedAfi);

		*checkedAfi = afi;

		Q_ASSERT(false); // TO DO

		return true;
	}

	void ArchFileBase::setChecked(bool checked)
	{ 
		m_checked = checked;
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