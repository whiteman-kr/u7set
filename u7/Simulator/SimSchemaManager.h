#pragma once

#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"

class SimIdeSimulator;

class SimSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit SimSchemaManager(SimIdeSimulator* simulator, QObject* parent = nullptr);

protected:
	virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

	// RealTime Trends (ITrendDataProvider)
	//
public:
	virtual bool trendData(QUuid trendUuid,
						   QString appSignalId,
						   QDateTime from,
						   QDateTime to,
						   E::TimeType timeType,
						   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const override;

	// Slots
	//
protected slots:
	void slot_projectUpdated();

public:
	SimIdeSimulator* simulator();
	const SimIdeSimulator* simulator() const;

	QString monitorId() const;
	void setMonitorId(QString equipmentId, bool emitUpdate);

	// Data
	//
private:
	SimIdeSimulator* m_simulator = nullptr;

	QString m_monitorId;
};

