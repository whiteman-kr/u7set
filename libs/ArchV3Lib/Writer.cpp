#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <thread>
#include <unordered_map>

#include <ArchV3Lib/Writer.h>

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

	void ArchWriter::start()
	{
		std::shared_ptr<ArchWriter> self = shared_from_this();

		m_thread = std::thread(
			[self]()
			{
				self->run();
			});
	}

	void ArchWriter::stop() 
	{ 
		m_quitRequested.store(true);
		m_cv.notify_one();

		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	void ArchWriter::requestQuit()
	{ 
	}

	bool ArchWriter::isWorkable() const
	{ 
		return m_isWorkable.load();
	}

	void ArchWriter::run()
	{
		m_isWorkable.store(init());

		if (!m_isWorkable.load())
		{
			logErr("IS NOT WORKABLE!");

			while (m_quitRequested.load(std::memory_order::relaxed) == false)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			return;
		}

		std::unique_lock<std::mutex> lock(m_cvMutex);

		while (m_quitRequested.load(std::memory_order::relaxed) == false)
		{
			bool signaled = m_cv.wait_for(lock,	std::chrono::milliseconds(5),
				[this]
				{
					return m_quitRequested.load(std::memory_order::relaxed);
				});

			if (m_quitRequested.load(std::memory_order::relaxed) == true)
			{
				lock.unlock();	
				break;
			}

			lock.unlock();

			if (signaled)
			{
				// real processing
			}
			else
			{
				// timeout, idle processing 
			}

			lock.lock();	
		}

		shutdown();
	}

	bool ArchWriter::init()
	{
		bool result = true;

		result = checkAndInitDirs();

		if (result == true)
		{
			initArchFiles();
		}

		return result;
	}

	void ArchWriter::shutdown()
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
		std::unordered_map<quint8, QStringList> signlsInGroups;

		for (quint32 n = 0; n < 256; n++)
		{
			signlsInGroups[static_cast<quint8>(n)] = QStringList();
		}

		for (const AppSignal& s : m_appSignals)
		{
			if (s.isBus() == true)
			{
				continue;
			}

			quint8 n = static_cast<quint8>(s.hash() & 0xFF);

			signlsInGroups[n].push_back(s.appSignalID());

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

		writeSignalInGropsFile(signlsInGroups);
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

	void ArchWriter::writeSignalInGropsFile(const std::unordered_map<quint8, QStringList>& signlsInGroups)
	{ 
		QFile file(m_archPath + QDir::separator() + "SignalsInGroups.txt");

		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream stream(&file);
			for (const auto& pair : signlsInGroups)
			{
				stream << QString("Group %1:\n").arg(pair.first, 2, 16, QChar('0')).toUpper();
				for (const QString& signalID : pair.second)
				{
					const AppSignal* appSignal = m_appSignals.getByAppSignalID(signalID);

					if (appSignal != nullptr)
					{
						stream << (appSignal->isAnalog() ? "Analog  " : (appSignal->isDiscrete() ? "Discrete" : "Unknown")) << "  "
							   << appSignal->appSignalID() << "\n";
					}
				}
			}
		}
		else
		{
			logErr(QString("Failed to open SignalsInGroups.txt for writing in %1").arg(m_archPath));
		}
	}
}