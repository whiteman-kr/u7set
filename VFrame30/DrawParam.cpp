#include "Stable.h"
#include "DrawParam.h"
#include <qstring.h>

namespace VFrame30
{
	CDrawParam::CDrawParam()
	{
		assert(false);
	}

	CDrawParam::CDrawParam(QPainter* painter) :
		m_painter(painter),
		m_controlBarSize(0)
	{
		assert(m_painter != nullptr);
	}
	
	CDrawParam::~CDrawParam(void)
	{
	}

	QPainter* CDrawParam::painter()
	{
		return m_painter;
	}

	QPaintDevice* CDrawParam::device()
	{
		return m_painter->device();
	}

	double CDrawParam::controlBarSize() const
	{
		return m_controlBarSize;
	}

	void CDrawParam::setControlBarSize(double value)
	{
		m_controlBarSize = value;
	}

	void DrawHelper::DrawText(QPainter* painter, const FontParam& font, SchemeUnit unit, const QString& str, const QRectF& rect)
	{
		if (painter == nullptr)
		{
			assert(painter);
			return;
		}

		if (str.isEmpty())
		{
			return;
		}

		painter->save();

		int dpiX = 96;
		int dpiY = 96;

		QPaintDevice* pPaintDevice = painter->device();
		if (pPaintDevice != nullptr)
		{
			dpiX = pPaintDevice->logicalDpiX();
			dpiY = pPaintDevice->logicalDpiY();
		}
		else
		{
			assert(pPaintDevice);
		}

		QFont f(font.name());

		f.setBold(font.bold());
		f.setItalic(font.italic());

		QRectF rc;

		if (unit == SchemeUnit::Display)
		{
			f.setPointSize(static_cast<int>(font.drawSize()));
			rc = rect;
		}
		else
		{
			assert(unit == SchemeUnit::Inch);
								
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			// Фонт инфо требуется для вычисления соотношения между Point и Pixels
			//
			QFontInfo fi(f);
			qreal pointsize = fi.pointSizeF();
			qreal pixelsize = static_cast<qreal>(fi.pixelSize());
			double p2p = pixelsize / pointsize;

			int pixelSize = static_cast<int>(font.drawSize() * dpiY * p2p);
			f.setPixelSize(pixelSize > 0 ? pixelSize : 1);

			rc.setLeft(rect.left() * dpiX);
			rc.setTop(rect.top() * dpiY);
			rc.setRight(rect.right() * dpiX);
			rc.setBottom(rect.bottom() * dpiY);
		}

		painter->setFont(f);

		painter->drawText(rc, str);

		painter->restore();
		return;
	}
	
}
