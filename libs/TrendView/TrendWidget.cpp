#include "TrendWidget.h"
#include "TrendScale.h"

#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>

#include <cstring>


namespace TrendLib
{
	RenderThread::RenderThread(TrendImpl* trendImpl, ITrendDataProvider* dataProvider, QObject* parent) :
		QThread(parent),
		m_trendImpl(trendImpl),
		m_drawParam(dataProvider)
	{
		qRegisterMetaType<TrendLib::TrendParam>("TrendParam");

		Q_ASSERT(m_trendImpl);
	}

	RenderThread::~RenderThread()
	{
		m_mutex.lock();
		m_interruptRequested = true;
		m_mutex.unlock();

		m_newJob.notify_one();

		bool ok = wait(5000);
		if (ok == false)
		{
			qDebug() << "TREND RENDER THREAD IS NOT FINISHED, IT WILL BE TERMINATED!!!";
			terminate();
		}

		return;
	}

	void RenderThread::render(const TrendParam& drawParam)
	{
		{
			std::unique_lock<std::mutex> locker(m_mutex);
			m_drawParam = drawParam;
			m_drawParam->signalDescriptionRect().clear();
			m_drawParam->setProject({}); // Do not show project name.
		}

		m_newJob.notify_one();

		if (isRunning() == false)
		{
			start(QThread::NormalPriority);
		}

		return;
	}

	void RenderThread::run()
	{
		do
		{
			std::unique_lock<std::mutex> locker(m_mutex);

			m_newJob.wait(locker,
						  [this]()
						  {
							  return m_interruptRequested || m_drawParam.has_value();
						  });

			if (m_interruptRequested == true)
			{
				break;
			}

			// Start new job
			//

			// Copy draw param under locked mutex.
			//
			std::optional<TrendParam> drawParam = std::move(m_drawParam.value());
			m_drawParam.reset();

			locker.unlock();

			// --
			//
			if (m_image.size() != drawParam->rectPx().size())
			{
				auto imageSize = drawParam->rectPx().size().toSize();
				m_image = QImage{imageSize, QImage::Format_RGB32};

				m_image.setDevicePixelRatio(drawParam->devicePixelRatio());
				m_image.setDotsPerMeterX(static_cast<int>(m_image.physicalDpiX() / 25.4 * 1000.0));
				m_image.setDotsPerMeterY(static_cast<int>(m_image.physicalDpiY() / 25.4 * 1000.0));
			}

			// All drawing are done in inches
			//
			drawParam->setDpi(m_image.dotsPerMeterX() / (1000.0 / 25.4),
							  m_image.dotsPerMeterY() / (1000.0 / 25.4),
							  m_image.devicePixelRatioF());

			m_trendImpl->draw(&m_image, drawParam.value());

			emit renderedImage(m_image, std::move(drawParam.value()));
		} while (true);

		return;
	}


	TrendWidget::TrendWidget(QWidget* parent) :
		QWidget(parent),
		m_trend(),
		m_trendParam(&m_trend.signalSet()),
		m_thread(&m_trend.impl(), &m_trend.signalSet()),
		m_pixmapDrawParam(&m_trend.signalSet())
	{
		setMouseTracking(true); // To enable mouseMoveEvent without pressed button

		connect(&m_thread, &RenderThread::renderedImage, this, &TrendWidget::updatePixmap);
	}

	bool TrendWidget::save(QString fileName, QString* errorMessage) const
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return false;
		}

		QFile file(fileName);

		bool ok = file.open(QIODevice::WriteOnly);
		if (ok == false)
		{
			*errorMessage = file.errorString();
		}

		// Serialize to protobuf
		//
		::Proto::TrendWidget message;
		ok = save(&message);

		if (ok == false)
		{
			*errorMessage = tr("Serialize trend structure error.");
			return false;
		}

		// Compress data and save to file
		//
		std::string serializedString;
		serializedString.reserve(message.ByteSizeLong());

		ok = message.SerializeToString(&serializedString);
		if (ok == false)
		{
			*errorMessage = tr("Serialize message to string error.");
			return false;
		}

		// The bytes are not copied!!!, keep serializedString alive
		//
		QByteArray ba = QByteArray::fromRawData(serializedString.data(), static_cast<int>(serializedString.size()));
		QByteArray compressedData = qCompress(ba, 3);

		qint64 written = file.write(compressedData);

		if (written != compressedData.size())
		{
			*errorMessage = tr("Write file error. ") + file.errorString();
			return false;
		}

		return ok;
	}

	bool TrendWidget::load(QString fileName, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return false;
		}

		// Read compressed data
		//
		QFile file(fileName);

		bool ok = file.open(QIODevice::ReadOnly);
		if (ok == false)
		{
			*errorMessage = file.errorString();
		}

		QByteArray ba = file.readAll();

		// Uncompress data
		//
		QByteArray uncompressedData = qUncompress(ba);

		// Deserialize
		//
		::Proto::TrendWidget message;

		ok = message.ParseFromArray(uncompressedData.constData(), static_cast<int>(uncompressedData.size()));

		if (ok == false)
		{
#ifdef Q_OS_WINDOWS
			std::array<char, 512> errMsgBuf;
			strerror_s(errMsgBuf.data(), std::size(errMsgBuf), errno);
			*errorMessage = tr("Parse trend file error. ") + QString(errMsgBuf.data());
#else
			*errorMessage = tr("Parse trend file error. ") + strerror(errno);
#endif
			return false;
		}

		ok = load(message);
		if (ok == false)
		{
			*errorMessage = tr("Read trend data structure error.");
			return false;
		}

		return ok;
	}

	bool TrendWidget::save(::Proto::TrendWidget* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		::Proto::Trend* trendMessage = message->mutable_trend();
		ok &= m_trend.impl().save(trendMessage);

		::Proto::TrendParam* trendParamMessage = message->mutable_trend_param();
		ok &= m_trendParam.save(trendParamMessage);

		return ok;
	}

	bool TrendWidget::load(const ::Proto::TrendWidget& message)
	{
		if (message.IsInitialized() == false || message.has_trend() == false || message.has_trend_param() == false)
		{
			Q_ASSERT(message.IsInitialized());
			Q_ASSERT(message.has_trend());
			Q_ASSERT(message.has_trend_param());
			return false;
		}

		bool ok = true;

		ok &= m_trend.impl().load(message.trend());
		ok &= m_trendParam.load(message.trend_param());

		return ok;
	}

	void TrendWidget::updateWidget()
	{
		double devicePixelRatio = devicePixelRatioF();
		QRectF hdRect{0, 0, rect().width() * devicePixelRatio, rect().height() * devicePixelRatio};

		m_trendParam.setRectPx(hdRect.toRect(), logicalDpiX(), logicalDpiY(), devicePixelRatio);

		// Somehow we draw image in physicalDpiX (for clear picture)
		// and use logical dpi for calculation mouse areas and all widget positioning
		//
		TrendParam trendParam{m_trendParam};
		trendParam.setDpi(physicalDpiX(), physicalDpiY(), devicePixelRatio);

		m_thread.render(trendParam);

		return;
	}

	bool TrendWidget::saveImageToFile(QString fileName) const
	{
		QImage image = m_pixmap.toImage();

		{
			QPainter p(&image);

			TrendParam drawParam = m_pixmapDrawParam;
			drawParam.setDpi(p.device()->physicalDpiX(), p.device()->physicalDpiY(), 1.0);
			drawParam.setProject({}); // Do not show project name.

			m_trend.impl().drawRulers(&p, drawParam);
		}

		bool ok = image.save(fileName, nullptr, -1);

		return ok;
	}

	bool TrendWidget::saveToPdf(QString fileName, QPageSize::PageSizeId pageSize, QPageLayout::Orientation pageOrientation) const
	{
		QPdfWriter pdfWriter(fileName);

		pdfWriter.setTitle("Trends");
		pdfWriter.setPageSize(QPageSize(pageSize));
		pdfWriter.setPageOrientation(pageOrientation);

		const int resolution = pdfWriter.resolution();
		const double resolutionF = static_cast<double>(resolution);

		auto rectPx = pdfWriter.pageLayout().paintRectPixels(resolution);
		rectPx.moveTopLeft({0, 0});

		TrendParam drawParam = m_trendParam;
		drawParam.setRectPx(rectPx, resolutionF, resolutionF, 1.0);

		// --
		//
		QPainter p(&pdfWriter);

		m_trend.impl().draw(&p, drawParam, true);
		m_trend.impl().drawRulers(&p, drawParam);

#if 0
		// Debug, draw bounding rect
		//
		p.resetTransform();

		QPen pen(Qt::red, 0, Qt::DashLine, Qt::PenCapStyle::RoundCap);
		p.setPen(pen);
		p.setBrush(Qt::NoBrush);
		p.drawRect(rectPx);
#endif
		return true;
	}

	bool TrendWidget::print(QPrinter* printer) const
	{
		if (printer == nullptr || printer->isValid() == false)
		{
			Q_ASSERT(printer);
			return false;
		}

		QPainter painter;

		bool ok = painter.begin(printer);
		if (ok == false)
		{
			Q_ASSERT(ok);
			return false;
		}

		const int resolution = printer->resolution();
		const double resolutionF = static_cast<double>(printer->resolution());

#if 0
		// Debug, printr all possible margins and sizes.
		//
		qDebug() << "Printer info:";
		qDebug() << "	Printer name: " << printer->printerName();
		qDebug() << "	Printer resolution: " << printer->resolution();
		qDebug() << "	Printer printer->paperRect(DevicePixel): " << printer->paperRect(QPrinter::Unit::DevicePixel);
		qDebug() << "	Printer printer->paperRect(Inch): " << printer->paperRect(QPrinter::Unit::Inch);
		qDebug() << "	Printer printer->pageRect(DevicePixel): " << printer->pageRect(QPrinter::Unit::DevicePixel);
		qDebug() << "	Printer printer->pageRect(Inch): " << printer->pageRect(QPrinter::Unit::Inch);
		
		qDebug() << "	Printer printer->pageLayout().fullRectPixels(resolution): "
				 << printer->pageLayout().fullRectPixels(printer->resolution());
		qDebug() << "	Printer printer->pageLayout().fullRect(Inch): " << printer->pageLayout().fullRect(QPageLayout::Inch);
		qDebug() << "	Printer printer->pageLayout().fullRect(DevicePixel): " << printer->pageLayout().fullRectPixels(resolution);
		
		qDebug() << "	Printer printer->pageLayout().marginsPixels(resolution): " << printer->pageLayout().marginsPixels(resolution);
		qDebug() << "	Printer printer->pageLayout().margins(Inch): " << printer->pageLayout().margins(QPageLayout::Inch);
#endif

		// Prepare DrawParam
		//
		printer->setFullPage(false);

		QRectF rc = printer->pageLayout().paintRectPixels(resolution);
		rc.moveTopLeft({0, 0});

		TrendParam drawParam = m_trendParam;
		drawParam.setBackColor1st(Qt::white);
		drawParam.setBackColor1st(qRgb(0xF0, 0xF0, 0xF0));

		drawParam.setRectPx(rc, resolutionF, resolutionF, printer->devicePixelRatioF());

		// Draw to printer
		//
		m_trend.impl().draw(&painter, drawParam, true);
		m_trend.impl().drawRulers(&painter, drawParam);

		// Finish printing
		//
		ok = painter.end();
		if (ok == false)
		{
			Q_ASSERT(ok);
			return false;
		}

		return true;
	}

	void TrendWidget::paintEvent(QPaintEvent*)
	{
		QPainter painter(this);

		if (m_pixmap.isNull() == true)
		{
			painter.fillRect(rect(), m_trendParam.backColor1st());
			painter.setPen(Qt::black);
			painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
			return;
		}

		double devicePixelRatio = devicePixelRatioF();
		QRectF hdRect{0, 0, rect().width() * devicePixelRatio, rect().height() * devicePixelRatio};

		if (m_pixmap.size() != hdRect.size().toSize())
		{
			// New pixmap is not ready yet, scale the current one
			//
			painter.fillRect(rect(), m_trendParam.backColor1st());
			painter.drawPixmap(rect(), m_pixmap, m_pixmap.rect());
			return;
		}

		painter.drawPixmap(rect(), m_pixmap, m_pixmap.rect());

		// Draw rulers
		//
		trend().impl().drawRulers(&painter, m_pixmapDrawParam);

		// Draw select view area
		//
		if (m_mouseAction == MouseAction::SelectViewSelectSecondPoint)
		{
			TrendParam drawParam = m_pixmapDrawParam;
			drawParam.setDpi(this->logicalDpiX(), this->logicalDpiY(), this->devicePixelRatioF());

			TrendImpl::adjustPainter(&painter, drawParam);

			QRectF selectionRect(m_startSelectViewPoint, m_finishSelectViewPoint);
			selectionRect = selectionRect.normalized();

			selectionRect = selectionRect.intersected(m_selectViewAreaSignal.tempDrawRect()).normalized();

			if (selectionRect.isEmpty() == false)
			{
				QPen p(Qt::blue, 0, Qt::DashLine, Qt::PenCapStyle::RoundCap);

				painter.setBrush(Qt::NoBrush);
				painter.setPen(p);

				painter.drawRect(selectionRect);
			}
		}

		return;
	}

	void TrendWidget::resizeEvent(QResizeEvent*)
	{
		updateWidget();
	}

	void TrendWidget::mousePressEvent(QMouseEvent* event)
	{
		int analogsCount = signalSet().analogSignalsCount();
		int discretesCount = signalSet().discreteSignalsCount();

		if (analogsCount + discretesCount == 0)
		{
			return;
		}

		int laneIndex = -1;
		int rulerIndex = -1;
		TimeStamp timeStamp;
		TrendSignalParam outSignal;

		TrendImpl::MouseOn mouseOn = mouseIsOver(event->pos(), &laneIndex, &timeStamp, &rulerIndex, &outSignal);

		if (m_mouseAction == MouseAction::SelectViewStart &&
			(mouseOn == TrendImpl::MouseOn::InsideTrendArea || mouseOn == TrendImpl::MouseOn::OnSignalDescription ||
			 mouseOn == TrendImpl::MouseOn::OnRuler))
		{
			if (event->buttons().testFlag(Qt::LeftButton) == false && event->buttons().testFlag(Qt::MiddleButton) == false)
			{
				// Cancel action
				//
				unsetCursor();
				m_mouseAction = MouseAction::None;
				update();
				return;
			}

			initSelectViewArea(event->pos(), laneIndex);

			m_mouseAction = MouseAction::SelectViewSelectSecondPoint;

			grabMouse();
			return;
		}

		if (m_mouseAction == MouseAction::SelectViewSelectSecondPoint)
		{
			unsetCursor();
			m_mouseAction = MouseAction::None;
			update();
			releaseMouse();
			return;
		}

		m_mouseAction = MouseAction::None;

		if (event->buttons().testFlag(Qt::LeftButton) == true)
		{
			if (mouseOn == TrendImpl::MouseOn::OnRuler)
			{
				if (rulerIndex == -1)
				{
					Q_ASSERT(rulerIndex != -1);
					return;
				}

				m_rulerMoveRulerIndex = rulerIndex;
				m_rulerMoveInitialMousePos = event->pos();

				m_rulerMoveInitialTimeStamp = rulerSet().rulers().at(rulerIndex).timeStamp();

				m_mouseAction = MouseAction::MoveRuler;
				this->grabMouse();
			}

			if (mouseOn == TrendImpl::MouseOn::OnSignalDescription)
			{
				if (outSignal.appSignalId().isEmpty() == true)
				{
					Q_ASSERT(outSignal.appSignalId().isEmpty() == false);
					return;
				}

				emit showSignalProperties(outSignal.appSignalId(), outSignal.archiveServerId());
			}
		}

		if (event->buttons().testFlag(Qt::LeftButton) == true || event->buttons().testFlag(Qt::MiddleButton) == true)
		{
			if (mouseOn == TrendImpl::MouseOn::InsideTrendArea)
			{
				m_mouseScrollInitialTime = m_trendParam.startTimeStamp();
				m_mouseScrollInitialMousePos = event->pos();
				m_mouseScrollSignal = outSignal;                          // tempDrawRect already calculated

				m_mouseScrollAnalogSignals = signalSet().analogSignals(); // tempDrawRect is not calculated

				m_mouseAction = MouseAction::Scroll;
				this->grabMouse();
			}

			return;
		}

		if (event->buttons().testFlag(Qt::RightButton) == true)
		{
			return;
		}

		return;
	}

	void TrendWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		event->accept();

		if (m_mouseAction == MouseAction::SelectViewSelectSecondPoint)
		{
			selectViewArea(event->pos());

			releaseMouse();
			unsetCursor();

			m_mouseAction = MouseAction::None;
			updateWidget();
			return;
		}

		if (m_mouseAction == MouseAction::MoveRuler) {}

		if (m_mouseAction == MouseAction::MoveRuler)
		{
			// This will call slider update
			//
			if (dynamic_cast<QWidget*>(parent()) != nullptr)
			{
				dynamic_cast<QWidget*>(parent())->update();
			}
		}

		m_mouseAction = MouseAction::None;
		releaseMouse();

		if (event->buttons().testFlag(Qt::LeftButton) == false && event->buttons().testFlag(Qt::MiddleButton) == false)
		{
			unsetCursor();
			mouseMoveEvent(event); // To set cursor
		}
		else
		{
			unsetCursor();
		}

		return;
	}

	void TrendWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (m_mouseAction == MouseAction::None)
		{
			int laneIndex = -1;
			int rulerIndex = -1;
			TimeStamp timeStamp;
			TrendSignalParam onSignal;

			TrendParam tp = m_pixmapDrawParam;
			tp.setDpi(this->logicalDpiX(), this->logicalDpiY(), this->devicePixelRatioF());

			TrendImpl::MouseOn mouseOn = m_trend.impl().mouseIsOver(event->pos(), tp, &laneIndex, &timeStamp, &rulerIndex, &onSignal);

			Qt::CursorShape newCursorShape = Qt::ArrowCursor;

			switch (mouseOn)
			{
			case TrendImpl::MouseOn::Outside:
				newCursorShape = Qt::ArrowCursor;
				break;
			case TrendImpl::MouseOn::OutsideTrendArea:
				newCursorShape = Qt::ArrowCursor;
				break;
			case TrendImpl::MouseOn::InsideTrendArea:
				newCursorShape = Qt::ArrowCursor;
				break;
			case TrendImpl::MouseOn::OnSignalDescription:
				newCursorShape = Qt::PointingHandCursor;
				break;
			case TrendImpl::MouseOn::OnRuler:
				newCursorShape = Qt::SplitHCursor;
				break;
			default:
				Q_ASSERT(false);
			}

			m_pixmapDrawParam.setHightlightRulerIndex(rulerIndex);

			if (newCursorShape == Qt::ArrowCursor)
			{
				this->unsetCursor();
				update();
			}
			else
			{
				if (this->cursor().shape() != newCursorShape)
				{
					this->setCursor(newCursorShape);
					update();
				}
			}

			return;
		}

		if (QWidget::mouseGrabber() == this)
		{
			switch (m_mouseAction)
			{
			case MouseAction::None:
				Q_ASSERT(false);
				break;

			case MouseAction::Scroll:
				{
					// Scroll time with a mouse mode
					//
					QRectF laneRect = m_trend.impl().calcLaneRect(0, m_trendParam);
					QRectF trenAreaRect = m_trend.impl().calcTrendArea(laneRect, m_trendParam); // TrendArea in inches
					QRectF trendAreaRectPixels = TrendImpl::inchRectToPixelRect(trenAreaRect, m_trendParam);

					double coefx = m_trendParam.duration() / trendAreaRectPixels.width();

					QPointF mouseOffset = m_mouseScrollInitialMousePos - event->pos();

					TimeStamp ts(m_mouseScrollInitialTime.timeStamp + static_cast<qint64>(mouseOffset.x() * coefx));
					m_trendParam.setStartTimeStamp(ts);

					// Scroll vertical area
					//
					if (event->modifiers().testFlag(Qt::AltModifier) == true)
					{
						std::vector<TrendSignalParam> analogsToShift;

						if (m_trendParam.viewMode() == E::TrendViewMode::Separated)
						{
							analogsToShift.push_back(m_mouseScrollSignal); // signalRect is calculated
						}
						else
						{
							analogsToShift = m_mouseScrollAnalogSignals;   // signalRect is not calculated yet
							auto discretes = signalSet().discreteSignals();

							TrendImpl::calcSignalRects(trenAreaRect, m_trendParam, &discretes, &analogsToShift);
						}

						for (const TrendSignalParam& trendSignal : analogsToShift)
						{
							bool ok = false;

							double highLimit = TrendScale::scaleHighLimit(trendSignal, scaleType(), &ok);
							if (ok == false)
							{
								continue;
							}

							double lowLimit = TrendScale::scaleLowLimit(trendSignal, scaleType(), &ok);
							if (ok == false)
							{
								continue;
							}

							QRectF signalRect = trendSignal.tempDrawRect();

							if (std::fabs(highLimit - lowLimit) > std::numeric_limits<double>::min() &&
								signalRect.height() > std::numeric_limits<double>::min())
							{
								double dy = mouseOffset.y() / m_trendParam.realDpiY();
								double k = (highLimit - lowLimit) / signalRect.height();

								highLimit -= dy * k;
								lowLimit -= dy * k;

								TrendSignalParam tsp = trendSignal;

								double newHighLimit = TrendScale::limitFromScaleValue(highLimit, scaleType(), &ok);
								if (ok == false)
								{
									continue;
								}

								double newLowLimit = TrendScale::limitFromScaleValue(lowLimit, scaleType(), &ok);
								if (ok == false)
								{
									continue;
								}

								tsp.setViewHighLimit(scaleType(), newHighLimit);
								tsp.setViewLowLimit(scaleType(), newLowLimit);

								signalSet().setSignalParam(tsp);
							}
						}
					}

					updateWidget();

					emit startTimeChanged(ts);
				}
				break;

			case MouseAction::MoveRuler:
				{
					Q_ASSERT(m_rulerMoveRulerIndex != -1);
					Q_ASSERT(m_rulerMoveRulerIndex >= 0 && m_rulerMoveRulerIndex < static_cast<int>(rulerSet().rulers().size()));

					int laneHeight = rect().height() / laneCount();
					int laneIndex = qBound<int>(0, event->pos().y() / laneHeight, laneCount() - 1);

					QRectF laneRect = m_trend.impl().calcLaneRect(laneIndex, m_trendParam);
					QRectF trenAreaRect = m_trend.impl().calcTrendArea(laneRect, m_trendParam); // TrendArea in inches
					QRectF trendAreaRectPixels = TrendImpl::inchRectToPixelRect(trenAreaRect, m_trendParam);

					qint64 laneStartTime = m_trendParam.startTimeStamp().timeStamp + m_trendParam.duration() * laneIndex;

					double coefx = m_trendParam.duration() / trendAreaRectPixels.width();

					int mouseOffset = static_cast<int>(event->pos().x() - trendAreaRectPixels.left());
					mouseOffset = qBound<int>(1, mouseOffset, static_cast<int>(trendAreaRectPixels.width()));

					TimeStamp ts(laneStartTime + static_cast<qint64>(mouseOffset * coefx));

					TrendRuler& mutableRuler = rulerSet().rulers().at(m_rulerMoveRulerIndex);
					mutableRuler.setTimeStamp(ts, trend().impl().rulerSet().rulerStep());

					update();
				}
				break;
			case MouseAction::SelectViewSelectSecondPoint:
				{
					m_finishSelectViewPoint = TrendImpl::pixelPointToInchPoint(event->pos(), m_trendParam);
					update();
				}
				break;

			default:
				Q_ASSERT(false);
				break;
			}
		}

		event->accept();
		return;
	}

	void TrendWidget::wheelEvent(QWheelEvent* event)
	{
		// While midButton is pressed, this is move mode, don't change zoom
		//
		if (event->buttons().testFlag(Qt::MiddleButton))
		{
			return;
		}

		int vertDegrees = event->angleDelta().y() / 8;
		int vertSteps = vertDegrees / 15;

		int horzDegrees = event->angleDelta().x() / 8; // Horz degrees work with pressed Alt
		int horzSteps = horzDegrees / 15;

		// In some cases horz scroll is not supported, then just check Alt manually
		//
		if (vertSteps != 0 && horzSteps == 0 && event->modifiers().testFlag(Qt::AltModifier) == true)
		{
			horzSteps = vertSteps;
			vertSteps = 0;
		}

		bool needUpdateWidget = false;

		// Calc time
		//
		if (vertSteps != 0)
		{
			qint64 startTime = m_trendParam.startTimeStamp().timeStamp;

			qint64 oldDuration = m_trendParam.duration();
			qint64 newLaneDuration = oldDuration;

			if (vertSteps < 0)
			{
				newLaneDuration = static_cast<qint64>(oldDuration * 1.1);
				startTime -= static_cast<qint64>((newLaneDuration - oldDuration) / 2.0);
			}
			else
			{
				newLaneDuration = static_cast<qint64>(oldDuration * 0.9);
				startTime += static_cast<qint64>((oldDuration - newLaneDuration) / 2.0);
			}

			// Set new values to controls and draw param
			//
			m_trendParam.setLaneDuration(newLaneDuration); // This func can limit value

			if (m_trendParam.duration() != oldDuration)
			{
				m_trendParam.setStartTimeStamp(TimeStamp{startTime});

				emit startTimeChanged(TimeStamp{startTime});
				emit durationChanged(newLaneDuration);

				needUpdateWidget = true;
			}
		}

		if (horzSteps != 0)
		{
			// Scale analog signals
			//
			std::vector<TrendSignalParam> signalsToScale;

			if (m_trendParam.viewMode() == E::TrendViewMode::Overlapped)
			{
				// Scale all analog signals
				//
				signalsToScale = signalSet().analogSignals();
			}

			if (m_trendParam.viewMode() == E::TrendViewMode::Separated)
			{
				// Scale analog signal where is mouse now
				//
				int laneIndex = -1;
				int rulerIndex = -1;
				TimeStamp timeStamp;
				TrendSignalParam trendSignal;

				TrendImpl::MouseOn mouseOn = mouseIsOver(event->position().toPoint(), &laneIndex, &timeStamp, &rulerIndex, &trendSignal);

				if (mouseOn != TrendImpl::MouseOn::OutsideTrendArea && mouseOn != TrendImpl::MouseOn::Outside &&
					trendSignal.appSignalId().isEmpty() == false && trendSignal.isAnalog() == true)
				{
					signalsToScale.push_back(trendSignal);
				}
			}

			// Scale view area
			//
			for (TrendSignalParam tsp : signalsToScale)
			{
				bool ok = false;

				double h = TrendScale::scaleHighLimit(tsp, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				double l = TrendScale::scaleLowLimit(tsp, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				double delta = std::fabs(h - l);
				if (delta < std::numeric_limits<double>::min())
				{
					continue;
				}

				if (horzSteps > 0)
				{
					h = h - delta * 0.1;
					l = l + delta * 0.1;
				}
				else
				{
					h = h + delta * 0.1;
					l = l - delta * 0.1;
				}

				double newHighLimit = TrendScale::limitFromScaleValue(h, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				double newLowLimit = TrendScale::limitFromScaleValue(l, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				tsp.setViewHighLimit(scaleType(), newHighLimit);
				tsp.setViewLowLimit(scaleType(), newLowLimit);

				signalSet().setSignalParam(tsp);
				needUpdateWidget = true;
			}
		}

		// --
		//
		if (needUpdateWidget == true)
		{
			updateWidget();
		}

		event->accept();
		return;
	}

	TrendImpl::MouseOn TrendWidget::mouseIsOver(const QPoint& mousePos,
												int* outLaneIndex,
												TimeStamp* timeStamp,
												int* rulerIndex,
												TrendSignalParam* onSignal)
	{
		TrendParam tp = m_pixmapDrawParam;

		return m_trend.impl().mouseIsOver(mousePos, tp, outLaneIndex, timeStamp, rulerIndex, onSignal);
	}

	void TrendWidget::resetRulerHighlight()
	{
		m_pixmapDrawParam.resetHightlightRulerIndex();
	}

	void TrendWidget::initSelectViewArea(QPoint pos, int laneIndex)
	{
		Q_ASSERT(laneIndex != -1);

		m_selectViewLaneIndex = laneIndex;

		QRectF laneRect = TrendImpl::calcLaneRect(laneIndex, m_trendParam);
		QRectF trendArea = TrendImpl::calcTrendArea(laneRect, m_trendParam, std::span<const TrendSignalParam>{signalSet().analogSignals()});

		m_startSelectViewPoint = TrendImpl::pixelPointToInchPoint(pos, m_trendParam);
		m_finishSelectViewPoint = m_startSelectViewPoint;

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		TrendImpl::calcSignalRects(trendArea, m_trendParam, &discretes, &analogs);

		for (const TrendSignalParam& tsp : discretes)
		{
			if (tsp.tempDrawRect().contains(m_startSelectViewPoint) == true)
			{
				m_selectViewAreaSignal = tsp;
				return;
			}
		}

		for (const TrendSignalParam& tsp : analogs)
		{
			if (tsp.tempDrawRect().contains(m_startSelectViewPoint) == true)
			{
				m_selectViewAreaSignal = tsp;
				return;
			}
		}

		// --
		//

		return;
	}

	void TrendWidget::selectViewArea(QPoint pos)
	{
		// Scale time
		//
		m_finishSelectViewPoint = TrendImpl::pixelPointToInchPoint(pos, m_trendParam);

		double left = qMin(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());
		double right = qMax(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());

		if (std::fabs(right - left) * m_trendParam.realDpiX() <= 1)
		{
			// Value is way too small
			//
			return;
		}

		// Calc time
		//
		QRectF signalRect = m_selectViewAreaSignal.tempDrawRect();

		qint64 startLaneTime = m_trendParam.startTimeStamp().timeStamp + m_selectViewLaneIndex * m_trendParam.duration();
		double coef = m_trendParam.duration() / signalRect.width();

		qint64 leftTime = startLaneTime + static_cast<qint64>((left - signalRect.left()) * coef);
		qint64 rightTime = startLaneTime + static_cast<qint64>((right - signalRect.left()) * coef);

		// Set new values to controls and draw param
		//
		m_trendParam.setStartTimeStamp(TimeStamp{leftTime});
		m_trendParam.setLaneDuration(rightTime - leftTime);

		emit startTimeChanged(TimeStamp{leftTime});
		emit durationChanged(rightTime - leftTime);

		// Scale vertical area (only for analogs)
		//

		if (m_selectViewAreaSignal.isAnalog() == true)
		{
			std::vector<TrendSignalParam> analogs;

			if (viewMode() == E::TrendViewMode::Overlapped)
			{
				analogs = signalSet().analogSignals();
				std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();

				// Analogs does not have calculated trend rect
				//
				QRectF laneRect = TrendImpl::calcLaneRect(m_selectViewLaneIndex, m_trendParam);
				QRectF trendArea = TrendImpl::calcTrendArea(laneRect, m_trendParam, analogs);

				TrendImpl::calcSignalRects(trendArea, m_trendParam, &discretes, &analogs); // calc rects
			}
			else
			{
				analogs.push_back(m_selectViewAreaSignal);
			}

			double top = qMin(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());
			double bottom = qMax(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());

			if (std::fabs(bottom - top) * m_trendParam.realDpiY() <= 1)
			{
				// Value is way too small
				//
				return;
			}

			for (TrendSignalParam& tsp : analogs)
			{
				// Calc time
				//
				QRectF signalRectTsp = tsp.tempDrawRect();

				bool ok = false;

				double highLimit = TrendScale::scaleHighLimit(tsp, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				double lowLimit = TrendScale::scaleLowLimit(tsp, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min() ||
					signalRectTsp.height() <= std::numeric_limits<double>::epsilon())
				{
					// Div by zero possible
					//
					continue;
				}

				double coefHeight = (highLimit - lowLimit) / signalRectTsp.height();

				//				qint64 newHighLimit = static_cast<qint64>(lowLimit + (signalRectTsp.bottom() - top) * coefHeight);
				//				qint64 newLowLimit = static_cast<qint64>(lowLimit + (signalRectTsp.bottom() - bottom) * coefHeight);
				double newHighLimit = lowLimit + (signalRectTsp.bottom() - top) * coefHeight;
				double newLowLimit = lowLimit + (signalRectTsp.bottom() - bottom) * coefHeight;

				newHighLimit = TrendScale::limitFromScaleValue(newHighLimit, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				newLowLimit = TrendScale::limitFromScaleValue(newLowLimit, scaleType(), &ok);
				if (ok == false)
				{
					continue;
				}

				tsp.setViewHighLimit(scaleType(), newHighLimit);
				tsp.setViewLowLimit(scaleType(), newLowLimit);

				signalSet().setSignalParam(tsp);
			}
		}

		return;
	}

	void TrendWidget::startSelectionViewArea()
	{
		m_mouseAction = MouseAction::SelectViewStart;
		unsetCursor();

		setCursor(Qt::CrossCursor);

		return;
	}

	void TrendWidget::updatePixmap(const QImage& image, TrendParam drawParam)
	{
		m_pixmap = QPixmap::fromImage(image);
		m_pixmapDrawParam = drawParam;

		update();
		return;
	}

	TrendLib::TrendSignalSet& TrendWidget::signalSet()
	{
		return m_trend.signalSet();
	}

	const TrendLib::TrendSignalSet& TrendWidget::signalSet() const
	{
		return m_trend.signalSet();
	}

	TrendLib::TrendRulerSet& TrendWidget::rulerSet()
	{
		return m_trend.impl().rulerSet();
	}

	const TrendLib::TrendRulerSet& TrendWidget::rulerSet() const
	{
		return m_trend.impl().rulerSet();
	}

	TrendLib::Trend& TrendWidget::trend()
	{
		return m_trend;
	}

	const TrendLib::Trend& TrendWidget::trend() const
	{
		return m_trend;
	}

	E::TrendViewMode TrendWidget::viewMode() const
	{
		return m_trendParam.viewMode();
	}

	void TrendWidget::setViewMode(E::TrendViewMode value)
	{
		m_trendParam.setViewMode(value);
		return;
	}

	E::TrendScaleType TrendWidget::scaleType() const
	{
		return m_trendParam.scaleType();
	}

	void TrendWidget::setScaleType(E::TrendScaleType value)
	{
		m_trendParam.setScaleType(value);
		return;
	}

	int TrendWidget::laneCount() const
	{
		return m_trendParam.laneCount();
	}

	void TrendWidget::setLaneCount(int value)
	{
		m_trendParam.setLaneCount(value);
	}

	E::TimeType TrendWidget::timeType() const
	{
		return m_trendParam.timeType();
	}

	void TrendWidget::setTimeType(E::TimeType value)
	{
		m_trendParam.setTimeType(value);
	}

	TimeStamp TrendWidget::startTime() const
	{
		TimeStamp ts(m_trendParam.startTime());
		return ts;
	}

	void TrendWidget::setStartTime(const TimeStamp& startTime)
	{
		m_trendParam.setStartTimeStamp(startTime);
	}

	TimeStamp TrendWidget::finishTime() const
	{
		return TimeStamp{TimeStamp{m_trendParam.startTime()}.timeStamp + m_trendParam.duration() * m_trendParam.laneCount()};
	}

	qint64 TrendWidget::duration() const
	{
		return m_trendParam.duration();
	}

	void TrendWidget::setLaneDuration(qint64 interval)
	{
		m_trendParam.setLaneDuration(interval);
	}

	E::TrendMode TrendWidget::trendMode() const
	{
		return m_trendParam.trendMode();
	}

	void TrendWidget::setTrendMode(E::TrendMode value)
	{
		if (m_trendParam.trendMode() != value)
		{
			m_trendParam.setTrendMode(value);
			updateWidget();

			emit trendModeChanged();
		}

		return;
	}

	QString TrendWidget::project() const
	{
		return m_trendParam.project();
	}

	void TrendWidget::setProject(const QString& value)
	{
		m_trendParam.setProject(value);
	}
} // namespace TrendLib
