#pragma once

#include "../../AppSignalLib/AppSignal.h"
#include "../../OnlineLib/CircularLogger.h"
#include "ArchFile.h"

namespace ArchV3
{
	class ArchWriter : public std::enable_shared_from_this<ArchWriter>, public LogWrapper
	{
	public:
		ArchWriter(	const QString& archDir, 
					const QString& projectName, 
					const QString& srcEquipmentID,
					const AppSignals& appSignals, 
					CircularLoggerShared logger);
		~ArchWriter();

		void start();
		void stop();

		void requestQuit();
		bool isWorkable() const;

	private:
		void run();

		bool init();
		void shutdown();

		bool checkAndInitDirs();
		void initArchFiles();

		QString archPath_00_FF(quint8 n);
		static QString clearString(QString str);		// copy OK

		void writeSignalInGropsFile(const std::unordered_map<quint8, QStringList>& signlsInGroups);

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_srcEquipmentID;
		const AppSignals& m_appSignals;

		//
		std::thread m_thread;
		std::mutex m_cvMutex;
		std::condition_variable m_cv;

		std::atomic<bool> m_isWorkable {false};
		std::atomic<bool> m_quitRequested {false};

		QString m_archPath;

		std::vector<std::unique_ptr<ArchFileBase>> m_archFiles;
	};

	using ArchWriterShared = std::shared_ptr<ArchWriter>;
}