#pragma once

struct TrendItemSignal;

namespace VFrame30
{
	class VduSchema;
} // namespace VFrame30

namespace Builder
{
	class Context;
	class IssueLogger;

	class VduTrendConfigGenerator
	{
	public:
		VduTrendConfigGenerator() = delete;

	public:
		static bool generate(QString vduEquipmentId,
							 QString vduDir,
							 const std::vector<VFrame30::VduSchema*>& schemas,
							 const std::map<Hash, int>& appSignalHashToSignalIndex,
							 std::set<TrendItemSignal>& outTrendSignals,
							 Builder::Context& context);
	};
} // namespace Builder