#pragma once

#include "../AppSignalLib/ISignalManager.h"
#include <AppSignalLists/SignalList.h>
#include <DbLib/DbObjectStorage.h>

class AppSignalSet;

namespace Builder
{
	class AppSignalListStorage : public DbObjectStorage<std::shared_ptr<AppSignalLists::AppSignalList>>
	{
	public:
		AppSignalListStorage(DbController* db);

		bool load(QString* errorMessage) override;
		bool save(const QUuid& uuid, QString* errorMessage) override;
	};

class AppSignalListsProvider : public ISignalManager
	{
	public:
		AppSignalListsProvider(const AppSignalSet* signalSet);

		virtual int signalsCount() const override;
		virtual std::vector<Hash> signalHashes() const override;
		virtual std::vector<AppSignalParam> signalList() const override;

		virtual bool signalExists(Hash hash) const override;
		virtual bool signalExists(const QString& appSignalId) const override;
		virtual bool signalsExist(const QStringList& signalIds) const override;

		virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
		virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

		std::map<Hash, AppSignalParam>::iterator begin() 
		{
			return m_params.begin();
		}
		std::map<Hash, AppSignalParam>::iterator end() 
		{
			return m_params.end();
		}

	private:
		std::map<Hash, AppSignalParam> m_params;
	};
}
