#pragma once
#include <vector>
#include "../CommonLib/Hash.h"

namespace ClientLib
{
	// Recently used AppSignals
	//
	class IRecentAppSignals
	{
	public:
		virtual ~IRecentAppSignals() = default;

		virtual void addRecentAppSignal(Hash h) = 0;
		virtual void addRecentAppSignals(const std::vector<Hash>& hashes) = 0;

		virtual std::vector<Hash> recentlyUsedAppSignals(const QString& appDataServivceId) = 0;
		virtual bool hasRecentlyUsedAppSignals() = 0;
	};

}
