#include "DrawParam.h"
#include "ClientSchemaView.h"


namespace VFrame30
{
	CDrawParam::CDrawParam(QPainter* painter, const SchemaView* view, double gridSize, int pinGridStep, SchemaUnit schemaUnit) :
		m_painter(painter),
		m_schemaView(view),
		m_session(view ? view->session() : Session{}),
		m_schemaUnit(schemaUnit),
		m_gridSize(gridSize),
		m_pinGridStep(pinGridStep)
	{
		Q_ASSERT(m_painter != nullptr);

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
	
	QPainter* CDrawParam::painter()
	{
		return m_painter;
	}

	QPaintDevice* CDrawParam::device()
	{
		return m_painter->device();
	}

	const QPaintDevice* CDrawParam::device() const
	{
		return m_painter->device();
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
		Q_ASSERT(drawMode() != DrawMode::Editor);
		return dynamic_cast<const ClientSchemaView*>(m_schemaView);
	}

	ClientSchemaView* CDrawParam::clientSchemaView()
	{
		Q_ASSERT(drawMode() != DrawMode::Editor);
		auto ptr = dynamic_cast<const ClientSchemaView*>(m_schemaView);
		return const_cast<ClientSchemaView*>(ptr);
	}

	ITimeStats* CDrawParam::timeStats()
	{
		auto c = clientSchemaView();
		return c ? c->timeStats() : nullptr;
	}

	ITimeStats* CDrawParam::timeStats() const
	{
		auto c = clientSchemaView();
		return c ? c->timeStats() : nullptr;
	}

	SchemaUnit CDrawParam::schemaUnit() const
	{
		return m_schemaUnit;
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

	double CDrawParam::realDpiX() const noexcept
	{
		return m_painter->device()->physicalDpiX() * m_painter->device()->devicePixelRatioF();
	}

	double CDrawParam::realDpiY() const noexcept
	{
		return m_painter->device()->physicalDpiY() * m_painter->device()->devicePixelRatioF();
	}

	double CDrawParam::devicePixelRatio() const noexcept
	{
		return m_painter->device()->devicePixelRatioF();
	}

	double CDrawParam::realDpiX(QPainter* painter) noexcept
	{
		return painter->device()->physicalDpiX() * painter->device()->devicePixelRatioF();
	}

	double CDrawParam::realDpiY(QPainter* painter) noexcept
	{
		return painter->device()->physicalDpiY() * painter->device()->devicePixelRatioF();
	}

	double CDrawParam::gridToDpiX(double pos) const noexcept
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return pos;
		}

		const double zoom = schemaView()->zoom() / 100.0;

		if (m_schemaUnit == SchemaUnit::Display)
		{
			return (double)qRound(pos * zoom) / zoom;
		}

		if (m_schemaUnit == SchemaUnit::Inch)
		{
			double dpix = this->realDpiX();
			return (static_cast<double>(static_cast<int>(pos * zoom * dpix)) / dpix) / zoom;
		}

		Q_ASSERT(false);
		return pos;

	}

	double CDrawParam::gridToDpiY(double pos) const noexcept
	{
		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return pos;
		}

		const double zoom = schemaView()->zoom() / 100.0;

		if (m_schemaUnit == SchemaUnit::Display)
		{
			return (double)qRound(pos * zoom) / zoom;
		}

		if (m_schemaUnit == SchemaUnit::Inch)
		{
			const double dpiy = this->realDpiY();
			return (static_cast<double>(static_cast<int>(pos * zoom * dpiy)) / dpiy) / zoom;
		}

		Q_ASSERT(false);
		return pos;
	}

	QPointF CDrawParam::gridToDpi(double x, double y) const noexcept
	{
		QPointF result;

		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			result.setX(x);
			result.setY(y);
			return result;
		}

		const double zoom = schemaView()->zoom() / 100.0;

		if (m_schemaUnit == SchemaUnit::Display)
		{
			result = QPointF((double)qRound(x * zoom) / zoom,
							 (double)qRound(y * zoom) / zoom);
		}
		else
		{
			Q_ASSERT(m_schemaUnit == SchemaUnit::Inch);

			const double dpix = this->realDpiX();
			const double dpiy = this->realDpiY();

			result = QPointF((static_cast<double>(static_cast<int>(x * zoom * dpix)) / dpix) / zoom,
							 (static_cast<double>(static_cast<int>(y * zoom * dpiy)) / dpiy) / zoom);
		}

		return result;
	}

	QPointF CDrawParam::gridToDpi(const QPointF& pos) const noexcept
	{
		QPointF result = pos;

		if (schemaView() == nullptr)
		{
			Q_ASSERT(schemaView() != nullptr);
			return result;
		}

		const double zoom = schemaView()->zoom() / 100.0;

		if (m_schemaUnit == SchemaUnit::Display)
		{
			result = QPointF((double)qRound(pos.x() * zoom) / zoom,
							 (double)qRound(pos.y() * zoom) / zoom);
		}
		else
		{
			Q_ASSERT(m_schemaUnit == SchemaUnit::Inch);

			const double dpix = this->realDpiX();
			const double dpiy = this->realDpiY();

			result = QPointF((static_cast<double>(static_cast<int>(pos.x() * zoom * dpix)) / dpix) / zoom,
							 (static_cast<double>(static_cast<int>(pos.y() * zoom * dpiy)) / dpiy) / zoom);
		}

		return result;
	}

	QRectF CDrawParam::gridToDpi(const QRectF& rect) const noexcept
	{
		return QRectF{gridToDpi(rect.topLeft()), gridToDpi(rect.bottomRight())};
	}

	DrawMode CDrawParam::drawMode() const noexcept
	{
		return m_schemaView->drawMode();
	}

	bool CDrawParam::infoMode() const noexcept
	{
		return m_infoMode;
	}

	void CDrawParam::setInfoMode(bool value)
	{
		m_infoMode = value;
	}

	bool CDrawParam::pdfMode() const noexcept
	{
		return m_pdfMode;
	}

	void CDrawParam::setPdfMode(bool value)
	{
		m_pdfMode = value;
	}

	bool CDrawParam::blinkPhase() const noexcept
	{
		return m_blinkPhase;
	}

	void CDrawParam::setBlinkPhase(bool value)
	{
		m_blinkPhase = value;
	}

	bool CDrawParam::drawNotesLayer() const noexcept
	{
		return m_drawNotesLayer;
	}

	void CDrawParam::setDrawNotesLayer(bool value)
	{
		m_drawNotesLayer = value;
	}

	const Session& CDrawParam::session() const noexcept
	{
		return m_session;
	}

	Session& CDrawParam::session()
	{
		return m_session;
	}

	const MonitorBehavior& CDrawParam::monitorBehavor() const noexcept
	{
		Q_ASSERT(m_schemaView);
		return clientSchemaView()->monitorBehavor();
	}

	const TuningClientBehavior& CDrawParam::tuningClientBehavior() const
	{
		Q_ASSERT(m_schemaView);
		return clientSchemaView()->tuningClientBehavior();
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

		QFont f(font.name());

		f.setBold(font.bold());
		f.setItalic(font.italic());
		//f.setStyleStrategy(QFont::StyleStrategy::NoAntialias);
		//f.setStyleStrategy(QFont::PreferDevice);

		if (unit == SchemaUnit::Display)
		{
			f.setPixelSize(static_cast<int>(font.drawSize()));

			painter->setFont(f);
			painter->drawText(rect, flags, str, boundingRect);
		}
		else
		{
			Q_ASSERT(unit == SchemaUnit::Inch);

			QRectF rc;
			const double dpiX = CDrawParam::realDpiX(painter);
			const double dpiY = CDrawParam::realDpiY(painter);

			painter->save();
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			int pixelSize = static_cast<int>(font.drawSize() * dpiY);
			f.setPixelSize(pixelSize > 0 ? pixelSize : 1);

			rc.setLeft(rect.left() * dpiX);
			rc.setTop(rect.top() * dpiY);
			rc.setRight(rect.right() * dpiX);
			rc.setBottom(rect.bottom() * dpiY);

			painter->setFont(f);
			painter->drawText(rc, flags, str, boundingRect);

			painter->restore();
		}

		return;
	}

	void DrawHelper::drawText(QPainter* painter, SchemaUnit unit, const QString& str, const QRectF& rect, int flags, QRectF* boundingRect/* = nullptr*/)
	{
		if (painter == nullptr || str.isEmpty() == true)
		{
			Q_ASSERT(painter);
			return;
		}

		if (unit == SchemaUnit::Display)
		{
			painter->drawText(rect, flags, str, boundingRect);
		}
		else
		{
			Q_ASSERT(unit == SchemaUnit::Inch);

			const double dpiX = CDrawParam::realDpiX(painter);
			const double dpiY = CDrawParam::realDpiY(painter);

			painter->save();
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			QRectF rc;
			rc.setLeft(rect.left() * dpiX);
			rc.setTop(rect.top() * dpiY);
			rc.setRight(rect.right() * dpiX);
			rc.setBottom(rect.bottom() * dpiY);

			painter->drawText(rc, flags, str, boundingRect);
			painter->restore();
		}

		return;
	}

}
