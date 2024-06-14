#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include <TuningLib/TuningUiItem.h>
#include <Settings.h>

namespace ClientLib
{
	class TuningSource;
	class TuningConnection;
	class TuningSignalManager;
}

namespace AppSignalLists
{
	class AppSignalListSet;
}

class TuningClientUiStorage : public TuningLib::TuningUiStorage
{
public:
	TuningClientUiStorage(const ClientLib::TuningSignalManager& tunigSignals,
						  const ClientLib::TuningConnection& tuningConnection,
						  const AppSignalLists::AppSignalListSet& appSignalListSet);

	void updateCounters(const std::vector<ClientLib::TuningSource>& sourceStates,
						TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
						TuningLib::TuningUiItem* uiItem = nullptr);

private:
	const ClientLib::TuningSignalManager& m_tunigSignals;
	const ClientLib::TuningConnection& m_tuningConnection;
	const AppSignalLists::AppSignalListSet& m_appSignalListSet;
};


#endif // CLIENTFILTERSTORAGE_H
