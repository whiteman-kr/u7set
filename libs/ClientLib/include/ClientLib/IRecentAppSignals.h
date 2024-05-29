#pragma once
#include <span>
#include <vector>

namespace ClientLib
{
	// Recently used AppSignals
	//
	class IRecentAppSignals
	{
	public:
		virtual ~IRecentAppSignals() = default;

		virtual void addRecentAppSignal(Hash h) = 0;
		virtual void addRecentAppSignals(std::span<const Hash> hashes) = 0;

		virtual std::vector<Hash> recentlyUsedAppSignals(const QString& appDataServivceId) = 0;
		virtual bool hasRecentlyUsedAppSignals() = 0;
	};

} // namespace ClientLib
