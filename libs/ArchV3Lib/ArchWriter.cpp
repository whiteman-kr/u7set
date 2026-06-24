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

		for (int i = 0; i < 256; i++)
		{
			path = m_archPath + QDir::separator() + QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
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

		for (const AppSignal& s : m_appSignals)
		{
			if (s.isAnalog())
			{
//				m_archFiles.push_back(std::make_unique<ArchV3Lib::AnalogArchFile>(/* parameters */));
			}
			else
			{
				if (s.isDiscrete())
				{
//					m_archFiles.push_back(std::make_unique<ArchV3Lib::DiscreteArchFile>(/* parameters */));
				}
			}
		}
	}

	QString ArchWriter::clearString(QString str) const
	{ 
		str.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
		str.replace(QRegularExpression("_+"), "_");
		str.remove(QRegularExpression("^_+"));
		str.remove(QRegularExpression("_+$"));

		return str;
	}
}