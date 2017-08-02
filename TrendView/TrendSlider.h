#ifndef TRENDSLIDER_H
#define TRENDSLIDER_H
#include <QWidget>
#include "TrendRuller.h"
#include "../lib/TimeStamp.h"

class QPushButton;
class TrendSliderRailSubcontrol;

// Keeps time in 100 ms intervals
//
class TrendSlider : public QWidget
{
	Q_OBJECT

public:
	TrendSlider(TrendLib::TrendRullerSet* rullerSet);

signals:
	void valueChanged(TimeStamp value);
	void paramsChanged(qint64 value, qint64 min, qint64 max, qint64 singleStep, qint64 pageStep);

public slots:
	void setTimeClicked();
	void lineLeftClicked();
	void lineRightClicked();
	void sliderRailChanged(qint64 newValue);

public:
	TimeStamp value() const;
	void setValue(const TimeStamp& val);
	void setValueShiftMinMax(const TimeStamp& val);

	TimeStamp max() const;
	void setMax(const TimeStamp& value);

	TimeStamp min() const;
	void setMin(const TimeStamp& value);

	qint64 signleStep() const;
	void setSingleStep(qint64 ms);

	qint64 pageStep() const;
	void setPageStep(qint64 ms);

	qint64 laneDuartion() const;
	void setLaneDuration(qint64 ms);

private:
	qint64 m_value = 0;
	qint64 m_max = 0;
	qint64 m_min = 0;
	qint64 m_singleStep = 1000;
	qint64 m_pageStep = 10000;
	qint64 m_laneDuartion = 0;

	QPushButton* m_setTimeButton = nullptr;
	QPushButton* m_lineLeftButton = nullptr;
	QPushButton* m_lineRightButton = nullptr;
	TrendSliderRailSubcontrol* m_railSubcontrol = nullptr;

	TrendLib::TrendRullerSet* m_rullerSet = nullptr;
};

class TrendSliderRailSubcontrol : public QWidget
{
	Q_OBJECT

public:
	TrendSliderRailSubcontrol(TrendSlider* threndSlider, TrendLib::TrendRullerSet* rullerSet);

signals:
	void valueChanged(qint64 value);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void timerEvent(QTimerEvent*) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;

	QRect sliderRect() const;

protected slots:
	void paramsChanged(qint64 value, qint64 min, qint64 max, qint64 singleStep, qint64 pageStep);

private:
	TrendSlider* m_trendSlider = nullptr;
	TrendLib::TrendRullerSet* m_rullerSet = nullptr;
	qint64 m_value = 0;
	qint64 m_max = 0;
	qint64 m_min = 0;
	qint64 m_singleStep = 0;
	qint64 m_pageStep = 0;

	int m_sliderWidth = 80;

	bool m_lastDrawHover = false;
	int m_railLastMousePos = 0;
	qint64 m_railPressMouseValue = 0;
};

#endif // TRENDSLIDER_H
