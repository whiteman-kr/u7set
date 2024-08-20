#pragma once

namespace TuningFilters
{
	class TuningFilter;
	class TuningFilterStorage;
} // namespace TuningFilters

namespace TuningLib
{
	class TuningUiItem;
	class TuningUiStorage;
} // namespace TuningLib

namespace AppSignalLists
{
	class AppSignalListSet;
}

namespace TuningFilters
{
	class TuningFilterToLists
	{
	public:
		static bool convertUi(const QString& softwareEquipmentId,
							  const QString& softwareTag,
							  TuningFilters::TuningFilterStorage& tuningFilterStorage,
							  TuningLib::TuningUiStorage& uiStorage,
							  AppSignalLists::AppSignalListSet& appSignalLists);

		static bool convertGeneric(TuningFilters::TuningFilterStorage& tuningFilterStorage,
								   AppSignalLists::AppSignalListSet& appSignalLists);

	private:
		static bool convertUiFilter(const QString& softwareEquipmentId,
									const QString& softwareTag,
									int& filterNumber,
									TuningFilters::TuningFilter* parentFilter,
									TuningLib::TuningUiItem* parentUi,
									AppSignalLists::AppSignalListSet& appSignalLists);

		static bool convertGenericFilter(int& filterNumber,
									TuningFilters::TuningFilter* parentFilter,
									AppSignalLists::AppSignalListSet& appSignalLists);
	};
} // namespace TuningFilters
