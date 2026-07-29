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
						   const QString& clientID,
						   const DbConnectionInfo& dbConnInfo,
						   const std::vector<ArchSignal>& archSignals, 
						   CircularLoggerShared logger) :
		LogWrapper(logger, QString("ArchWriter(%1)").arg(clientID)),
		m_storage(archDir, projectName, clientID, dbConnInfo, archSignals, logger)
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

	void ArchWriter::pushArchData(const char* archData, size_t archDataSize)
	{
		if (archData == nullptr || archDataSize == 0)
		{
			return;
		}

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			m_queue.emplace_back(archData, archData + archDataSize);
		}

		m_cv.notify_one();
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
					return m_quitRequested.load(std::memory_order::relaxed) || !m_queue.empty();
				});

			if (m_quitRequested.load(std::memory_order::relaxed) == true)
			{
				lock.unlock();	
				break;
			}

			lock.unlock();

			if (signaled)
			{
				processArchData();
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
		bool result = m_storage.init();

		return result;
	}

	void ArchWriter::shutdown()
	{
	}

	void ArchWriter::processArchData()
	{
		std::deque<ArchData> queue;

		{
			std::lock_guard<std::mutex> lock(m_queueMutex);
			queue.swap(m_queue);
		}

		while (!queue.empty())
		{
			const ArchData& archData = queue.front();
			m_storage.processArchData(archData.data(), archData.size());
			queue.pop_front();
		}
	}

	void ArchWriter::writeSignalInGropsFile(const std::unordered_map<quint8, QStringList>& signlsInGroups)
	{ 
		//QFile file(m_archPath + QDir::separator() + "SignalsInGroups.txt");

		//if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		//{
		//	QTextStream stream(&file);
		//	for (const auto& pair : signlsInGroups)
		//	{
		//		stream << QString("Group %1:\n").arg(pair.first, 2, 16, QChar('0')).toUpper();
		//		for (const QString& signalID : pair.second)
		//		{
		//			const AppSignal* appSignal = m_appSignals.getByAppSignalID(signalID);

		//			if (appSignal != nullptr)
		//			{
		//				stream << (appSignal->isAnalog() ? "Analog  " : (appSignal->isDiscrete() ? "Discrete" : "Unknown")) << "  "
		//					   << appSignal->appSignalID() << "\n";
		//			}
		//		}
		//	}
		//}
		//else
		//{
		//	logErr(QString("Failed to open SignalsInGroups.txt for writing in %1").arg(m_archPath));
		//}
	}
}