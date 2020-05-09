#pragma once
#include <QTabWidget>

// TabWidgetEx is extended version of QTabWidget
//
// 1. Draw line on top of tab button
// 2. Closable by middle mouse button press
//

class TabBarEx : public QTabBar
{
	Q_OBJECT

public:
	TabBarEx(QWidget* parent);

protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* pe) override;

public:
	bool drawTopLine() const;
	void setDrawTopLine(bool value);

	QRgb topLineColor() const;
	void setTopLineColor(QRgb value);

private:
	bool m_drawTopLine = true;
	QRgb m_topLineColor = 0x000080;
};


class TabWidgetEx : public QTabWidget
{
	Q_OBJECT

public:
	TabWidgetEx(QWidget* parent);

	TabBarEx* tabBarEx();
	const TabBarEx* tabBarEx() const;
};
