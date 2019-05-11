#ifndef DIALOGTRENDSIGNALPOINT_H
#define DIALOGTRENDSIGNALPOINT_H

#include <QDialog>
#include "TrendSignal.h"

namespace Ui {
	class DialogTrendSignalPoint;
}

class DialogTrendSignalPoint : public QDialog
{
	Q_OBJECT

public:
	explicit DialogTrendSignalPoint(std::vector<TrendLib::TrendStateItem>* stateItems, E::TimeType timeType, E::SignalType signalType, int precision, QWidget *parent = nullptr);

	~DialogTrendSignalPoint();

private:
	virtual void accept() override;


private:
	Ui::DialogTrendSignalPoint *ui;

	std::vector<TrendLib::TrendStateItem>* m_stateItems;

	int m_precision = 0;
	E::TimeType m_timeType;
	E::SignalType m_signalType;
};

#endif // DIALOGTRENDSIGNALPOINT_H
