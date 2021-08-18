#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "MeasureBase.h"

// ==============================================================================================

enum ChartType
{
	NoChartType	= -1,
	LinearityEl	= 0,
	LinearityEn	= 1,
	Value20El	= 2,
	Value20En	= 3,
};

const int ChartTypeCount = 4;

#define ERR_GRAPH_TYPE_TYPE(type) (TO_INT(type) < 0 || TO_INT(type) >= ChartTypeCount)

// ==============================================================================================

class ChartView : public QtCharts::QChartView
{

public:

	ChartView(QtCharts::QChart* chart, QWidget* parent = nullptr);

protected:

	bool viewportEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:

	bool m_isTouching = false;
};

// ==============================================================================================

#endif // CHARTVIEW_H
