#ifndef TRENDVIEW_H
#define TRENDVIEW_H

#include <QWaitCondition>

namespace TrendLib
{
	class RenderThread : public QThread
	{
		Q_OBJECT

	public:
		RenderThread(QObject* parent = 0);
		virtual ~RenderThread();

		void render(double centerX, double centerY, double scaleFactor, QSize resultSize);

	signals:
		void renderedImage(const QImage& image, double scaleFactor);

	protected:
		virtual void run() override;

	private:
		QMutex m_mutex;
		QWaitCondition m_condition;

		double m_centerX = 0;
		double m_centerY = 0;
		double m_scaleFactor = 0;
		QSize m_resultSize = {0, 0};

		bool m_restart = false;
		bool m_abort = false;
	};


	class TrendWidget : public QWidget
	{
		Q_OBJECT

	public:
		explicit TrendWidget(QWidget* parent = nullptr);
		virtual ~TrendWidget();

	public:
		void updateWidget();

	protected:
		virtual void paintEvent(QPaintEvent* event);
		virtual void resizeEvent(QResizeEvent*);

	protected slots:
		void updatePixmap(const QImage& image, double scaleFactor);

	private:
		RenderThread m_thread;
		QPixmap m_pixmap;
	};
}

#endif // TRENDVIEW_H
