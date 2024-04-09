#pragma once

#include "./include/TrendView/Trend.h"
#include "./include/TrendView/ITrendDataProvider.h"
#include "./include/TrendView/Trendparam.h"

#include "TrendImpl.h"
#include "TrendRuler.h"
#include <mutex>
#include <condition_variable>
#include <QPixmap>
#include <QImage>
#include <QWidget>
#include <QPageSize>
#include <QPageLayout>


class QPrinter;

namespace Proto
{
	class TrendWidget;
}


namespace TrendLib
{
	class RenderThread : public QThread
	{
		Q_OBJECT

	public:
		explicit RenderThread(TrendImpl* trendImpl, ITrendDataProvider* dataProvider, QObject* parent = nullptr);
		virtual ~RenderThread();

	public:
		void render(const TrendParam& drawParam);

	signals:
		void renderedImage(const QImage& image, TrendLib::TrendParam drawParam);

	protected:
		virtual void run() override;

	private:
		TrendImpl* m_trendImpl = nullptr;

		std::mutex m_mutex;
		std::optional<TrendParam> m_drawParam;
		bool m_interruptRequested = false;

		std::condition_variable m_newJob;

		// Draw cache
		//
		QImage m_image;
	};


	class TrendWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TrendWidget(QWidget* parent = nullptr);
		virtual ~TrendWidget() = default;

	public:
		bool save(QString fileName, QString* errorMessage) const;
		bool load(QString fileName, QString* errorMessage);

		bool save(::Proto::TrendWidget* message) const;
		bool load(const ::Proto::TrendWidget& message);

	public:
		void updateWidget();

		bool saveImageToFile(QString fileName) const;
		bool saveToPdf(QString fileName, QPageSize::PageSizeId pageSize, QPageLayout::Orientation pageOrientation) const;
		bool print(QPrinter* printer) const;

		// Events
		//
	protected:
		virtual void paintEvent(QPaintEvent* event) override;

		virtual void resizeEvent(QResizeEvent*) override;

		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event) override;

		// Methods
		//
	public:
		TrendImpl::MouseOn mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp, int* rulerIndex, TrendSignalParam* outSignalId);

		void resetRulerHighlight();

	protected:
		void initSelectViewArea(QPoint pos, int laneIndex);
		void selectViewArea(QPoint pos);

		// slots
		//
	public slots:
		void startSelectionViewArea();

	protected slots:
		void updatePixmap(const QImage& image, TrendLib::TrendParam drawParam);

		// Signals
	signals:
		void startTimeChanged(TimeStamp startTime);
		void durationChanged(qint64 duration);
		void showSignalProperties(QString appSignalId, QString archiveServerId);

		void trendModeChanged();

		// Properties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::TrendRulerSet& rulerSet();
		const TrendLib::TrendRulerSet& rulerSet() const;

		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		TrendLib::TrendImpl& trendImpl();
		const TrendLib::TrendImpl& trendImpl() const;

		E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		int laneCount() const;
		void setLaneCount(int value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		TimeStamp startTime() const;
		void setStartTime(const TimeStamp& startTime);

		TimeStamp finishTime() const;

		qint64 duration() const;
		void setLaneDuration(qint64 interval);

		E::TrendMode trendMode() const;
		void setTrendMode(E::TrendMode value);

	private:
		Trend m_trend;
		TrendParam m_trendParam;

		RenderThread m_thread;
		QPixmap m_pixmap;
		TrendParam m_pixmapDrawParam;			// DrawParmas whcih was used to generate m_pixmap;

		enum class MouseAction
		{
			None,
			Scroll,
			MoveRuler,
			SelectViewStart,
			SelectViewSelectSecondPoint
		};

		MouseAction m_mouseAction = MouseAction::None;

		// MouseScroll/RulerMove/... vaiables
		//
		TimeStamp m_mouseScrollInitialTime;
		QPoint m_mouseScrollInitialMousePos;
		TrendSignalParam m_mouseScrollSignal;
		std::vector<TrendSignalParam> m_mouseScrollAnalogSignals;

		int m_rulerMoveRulerIndex = -1;
		QPoint m_rulerMoveInitialMousePos;
		TimeStamp m_rulerMoveInitialTimeStamp;

		QPointF m_startSelectViewPoint;
		QPointF m_finishSelectViewPoint;
		int m_selectViewLaneIndex;
		TrendSignalParam m_selectViewAreaSignal;
	};
}

