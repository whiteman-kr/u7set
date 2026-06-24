#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <thread>

#include <ArchV3Lib/ArchWriter.h>

namespace ArchV3
{
	ArchWriter::ArchWriter(const QString& archDir, const QString& projectName, const QString& srcEquipmentID,
						   const AppSignals& appSignals) :
		m_archDir(archDir),
		m_projectName(projectName),
		m_srcEquipmentID(srcEquipmentID)
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

}