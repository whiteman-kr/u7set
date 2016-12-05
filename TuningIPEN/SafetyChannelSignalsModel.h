#pragma once

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QHash>
#include "TuningIPENService.h"

namespace TuningIPEN
{

	class TripleChannelSignalsModel;

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

	public:
		explicit SafetyChannelSignalsDelegate(QObject *parent = Q_NULLPTR);

	signals:
		void aboutToChangeDiscreteSignal(const QModelIndex& index);

	protected:
		bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
	};

	class SafetyChannelSignalsModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		explicit SafetyChannelSignalsModel(TuningIPEN::TuningSourceInfo& sourceInfo, TuningIPEN::TuningIPENService* service, QObject *parent = 0);

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
		TuningIPEN::TuningSourceInfo& m_sourceInfo;
		QVector<SignalState> m_states;
		TuningIPEN::TuningIPENService* m_service;
		QHash<QString, int> signalIdMap;
	};

	class SafetyChannelSignalsProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		SafetyChannelSignalsProxyModel(TripleChannelSignalsModel* tripleSignalModel, SafetyChannelSignalsModel* sourceModel, QObject* parent = 0);

		bool filterAcceptsRow(int source_row, const QModelIndex&) const override;

	private:
		TripleChannelSignalsModel* m_tripleSignalModel;
		SafetyChannelSignalsModel* m_sourceModel;
	};

}
