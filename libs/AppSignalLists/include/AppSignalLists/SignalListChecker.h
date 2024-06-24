#pragma once

#include "AppSignalLists/SignalList.h"

namespace AppSignalLists
{
	//
	// AppSignalListSetChecker
	//
	class AppSignalListSetChecker
	{
		// Checking and removing "dangling" list items
		//
	public:
		[[nodiscard]] static void checkForDanglingItems(const std::vector<Hash>& allHashes, AppSignalListSet& listSet, QWidget* parent, ILogFile* logFile);

	private:
		[[nodiscard]] static std::set<Hash> findDanglingListItems(const std::set<Hash>& allHashes, AppSignalListSet& listSet, std::vector<std::pair<QString, QString>>& notFoundSignalsInLists);
		[[nodiscard]] static std::set<Hash> findDanglingCachedHashes(const std::set<Hash>& allHashes, AppSignalListSet& listSet, std::map<QString, int>& notFoundCachedHashesInLists);

		[[nodiscard]] static void removeListItems(AppSignalListSet& listSet, std::set<Hash> hashesToRemove);
		[[nodiscard]] static void removeCachedHashes(AppSignalListSet& listSet, std::set<Hash> hashesToRemove);
	};
}