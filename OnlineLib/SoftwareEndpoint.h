#pragma once

#include <CommonLib/HostAddressPort.h>

namespace SoftwareEndpoint
{
	struct ConfigService
	{
		QString equipmentId;
		HostAddressPort address;

		void clear()
		{
			equipmentId.clear();
			address.clear();
		}
	};

	struct TuningService
	{
		QString equipmentId;
		QString shortenId;			// Short version of tuningServiceID
		HostAddressPort clientRequestAddress;
		QStringList drivenSources;
		bool singleLmControl = false;

		bool operator==(const TuningService&) const = default;
	};

	struct AppDataService
	{
		QString equipmentId;
		QString shortenId;			// Short version of equipmentId
		HostAddressPort address;
		HostAddressPort realtimeAddress;

		bool operator==(const AppDataService&) const = default;
	};

	struct ArchiveService
	{
		QString equipmentId;		// ArchiveService equipmentId
		QString shortenId;			// Short version of equipmentId
		QString appDataServiceId;	// ID of the source AppDataService for this ArchiveService
		HostAddressPort address;	// ArchiveService ip address and port

		bool operator==(const ArchiveService&) const = default;
	};

	struct DiagDataService
	{
		QString equipmentId;
		QString shortenId;			// Short version of equipmentId
		HostAddressPort address;
		HostAddressPort realtimeAddress;

		bool operator==(const DiagDataService&) const = default;
	};
}