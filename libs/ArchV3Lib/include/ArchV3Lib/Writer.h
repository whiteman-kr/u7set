#pragma once

#include <deque>

#include "../../OnlineLib/CircularLogger.h"
#include "ArchFile.h"
#include "ArchSignal.h"
#include "Storage.h"

namespace ArchV3
{
	class ArchWriter : public std::enable_shared_from_this<ArchWriter>, public LogWrapper
	{
	public:
		ArchWriter(	const QString& archDir, 
					const QString& projectName, 
					const QString& clientID,
					const std::vector<ArchSignal>& archSignals, 
					CircularLoggerShared logger);
		~ArchWriter();

		void start();
		void stop();

		void requestQuit();
		bool isWorkable() const;

		void pushArchData(const char* archData, size_t archDataSize);

	private:
		void run();

		bool init();
		void shutdown();

		void writeSignalInGropsFile(const std::unordered_map<quint8, QStringList>& signlsInGroups);

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_srcEquipmentID;

		//

		Storage m_storage;

		std::thread m_thread;
		std::mutex m_cvMutex;
		std::condition_variable m_cv;
		
		using ArchData = std::vector<char>;

		std::mutex m_queueMutex;
		std::deque<ArchData> m_queue;

		std::atomic<bool> m_isWorkable {false};
		std::atomic<bool> m_quitRequested {false};

		QString m_archPath;

		std::vector<std::unique_ptr<ArchFileBase>> m_archFiles;
	};

	using ArchWriterShared = std::shared_ptr<ArchWriter>;
}