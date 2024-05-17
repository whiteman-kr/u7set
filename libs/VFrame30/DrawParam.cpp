#include <VFrame30/DrawParam.h>
#include <VFrame30/ClientSchemaView.h>

#include <Behavior/MonitorBehavior.h>
#include <Behavior/TuningClientBehavior.h>
#include <QSvgRenderer>

namespace
{
	struct DrawTextCacheItem
	{
		DrawTextCacheItem(const QFont& font, SchemaUnit unit, const QString& text, QSize size, int flags, QRgb textColor, double dpiX, double dpiY, double zoom) :
			font{font},
			unit{unit},
			text{text},
			size{size},
			flags{flags},
			textColor{textColor},
			dpiX{dpiX},
			dpiY{dpiY},
			zoom{zoom}
		{
		}

		const QFont font{};
		const SchemaUnit unit{};
		const QString text;
		const QSize size;
		int flags{};
		QRgb textColor{};
		double dpiX{};
		double dpiY{};
		double zoom{};

		QImage image{};

		Hash hash()
		{
			return getHash(font, unit, text, size, flags, textColor, dpiX, dpiY, zoom);
		}

		static Hash getHash(const QFont& font, SchemaUnit unit, const QString& text, QSize size,
							int flags, QRgb textColor, double dpiX, double dpiY, double zoom)
		{
			Hash result = ::calcHash(font.key(), 0);
			result = ::calcHash(&unit, sizeof(unit), result);
			result = ::calcHash(text, result);
			result = ::calcHash(size, result);
			result = ::calcHash(&flags, sizeof(flags), result);
			result = ::calcHash(&textColor, sizeof(textColor), result);
			result = ::calcHash(&dpiX, sizeof(dpiX), result);
			result = ::calcHash(&dpiY, sizeof(dpiY), result);
			result = ::calcHash(&zoom, sizeof(zoom), result);

			return result;
		}
	};

	struct DrawSvgCacheItem
	{
		DrawSvgCacheItem(SchemaUnit unit, const QString& svg, QSize size, double dpiX, double dpiY, double zoom) :
			unit{unit},
			svg{svg},
			size{size},
			dpiX{dpiX},
			dpiY{dpiY},
			zoom{zoom}
		{
		}

		const SchemaUnit unit{};
		const QString svg;
		const QSize size;
		double dpiX{};
		double dpiY{};
		double zoom{};

		QImage image{};

		Hash hash()
		{
			return getHash(unit, svg, size, dpiX, dpiY, zoom);
		}

		static Hash getHash(SchemaUnit unit, const QString& svg, QSize size, double dpiX, double dpiY, double zoom)
		{
			Hash result = ::calcHash(&unit, sizeof(unit));
			result = ::calcHash(svg, result);
			result = ::calcHash(size, result);
			result = ::calcHash(&dpiX, sizeof(dpiX), result);
			result = ::calcHash(&dpiY, sizeof(dpiY), result);
			result = ::calcHash(&zoom, sizeof(zoom), result);

			return result;
		}
	};
}


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

	VFrame30::ITimeStats* CDrawParam::timeStats()
	{
		if (drawMode() == DrawMode::Editor)
		{
			return nullptr;
		}

		auto c = clientSchemaView();
		return c ? c->timeStats() : nullptr;
	}

	VFrame30::ITimeStats* CDrawParam::timeStats() const
	{
		if (drawMode() == DrawMode::Editor)
		{
			return nullptr;
		}

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

	const Behavior::MonitorBehavior& CDrawParam::monitorBehavior() const noexcept
	{
		Q_ASSERT(clientSchemaView());
		Q_ASSERT(clientSchemaView()->monitorBehavior() != nullptr);

		return *clientSchemaView()->monitorBehavior();
	}

	const Behavior::TuningClientBehavior& CDrawParam::tuningClientBehavior() const
	{
		Q_ASSERT(clientSchemaView());
		Q_ASSERT(clientSchemaView()->tuningClientBehavior());

		return *clientSchemaView()->tuningClientBehavior();
	}

	const QStringList& CDrawParam::highlightIds() const
	{
		return m_highlightIds;
	}

	void CDrawParam::setHighlightIds(const QStringList& value)
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

		const double dpiX = CDrawParam::realDpiX(painter);
		const double dpiY = CDrawParam::realDpiY(painter);

		QFont f = font.qfont(unit, dpiY);

		if (unit == SchemaUnit::Display)
		{
			painter->setFont(f);
			painter->drawText(rect, flags, str, boundingRect);
		}
		else
		{
			Q_ASSERT(unit == SchemaUnit::Inch);

			painter->save();
			painter->scale(1.0 / dpiX, 1.0 / dpiY);

			QRectF rc;
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

	void DrawHelper::drawTextCahed(QPainter* painter,
								   const FontParam& font,
								   SchemaUnit unit,
								   const QString& str,
								   const QRectF& rect,
								   int flags,
								   double zoom)
	{
		if (painter == nullptr || str.isEmpty() || rect.isEmpty() == true)
		{
			Q_ASSERT(painter);
			return;
		}

		QRgb textColor = painter->pen().color().rgb();		// Text color is taken from the current pen.

		const double dpiX = CDrawParam::realDpiX(painter);
		const double dpiY = CDrawParam::realDpiY(painter);
		QFont f = font.qfont(unit, dpiY);

		// Draw in pixels, for now we do not cache text out for SchemaUnit::Display, as this unit mode is discouraged.
		//
		if (unit == SchemaUnit::Display)
		{
			painter->setFont(f);
			painter->drawText(rect, flags, str);
			return;
		}

		// Draw in inches.
		//

		// Get cached image, if there is no one, create it.
		//
		const double reduceZoom = (dpiY > 120) ? 300 : 600;					// Kind of HiDpi Screen?
		const double sizeReduceFactor = (zoom > reduceZoom) ? 0.5 : 1.0;	// It makes images smaller depending on zoom.

		const double imageWidth = rect.width() * dpiX * (zoom / 100.0) * sizeReduceFactor;
		const double imageHeight = rect.height() * dpiY * (zoom / 100.0) * sizeReduceFactor;

		QSize imageSize{static_cast<int>(imageWidth), static_cast<int>(imageHeight)};
		QRect clipRectInt{0, 0, imageSize.width(), imageSize.height()};

		Hash cacheItemHash = DrawTextCacheItem::getHash(f, unit, str, imageSize, flags, textColor, dpiX, dpiY, zoom);
		bool newCacheItem = false;

thread_local QCache<Hash, DrawTextCacheItem> cache{50'000'000};			// 50Mb of images.

		DrawTextCacheItem* cacheItem = cache.object(cacheItemHash);
		if (cacheItem == nullptr)
		{
			cacheItem = new DrawTextCacheItem{f, unit, str, imageSize, flags, textColor, dpiX, dpiY, zoom};
			newCacheItem = true;
		}

		// Draw in inches
		//
		if (unit == SchemaUnit::Inch)
		{
			Q_ASSERT(unit == SchemaUnit::Inch);

			if (cacheItem->image.size() != clipRectInt.size())		// if image size is different, then it was just created.
			{
				// Create image and draw text to it.
				//
				double devicePixelRatioF = painter->device()->devicePixelRatioF();

				cacheItem->image = QImage{clipRectInt.size(), QImage::Format_ARGB32_Premultiplied};
				cacheItem->image.setDotsPerMeterX(static_cast<int>(painter->device()->physicalDpiX() / 25.4 * 1000.0));
				cacheItem->image.setDotsPerMeterY(static_cast<int>(painter->device()->physicalDpiY() / 25.4 * 1000.0));
				cacheItem->image.setDevicePixelRatio(devicePixelRatioF);

				cacheItem->image.fill(qRgba(0, 0, 0, 0));	// Transparent

				QPainter cacheImagePainter{&cacheItem->image};

				SchemaView::Ajust(&cacheImagePainter,
								  painter->device()->physicalDpiX(),
								  painter->device()->physicalDpiY(),
								  devicePixelRatioF,
								  unit,
								  -0.5 / devicePixelRatioF,
								  -0.5 / devicePixelRatioF,
								  zoom * sizeReduceFactor);

				QRectF textRect = rect;
				textRect.moveTo(0, 0);

				cacheImagePainter.setPen(textColor);
				cacheImagePainter.setFont(f);

				DrawHelper::drawText(&cacheImagePainter, font, unit, str, textRect, flags);
			}
		}

		// Draw cached image
		//
		Q_ASSERT(cacheItem != nullptr && cacheItem->image.isNull() == false);

		painter->drawImage(rect, cacheItem->image, cacheItem->image.rect());

		// Add to cache new item, cache only images not greater then specified size.
		//
		if (newCacheItem == true && cacheItem->image.sizeInBytes() < 2'000'000)
		{
			cache.insert(cacheItemHash, cacheItem, cacheItem->image.sizeInBytes());
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

	bool DrawHelper::drawSvgCached(QPainter& painter, SchemaUnit unit, const QRectF& rect, const QString& svg, double zoom)
	{
		// Add some extra space for drawing, to avoid artifacts in the edges of the image.
		//
		const int extraSizePx = 10;

		const double zoomFactor = zoom / 100.0;
		const double devicePixelRatioF = painter.device()->devicePixelRatioF();
		const double dpiX = CDrawParam::realDpiX(&painter);
		const double dpiY = CDrawParam::realDpiY(&painter);

		const double imageWidth = (unit == SchemaUnit::Inch) ?
										rect.width() * dpiX * zoomFactor :
										rect.width() * zoomFactor;

		const double imageHeight = (unit == SchemaUnit::Inch) ?
										rect.height() * dpiY * zoomFactor :
										rect.height() * zoomFactor;

		QSize imageSize{static_cast<int>(std::lround(imageWidth)) + extraSizePx,
						static_cast<int>(std::lround(imageHeight)) + extraSizePx};

		Hash cacheItemHash = DrawSvgCacheItem::getHash(unit, svg, imageSize, dpiX, dpiY, zoom);
		bool newCacheItem = false;

thread_local QCache<Hash, DrawSvgCacheItem> cache{100'000'000};			// 100Mb of images.

		DrawSvgCacheItem* cacheItem = cache.object(cacheItemHash);
		if (cacheItem == nullptr)
		{
			cacheItem = new DrawSvgCacheItem{unit, svg, imageSize, dpiX, dpiY, zoom};
			newCacheItem = true;
		}

		// --
		//
		if (cacheItem->image.size() != imageSize)		// if image size is different, then it was just created.
		{
			// Create image and draw text to it.
			//
			cacheItem->image = QImage{imageSize, QImage::Format_ARGB32_Premultiplied};
			Q_ASSERT(cacheItem->image.isNull() == false);

			if (unit == SchemaUnit::Inch)
			{
				cacheItem->image.setDotsPerMeterX(static_cast<int>(painter.device()->physicalDpiX() / 25.4 * 1000.0));
				cacheItem->image.setDotsPerMeterY(static_cast<int>(painter.device()->physicalDpiY() / 25.4 * 1000.0));
				cacheItem->image.setDevicePixelRatio(devicePixelRatioF);
			}
			else
			{
				cacheItem->image.setDevicePixelRatio(devicePixelRatioF);
			}

			cacheItem->image.fill(qRgba(0, 0, 0, 0));		// Transparent

			QPainter cacheImagePainter{&cacheItem->image};

			// Adjust painter for the image, note that extraSizePx is taken and
			// (-0.5) / devicePixelRatioF used to neglect half pixel align.
			//
			SchemaView::Ajust(&cacheImagePainter,
							  painter.device()->physicalDpiX(),
							  painter.device()->physicalDpiY(),
							  devicePixelRatioF,
							  unit,
							  0, 
							  0,
							  zoom);

			// The painter already adjusted to paint in pixels or inches.
			//
			double dpiXHere = unit == SchemaUnit::Inch ? dpiX : 1.0;
			double dpiYHere = unit == SchemaUnit::Inch ? dpiY : 1.0;

			const double extendedInX = extraSizePx / dpiXHere / zoomFactor;
			const double extendedInY = extraSizePx / dpiYHere / zoomFactor;

			QRectF imageRect = rect;
			imageRect.moveTo(extendedInX / 2.0, extendedInY / 2.0);

			// Render svg.
			//
			QSvgRenderer svgRenderer;

			bool loadSvgResult = svgRenderer.load(svg.toUtf8());
			if (loadSvgResult == false)
			{
				return false;
			}

			svgRenderer.render(&cacheImagePainter, imageRect);
		}

		// Draw cached image
		//
		Q_ASSERT(cacheItem != nullptr && cacheItem->image.isNull() == false);

		// Make a rect which takes into account extra size of the cached image.
		//
		QRectF extendedDstRect{rect};

		if (unit == SchemaUnit::Inch)
		{
			const double extendedInX = extraSizePx / dpiX / zoomFactor;
			const double extendedInY = extraSizePx / dpiY / zoomFactor;

			extendedDstRect.translate(-extendedInX / 2.0, -extendedInY / 2.0);

			extendedDstRect.setWidth(rect.width() + extendedInX);
			extendedDstRect.setHeight(rect.height() + extendedInY);
		}
		else
		{
			const double extendedPxX = extraSizePx / zoomFactor;
			const double extendedPxY = extraSizePx / zoomFactor;

			extendedDstRect.translate(-extendedPxX / 2.0, -extendedPxY / 2.0);

			extendedDstRect.setWidth(rect.width() + extendedPxX);
			extendedDstRect.setHeight(rect.height() + extendedPxY);
		}

		// Draw cached image into a painter.
		//
		painter.drawImage(extendedDstRect, cacheItem->image);

		// Add to cache new item, cache only images not greater then specified size.
		//
		if (newCacheItem == true && cacheItem->image.sizeInBytes() < 37'000'000)
		{
			cache.insert(cacheItemHash, cacheItem, cacheItem->image.sizeInBytes());
		}

#if 0
		painter.setBrush(Qt::NoBrush);

		QPen red{Qt::red};
		red.setCosmetic(true);
		painter.setPen(red);
		painter.drawRect(extendedDstRect);

		QPen green{Qt::green};
		green.setCosmetic(true);
		painter.setPen(green);
		painter.drawRect(rect);
#endif

		return true;
	}

}
