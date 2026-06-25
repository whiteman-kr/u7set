#pragma once

#include "../../AppSignalLib/AppSignal.h"
#include "../../OnlineLib/CircularLogger.h"
#include "ArchFile.h"

namespace ArchV3
{
	class ArchWriter : public LogWrapper
	{
	public:
		ArchWriter(	const QString& archDir, 
					const QString& projectName, 
					const QString& srcEquipmentID,
					const AppSignals& appSignals, 
					CircularLoggerShared logger);
		~ArchWriter();

		void run();

		void requestQuit();

	private:
		bool start();
		void stop();

		bool checkAndInitDirs();
		void initArchFiles();

		QString archPath_00_FF(quint8 n);
		static QString clearString(QString str);		// copy OK

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_srcEquipmentID;
		const AppSignals& m_appSignals;

		//

		std::atomic_bool m_quitRequested{false};

		QString m_archPath;

		std::vector<std::unique_ptr<ArchFileBase>> m_archFiles;
	};
}