#ifndef TRENDVIEW_H
#define TRENDVIEW_H

#include "Trend.h"
#include "TrendRuller.h"

namespace TrendLib
{
	class RenderThread : public QThread
	{
		Q_OBJECT

	public:
		explicit RenderThread(Trend* trend, QObject* parent = 0);
		virtual ~RenderThread();

	public:
		void render(const TrendDrawParam& drawParam);

	signals:
		void renderedImage(const QImage& image, TrendDrawParam drawParam);

	protected:
		virtual void run() override;

	private:
		Trend* m_trend = nullptr;

		QMutex m_mutex;
		TrendDrawParam m_drawParam;

		bool m_newJob = false;

		// Draw cache
		//
		QImage m_image;
	};


	class TrendWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TrendWidget(QWidget* parent = nullptr);
		virtual ~TrendWidget();

	public:
		void updateWidget();

		bool saveImageToFile(QString fileName) const;
		bool saveToPdf(QString fileName, QPageSize::PageSizeId pageSize, QPageLayout::Orientation pageOrientation) const;

		// Events
		//
	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void resizeEvent(QResizeEvent*) override;

		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Methods
		//
	public:
		Trend::MouseOn mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp, int* rullerIndex, QString* outSignalId);

		void resetRullerHighlight();

	protected:

		// --
		//
	protected slots:
		void updatePixmap(const QImage& image, TrendDrawParam drawParam);

		// Signals
	signals:
		void startTimeChanged(TimeStamp startTime);
		void showSignalProperties(QString appSignalId);

		// Properties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::TrendRullerSet& rullerSet();
		const TrendLib::TrendRullerSet& rullerSet() const;

		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		TrendView view() const;
		void setView(TrendView value);

		int laneCount() const;
		void setLaneCount(int value);

		TimeType timeType() const;
		void setTimeType(TimeType value);

		void setStartTime(const TimeStamp& startTime);
		void setDuration(qint64 interval);

	private:
		RenderThread m_thread;
		QPixmap m_pixmap;
		TrendDrawParam m_pixmapDrawParam;			// DrawParmas whcih was used to generate m_pixmap;

		Trend m_trend;
		TrendDrawParam m_drawParam;

		enum class MouseAction
		{
			None,
			Scroll,
			MoveRuller,
			SelectView
		};

		MouseAction m_mouseAction = MouseAction::None;

		// MouseScroll/RullerMove/... vaiables
		//
		TimeStamp m_mouseScrollInitialTime;
		QPoint m_mouseScrollInitialMousePos;

		int m_rullerMoveRullerIndex = -1;
		QPoint m_rullerMoveInitialMousePos;
		TimeStamp m_rullerMoveInitialTimeStamp;
	};
}

#endif // TRENDVIEW_H
