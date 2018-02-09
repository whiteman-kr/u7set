#ifndef CLIENTFILTERSTORAGE_H
#define CLIENTFILTERSTORAGE_H

#include "../lib/Tuning/TuningFilter.h"
#include "TuningClientTcpClient.h"

class TuningClientFilterStorage : public TuningFilterStorage
{
public:
	TuningClientFilterStorage();

	// Schemas loading

	bool loadSchemasDetails(const QByteArray& data, QString* errorCode);
	int schemaDetailsCount();
	VFrame30::SchemaDetails schemaDetails(int index);

	// Operations

	void createSignalsAndEqipmentHashes(const TuningSignalManager* objects, TuningFilter* filter = nullptr);

	void checkAndRemoveFilterSignals(const std::vector<Hash>& signalHashes, bool& removedNotFound, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parentWidget);

	void updateCounters(const TuningSignalManager* objects, const TuningClientTcpClient* tcpClient, TuningFilter* filter = nullptr);

	void createAutomaticFilters(const TuningSignalManager* objects, bool bySchemas, bool byEquipment, bool discreteCounters, const QStringList& tuningSourcesEquipmentIds);

	void removeFilters(TuningFilter::Source sourceType);

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

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
