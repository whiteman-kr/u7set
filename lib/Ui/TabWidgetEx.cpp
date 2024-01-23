#include "TabWidgetEx.h"

TabBarEx::TabBarEx(QWidget* parent) :
	QTabBar(parent)
{
}

void TabBarEx::mouseReleaseEvent(QMouseEvent* event)
{
	QTabBar::mouseReleaseEvent(event);

	if (event->button() != Qt::MouseButton::MiddleButton)
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

	QPainter painter(this);
	painter.setRenderHint(QPainter::RenderHint::Antialiasing, true);

	int dpiY = painter.device()->logicalDpiY();
	int dpiX = painter.device()->logicalDpiX();
	int currentTabIndex = currentIndex();

	switch (m_style)
	{
	case Style::Default:
		break;
	case Style::TopLineActive:
		{
			if (currentTabIndex != -1)
			{
				QRect tabrect = tabRect(currentTabIndex);

				qreal lineWeight = (dpiY > 100) ? 2 : 1;

				QPen pen{QBrush{topLineColor()}, lineWeight};
				painter.setPen(pen);

				painter.drawLine(tabrect.left(), tabrect.top() + lineWeight, tabrect.right(), tabrect.top() + lineWeight);
			}
		}
		break;
	case Style::TopLineRoundedAlways:
		{
			qreal lineWeight = dpiY / 36; // from inch
			double margin = dpiX / 16;
			
			QPen pen{QBrush{topLineColor()}, lineWeight};
			pen.setCapStyle(Qt::PenCapStyle::RoundCap);
			painter.setPen(pen);

			for (int i = 0; i < count(); i++)
			{
				auto colorIt = m_tabTextToLineColor.find(tabText(i));
				if (colorIt != m_tabTextToLineColor.end())
				{
					QColor c = colorIt->second;
					c.setAlpha(qAlpha(colorIt->second));

					if (i != currentTabIndex)
					{
						// Dim not active tab page.
						//
						c.setAlpha(80);
					}

					pen.setColor(c);
				}
				else
				{
					pen.setColor(Qt::transparent);
				}

				QRectF tabrect = tabRect(i).toRectF();

				painter.setPen(pen);
				
				if (i == currentTabIndex)
				{
					std::array<QPointF, 4> points = {
						QPointF{tabrect.left(), tabrect.bottom() - lineWeight},
						QPointF{tabrect.left(), tabrect.top() + lineWeight},
						QPointF{tabrect.right(), tabrect.top() + lineWeight},
						QPointF{tabrect.right(), tabrect.bottom() - lineWeight},
					};

					if (i == count() - 1)
					{
						// The last tab needs to dbe corrected (move the right line slightly right).
						//
						points[2].rx() -= lineWeight;
						points[3].rx() -= lineWeight;
					}

					painter.drawPolyline(points.data(), std::ssize(points));
				}
				else
				{
					painter.drawLine(tabrect.left() + margin, tabrect.top() + lineWeight, tabrect.right() - margin, tabrect.top() + lineWeight);
				}
			}
		}
		break;
	}

	return;
}

TabBarEx::Style TabBarEx::topStyle() const
{
	return m_style;
}

void TabBarEx::setTopStyle(TabBarEx::Style value)
{
	m_style = value;
}

QRgb TabBarEx::topLineColor() const
{
	return m_topLineColor;
}

void TabBarEx::setTopLineColor(QRgb value)
{
	m_topLineColor = value;
}

void TabBarEx::setTabColors(std::map<QString, QRgb> tabTextToLineColor)
{
	m_tabTextToLineColor = std::move(tabTextToLineColor);
}

TabWidgetEx::TabWidgetEx(QWidget* parent) :
	QTabWidget(parent)
{
	setTabBar(new TabBarEx{this});

	setMovable(true);

	tabBar()->setElideMode(Qt::ElideRight);
	setTabsClosable(true);

	QString ss = QString(R"(
QTabBar::close-button
{
	image: url(":/Images/Images/CloseButtonGray.svg");
}
QTabBar::close-button:hover
{
	image: url(":/Images/Images/CloseButtonBlack.svg");
})");

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
