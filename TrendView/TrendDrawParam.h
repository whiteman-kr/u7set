#ifndef DRAWPARAM_H
#define DRAWPARAM_H

namespace TrendLib
{
	enum class TrendView
	{
		Separated,
		Overlapped
	};
}
Q_DECLARE_METATYPE(TrendLib::TrendView)

namespace TrendLib
{
	class TrendDrawParam
	{
	public:
		TrendDrawParam();

	public:
		QRect rect() const;
		void setRect(const QRect& value);

		int dpiX() const;
		int dpiY() const;
		void setDpi(int dpiX, int dpiY);

		TrendView view() const;
		void setView(TrendView value);

		int laneCount() const;
		void setLaneCount(int value);

		QColor backgroundColor() const;

	private:
		QRect m_rect;
		int m_dpiX = 96;
		int m_dpiY = 96;

		TrendView m_view = TrendView::Separated;
		int m_laneCount = 1;

		QColor m_backgroundColor = {qRgb(0xE0, 0xE0, 0xE0)};
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendDrawParam)

#endif // DRAWPARAM_H
