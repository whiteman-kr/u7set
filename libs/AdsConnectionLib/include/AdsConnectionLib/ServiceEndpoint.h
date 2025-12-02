#pragma once
#include <compare>
#include <format>
#include <string>

struct ServiceAddress
{
	std::string address;
	uint16_t port = 0;

	static ServiceAddress create(std::string address, int port)
	{
		return ServiceAddress{.address = std::move(address), .port = static_cast<uint16_t>(port)};
	}

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

	static ServiceEndpoint create(std::string equipmentId, std::string shortenId, std::string address, int port)
	{
		return ServiceEndpoint{.equipmentId = std::move(equipmentId),
							   .shortenId = std::move(shortenId),
							   .address = ServiceAddress::create(std::move(address), port)};
	}

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
