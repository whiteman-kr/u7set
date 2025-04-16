#pragma once

#include "../AppDataService/AppDataSource.h"
#include "Columns.h"

class ArchiveSignalsModel : public QAbstractTableModel
{
public:
	ArchiveSignalsModel(QWidget* parent);

	const Columns& columns() const;

	int size() const;

	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

	void updateData(const Network::ServiceInfo& srvInfo);

	QString fineSize(qint64 size) const;

	const Network::ArchSignalInfo& at(int index);

private:
	std::vector<Network::ArchSignalInfo> m_archSignals;

	inline static const Columns m_columns =
	{
		{"AppSignalID", 400},
		{"Records per minute", 120},
		{"Archive size per day", 120},
		{"Aperture type", 120},
		{"Coarse aperture", 120},
		{"Fine aperture", 120},
		{"Aperture overrided", 120},
	};
};

