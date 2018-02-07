#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"

class TuningClientFilterStorage : public TuningFilterStorage
{
public:
	TuningClientFilterStorage();

	void checkAndRemoveFilterSignals(const std::vector<Hash>& signalHashes, bool& removedNotFound, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parentWidget);

	void updateCounters(const TuningSignalManager* objects, const TuningClientTcpClient* tcpClient, TuningFilter* filter = nullptr);

	void createAutomaticFilters(const TuningSignalManager* objects, bool bySchemas, bool byEquipment, const QStringList& tuningSourcesEquipmentIds);

	void removeFilters(TuningFilter::Source sourceType);

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

};

#endif // CLIENTFILTERSTORAGE_H
