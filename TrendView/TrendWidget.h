#ifndef TRENDVIEW_H
#define TRENDVIEW_H

#include "Trend.h"

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
		void renderedImage(const QImage& image);

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
	protected:

		// --
		//
	protected slots:
		void updatePixmap(const QImage& image);

		// Signals
	signals:
		void startTimeChanged(TimeStamp startTime);

		// Properties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendView view() const;
		void setView(TrendView value);

		int laneCount() const;
		void setLaneCount(int value);

		void setStartTime(const TimeStamp& startTime);
		void setDuration(qint64 interval);

	private:
		RenderThread m_thread;
		QPixmap m_pixmap;

		Trend m_trend;
		TrendDrawParam m_drawParam;

		// Mouse scroll vaiables
		//
		TimeStamp m_mouseScrollInitialTime;
		QPoint m_mouseScrollInitialMousePos;
	};
}

#endif // TRENDVIEW_H
