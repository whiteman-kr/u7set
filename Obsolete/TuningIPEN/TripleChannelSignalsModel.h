#pragma once

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QHash>
#include "TuningIPENService.h"

namespace TuningIPEN
{

	struct SignalProperties
	{
		int signalIndex;
		double currentValue;
		double newValue;
		bool validity;

		SignalProperties(int signalIndex) :
			signalIndex(signalIndex),
			newValue(qQNaN()),
			validity(false)
		{
		}
	};

	struct SourceState
	{
		int index;
		QHash<QString, int> idToSignalIndexMap;
		QHash<QString, int> idToStateIndexMap;
		QList<SignalProperties> signalStates;
	};

	class TripleChannelSignalsDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

	public:
		explicit TripleChannelSignalsDelegate(QObject *parent = Q_NULLPTR);

	signals:
		void aboutToChangeDiscreteSignal(const QModelIndex& index);

	protected:
		bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
	};

	class TripleChannelSignalsModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		explicit TripleChannelSignalsModel(QVector<TuningSourceInfo>& sourceInfo, TuningIPENService* service, QObject *parent = 0);

		int rowCount(const QModelIndex &parent = QModelIndex()) const ;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &currentIndex, int role = Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
		Qt::ItemFlags flags(const QModelIndex & index) const override;

		void addTripleSignal(QVector<QString> ids);

		QVector<const SignalProperties*> state(const QModelIndex& index) const;
		QVector<SignalProperties*> state(const QModelIndex& index);
		QVector<Signal*> signal(const QModelIndex &index) const;

		bool contains(const QString& id);

	public slots:
		void updateSignalStates();
		void updateSignalState(QString appSignalID, double value, double lowLimit, double highLimit, bool validity);
		void changeDiscreteSignal(const QModelIndex& currentIndex);

	private:
		QVector<TuningSourceInfo>& m_sourceInfo;
		TuningIPENService* m_service;
		QHash<QString, int> m_idToChannelMap;
		SourceState m_sourceStates[3];
	};

}
