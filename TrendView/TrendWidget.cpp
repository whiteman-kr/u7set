#include "TrendWidget.h"
#include "../Proto/trends.pb.h"
#include "TrendScale.h"

namespace TrendLib
{
	RenderThread::RenderThread(Trend* trend, QObject* parent)
		: QThread(parent),
		m_trend(trend)
	{
		qRegisterMetaType<TrendLib::TrendParam>("TrendParam");

		Q_ASSERT(m_trend);
	}

	RenderThread::~RenderThread()
	{
		requestInterruption();

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
		QMutexLocker locker(&m_mutex);

		m_drawParam = drawParam;
		m_newJob = true;

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
			QThread::msleep(5);

			if (m_newJob == false)
			{
				continue;
			}

			// Start new job
			//
			m_mutex.lock();
			TrendParam drawParam = m_drawParam;
			drawParam.signalDescriptionRect().clear();
			m_mutex.unlock();

			// Set m_newJob to false, so it can be raised again while current drawing in progress
			//
			m_newJob = false;

			if (m_image.size() != drawParam.rect().size())
			{
				QSize pixelSize = drawParam.rect().size().toSize();
				m_image = QImage(pixelSize, QImage::Format_RGB32);
			}

			// All drawing are done in inches
			//
			m_trend->draw(&m_image, drawParam);

			emit renderedImage(m_image, drawParam);

		}
		while (isInterruptionRequested() == false);

		return;
	}


	TrendWidget::TrendWidget(QWidget* parent) :
		QWidget(parent),
		m_thread(&m_trend)
	{
		setMouseTracking(true);		// To enable mouseMoveEvent without pressed button

		connect(&m_thread, &RenderThread::renderedImage, this, &TrendWidget::updatePixmap);
	}

	TrendWidget::~TrendWidget()
	{
		m_thread.requestInterruption();
		bool finished = m_thread.wait(10000);

		if (finished == false)
		{
			qDebug() << "Force thread termination TrendLib::RenderThread";
			m_thread.terminate();
		}

		return;
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
		serializedString.reserve(message.ByteSize());

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
		qDebug() << ba.size();

		// Uncompress data
		//
		QByteArray uncommpressedData = qUncompress(ba);
		qDebug() << uncommpressedData.size();

		// Deserialize
		//
		::Proto::TrendWidget message;

		ok = message.ParseFromArray(uncommpressedData.constData(), uncommpressedData.size());

		if (ok == false)
		{
			*errorMessage = tr("Parse trend file error. ") + strerror(errno);
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
		ok &= m_trend.save(trendMessage);

		::Proto::TrendParam* trendParamMessage = message->mutable_trend_param();
		ok &= m_trendParam.save(trendParamMessage);

		return ok;
	}

	bool TrendWidget::load(const ::Proto::TrendWidget& message)
	{
		if (message.IsInitialized() == false ||
			message.has_trend() == false ||
			message.has_trend_param() == false)
		{
			Q_ASSERT(message.IsInitialized());
			Q_ASSERT(message.has_trend());
			Q_ASSERT(message.has_trend_param());
			return false;
		}

		bool ok = true;

		ok &= m_trend.load(message.trend());
		ok &= m_trendParam.load(message.trend_param());

		return ok;
	}

	void TrendWidget::updateWidget()
	{
		m_trendParam.setRect(rect());
		m_trendParam.setDpi(logicalDpiX(), logicalDpiY());

		m_thread.render(m_trendParam);
	}

	bool TrendWidget::saveImageToFile(QString fileName) const
	{
		QPixmap pixmap = m_pixmap;
		TrendParam drawParam = m_pixmapDrawParam;

		{
			QPainter p(&pixmap);
			m_trend.drawRulers(&p, drawParam);
		}

		bool ok = pixmap.save(fileName, nullptr, -1);

		return ok;
	}

	bool TrendWidget::saveToPdf(QString fileName, QPageSize::PageSizeId pageSize, QPageLayout::Orientation pageOrientation) const
	{
		QPdfWriter pdfWriter(fileName);

		pdfWriter.setTitle("Trends");
		pdfWriter.setPageSize(QPageSize(pageSize));
		pdfWriter.setPageOrientation(pageOrientation);
		pdfWriter.pageLayout().setUnits(QPageLayout::Inch);

		TrendParam drawParam = m_trendParam;

		QRectF rc(pdfWriter.pageLayout().paintRect(QPageLayout::Inch));
		int resolution = pdfWriter.resolution();

		QRectF drawRect(rc.left() * static_cast<double>(resolution),
						rc.top() * static_cast<double>(resolution),
						rc.width() * static_cast<double>(resolution),
						rc.height() * static_cast<double>(resolution));

		drawParam.setRect(drawRect);
		drawParam.setDpi(resolution, resolution);

		// --
		//
		QPainter p(&pdfWriter);

		m_trend.draw(&p, drawParam, true);
		m_trend.drawRulers(&p, drawParam);

		return true;
	}

	bool TrendWidget::print(QPrinter* printer) const
	{
		if (printer == nullptr ||
			printer->isValid() == false)
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

		// Prepare DrawParam
		//
		TrendParam drawParam = m_trendParam;

		QRectF rc{printer->pageLayout().paintRect(QPageLayout::Inch)};
		int resolution = printer->resolution();

		QRectF drawRect{rc.left() * static_cast<double>(resolution),
						rc.top() * static_cast<double>(resolution),
						rc.width() * static_cast<double>(resolution),
						rc.height() * static_cast<double>(resolution)};

		drawParam.setRect(drawRect);
		drawParam.setDpi(resolution, resolution);

		// Draw to printer
		//
		m_trend.draw(&painter, drawParam, true);
		m_trend.drawRulers(&painter, drawParam);

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

		if (m_pixmap.size() != rect().size())
		{
			// New pixmap is not ready yet, scale the current one
			//
			painter.fillRect(rect(), m_trendParam.backColor1st());
			painter.drawPixmap(m_pixmap.rect(), m_pixmap, m_pixmap.rect());
			return;
		}

		painter.drawPixmap(0, 0, m_pixmap);

		// Draw rulers
		//
		trend().drawRulers(&painter, m_pixmapDrawParam);

		// Draw select view area
		//
		if (m_mouseAction == MouseAction::SelectViewSelectSecondPoint)
		{
			Trend::adjustPainter(&painter, m_pixmapDrawParam.dpiX(), m_pixmapDrawParam.dpiY());

			QRectF selectionRect(m_startSelectViewPoint, m_finishSelectViewPoint);
			selectionRect = selectionRect.normalized();

			selectionRect = selectionRect.intersected(m_selectViewAreaSignal.tempDrawRect()).normalized();

			if (selectionRect.isEmpty() == false)
			{
				//painter.fillRect(m_allowedSelectViewArea, Qt::blue);

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
		int discretesCount = signalSet().discretesSignalsCount();

		if (analogsCount + discretesCount == 0)
		{
			return;
		}

		int laneIndex = -1;
		int rulerIndex = -1;
		TimeStamp timeStamp;
		TrendSignalParam outSignal;

		Trend::MouseOn mouseOn = mouseIsOver(event->pos(), &laneIndex, &timeStamp, &rulerIndex, &outSignal);

		if (m_mouseAction == MouseAction::SelectViewStart &&
			(mouseOn == Trend::MouseOn::InsideTrendArea ||
			 mouseOn == Trend::MouseOn::OnSignalDescription ||
			 mouseOn == Trend::MouseOn::OnRuler))
		{
			if (event->buttons().testFlag(Qt::LeftButton) == false &&
				event->buttons().testFlag(Qt::MiddleButton) == false)
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
			if (mouseOn == Trend::MouseOn::OnRuler)
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

			if (mouseOn == Trend::MouseOn::OnSignalDescription)
			{
				if (outSignal.appSignalId().isEmpty() == true)
				{
					Q_ASSERT(outSignal.appSignalId().isEmpty() == false);
					return;
				}

				emit showSignalProperties(outSignal.appSignalId());
			}
		}

		if (event->buttons().testFlag(Qt::LeftButton) == true ||
			event->buttons().testFlag(Qt::MiddleButton) == true)
		{
			if (mouseOn == Trend::MouseOn::InsideTrendArea)
			{
				m_mouseScrollInitialTime = m_trendParam.startTimeStamp();
				m_mouseScrollInitialMousePos = event->pos();
				m_mouseScrollSignal = outSignal;								// tempDrawRect already calculated

				m_mouseScrollAnalogSignals = signalSet().analogSignals();		// tempDrawRect is not calculated

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

		if (m_mouseAction == MouseAction::MoveRuler)
		{
		}

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

		if (event->buttons().testFlag(Qt::LeftButton) == false &&
			event->buttons().testFlag(Qt::MiddleButton) == false)
		{
			unsetCursor();
			mouseMoveEvent(event);		// To set cursor
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

			Trend::MouseOn mouseOn = m_trend.mouseIsOver(event->pos(), m_pixmapDrawParam, &laneIndex, &timeStamp, &rulerIndex, &onSignal);

			Qt::CursorShape newCursorShape = Qt::ArrowCursor;

			switch (mouseOn)
			{
			case Trend::MouseOn::Outside:				newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::OutsideTrendArea:		newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::InsideTrendArea:		newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::OnSignalDescription:	newCursorShape = Qt::PointingHandCursor;	break;
			case Trend::MouseOn::OnRuler:				newCursorShape = Qt::SplitHCursor;			break;
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
					QRectF laneRect = m_trend.calcLaneRect(0, m_trendParam);
					QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_trendParam);	// TrendArea in inches
					QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_trendParam);

					double coefx = m_trendParam.duration() / trendAreaRectPixels.width();

					QPointF mouseOffset = m_mouseScrollInitialMousePos - event->pos();

					TimeStamp ts(m_mouseScrollInitialTime.timeStamp + static_cast<qint64>(mouseOffset.x() * coefx));
					m_trendParam.setStartTimeStamp(ts);

					// Scroll vertical area
					//
					if (event->modifiers().testFlag(Qt::AltModifier) == true)
					{
						std::vector<TrendSignalParam> analogsToShift;

						if (m_trendParam.viewMode() == TrendViewMode::Separated)
						{
							analogsToShift.push_back(m_mouseScrollSignal);		// signalRect is calculated
						}
						else
						{
							analogsToShift = m_mouseScrollAnalogSignals;		// signalRect is not calculated yet
							auto discretes = signalSet().discreteSignals();

							Trend::calcSignalRects(trenAreaRect, m_trendParam, &discretes, &analogsToShift);
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

							if (std::fabs(highLimit - lowLimit) > DBL_MIN &&
								signalRect.height() > DBL_MIN)
							{
								double dy = mouseOffset.y() / m_trendParam.dpiY();
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

					QRectF laneRect = m_trend.calcLaneRect(laneIndex, m_trendParam);
					QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_trendParam);	// TrendArea in inches
					QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_trendParam);

					qint64 laneStartTime = m_trendParam.startTimeStamp().timeStamp + m_trendParam.duration() * laneIndex;

					double coefx = m_trendParam.duration() / trendAreaRectPixels.width();

					int mouseOffset = static_cast<int>(event->pos().x() - trendAreaRectPixels.left());
					mouseOffset = qBound<int>(1, mouseOffset, static_cast<int>(trendAreaRectPixels.width()));

					TimeStamp ts(laneStartTime + static_cast<qint64>(mouseOffset * coefx));

					TrendRuler& mutableRuler = rulerSet().rulers().at(m_rulerMoveRulerIndex);
					mutableRuler.setTimeStamp(ts, trend().rulerSet().rulerStep());

					update();
				}
				break;
			case MouseAction::SelectViewSelectSecondPoint:
				{
					m_finishSelectViewPoint = Trend::pixelPointToInchPoint(event->pos(), m_trendParam);
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
		if (event->buttons().testFlag(Qt::MidButton))
		{
			return;
		}

		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15;

		if (numSteps == 0)
		{
			event->ignore();
			return;
		}

		bool needUpdateWidget = false;

		// Calc time
		//
		if (event->modifiers().testFlag(Qt::AltModifier) == false)
		{
			qint64 startTime = m_trendParam.startTimeStamp().timeStamp;

			qint64 oldDuration = m_trendParam.duration();
			qint64 newLaneDuration = oldDuration;

			if (numSteps < 0)
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
			m_trendParam.setLaneDuration(newLaneDuration);		// This func can limit value

			if (m_trendParam.duration() != oldDuration)
			{
				m_trendParam.setStartTimeStamp(TimeStamp{startTime});

				emit startTimeChanged(TimeStamp{startTime});
				emit durationChanged(newLaneDuration);

				needUpdateWidget = true;
			}
		}

		if (event->modifiers().testFlag(Qt::AltModifier) == true)
		{
			// Scale analog signals
			//
			std::vector<TrendSignalParam> signalsToScale;

			if (m_trendParam.viewMode() == TrendViewMode::Overlapped)
			{
				// Scale all analog signals
				//
				signalsToScale = signalSet().analogSignals();
			}

			if (m_trendParam.viewMode() == TrendViewMode::Separated)
			{
				// Scale analog signal where is mouse now
				//
				int laneIndex = -1;
				int rulerIndex = -1;
				TimeStamp timeStamp;
				TrendSignalParam trendSignal;

				Trend::MouseOn mouseOn = mouseIsOver(event->pos(), &laneIndex, &timeStamp, &rulerIndex, &trendSignal);

				if (mouseOn != Trend::MouseOn::OutsideTrendArea &&
					mouseOn != Trend::MouseOn::Outside &&
					trendSignal.appSignalId().isEmpty() == false &&
					trendSignal.isAnalog() == true)
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
				if (delta < DBL_MIN)
				{
					continue;
				}

				if (numSteps > 0)
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

	Trend::MouseOn TrendWidget::mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp, int* rulerIndex, TrendSignalParam* onSignal)
	{
		return m_trend.mouseIsOver(mousePos, m_pixmapDrawParam, outLaneIndex, timeStamp, rulerIndex, onSignal);
	}

	void TrendWidget::resetRulerHighlight()
	{
		m_pixmapDrawParam.resetHightlightRulerIndex();
	}

	void TrendWidget::initSelectViewArea(QPoint pos, int laneIndex)
	{
		Q_ASSERT(laneIndex != -1);

		m_selectViewLaneIndex = laneIndex;

		QRectF laneRect = Trend::calcLaneRect(laneIndex, m_trendParam);

		int analogsCount = static_cast<int>(signalSet().analogSignalsCount());
		QRectF trendArea = Trend::calcTrendArea(laneRect, m_trendParam, analogsCount);

		m_startSelectViewPoint = Trend::pixelPointToInchPoint(pos, m_trendParam);
		m_finishSelectViewPoint = m_startSelectViewPoint;

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		Trend::calcSignalRects(trendArea, m_trendParam, &discretes, &analogs);

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
		m_finishSelectViewPoint = Trend::pixelPointToInchPoint(pos, m_trendParam);

		double left = qMin(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());
		double right = qMax(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());

		if (std::fabs(right - left) * m_trendParam.dpiX() <= 1)
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

			if (viewMode() == TrendLib::TrendViewMode::Overlapped)
			{
				analogs = signalSet().analogSignals();
				std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();

				// Analogs does not have calculated trend rect
				//
				QRectF laneRect = Trend::calcLaneRect(m_selectViewLaneIndex, m_trendParam);

				int analogsCount = static_cast<int>(analogs.size());
				QRectF trendArea = Trend::calcTrendArea(laneRect, m_trendParam, analogsCount);

				Trend::calcSignalRects(trendArea, m_trendParam, &discretes, &analogs);  // calc rects
			}
			else
			{

				analogs.push_back(m_selectViewAreaSignal);
			}

			double top = qMin(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());
			double bottom = qMax(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());

			if (std::fabs(bottom - top) * m_trendParam.dpiY() <= 1)
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

				if (std::fabs(highLimit - lowLimit) <= DBL_MIN ||
					signalRectTsp.height() <= DBL_EPSILON)
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
		return m_trend.rulerSet();
	}

	const TrendLib::TrendRulerSet& TrendWidget::rulerSet() const
	{
		return m_trend.rulerSet();
	}

	TrendLib::Trend& TrendWidget::trend()
	{
		return m_trend;
	}

	const TrendLib::Trend& TrendWidget::trend() const
	{
		return m_trend;
	}

	TrendViewMode TrendWidget::viewMode() const
	{
		return m_trendParam.viewMode();
	}

	void TrendWidget::setViewMode(TrendViewMode value)
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

}
