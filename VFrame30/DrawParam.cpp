#include "DrawParam.h"
#include "Schema.h"
#include "ClientSchemaView.h"


namespace VFrame30
{
	CDrawParam::CDrawParam(QPainter* painter, Schema* schema, const SchemaView* view, double gridSize, int pinGridStep) :
		m_painter(painter),
		m_schema(schema),
		m_schemaView(view),
		m_gridSize(gridSize),
		m_pinGridStep(pinGridStep)
	{
		Q_ASSERT(m_painter != nullptr);
		Q_ASSERT(m_schema != nullptr);

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

	const ClientSchemaView* CDrawParam::clientSchemaView() const
	{
		Q_ASSERT(isMonitorMode());
		return dynamic_cast<const ClientSchemaView*>(m_schemaView);
	}

	ClientSchemaView* CDrawParam::clientSchemaView()
	{
		Q_ASSERT(isMonitorMode());
		auto ptr = dynamic_cast<const ClientSchemaView*>(m_schemaView);
		return const_cast<ClientSchemaView*>(ptr);
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

	int CDrawParam::dpiX() const
	{
		if (m_dpiX == -1)
		{
			CDrawParam* mutable_this = const_cast<CDrawParam*>(this);

			if (m_painter != nullptr && m_painter->device() != nullptr)
			{
				mutable_this->m_dpiX = m_painter->device()->physicalDpiX();
			}
			else
			{
				Q_ASSERT(m_painter);
				mutable_this->m_dpiX = 96;
			}
		}

		return m_dpiX;
	}

	int CDrawParam::dpiY() const
	{
		if (m_dpiY == -1)
		{
			CDrawParam* mutable_this = const_cast<CDrawParam*>(this);

			if (m_painter != nullptr && m_painter->device() != nullptr)
			{
				mutable_this->m_dpiY = m_painter->device()->physicalDpiY();
			}
			else
			{
				Q_ASSERT(m_painter);
				mutable_this->m_dpiY = 96;
			}
		}

		return m_dpiY;
	}

	double CDrawParam::gridToDpiX(double pos) const
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return pos;
		}

		double zoom = schemaView()->zoom() / 100.0;

		if (schema()->unit() == SchemaUnit::Display)
		{
			return (double)qRound(pos * zoom) / zoom;
		}

		if (schema()->unit() == SchemaUnit::Inch)
		{
			int dpix = this->dpiX();
			return (static_cast<double>(static_cast<int>(pos * zoom * dpix)) / dpix) / zoom;
		}

		Q_ASSERT(false);
		return pos;

	}

	double CDrawParam::gridToDpiY(double pos) const
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return pos;
		}

		double zoom = schemaView()->zoom() / 100.0;

		if (schema()->unit() == SchemaUnit::Display)
		{
			return (double)qRound(pos * zoom) / zoom;
		}

		if (schema()->unit() == SchemaUnit::Inch)
		{
			int dpiy = this->dpiY();
			return (static_cast<double>(static_cast<int>(pos * zoom * dpiy)) / dpiy) / zoom;
		}

		Q_ASSERT(false);
		return pos;
	}

	QPointF CDrawParam::gridToDpi(double x, double y) const
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return QPointF(x, y);
		}

		double zoom = schemaView()->zoom() / 100.0;

		if (schema()->unit() == SchemaUnit::Display)
		{
			return QPointF((double)qRound(x * zoom) / zoom,
						   (double)qRound(y * zoom) / zoom);
		}

		if (schema()->unit() == SchemaUnit::Inch)
		{
			int dpix = this->dpiX();
			int dpiy = this->dpiY();

			return QPointF((static_cast<double>(static_cast<int>(x * zoom * dpix)) / dpix) / zoom,
						   (static_cast<double>(static_cast<int>(y * zoom * dpiy)) / dpiy) / zoom);
		}

		Q_ASSERT(false);
		return QPointF(x, y);
	}

	QPointF CDrawParam::gridToDpi(QPointF pos) const
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return pos;
		}

		double zoom = schemaView()->zoom() / 100.0;

		if (schema()->unit() == SchemaUnit::Display)
		{
			return QPointF((double)qRound(pos.x() * zoom) / zoom,
						   (double)qRound(pos.y() * zoom) / zoom);
		}

		if (schema()->unit() == SchemaUnit::Inch)
		{
			int dpix = this->dpiX();
			int dpiy = this->dpiY();

			return QPointF((static_cast<double>(static_cast<int>(pos.x() * zoom * dpix)) / dpix) / zoom,
						   (static_cast<double>(static_cast<int>(pos.y() * zoom * dpiy)) / dpiy) / zoom);
		}

		Q_ASSERT(false);
		return pos;
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

	bool CDrawParam::blinkPhase() const
	{
		return m_blinkPhase;
	}

	void CDrawParam::setBlinkPhase(bool value)
	{
		m_blinkPhase = value;
	}

	bool CDrawParam::drawNotesLayer() const
	{
		return m_drawNotesLayer;
	}

	void CDrawParam::setDrawNotesLayer(bool value)
	{
		m_drawNotesLayer = value;
	}

	AppSignalController* CDrawParam::appSignalController()
	{
		return m_appSignalController;
	}

	void CDrawParam::setAppSignalController(AppSignalController* value)
	{
		m_appSignalController = value;
	}

	TuningController* CDrawParam::tuningController()
	{
		return m_tuningController;
	}

	void CDrawParam::setTuningController(TuningController* value)
	{
		m_tuningController = value;
	}

	const Session& CDrawParam::session() const
	{
		return m_session;
	}

	Session& CDrawParam::session()
	{
		return m_session;
	}

	const QStringList& CDrawParam::hightlightIds() const
	{
		return m_highlightIds;
	}

	void CDrawParam::setHightlightIds(const QStringList& value)
	{
		m_highlightIds = value;
	}

	void DrawHelper::drawText(QPainter* painter, const FontParam& font, SchemaUnit unit, const QString& str, const QRectF& rect, int flags, QRectF* boundingRect/* = nullptr*/)
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
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
			Q_ASSERT(pPaintDevice);
		}

		QFont f(font.name());

		f.setBold(font.bold());
		f.setItalic(font.italic());
		//f.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
		//f.setStyleStrategy(QFont::PreferDevice);

		QRectF rc;

		if (unit == SchemaUnit::Display)
		{
			f.setPixelSize(static_cast<int>(font.drawSize()));
			rc = rect;
		}
		else
		{
			Q_ASSERT(unit == SchemaUnit::Inch);
								
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

	void DrawHelper::drawText(QPainter* painter, SchemaUnit unit, const QString& str, const QRectF& rect, int flags, QRectF* boundingRect/* = nullptr*/)
	{
		if (painter == nullptr || str.isEmpty() == true)
		{
			Q_ASSERT(painter);
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
			Q_ASSERT(pPaintDevice);
		}

		QRectF rc;

		if (unit == SchemaUnit::Display)
		{
			rc = rect;
		}
		else
		{
			Q_ASSERT(unit == SchemaUnit::Inch);

			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			rc.setLeft(rect.left() * dpiX);
			rc.setTop(rect.top() * dpiY);
			rc.setRight(rect.right() * dpiX);
			rc.setBottom(rect.bottom() * dpiY);
		}

		painter->drawText(rc, flags, str, boundingRect);
		painter->restore();

		return;
	}

}
