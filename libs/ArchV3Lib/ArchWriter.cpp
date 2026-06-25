#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <thread>

#include <ArchV3Lib/ArchWriter.h>

#include <QRegularExpression>
#include <QDir>

#include "../../UtilsLib/WUtils.h"

namespace ArchV3
{
	ArchWriter::ArchWriter(const QString& archDir, 
						   const QString& projectName, 
						   const QString& srcEquipmentID,
						   const AppSignals& appSignals,
						   CircularLoggerShared logger) :
		LogWrapper(logger, QString("ArchWriter")), 
		m_archDir(archDir),
		m_projectName(clearString(projectName)),
		m_srcEquipmentID(clearString(srcEquipmentID)),
		m_appSignals(appSignals)
	{ 
	}

	ArchWriter::~ArchWriter()
	{
	}

	void ArchWriter::run()
	{ 
		bool workable = start();

		if (!workable)
		{
			logErr("IS NOT WORKABLE!");

			while (m_quitRequested.load(std::memory_order::relaxed) == false)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			return;
		}

		// do work

		stop();
	}

	void ArchWriter::requestQuit()
	{ 
		m_quitRequested.store(true);
	}

	bool ArchWriter::start()
	{
		bool result = true;

		result = checkAndInitDirs();

		if (result == true)
		{
			initArchFiles();
		}

		return result;
	}

	void ArchWriter::stop()
	{
	}

	bool ArchWriter::checkAndInitDirs()
	{ 
		m_archPath = m_archDir + QDir::separator() + m_projectName + QDir::separator() + m_srcEquipmentID;

		bool result = QDir().mkpath(m_archPath);

		if (result == false)
		{
			logErr(QString("Failed to create arch directory: %1").arg(m_archPath));
			return false;
		}

		QString path;

		for (quint32 n = 0; n < 256; n++)
		{
			path = archPath_00_FF(static_cast<quint8>(n));

			bool res = QDir().mkpath(path);

			if (res == false)
			{
				logErr(QString("Failed to create arch subdirectory: %1").arg(path));
				result = false;
			}
		}

		return result;
	}

	void ArchWriter::initArchFiles()
	{
		size_t analogsCount = 0;
		size_t discretesCount = 0;

		for (const AppSignal& s : m_appSignals)
		{
			if (s.isAnalog())
			{
				analogsCount++;
			}
			else
			{
				if (s.isDiscrete())
				{
					discretesCount++;
				}
			}
		}

		m_archFiles.reserve(analogsCount + discretesCount);

		QString path;

		for (const AppSignal& s : m_appSignals)
		{
			if (s.isBus() == true)
			{
				continue;
			}

			quint8 n = static_cast<quint8>(s.hash() & 0xFF);

			path = archPath_00_FF(n) + QDir::separator() + clearString(s.appSignalID());

			if (s.isAnalog())
			{
				std::unique_ptr<AnalogArchFile> file = std::make_unique<AnalogArchFile>(*this);
				file->setFilePath(path);
				m_archFiles.push_back(std::move(file));
			}
			else
			{
				if (s.isDiscrete())
				{
					std::unique_ptr<DiscreteArchFile> file = std::make_unique<DiscreteArchFile>(*this);
					file->setFilePath(path);
					m_archFiles.push_back(std::move(file));
				}
			}
		}
	}

	QString ArchWriter::archPath_00_FF(quint8 n)
	{ 
		return m_archPath + QDir::separator() + QString("%1").arg(n, 2, 16, QChar('0')).toUpper();
	}

	QString ArchWriter::clearString(QString str)
	{ 
		str.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
		str.replace(QRegularExpression("_+"), "_");
		str.remove(QRegularExpression("^_+"));
		str.remove(QRegularExpression("_+$"));

		return str;
	}
}