#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <ArchV3Lib/Core.h>
#include "../../UtilsLib/WUtils.h"

namespace ArchV3
{
	Core::Core(const QString& archDir,
			   const QByteArray& archInfoV3Data,
			   const DbConnectionInfo& dbConnInfo, 
			   CircularLoggerShared logger) :
		LogWrapper(logger, "ArchV3::Core"),
		m_archDir(archDir),
		m_dbConnInfo(dbConnInfo),
		m_logger(logger)
	{ 
		m_archInfoV3 = std::make_unique<Proto::ArchInfoV3>();

		bool res = m_archInfoV3->ParseFromArray(archInfoV3Data.constData(), TO_INT(archInfoV3Data.size()));

		if (res == false)
		{
			m_archInfoV3.reset();
		}
	}

	Core::~Core() 
	{
	}

	bool Core::init()
	{ 
		m_workable.store(false);

		if (m_archInfoV3 == nullptr)
		{
			return false;
		}

		bool result = true;

		m_projectName = QString::fromStdString(m_archInfoV3->buildinfo().project());

		for (const Proto::ClientArchSignals& clientArchSignals : m_archInfoV3->clientarchsignals())
		{
			QString clientID = QString::fromStdString(clientArchSignals.clientid());

			auto [it, inserted] = m_clientSignals.emplace(clientID, std::vector<ArchSignal>{});

			if (inserted == false)
			{
				Q_ASSERT(false);
				continue;
			}

			std::vector<ArchSignal>& archSignals = it->second;

			archSignals.resize(TO_SIZE_T(clientArchSignals.archsignal_size()));

			int index = 0;

			for (const Proto::ArchSignal& as : clientArchSignals.archsignal())
			{
				archSignals[index++].loadFromProto(as);
			}

			Db db(m_projectName, clientID, m_dbConnInfo, getLog());

			bool res = db.open();

			if (res == false)
			{
				result = false;
				continue;
			}

			std::vector<QString> filesToDelete;

			res = db.registerSignals(archSignals, &filesToDelete);

			if (res == false)
			{
				result = false;
				continue;
			}

			Storage s(m_archDir, m_projectName, clientID);

			s.deleteFiles(filesToDelete);

		}

		m_archInfoV3.reset();

		RETURN_IF_FALSE(result);

		m_workable.store(true);
		return true;
	}

	bool Core::isWorkable() const
	{ 
		return m_workable.load(); 
	}

	ArchWriterShared Core::getArchWriter(const QString& srcEquipmentID)
	{
		//std::lock_guard<std::mutex> lock(m_archWritersMutex);

		//auto it = m_archWriters.find(srcEquipmentID);

		//if (it != m_archWriters.end())
		//{
		//	return it->second;
		//}

		//auto archWriter = std::make_shared<ArchWriter>(
		//	m_archDir,
		//	m_projectName,
		//	srcEquipmentID,
		//	m_logger
		//);

		//m_archWriters[srcEquipmentID] = archWriter;
//		return archWriter;
		return nullptr;
	}
} // namespace ArchV3