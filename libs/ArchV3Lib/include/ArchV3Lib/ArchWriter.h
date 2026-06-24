#pragma once

#include "../../AppSignalLib/AppSignal.h"

namespace ArchV3
{
	class ArchWriter
	{
	public:
		ArchWriter(const QString& archDir, const QString& projectName, const QString& srcEquipmentID,
					const AppSignals& appSignals);
		~ArchWriter();

		void run();

		void requestQuit();

	private:
		bool start();
		void stop();

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_srcEquipmentID;

		//

		std::atomic_bool m_quitRequested{false};

		QString m_archPath;
	};
}