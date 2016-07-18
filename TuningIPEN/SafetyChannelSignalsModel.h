#pragma once

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QHash>
/*<<<<<<< HEAD

class TuningDataSourceInfo;
class TuningService;
class Signal;
=======*/
#include "../TuningService/TuningService.h"
//>>>>>>> f604605454aed8c615d8c9096c4216c91d30e089

struct SignalState
{
	double currentValue;
	double newValue;
	double lowLimit;
	double highLimit;
	bool validity;
};

class SafetyChannelSignalsDelegate : public QStyledItemDelegate
{
	Q_OBJECT

signals:
	void aboutToChangeDiscreteSignal(const QModelIndex& index);

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
};

class SafetyChannelSignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit SafetyChannelSignalsModel(Tuning::TuningDataSourceInfo& sourceInfo, Tuning::TuningService* service, QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const ;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &currentIndex, int role = Qt::DisplayRole) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	Signal& signal(const QModelIndex &index);

public slots:
	void updateSignalStates();
	void updateSignalState(QString appSignalID, double value, double lowLimit, double highLimit, bool validity);
	void changeDiscreteSignal(const QModelIndex& index);

private:
	Tuning::TuningDataSourceInfo& m_sourceInfo;
	QVector<SignalState> m_states;
	Tuning::TuningService* m_service;
	QHash<QString, int> signalIdMap;
};
