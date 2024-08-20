#include <AppSignalLists/SignalListChecker.h>
#include <AppSignalLists/SignalList.h>
#include "../UtilsLib/LogFile.h"
#include "SignalListCheckerPrivate.h"

namespace AppSignalLists
{

	//
	// AppSignalListSetChecker
	//
	void AppSignalListSetChecker::checkForDanglingItems(const std::vector<Hash>& allHashes,
														AppSignalListSet& listSet,
														QWidget* parent,
														ILogFile* logFile)
	{
		// Check if some hashes stored in lists do not exist in the project database
		//
		if (allHashes.empty() == true)
		{
			return;
		}

		// Prevent re-entering checking function
		//
		if (ActiveCheck::active() == true)
		{
			return;
		}
		std::shared_ptr<ActiveCheck> ac = std::make_shared<ActiveCheck>();

		bool saveLists = false;

		std::set<Hash> allHashesSet;
		allHashesSet.insert(allHashes.begin(), allHashes.end());

		// Check user-added items
		//
		{
			std::vector<std::pair<QString, QString>> notFoundSignalsInLists;

			std::set<Hash> notFoundListItemsHashes = findDanglingListItems(allHashesSet, listSet, notFoundSignalsInLists);
			if (notFoundListItemsHashes.empty() == false)
			{
				for (const auto& [id, listId] : notFoundSignalsInLists)
				{
					logFile->writeError(
						QObject::tr("List '%1': signal with id '%2' does not exist in the project database.").arg(listId).arg(id));
				}

				DialogCheckAppSignalLists d(notFoundSignalsInLists, parent);
				if (d.exec() != QDialog::Accepted)
				{
					return;
				}

				saveLists = true;
				AppSignalListSetChecker::removeListItems(listSet, notFoundListItemsHashes);
				AppSignalListSetChecker::removeCachedHashes(listSet, notFoundListItemsHashes);
			}
		}

		// Check cached items
		//
		{
			std::map<QString, int> notFoundCachedHashesInLists; // Key is list id, value is dangling hashes found
			//
			std::set<Hash> notFoundCachedHashes = findDanglingCachedHashes(allHashesSet, listSet, notFoundCachedHashesInLists);
			if (notFoundCachedHashes.empty() == false)
			{
				for (const auto& [listId, count] : notFoundCachedHashesInLists)
				{
					logFile->writeError(QObject::tr("List '%1': found '%2' dangling hashes.").arg(listId).arg(count));
				}
				QMessageBox::critical(
					parent,
					qAppName(),
					QObject::tr("Warning!\n\nFound %1 dangling cached items in %2 list(s).\n\nPossible reasons:\na)Client is connected to "
								"a server with wrong list of signals.\nb) Project configuration mismatch between client and server.\n\nDo "
								"you wish to remove them permanently?")
						.arg(notFoundCachedHashes.size())
						.arg(notFoundCachedHashesInLists.size()));
				/*
				* Should these records be removed? Possibly, no. But let the code stay here for some time.
				if (QMessageBox::critical(parent,
										  qAppName(),
										  QObject::tr("Warning!\n\nFound %1 dangling cached items in %2 list(s).\n\nPossible
				reasons:\na)Client is connected to a server with wrong list of signals.\nb) Project configuration mismatch between client
				and server.\n\nDo you wish to remove them permanently?") .arg(notFoundCachedHashes.size())
											  .arg(notFoundCachedHashesInLists.size()),
										  QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
										  QMessageBox::StandardButton::No) == QMessageBox::Yes)
				{
					saveLists = true;
					removeCachedHashes(listSet, notFoundCachedHashes);
				}*/
			}
		}

		// Save lists
		//
		if (saveLists == true)
		{
			QString errorMessage;
			if (listSet.save(&errorMessage) == false)
			{
				QMessageBox::critical(parent, qAppName(), errorMessage);
			}
			else
			{
				QMessageBox::information(parent, qAppName(), QObject::tr("List items checking completed."));
			}
		}
	}

	std::set<Hash> AppSignalListSetChecker::findDanglingListItems(const std::set<Hash>& allHashes,
																  AppSignalListSet& listSet,
																  std::vector<std::pair<QString, QString>>& notFoundSignalsInLists)
	{
		std::set<Hash> result;

		for (const auto& list : listSet.lists())
		{
			// Check hashes of user-added signals to the list
			//
			auto itemsHashes = list->itemsHashes();
			for (Hash itemHash : itemsHashes)
			{
				if (allHashes.contains(itemHash) == false)
				{
					result.insert(itemHash);
					notFoundSignalsInLists.emplace_back(list->itemByHash(itemHash).appSignalId(), list->id());
				}
			}
		}

		return result;
	}

	std::set<Hash> AppSignalListSetChecker::findDanglingCachedHashes(const std::set<Hash>& allHashes,
																	 AppSignalListSet& listSet,
																	 std::map<QString, int>& notFoundCachedHashesInLists)
	{
		std::set<Hash> result;

		for (const auto& list : listSet.lists())
		{
			QString listId = list->id();

			auto listHashesCache = list->itemsHashes();
			for (Hash cachedHash : listHashesCache)
			{
				if (allHashes.contains(cachedHash) == false)
				{
					auto it = notFoundCachedHashesInLists.find(listId);
					if (it == notFoundCachedHashesInLists.end())
					{
						notFoundCachedHashesInLists[listId] = 1;
					}
					else
					{
						it->second++;
					}

					result.insert(cachedHash);
				}
			}
		}

		return result;
	}

	void AppSignalListSetChecker::removeListItems(AppSignalListSet& listSet, const std::set<Hash>& hashesToRemove)
	{
		for (const auto& list : listSet.lists())
		{
			for (Hash itemHash : hashesToRemove)
			{
				list->remove(itemHash);
			}
		}

		return;
	}

	void AppSignalListSetChecker::removeCachedHashes(AppSignalListSet& listSet, const std::set<Hash>& hashesToRemove)
	{
		for (const auto& list : listSet.lists())
		{
			auto& mutableAppListHashesCache = list->mutableAppListHashesCache();
			auto& mutableTuningListHashesCache = list->mutableTuningListHashesCache();

			for (Hash itemHash : hashesToRemove)
			{
				mutableAppListHashesCache.erase(itemHash);
				mutableTuningListHashesCache.erase(itemHash);
			}
		}

		return;
	}

} // namespace AppSignalLists