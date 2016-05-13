#pragma once

#include <QAbstractTableModel>
#include <QHash>

class TuningDataSourceInfo;
class TuningService;

class SafetyChannelSignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SafetyChannelSignalsModel(TuningDataSourceInfo& sourceInfo, TuningService* service, QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

public slots:
	void updateSignalStates();
	void updateSignalState(QString appSignalID, double value);

private:
	TuningDataSourceInfo& m_sourceInfo;
	QVector<QPair<double, double>> m_values;
	TuningService* m_service;
	QHash<QString, int> signalIdMap;
};
