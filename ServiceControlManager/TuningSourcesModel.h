#pragma once

#include "../AppDataService/AppDataSource.h"
#include "Columns.h"

class TuningSourcesModel : public QAbstractTableModel
{
public:
	TuningSourcesModel(QWidget* parent);

	const Columns& columns() const;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	void updateData(const Network::ServiceInfo& srvInfo);

private:
	std::vector<Network::TuningSourceInfoState> m_sources;
	std::vector<int> m_sourcesErrorCount;

	inline static const Columns m_columns =
	{
		{"EquipmentID", 400},					// 1
		{"Source IP", 180},						// 2
		{"Is reply", 140},						// 3
		{"Request count", 140},					// 4
		{"No Ack count", 140},					// 5
		{"Control is active", 140},				// 6
		{"Writing disabled", 140},				// 7
		{"Has unapplied params", 140},			// 8
		{"Set SOR", 140},						// 9
		{"Errors count", 140},					// 10
	};
};

