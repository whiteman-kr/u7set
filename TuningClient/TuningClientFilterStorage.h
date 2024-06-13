#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include <ClientLib/TuningSourceState.h>
#include <TuningLib/TuningFilter.h>
#include "../OnlineLib/SoftwareSettings.h"

namespace ClientLib
{
	class TuningConnection;
	class TuningSignalManager;
}

class TuningClientFilterStorage : public TuningFilters::TuningFilterStorage
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
						TuningFilters::TuningFilter* filter = nullptr);

	void removeFilters(TuningFilters::TuningFilter::Source sourceType);
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
