#pragma once

#include <utility>

#include <QString>
#include <QMetaType>


namespace TrendLib
{
	class TrendSignalParam;

	struct ArchiveServer
	{
		ArchiveServer() = default;
		ArchiveServer(QString equipmentId, QString shortEquipmentId, QString dataServiceId) :
			equipmentId(std::move(equipmentId)),
			shortEquipmentId(std::move(shortEquipmentId)),
			dataServiceId(std::move(dataServiceId))
		{
		}

		QString equipmentId;			///< ArchiveServiceDataID
		QString shortEquipmentId;		///< Shortened ArchiveServiceDataID
		QString dataServiceId;			///< AppDataServiceID or DiagDataServiceID
	};


	struct TrendSignalPlusServerId
	{
		QString appSignalId;
		QString archiveServerId;

		[[nodiscard]] QString toString() const
		{
			return appSignalId + "@" + archiveServerId;
		}

		bool operator==(const TrendSignalPlusServerId& that) const;
		bool operator==(const TrendSignalParam& that) const;

		bool operator<(const TrendSignalPlusServerId& that) const;
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendSignalPlusServerId)
