#include "TrendWidget.h"
#include <cstdlib>
#include <fstream>
#include <QPaintEngine>
#include <QPainter>
#include <QPrinter>
#include <QWidget>
#include <QPdfWriter>
#include <QMouseEvent>
#include "../Proto/trends.pb.h"

namespace TrendLib
{
	RenderThread::RenderThread(Trend* trend, QObject* parent)
		: QThread(parent),
		m_trend(trend)
	{
		qRegisterMetaType<TrendLib::TrendDrawParam>("TrendDrawParam");

		assert(m_trend);
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

	void RenderThread::render(const TrendDrawParam& drawParam)
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
			TrendDrawParam drawParam = m_drawParam;
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
			assert(errorMessage);
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

		int written = file.write(compressedData);

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
			assert(errorMessage);
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
		QByteArray uncommpressedData = qUncompress(ba);

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
			assert(message);
			return false;
		}

		bool ok = true;

		::Proto::Trend* trendMessage = message->mutable_trend();
		ok &= m_trend.save(trendMessage);

		::Proto::TrendParam* trendParamMessage = message->mutable_trend_param();
		ok &= m_drawParam.save(trendParamMessage);

		return ok;
	}

	bool TrendWidget::load(const ::Proto::TrendWidget& message)
	{
		if (message.IsInitialized() == false ||
			message.has_trend() == false ||
			message.has_trend_param() == false)
		{
			assert(message.IsInitialized());
			assert(message.has_trend());
			assert(message.has_trend_param());
			return false;
		}

		bool ok = true;

		ok &= m_trend.load(message.trend());
		ok &= m_drawParam.load(message.trend_param());

		return ok;
	}

	void TrendWidget::updateWidget()
	{
		m_drawParam.setRect(rect());
		m_drawParam.setDpi(physicalDpiX(), physicalDpiY());

		m_thread.render(m_drawParam);
	}

	bool TrendWidget::saveImageToFile(QString fileName) const
	{
		QPixmap pixmap = m_pixmap;
		TrendDrawParam drawParam = m_pixmapDrawParam;

		{
			QPainter p(&pixmap);
			m_trend.drawRullers(&p, drawParam);
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

		TrendDrawParam drawParam = m_drawParam;

		QRectF rc(pdfWriter.pageLayout().paintRect(QPageLayout::Inch));
		double resolution = pdfWriter.resolution();

		QRectF drawRect(rc.left() * resolution,
						rc.top() * resolution,
						rc.width() * resolution,
						rc.height() * resolution);

		drawParam.setRect(drawRect);
		drawParam.setDpi(resolution, resolution);

		// --
		//
		QPainter p(&pdfWriter);

		m_trend.draw(&p, drawParam, true);
		m_trend.drawRullers(&p, drawParam);

		return true;
	}

	bool TrendWidget::print(QPrinter* printer) const
	{
		if (printer == nullptr ||
			printer->isValid() == false)
		{
			assert(printer);
			return false;
		}

		QPainter painter;

		bool ok = painter.begin(printer);
		if (ok == false)
		{
			assert(ok);
			return false;
		}

		// Prepare DrawParam
		//
		TrendDrawParam drawParam = m_drawParam;

		QRectF rc(printer->pageLayout().paintRect(QPageLayout::Inch));
		double resolution = printer->resolution();

		QRectF drawRect(rc.left() * resolution,
						rc.top() * resolution,
						rc.width() * resolution,
						rc.height() * resolution);

		drawParam.setRect(drawRect);
		drawParam.setDpi(resolution, resolution);

		// Draw to printer
		//
		m_trend.draw(&painter, drawParam, true);
		m_trend.drawRullers(&painter, drawParam);

		// Finish printing
		//
		ok = painter.end();
		if (ok == false)
		{
			assert(ok);
			return false;
		}

		return true;
	}

	void TrendWidget::paintEvent(QPaintEvent*)
	{
		QPainter painter(this);

		if (m_pixmap.isNull() == true)
		{
			painter.fillRect(rect(), m_drawParam.backColor1st());
			painter.setPen(Qt::black);
			painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
			return;
		}

		if (m_pixmap.size() != rect().size())
		{
			// New pixmap is not ready yet, scale the current one
			//
			painter.fillRect(rect(), m_drawParam.backColor1st());
			painter.drawPixmap(m_pixmap.rect(), m_pixmap, m_pixmap.rect());
			return;
		}

		painter.drawPixmap(0, 0, m_pixmap);

		// Draw rullers
		//
		trend().drawRullers(&painter, m_pixmapDrawParam);

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
		int rullerIndex = -1;
		TimeStamp timeStamp;
		QString outSignalId;

		Trend::MouseOn mouseOn = mouseIsOver(event->pos(), &laneIndex, &timeStamp, &rullerIndex, &outSignalId);

		if (m_mouseAction == MouseAction::SelectViewStart &&
			(mouseOn == Trend::MouseOn::InsideTrendArea ||
			 mouseOn == Trend::MouseOn::OnSignalDescription ||
			 mouseOn == Trend::MouseOn::OnRuller))
		{
			if (event->buttons().testFlag(Qt::LeftButton) == false)
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
			if (mouseOn == Trend::MouseOn::OnRuller)
			{
				if (rullerIndex == -1)
				{
					assert(rullerIndex != -1);
					return;
				}

				m_rullerMoveRullerIndex = rullerIndex;
				m_rullerMoveInitialMousePos = event->pos();

				m_rullerMoveInitialTimeStamp = rullerSet().rullers().at(rullerIndex).timeStamp();

				m_mouseAction = MouseAction::MoveRuller;
				this->grabMouse();
			}

			if (mouseOn == Trend::MouseOn::OnSignalDescription)
			{
				if (outSignalId.isEmpty() == true)
				{
					assert(outSignalId.isEmpty() == false);
					return;
				}

				emit showSignalProperties(outSignalId);
			}

			if (mouseOn == Trend::MouseOn::InsideTrendArea)
			{
				m_mouseScrollInitialTime = m_drawParam.startTimeStamp();
				m_mouseScrollInitialMousePos = event->pos();

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

		if (m_mouseAction == MouseAction::MoveRuller)
		{

		}

		if (m_mouseAction == MouseAction::MoveRuller)
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

		if (event->buttons().testFlag(Qt::LeftButton) == false)
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
			int rullerIndex = -1;
			TimeStamp timeStamp;
			QString onSignalId;

			Trend::MouseOn mouseOn = m_trend.mouseIsOver(event->pos(), m_pixmapDrawParam, &laneIndex, &timeStamp, &rullerIndex, &onSignalId);

			Qt::CursorShape newCursorShape = Qt::ArrowCursor;

			switch (mouseOn)
			{
			case Trend::MouseOn::Outside:				newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::OutsideTrendArea:		newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::InsideTrendArea:		newCursorShape = Qt::ArrowCursor;			break;
			case Trend::MouseOn::OnSignalDescription:	newCursorShape = Qt::PointingHandCursor;	break;
			case Trend::MouseOn::OnRuller:				newCursorShape = Qt::SplitHCursor;			break;
			default:
				assert(false);
			}

			m_pixmapDrawParam.setHightlightRullerIndex(rullerIndex);

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
				assert(false);
				break;
			case MouseAction::Scroll:
				{
					// Scroll area with a mouse mode
					//
					QRectF laneRect = m_trend.calcLaneRect(0, m_drawParam);
					QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_drawParam);	// TrendArea in inches
					QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_drawParam);

					double coefx = m_drawParam.duration() / trendAreaRectPixels.width();

					QPointF mouseOffset = m_mouseScrollInitialMousePos - event->pos();

					TimeStamp ts(m_mouseScrollInitialTime.timeStamp + static_cast<qint64>(mouseOffset.x() * coefx));
					m_drawParam.setStartTimeStamp(ts);

					updateWidget();

					emit startTimeChanged(ts);
				}
				break;
			case MouseAction::MoveRuller:
				{
					assert(m_rullerMoveRullerIndex != -1);
					assert(m_rullerMoveRullerIndex >= 0 && m_rullerMoveRullerIndex < static_cast<int>(rullerSet().rullers().size()));

					int laneHeight = rect().height() / laneCount();
					int laneIndex = qBound<int>(0, event->pos().y() / laneHeight, laneCount() - 1);

					QRectF laneRect = m_trend.calcLaneRect(laneIndex, m_drawParam);
					QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_drawParam);	// TrendArea in inches
					QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_drawParam);

					qint64 laneStartTime = m_drawParam.startTimeStamp().timeStamp + m_drawParam.duration() * laneIndex;

					double coefx = m_drawParam.duration() / trendAreaRectPixels.width();

					int mouseOffset = event->pos().x() - trendAreaRectPixels.left();
					mouseOffset = qBound<int>(1, mouseOffset, trendAreaRectPixels.width());

					TimeStamp ts(laneStartTime + static_cast<qint64>(mouseOffset * coefx));

					TrendRuller& mutableRuller = rullerSet().rullers().at(m_rullerMoveRullerIndex);
					mutableRuller.setTimeStamp(ts);

					update();
				}
				break;
			case MouseAction::SelectViewSelectSecondPoint:
				{
					m_finishSelectViewPoint = Trend::pixelPointToInchPoint(event->pos(), m_drawParam);
					update();
				}
				break;

			default:
				assert(false);
				break;
			}
		}

		event->accept();
		return;
	}

	Trend::MouseOn TrendWidget::mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp, int* rullerIndex, QString* outSignalId)
	{
		return m_trend.mouseIsOver(mousePos, m_pixmapDrawParam, outLaneIndex, timeStamp, rullerIndex, outSignalId);
	}

	void TrendWidget::resetRullerHighlight()
	{
		m_pixmapDrawParam.resetHightlightRullerIndex();
	}

	void TrendWidget::initSelectViewArea(QPoint pos, int laneIndex)
	{
		assert(laneIndex != -1);

		m_selectViewLaneIndex = laneIndex;

		QRectF laneRect = Trend::calcLaneRect(laneIndex, m_drawParam);

		int analogsCount = static_cast<int>(signalSet().analogSignalsCount());
		QRectF trendArea = Trend::calcTrendArea(laneRect, m_drawParam, analogsCount);

		m_startSelectViewPoint = Trend::pixelPointToInchPoint(pos, m_drawParam);
		m_finishSelectViewPoint = m_startSelectViewPoint;

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		Trend::calcSignalRects(trendArea, m_drawParam, &discretes, &analogs);

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
		m_finishSelectViewPoint = Trend::pixelPointToInchPoint(pos, m_drawParam);

		double left = qMin(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());
		double right = qMax(m_startSelectViewPoint.x(), m_finishSelectViewPoint.x());

		if (fabs(right - left) * m_drawParam.dpiX() <= 1)
		{
			// Value is way too small
			//
			return;
		}

		// Calc time
		//
		QRectF signalRect = m_selectViewAreaSignal.tempDrawRect();

		qint64 startLaneTime = m_drawParam.startTimeStamp().timeStamp + m_selectViewLaneIndex * m_drawParam.duration();
		double coef = m_drawParam.duration() / signalRect.width();

		qint64 leftTime = startLaneTime + static_cast<qint64>((left - signalRect.left()) * coef);
		qint64 rightTime = startLaneTime + static_cast<qint64>((right - signalRect.left()) * coef);

		// Set new values to controls and draw param
		//
		m_drawParam.setStartTimeStamp(leftTime);
		m_drawParam.setDuration(rightTime - leftTime);

		emit startTimeChanged(leftTime);
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
				QRectF laneRect = Trend::calcLaneRect(m_selectViewLaneIndex, m_drawParam);

				int analogsCount = static_cast<int>(analogs.size());
				QRectF trendArea = Trend::calcTrendArea(laneRect, m_drawParam, analogsCount);

				Trend::calcSignalRects(trendArea, m_drawParam, &discretes, &analogs);  // calc rects
			}
			else
			{

				analogs.push_back(m_selectViewAreaSignal);
			}

			double top = qMin(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());
			double bottom = qMax(m_startSelectViewPoint.y(), m_finishSelectViewPoint.y());

			if (fabs(bottom - top) * m_drawParam.dpiY() <= 1)
			{
				// Value is way too small
				//
				return;
			}

			for (TrendSignalParam& tsp : analogs)
			{
				// Calc time
				//
				QRectF signalRect = tsp.tempDrawRect();

				double highLimit = qMax(tsp.viewHighLimit(), tsp.viewLowLimit());
				double lowLimit = qMin(tsp.viewHighLimit(), tsp.viewLowLimit());

				if (fabs(highLimit - lowLimit) <= DBL_MIN)
				{
					// Div by zero possible
					//
					continue;
				}

				double coef = (highLimit - lowLimit) / signalRect.height();

				qint64 newHighLimit = lowLimit + (signalRect.bottom() - top) * coef;
				qint64 newLowLimit = lowLimit + (signalRect.bottom() - bottom) * coef;

				tsp.setViewLowLimit(newLowLimit);
				tsp.setViewHighLimit(newHighLimit);

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

	void TrendWidget::updatePixmap(const QImage& image, TrendDrawParam drawParam)
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

	TrendLib::TrendRullerSet& TrendWidget::rullerSet()
	{
		return m_trend.rullerSet();
	}

	const TrendLib::TrendRullerSet& TrendWidget::rullerSet() const
	{
		return m_trend.rullerSet();
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
		return m_drawParam.viewMode();
	}

	void TrendWidget::setViewMode(TrendViewMode value)
	{
		m_drawParam.setViewMode(value);
		return;
	}

	int TrendWidget::laneCount() const
	{
		return m_drawParam.laneCount();
	}

	void TrendWidget::setLaneCount(int value)
	{
		m_drawParam.setLaneCount(value);
	}

	TimeType TrendWidget::timeType() const
	{
		return m_drawParam.timeType();
	}

	void TrendWidget::setTimeType(TimeType value)
	{
		m_drawParam.setTimeType(value);
	}

	TimeStamp TrendWidget::startTime() const
	{
		TimeStamp ts(m_drawParam.startTime());
		return ts;
	}

	void TrendWidget::setStartTime(const TimeStamp& startTime)
	{
		m_drawParam.setStartTimeStamp(startTime);
	}

	qint64 TrendWidget::duration() const
	{
		return m_drawParam.duration();
	}

	void TrendWidget::setDuration(qint64 interval)
	{
		m_drawParam.setDuration(interval);
	}

}
