#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QDir>

#include <ArchV3Lib/ArchFile.h>
#include <ArchV3Lib/Writer.h>

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

	bool ArchFileBase::isOpen() const
	{
		return m_file.isOpen(); 
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

	bool ArchFileBase::flush(qint64 timeUTC)
	{
		bool result = true;

		if (m_file.isOpen() == true)
		{
			result = m_file.flush();

			if (result == true)
			{
				m_lastFlushTime = timeUTC;
			}
		}

		return result;
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