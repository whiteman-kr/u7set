#include "TabWidgetEx.h"

TabBarEx::TabBarEx(QWidget* parent) :
	QTabBar(parent)
{
}

void TabBarEx::mouseReleaseEvent(QMouseEvent* event)
{
	QTabBar::mouseReleaseEvent(event);

	if (event->button() != Qt::MouseButton::MidButton)
	{
		return;
	}

	int index = tabAt(event->pos());
	if (index == -1)
	{
		return;
	}

	emit tabCloseRequested(index);
	return;
}

void TabBarEx::paintEvent(QPaintEvent* pe)
{
	QTabBar::paintEvent(pe);

	if (drawTopLine() == false)
	{
		return;
	}

	QPainter p(this);

	int dpiY = p.device()->logicalDpiY();
	int lineWeight = (dpiY > 100) ? 2 : 1;

	int index = currentIndex();

	if (index != -1)
	{
		QRect tabrect = tabRect(index);

		p.setPen(QPen(QBrush{topLineColor()}, lineWeight));
		p.drawLine(tabrect.left(), tabrect.top() + lineWeight / 2,
				   tabrect.right(), tabrect.top() + lineWeight / 2);
	}

	return;
}

bool TabBarEx::drawTopLine() const
{
	return m_drawTopLine;
}

void TabBarEx::setDrawTopLine(bool value)
{
	m_drawTopLine = value;
}

QRgb TabBarEx::topLineColor() const
{
	return m_topLineColor;
}

void TabBarEx::setTopLineColor(QRgb value)
{
	m_topLineColor = value;
}


TabWidgetEx::TabWidgetEx(QWidget* parent) :
	QTabWidget(parent)
{
	setTabBar(new TabBarEx{this});

	setMovable(true);

	tabBar()->setElideMode(Qt::ElideRight);
	setTabsClosable(true);

	QSize sz = fontMetrics().size(Qt::TextSingleLine, "A");
	int h = static_cast<int>(sz.height() * 1.75);

	QString ss = QString(R"(
QTabBar::tab:top
{
height: %1px;
}
QTabBar::close-button
{
image: url(":/Images/Images/CloseButtonGray.svg");
}
QTabBar::close-button:hover
{
image: url(":/Images/Images/CloseButtonBlack.svg");
})")
				 .arg(h);

	setStyleSheet(ss);

	return;
}

TabBarEx* TabWidgetEx::tabBarEx()
{
	TabBarEx* tb = dynamic_cast<TabBarEx*>(tabBar());
	Q_ASSERT(tb);

	return tb;
}

const TabBarEx* TabWidgetEx::tabBarEx() const
{
	const TabBarEx* tb = dynamic_cast<const TabBarEx*>(tabBar());
	Q_ASSERT(tb);

	return tb;
}
