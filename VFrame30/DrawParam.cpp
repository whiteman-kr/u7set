#include "Stable.h"
#include "DrawParam.h"
#include <qstring.h>

namespace VFrame30
{
	CDrawParam::CDrawParam(QPainter* painter, Schema* scheme, double gridSize, int pinGridStep) :
		m_painter(painter),
		m_scheme(scheme),
		m_gridSize(gridSize),
		m_pinGridStep(pinGridStep)
	{
		assert(m_painter != nullptr);
		assert(m_scheme != nullptr);
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

	const Schema* CDrawParam::scheme() const
	{
		return m_scheme;
	}

	double CDrawParam::controlBarSize() const
	{
		return m_controlBarSize;
	}

	void CDrawParam::setControlBarSize(double value)
	{
		m_controlBarSize = value;
	}

	double CDrawParam::gridSize() const
	{
		return m_gridSize;
	}

	void CDrawParam::setGridSize(double value)
	{
		m_gridSize = value;
	}

	int CDrawParam::pinGridStep() const
	{
		return m_pinGridStep;
	}

	void CDrawParam::setPinGridStep(int value)
	{
		m_pinGridStep = value;
	}


	void DrawHelper::DrawText(QPainter* painter, const FontParam& font, SchemaUnit unit, const QString& str, const QRectF& rect, int flags)
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
		f.setStyleStrategy(QFont::PreferDevice);

		QRectF rc;

		if (unit == SchemaUnit::Display)
		{
			f.setPointSize(static_cast<int>(font.drawSize()));
			rc = rect;
		}
		else
		{
			assert(unit == SchemaUnit::Inch);
								
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			// FontInfo is required to claculate point to pixels ratio
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

		painter->drawText(rc, flags, str);

		painter->restore();
		return;
	}

//	void DrawHelper::DrawText(QPainter* painter, const FontParam& font, SchemaUnit unit, const QString& str, QPointF point, int flags)
//	{
//		if (painter == nullptr)
//		{
//			assert(painter);
//			return;
//		}

//		if (str.isEmpty())
//		{
//			return;
//		}

//		painter->save();

//		int dpiX = 96;
//		int dpiY = 96;

//		QPaintDevice* pPaintDevice = painter->device();
//		if (pPaintDevice != nullptr)
//		{
//			dpiX = pPaintDevice->logicalDpiX();
//			dpiY = pPaintDevice->logicalDpiY();
//		}
//		else
//		{
//			assert(pPaintDevice);
//		}

//		QFont f(font.name());

//		f.setBold(font.bold());
//		f.setItalic(font.italic());

//		QPointF pt;

//		if (unit == SchemaUnit::Display)
//		{
//			f.setPointSize(static_cast<int>(font.drawSize()));
//			pt = point;
//		}
//		else
//		{
//			assert(unit == SchemaUnit::Inch);

//			painter->scale(1.0 / dpiX, 1.0 / dpiY);

//			// FontInfo is required to claculate point to pixels ratio
//			//
//			QFontInfo fi(f);
//			qreal pointsize = fi.pointSizeF();
//			qreal pixelsize = static_cast<qreal>(fi.pixelSize());
//			double p2p = pixelsize / pointsize;

//			int pixelSize = static_cast<int>(font.drawSize() * dpiY * p2p);
//			f.setPixelSize(pixelSize > 0 ? pixelSize : 1);

//			pt.setX(point.x() * dpiX);
//			pt.setY(point.y() * dpiY);
//		}

//		painter->setFont(f);

//		painter->drawText(pt, str);

//		painter->restore();
//		return;
//	}
	
}
