#pragma once

#include "../AppDataService/AppDataSource.h"
#include "Columns.h"

class AppDataSourcesModel : public QAbstractTableModel
{
public:
	AppDataSourcesModel(QWidget* parent);

	const Columns& columns() const;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	void updateData(const Network::ServiceInfo& srvInfo);

private:
	std::vector<Network::AppDataSourceState> m_sources;
	std::vector<int> m_sourcesErrorCount;

	inline static const Columns m_columns =
	{
		{"EquipmentID", 400},
		{"Source IP", 180},
		{"Receives data", 120},
		{"Uptime", 120},
		{"Receiving speed", 120},
		{"Packet count", 120},
		{"Lost packet count", 120},
		{"Errors count", 120},
	};

	inline static const QBrush YELLOW_BRUSH = QBrush(QColor(255, 255, 127));
};

