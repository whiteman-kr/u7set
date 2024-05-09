#pragma once
#include <QTabBar>

// TabWidgetEx is extended version of QTabWidget
//
// 1. Draw line on top of tab button
// 2. Closable by middle mouse button press
//

namespace UiLib
{
	class TabBarEx : public QTabBar
	{
		Q_OBJECT

	public:
		TabBarEx(QWidget* parent);

	protected:
		void mouseReleaseEvent(QMouseEvent* event) override;
		void paintEvent(QPaintEvent* pe) override;

	public:
		enum Style
		{
			Default,
			TopLineActive,
			TopLineRoundedAlways
		};

		[[nodiscard]] TabBarEx::Style topStyle() const;
		void setTopStyle(TabBarEx::Style value);

		[[nodiscard]] QRgb topLineColor() const;
		void setTopLineColor(QRgb value);

		void setTabColors(std::map<QString, QRgb> tabTextToLineColor);

	private:
		Style m_style = Style::TopLineActive;
		QRgb m_topLineColor = 0x000080;

		std::map<QString, QRgb> m_tabTextToLineColor;
	};


	class TabWidgetEx : public QTabWidget
	{
		Q_OBJECT

	public:
		TabWidgetEx(QWidget* parent);

		[[nodiscard]] TabBarEx* tabBarEx();
		[[nodiscard]] const TabBarEx* tabBarEx() const;
	};
} // namespace UiLib