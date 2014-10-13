#pragma once

#include "VideoFrame.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT VideoFrameView : public QWidget
	{
		Q_OBJECT

	public:
		explicit VideoFrameView(QWidget *parent = 0);
		explicit VideoFrameView(std::shared_ptr<VFrame30::Scheme>& videoFrame, QWidget *parent = 0);

	protected:
		void init();
		
		// Painting
		//
	protected:
		virtual void paintEvent(QPaintEvent*) override;

		void Ajust(QPainter* painter, double startX, double startY) const;

		// Methods
	public:
		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

		std::shared_ptr<Scheme>& videoFrame();
		std::shared_ptr<Scheme> videoFrame() const;

		void setVideoFrame(std::shared_ptr<Scheme>& videoFrame, bool repaint);
		
		// Events
		//
	protected:
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Properties
		//
	public:
		double zoom() const;
		void setZoom(double value, bool repaint = true, int dpiX = 0, int dpiY = 0);

		// Data
		//
	private:
		std::shared_ptr<VFrame30::Scheme> m_videoFrame;
		double m_zoom;
	};
}


