#ifndef DIALOGTRENDSIGNALPOINTS_H
#define DIALOGTRENDSIGNALPOINTS_H

#include "TrendScale.h"
#include <QAbstractTableModel>
#include <QDialog>
#include <TrendView/TrendSignalSet.h>

namespace Ui
{
	class DialogTrendSignalPoints;
}

//
// TrendPointsModel
//

class TrendPointsModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum class Columns
	{
		Number,
		Record,
		Index,
		Time,
		Value,
		Realtime,
		Count
	};

public:
	explicit TrendPointsModel(QObject* parent = nullptr);

	void setSignalData(std::list<std::shared_ptr<const TrendLib::OneHourData>>& signalData,
					   const TrendLib::TrendSignalParam& trendSignal,
					   E::TimeType timeType);

	int stateItemIndex(const TrendLib::TrendStateItem& stateItem) const;
	TrendLib::TrendStateItem stateItemByIndex(int index, int* oneHourIndex, int* recordIndex, int* stateIndex, bool* ok) const;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


private:
	// m_signalData is always a deep copy even if TREND_ZERO_COPY_TREND_DATA is defined, so it can be accessed without locks.
	//
	std::list<std::shared_ptr<const TrendLib::OneHourData>> m_signalData;

	E::TimeType m_timeType = E::TimeType::Local;
	E::TrendScaleType m_scaleType = E::TrendScaleType::Linear;
	TrendLib::TrendSignalParam m_trendSignal;
	int m_rowCount = 0;
};

//
// DialogTrendSignalPoints
//

class DialogTrendSignalPoints : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTrendSignalPoints(const TrendLib::TrendSignalParam& trendSignal,
									 TrendLib::TrendSignalSet* trendSignalSet,
									 E::TimeType timeType,
									 E::TrendMode trendMode,
									 QWidget* parent = nullptr);
	~DialogTrendSignalPoints();

signals:
	void signalPointsChanged();

private slots:
	void on_buttonAdd_clicked();
	void on_buttonEdit_clicked();
	void on_buttonRemove_clicked();
	void on_comboTimeType_currentIndexChanged(int index);

	void on_tableView_doubleClicked(const QModelIndex& index);

private:
	bool eventFilter(QObject* obj, QEvent* event) override;
	void copySelection();
	void updatePoints();

private:
	Ui::DialogTrendSignalPoints* ui;

	TrendPointsModel m_pointsModel;

	TrendLib::TrendSignalParam m_trendSignal;
	TrendLib::TrendSignalSet* m_trendSignalSet = nullptr;
	E::TimeType m_timeType = E::TimeType::Plant;
	E::TrendMode m_trendMode = E::TrendMode::Archive;

	TrendLib::TrendStateItem m_editStateItem{};
};

#endif // DIALOGTRENDSIGNALPOINTS_H
