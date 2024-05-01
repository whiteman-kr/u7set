#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include "../lib/Tuning/TuningFilter.h"
#include "../OnlineLib/SoftwareSettings.h"

#include <ClientLib/TuningSourceState.h>

namespace ClientLib
{
	class TuningConnection;
}

class TuningClientFilterStorage : public TuningFilterStorage
{
public:
	TuningClientFilterStorage();

	// Operations

	void checkAndRemoveFilterSignals(const std::vector<Hash> &signalHashes,
									 bool &removedNotFound,
									 std::vector<std::pair<QString, QString>> &notFoundSignalsAndFilters,
									 QWidget* parentWidget);

	void updateCounters(const ClientLib::TuningSignalManager& tunigSignals,
						const ClientLib::TuningConnection& tuningConnection,
						const std::vector<ClientLib::TuningSource>& sourceStates,
						TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
						TuningFilter* filter = nullptr);

	void removeFilters(TuningFilter::Source sourceType);
};

class DialogCheckFilterSignals : public QDialog
{
	Q_OBJECT

public:

	DialogCheckFilterSignals(std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parent);

private slots:

	void buttonClicked(QAbstractButton* button);

private:

	QDialogButtonBox* m_buttonBox = nullptr;
};


#endif // CLIENTFILTERSTORAGE_H
