#pragma once

namespace TuningFilters
{
	class TuningFilter;
	class TuningFilterStorage;
}

namespace TuningLib
{
	class TuningUiItem;
	class TuningUiStorage;
}

namespace AppSignalLists
{
	class AppSignalListSet;
}

namespace TuningFilters
{
	class TuningFilterToLists
	{
	public:

		static bool convert(TuningFilters::TuningFilterStorage& tuningFilterStorage,
							TuningLib::TuningUiStorage& uiStorage,
							AppSignalLists::AppSignalListSet& appSignalLists,
							const QStringList& appSignalListsSystemTags);

	private:
		static bool convertFilter(TuningFilters::TuningFilter* parentFilter,
								  TuningLib::TuningUiItem* parentUi,
								  AppSignalLists::AppSignalListSet& appSignalLists,
								  const QStringList& appSignalListsSystemTags);


	};
} // namespace TuningFilters
