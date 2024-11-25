	#pragma once

#include "../AppSignalLib/ISignalManager.h"
#include <AppSignalLists/SignalList.h>

class AppSignalSet;

namespace Builder
{
	class AppSignalListStorage : public DbObjectStorage<std::shared_ptr<AppSignalLists::AppSignalList>>
	{
	public:
		AppSignalListStorage(DbController* db);
		
		using DbObjectStorage::get;
		std::shared_ptr<AppSignalLists::AppSignalList> get(const QString& id, bool* ok = nullptr) const;

		std::vector<std::pair<QString, QString>> checkForSameIds() const;

		bool load(QString* errorMessage) override;
		bool save(const QUuid& uuid, QString* errorMessage) override;
	};

	class AppSignalListsProvider : public ISignalManager
	{
	public:
		AppSignalListsProvider(const std::vector<AppSignal*>& signalsVector);

		virtual int signalsCount() const override;
		virtual std::vector<Hash> signalHashes() const override;
		virtual std::vector<AppSignalParam> signalList() const override;

		virtual bool signalExists(Hash hash) const override;
		virtual bool signalExists(const QString& appSignalId) const override;
		virtual bool signalsExist(const QStringList& signalIds) const override;

		virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

		std::map<Hash, AppSignalParam>::const_iterator begin() const
		{
			return m_params.begin();
		}

		std::map<Hash, AppSignalParam>::const_iterator end() const
		{
			return m_params.end();
		}

	private:
		std::map<Hash, AppSignalParam> m_params;	 // Hash from AppSignalID
	};
}
