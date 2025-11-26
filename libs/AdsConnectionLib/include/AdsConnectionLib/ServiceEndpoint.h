#pragma once
#include <compare>
#include <format>
#include <string>

struct ServiceAddress
{
	std::string address;
	uint16_t port = 0;

	auto operator<=>(const ServiceAddress&) const = default;

	friend std::ostream& operator<<(std::ostream& os, const ServiceAddress& sa) { return os << sa.address << ":" << sa.port; }

	std::string to_string() const { return std::format("{}:{}", address, port); }
	operator std::string() const { return to_string(); }
};

struct ServiceEndpoint
{
	std::string equipmentId;
	std::string shortenId; // Short version of equipmentId
	ServiceAddress address;

	//--
	//
	auto operator<=>(const ServiceEndpoint&) const = default;

	friend std::ostream& operator<<(std::ostream& os, const ServiceEndpoint& se)
	{
		return os << se.equipmentId << " (" << se.address.to_string() << ")";
	}

	std::string to_string() const { return std::format("{} ({})", equipmentId, address.to_string()); }
	operator std::string() const { return to_string(); }
};
