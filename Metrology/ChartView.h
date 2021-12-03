#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChart>
#include <QChartView>

#include "MeasureBase.h"

// ==============================================================================================

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))	// for Qt5
	using namespace QtCharts;
#endif

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

#define ERR_GRAPH_TYPE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= ChartTypeCount)

// ==============================================================================================

class ChartView : public QChartView
{

public:

	ChartView(QChart* chart, QWidget* parent = nullptr);

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
