#include "DrawParam.h"

namespace VFrame30
{
	CDrawParam::CDrawParam(QPainter* painter, Schema* schema, const SchemaView* view, double gridSize, int pinGridStep) :
		m_painter(painter),
		m_schema(schema),
		m_schemaView(view),
		m_gridSize(gridSize),
		m_pinGridStep(pinGridStep)
	{
		assert(m_painter != nullptr);
		assert(m_schema != nullptr);

		if (dynamic_cast<const QPdfWriter*>(painter->device()) != nullptr)
		{
			const QPdfWriter* pdfDevice = dynamic_cast<const QPdfWriter*>(painter->device());

			if (pdfDevice->resolution() >= 600)
			{
				m_cosmeticPenWidth = 1.0 / 256.0;
			}
			else
			{
				m_cosmeticPenWidth = 0.0;
			}
		}

		return;
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

	const Schema* CDrawParam::schema() const
	{
		return m_schema;
	}

	const SchemaView* CDrawParam::schemaView() const
	{
		return m_schemaView;
	}

	SchemaView* CDrawParam::schemaView()
	{
		return const_cast<SchemaView*>(m_schemaView);
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

	double CDrawParam::cosmeticPenWidth() const
	{
		return m_cosmeticPenWidth;
	}

	int CDrawParam::dpiX()
	{
		if (m_dpiX == -1)
		{
			if (m_painter != nullptr && m_painter->device() != nullptr)
			{
				m_dpiX = m_painter->device()->physicalDpiX();
			}
			else
			{
				assert(m_painter);
				m_dpiX = 96;
			}
		}

		return m_dpiX;
	}

	int CDrawParam::dpiY()
	{
		if (m_dpiY == -1)
		{
			if (m_painter != nullptr && m_painter->device() != nullptr)
			{
				m_dpiY = m_painter->device()->physicalDpiY();
			}
			else
			{
				assert(m_painter);
				m_dpiY = 96;
			}
		}

		return m_dpiY;
	}

	bool CDrawParam::isEditMode() const
	{
		return m_isEditMode;
	}

	void CDrawParam::setEditMode(bool value)
	{
		m_isEditMode = value;
	}

	bool CDrawParam::isMonitorMode() const
	{
		return !isEditMode();
	}

	void CDrawParam::setMonitorMode(bool value)
	{
		setEditMode(!value);
	}

	bool CDrawParam::infoMode() const
	{
		return m_infoMode;
	}

	void CDrawParam::setInfoMode(bool value)
	{
		m_infoMode = value;
	}

	AppSignalManager* CDrawParam::appSignalManager()
	{
		return m_appSignalmanager;
	}

	void CDrawParam::setAppSignalManager(AppSignalManager* value)
	{
		m_appSignalmanager = value;
	}

	const Session& CDrawParam::session() const
	{
		return m_session;
	}

	Session& CDrawParam::session()
	{
		return m_session;
	}

	void DrawHelper::drawText(QPainter* painter, const FontParam& font, SchemaUnit unit, const QString& str, const QRectF& rect, int flags, QRectF* boundingRect/* = nullptr*/)
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

		double dpiX = 96;
		double dpiY = 96;

		QPaintDevice* pPaintDevice = painter->device();
		if (pPaintDevice != nullptr)
		{
			dpiX = pPaintDevice->physicalDpiX();
			dpiY = pPaintDevice->physicalDpiY();
		}
		else
		{
			assert(pPaintDevice);
		}

		QFont f(font.name());

		f.setBold(font.bold());
		f.setItalic(font.italic());
		//f.setStyleStrategy(QFont::PreferDevice);

		QRectF rc;

		if (unit == SchemaUnit::Display)
		{
			f.setPixelSize(static_cast<int>(font.drawSize()));
			rc = rect;
		}
		else
		{
			assert(unit == SchemaUnit::Inch);
								
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			int pixelSize = static_cast<int>(font.drawSize() * dpiY);
			f.setPixelSize(pixelSize > 0 ? pixelSize : 1);

			rc.setLeft(rect.left() * dpiX);
			rc.setTop(rect.top() * dpiY);
			rc.setRight(rect.right() * dpiX);
			rc.setBottom(rect.bottom() * dpiY);
		}

		painter->setFont(f);

		painter->drawText(rc, flags, str, boundingRect);

		painter->restore();
		return;
	}
	
}
