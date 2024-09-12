#include <SchemaClientLib/SchemaClientConfigController.h>
#include <CommonLib/HostAddressPort.h>
#include "../OnlineLib/SoftwareSettings.h"


namespace SchemaClientLib
{

	SchemaClientConfigController::SchemaClientConfigController(const SoftwareInfo& softwareInfo,
															   const HostAddressPort& address1,
															   const HostAddressPort& address2,
															   ILogFile* logFile) :
		ClientLib::ConfigController{softwareInfo, address1, address2, logFile}
	{
		return;
	}

	bool SchemaClientConfigController::getSchemasDetails()
	{
		// Get SchemaDetails.pbuf file
		//
		QByteArray ba;
		QString fileName = "/" + m_softwareInfo.equipmentID() + QStringLiteral("/SchemaDetails.pbuf");

		bool ok = getFileBlocked(fileName, &ba, nullptr);

		QWriteLocker locker(&m_schemaDetailsLock);
		m_schemaDetailsSet.clear();

		if (ok == true)
		{
			m_schemaDetailsSet.Load(ba);
		}

		return ok;
	}

	bool SchemaClientConfigController::getAppSignalLists(const std::vector<OnlineLib::BuildFileInfo>& files)
	{
		bool ok = true;
		std::list<QByteArray> listsData;
		QByteArray ba;

		for (const auto& fi : files)
		{
			if (fi.tag == CfgFileTag::APPSIGNALLISTS)
			{
				bool fileOk = getFileBlocked(fi.pathFileName, &ba, nullptr);
				if (fileOk == true)
				{
					listsData.push_back(ba);
				}

				ok &= fileOk;
			}
		}

		QWriteLocker locker(&m_appSignalListSetLock);
		m_appSignalListSet.clear();

		for (auto&& data : listsData)
		{
			ok &= m_appSignalListSet.add(std::move(data));
		}

		return ok;
	}

	VFrame30::SchemaDetailsSet SchemaClientConfigController::schemasDetailsSet() const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet;
	}

	std::vector<VFrame30::SchemaDetails> SchemaClientConfigController::schemasDetails() const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemasDetails();
	}

	std::set<QString> SchemaClientConfigController::schemaAppSignals(const QString& schemaId)
	{
		QReadLocker l(&m_schemaDetailsLock);

		std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetailsSet.schemaDetails(schemaId);
		if (details == nullptr)
		{
			return {};
		}

		return details->m_signals;
	}

	QStringList SchemaClientConfigController::schemasByAppSignalId(const QString& appSignalId) const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemasByAppSignalId(appSignalId);
	}

	QStringList SchemaClientConfigController::schemasByLoopbackId(const QString& loopbackId) const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemasByLoopbackId(loopbackId);
	}

	int SchemaClientConfigController::schemaCount() const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemaCount();
	}

	QString SchemaClientConfigController::schemaCaptionById(const QString& schemaId) const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemaCaptionById(schemaId);
	}

	QString SchemaClientConfigController::schemaCaptionByIndex(int schemaIndex) const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemaCaptionByIndex(schemaIndex);
	}

	QString SchemaClientConfigController::schemaIdByIndex(int schemaIndex) const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.schemaIdByIndex(schemaIndex);
	}

	bool SchemaClientConfigController::schemaHasTags(const QString& schemaId, const QStringList& tags) const
	{
		QReadLocker l{&m_schemaDetailsLock};

		auto details = m_schemaDetailsSet.schemaDetails(schemaId);
		if (details == nullptr)
		{
			Q_ASSERT(details);
			return false;
		}

		const std::set<QString>& detailsTags = details->schemaTags();
		for (const QString& tag : tags)
		{
			if (detailsTags.contains(tag.trimmed().toLower()) == true)
			{
				return true;
			}
		}
		return false;
	}

	std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> SchemaClientConfigController::trendSchemaItems() const
	{
		QReadLocker l(&m_schemaDetailsLock);
		return m_schemaDetailsSet.trendIndicators();
	}

	AppSignalLists::AppSignalListSet SchemaClientConfigController::appSignalListSet() const 
	{
		QReadLocker l(&m_appSignalListSetLock);
		return m_appSignalListSet;
	}

} // namespace ClientLib