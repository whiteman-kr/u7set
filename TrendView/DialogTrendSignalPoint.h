#ifndef DIALOGTRENDSIGNALPOINT_H
#define DIALOGTRENDSIGNALPOINT_H

#include <QDialog>
#include "TrendSignal.h"
#include "TrendScale.h"

namespace Ui {
	class DialogTrendSignalPoint;
}

class DialogTrendSignalPoint : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTrendSignalPoint(std::vector<TrendLib::TrendStateItem>* stateItems,
									E::TimeType timeType,
									TrendLib::TrendSignalParam trendSignal,
									QWidget *parent = nullptr);

	~DialogTrendSignalPoint();

private:
	virtual void accept() override;


private:
	Ui::DialogTrendSignalPoint *ui;

	std::vector<TrendLib::TrendStateItem>* m_stateItems;

	int m_precision = 0;
	E::TimeType m_timeType;
	TrendLib::TrendSignalParam m_trendSignal;
	TrendLib::TrendScaleType m_scaleType = TrendLib::TrendScaleType::Linear;
};

#endif // DIALOGTRENDSIGNALPOINT_H
