#ifndef TRENDVIEW_H
#define TRENDVIEW_H

#include <QWaitCondition>
#include "TrendSignal.h"
#include "TrendDrawParam.h"

namespace TrendLib
{
	class RenderThread : public QThread
	{
		Q_OBJECT

	public:
		explicit RenderThread(TrendSignalSet* signalSet, QObject* parent = 0);
		virtual ~RenderThread();

		void render(const TrendDrawParam& drawParam);

	private:
		void drawLane(QPainter* painter, const QRectF& rect, const TrendDrawParam& drawParam);

		void drawSignal(QPainter* painter, const TrendSignal& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor);
		void drawDiscrete(QPainter* painter, const TrendSignal& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor);
		void drawAnalog(QPainter* painter, const TrendSignal& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor);

		static double timeToPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration);
		static void drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendDrawParam& drawParam, int flags, QRectF* boundingRect = nullptr);

	signals:
		void renderedImage(const QImage& image);

	protected:
		virtual void run() override;

	private:
		TrendSignalSet* m_signalSet = nullptr;

		QMutex m_mutex;
		QWaitCondition m_condition;

		TrendDrawParam m_drawParam;

		bool m_restart = false;
		volatile bool m_abort = false;

		// Draw cache
		//
		QImage m_image;
	};


	class TrendWidget : public QWidget
	{
		Q_OBJECT

	public:
		TrendWidget(TrendSignalSet* signalSet, QWidget* parent = nullptr);
		virtual ~TrendWidget();

	public:
		void updateWidget();

	protected:
		virtual void paintEvent(QPaintEvent* event);
		virtual void resizeEvent(QResizeEvent*);

	protected slots:
		void updatePixmap(const QImage& image);

	public:
		TrendView view() const;
		void setView(TrendView value);

		int laneCount() const;
		void setLaneCount(int value);

		void setStartTime(const TimeStamp& startTime);
		void setDuration(qint64 interval);

	private:
		RenderThread m_thread;
		QPixmap m_pixmap;

		TrendSignalSet* m_signalSet;
		TrendDrawParam m_drawParam;
	};
}

#endif // TRENDVIEW_H
