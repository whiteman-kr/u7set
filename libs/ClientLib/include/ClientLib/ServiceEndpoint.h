#pragma once

#include "../../../../OnlineLib/SoftwareEndpoint.h"
#include <AdsConnectionLib/ServiceEndpoint.h>


inline ServiceEndpoint toServiceEndpoint(const ::SoftwareEndpoint::AppDataService& ads)
{
	ServiceEndpoint serviceEndpoint;
	serviceEndpoint.equipmentId = ads.equipmentId.toStdString();
	serviceEndpoint.shortenId = ads.shortenId.toStdString();
	serviceEndpoint.address.address = ads.address.address().toString().toStdString();
	serviceEndpoint.address.port = static_cast<uint16_t>(ads.address.port());

	return serviceEndpoint;
}

inline std::vector<ServiceEndpoint> toServiceEndpoint(const std::vector<::SoftwareEndpoint::AppDataService>& adses)
{
	std::vector<ServiceEndpoint> result;
	result.reserve(adses.size());

	for (const ::SoftwareEndpoint::AppDataService& ads : adses)
	{
		result.push_back(toServiceEndpoint(ads));
	}

	return result;
}