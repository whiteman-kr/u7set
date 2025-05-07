#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/DrawParam.h>

#include <Behavior/MonitorBehavior.h>
#include <Behavior/TuningClientBehavior.h>

#include <QSvgRenderer>

namespace
{
	struct DrawTextCacheItem
	{
		DrawTextCacheItem(const QFont& font,
						  SchemaUnit unit,
						  const QString& text,
						  QSize size,
						  int flags,
						  QRgb textColor,
						  double dpiX,
						  double dpiY,
						  double zoom) :
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

		QPixmap pixmap{};

		Hash hash() { return getHash(font, unit, text, size, flags, textColor, dpiX, dpiY, zoom); }

		static Hash getHash(const QFont& font,
							SchemaUnit unit,
							const QString& text,
							QSize size,
							int flags,
							QRgb textColor,
							double dpiX,
							double dpiY,
							double zoom)
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

		Hash hash() { return getHash(unit, svg, size, dpiX, dpiY, zoom); }

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
} // namespace


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
		if (m_cachedDpiX == 0.0)
		{
			auto device = m_painter->device();
			double dpiX;

			if (auto widget = dynamic_cast<const QWidget*>(device); widget != nullptr)
			{
				auto screen = widget->screen();
				dpiX = screen ? screen->physicalDotsPerInchX() : device->physicalDpiX();
			}
			else
			{
				dpiX = device->physicalDpiX();
			}

			m_cachedDpiX = dpiX * device->devicePixelRatioF();
		}

		return m_cachedDpiX;
	}

	double CDrawParam::realDpiY() const noexcept
	{
		if (m_cachedDpiY == 0.0)
		{
			auto device = m_painter->device();
			double dpiY;

			if (auto widget = dynamic_cast<const QWidget*>(device); widget != nullptr)
			{
				auto screen = widget->screen();
				dpiY = screen ? screen->physicalDotsPerInchY() : device->physicalDpiY();
			}
			else
			{
				dpiY = device->physicalDpiY();
			}

			m_cachedDpiY = dpiY * device->devicePixelRatioF();
		}

		return m_cachedDpiY;
	}

	double CDrawParam::devicePixelRatio() const noexcept
	{
		return m_painter->device()->devicePixelRatioF();
	}

	double CDrawParam::realDpiX(QPainter* painter) noexcept
	{
		const QPaintDevice* device = painter->device();
		double devicePixelRatioF = device->devicePixelRatioF();

		auto widget = dynamic_cast<const QWidget*>(device);

		if (widget != nullptr)
		{
			const auto screen = widget->screen();
			if (screen != nullptr)
			{
				return screen->physicalDotsPerInchX() * devicePixelRatioF;
			}
		}

		return device->physicalDpiX() * devicePixelRatioF;
	}

	double CDrawParam::realDpiY(QPainter* painter) noexcept
	{
		const QPaintDevice* device = painter->device();
		double devicePixelRatioF = device->devicePixelRatioF();

		auto widget = dynamic_cast<const QWidget*>(device);

		if (widget != nullptr)
		{
			const auto screen = widget->screen();
			if (screen != nullptr)
			{
				return screen->physicalDotsPerInchY() * devicePixelRatioF;
			}
		}

		return device->physicalDpiY() * devicePixelRatioF;
	}

	double CDrawParam::realScreenDpiX(QPainter* painter) noexcept
	{
		auto device = painter->device();
		double dpiX;

		if (auto widget = dynamic_cast<const QWidget*>(device); widget != nullptr)
		{
			auto screen = widget->screen();
			dpiX = screen ? screen->physicalDotsPerInchX() : device->physicalDpiX();
		}
		else
		{
			dpiX = device->physicalDpiX();
		}

		return dpiX * device->devicePixelRatioF();
	}

	double CDrawParam::realScreenDpiY(QPainter* painter) noexcept
	{
		auto device = painter->device();
		double dpiY;

		if (auto widget = dynamic_cast<const QWidget*>(device); widget != nullptr)
		{
			auto screen = widget->screen();
			dpiY = screen ? screen->physicalDotsPerInchY() : device->physicalDpiY();
		}
		else
		{
			dpiY = device->physicalDpiY();
		}

		return dpiY * device->devicePixelRatioF();
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
			double dpix = realDpiX();
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
			const double dpiy = realDpiY();
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
			result = QPointF((double)qRound(x * zoom) / zoom, (double)qRound(y * zoom) / zoom);
		}
		else
		{
			Q_ASSERT(m_schemaUnit == SchemaUnit::Inch);

			const double dpix = realDpiX();
			const double dpiy = realDpiY();

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
			result = QPointF((double)qRound(pos.x() * zoom) / zoom, (double)qRound(pos.y() * zoom) / zoom);
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

	double CDrawParam::gridToDpi(double pos, double dpi, double zoom, SchemaUnit unit) noexcept
	{
		zoom /= 100.0;

		if (unit == SchemaUnit::Display)
		{
			return (double)qRound(pos * zoom) / zoom;
		}

		if (unit == SchemaUnit::Inch)
		{
			return (static_cast<double>(static_cast<int>(pos * zoom * dpi)) / dpi) / zoom;
		}

		Q_ASSERT(false);
		return pos;
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

	void DrawHelper::drawText(QPainter* painter,
							  const FontParam& font,
							  SchemaUnit unit,
							  const QString& str,
							  const QRectF& rect,
							  int flags,
							  QRectF* boundingRect /* = nullptr*/,
							  std::pair<double, double> dpi /*= {0, 0}*/)
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

		const double dpiX = dpi.first == 0 ? CDrawParam::realDpiX(painter) : dpi.first;
		const double dpiY = dpi.second == 0 ? CDrawParam::realDpiY(painter) : dpi.second;

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
								   QRectF rect,
								   int flags,
								   double zoom)
	{
		Q_ASSERT(painter);

		if (str.isEmpty() || rect.isEmpty() == true)
		{
			return;
		}

		double devicePixelRatioF = painter->device()->devicePixelRatioF();
		const double dpiX = CDrawParam::realScreenDpiX(painter);
		const double dpiY = CDrawParam::realScreenDpiY(painter);
		double zoomFactor = zoom / 100.0;

		if (unit == SchemaUnit::Display || font.drawSize() > 0.5 || dpiX > 400 || painter->worldTransform().isRotating() == true)
		{
			return drawText(painter, font, unit, str, rect, flags);
		}
#if 0
		// Fall back to drawText, still draw cached if Ctrl is pressed.
		//
		if ((QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) == 0)
		{
			return drawText(painter, font, unit, str, rect, flags);
		}
#endif
		QRgb textColor = painter->pen().color().rgba();                  // Text color is taken from the current pen.

		const double reduceZoom = (dpiY > 120) ? 300 : 600;              // Kind of HiDpi Screen?
		const double sizeReduceFactor = (zoom > reduceZoom) ? 0.5 : 1.0; // It makes images smaller depending on zoom.

		// Draw in pixels, for now we do not cache text out for SchemaUnit::Display, as this unit mode is discouraged.
		//
		QFont f{font.name()};

		f.setBold(font.bold());
		f.setItalic(font.italic());
		f.setUnderline(font.underline());
		int pixelSize = static_cast<int>(font.drawSize() * dpiY / devicePixelRatioF * zoomFactor * sizeReduceFactor);
		f.setPixelSize(pixelSize > 0 ? pixelSize : 1);

		const double imageWidth = std::ceil(rect.width() * dpiX * (zoom / 100.0) * sizeReduceFactor);
		const double imageHeight = std::ceil(rect.height() * dpiY * (zoom / 100.0) * sizeReduceFactor);

		QSize imageSize{static_cast<int>(imageWidth), static_cast<int>(imageHeight)};

		Hash cacheItemHash = DrawTextCacheItem::getHash(f, unit, str, imageSize, flags, textColor, dpiX, dpiY, zoom);
		bool newCacheItem = false;

		thread_local QCache<Hash, DrawTextCacheItem> cache{50'000'000}; // 50Mb of images.

		DrawTextCacheItem* cacheItem = cache.object(cacheItemHash);
		if (cacheItem == nullptr)
		{
			cacheItem = new DrawTextCacheItem{f, unit, str, imageSize, flags, textColor, dpiX, dpiY, zoom};
			newCacheItem = true;
		}

		if (newCacheItem == true)
		{
			QPixmap pixmap{imageSize};
			pixmap.setDevicePixelRatio(painter->device()->devicePixelRatioF());
			pixmap.fill(Qt::transparent);

			QPainter cacheImagePainter{&pixmap};

			QRectF textRect{0,
							0,
							rect.width() * dpiX / devicePixelRatioF * zoomFactor * sizeReduceFactor,
							rect.height() * dpiY / devicePixelRatioF * zoomFactor * sizeReduceFactor};

			cacheImagePainter.setPen(painter->pen());
			cacheImagePainter.setFont(f);
			cacheImagePainter.drawText(textRect, flags, str, nullptr);

			cacheItem->pixmap = pixmap;
		}

		// Draw cached pixmap.
		//
		painter->setWorldMatrixEnabled(false);

		QRectF rc;
		rc.setLeft(rect.left() * dpiX / devicePixelRatioF * zoomFactor);
		rc.setTop(rect.top() * dpiY / devicePixelRatioF * zoomFactor);
		rc.setRight(rect.right() * dpiX / devicePixelRatioF * zoomFactor);
		rc.setBottom(rect.bottom() * dpiY / devicePixelRatioF * zoomFactor);
		rc.translate(0.5, 0.5);

		if (sizeReduceFactor == 1.0)
		{
			painter->drawPixmap(rc.topLeft(), cacheItem->pixmap);
		}
		else
		{
			// Scale pixmap to rect rc.
			//
			double imageWidthF = rect.width() * dpiX * (zoom / 100.0) * sizeReduceFactor;
			double imageHeightF = rect.height() * dpiY * (zoom / 100.0) * sizeReduceFactor;

			painter->drawPixmap(rc, cacheItem->pixmap, QRectF{0, 0, imageWidthF, imageHeightF});
		}

		painter->setWorldMatrixEnabled(true);

		// Save new item to cache.
		//
		if (newCacheItem == true)
		{
			int pixmapBytes = cacheItem->pixmap.size().width() * cacheItem->pixmap.size().height() * 4; // 4 bytes per pixel.
			if (pixmapBytes < 3'000'000)                                                                // up to 3Mb per image.
			{
				cache.insert(cacheItemHash, cacheItem, pixmapBytes);
			}
			else
			{
				delete cacheItem;                                                                       // Too big image, do not cache it.
			}
		}

		return;
	}

	void DrawHelper::drawText(QPainter* painter,
							  SchemaUnit unit,
							  const QString& str,
							  const QRectF& rect,
							  int flags,
							  QRectF* boundingRect /* = nullptr*/)
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

		const double imageWidth = (unit == SchemaUnit::Inch) ? rect.width() * dpiX * zoomFactor : rect.width() * zoomFactor;

		const double imageHeight = (unit == SchemaUnit::Inch) ? rect.height() * dpiY * zoomFactor : rect.height() * zoomFactor;

		QSize imageSize{static_cast<int>(std::lround(imageWidth)) + extraSizePx, static_cast<int>(std::lround(imageHeight)) + extraSizePx};

		Hash cacheItemHash = DrawSvgCacheItem::getHash(unit, svg, imageSize, dpiX, dpiY, zoom);
		bool newCacheItem = false;

		thread_local QCache<Hash, DrawSvgCacheItem> cache{100'000'000}; // 100Mb of images.

		DrawSvgCacheItem* cacheItem = cache.object(cacheItemHash);
		if (cacheItem == nullptr)
		{
			cacheItem = new DrawSvgCacheItem{unit, svg, imageSize, dpiX, dpiY, zoom};
			newCacheItem = true;
		}

		// --
		//
		if (cacheItem->image.size() != imageSize) // if image size is different, then it was just created.
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

			cacheItem->image.fill(qRgba(0, 0, 0, 0)); // Transparent

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
		if (newCacheItem == true)
		{
			if (cacheItem->image.sizeInBytes() < 37'000'000)
			{
				cache.insert(cacheItemHash, cacheItem, cacheItem->image.sizeInBytes());
			}
			else
			{
				delete cacheItem; // Too big image, do not cache it.
			}
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

} // namespace VFrame30
