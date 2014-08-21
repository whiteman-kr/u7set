#pragma once

#include "VideoFrameWidgetAgent.h"
#include "FrameHistoryItem.h"
#include <QScrollArea>

namespace VFrame30
{
	class CVideoFrame;
	class VideoFrameView;
	class VideoFrameManager;

	//
	//
	// VideoFrameBaseWidget -- QScrollArea with VideoFrameView viewport
	//
	//
	class VFRAME30LIBSHARED_EXPORT VideoFrameBaseWidget : public QScrollArea
	{
		Q_OBJECT

	private:
		VideoFrameBaseWidget();	// DELETED

	public:
		explicit VideoFrameBaseWidget(std::shared_ptr<CVideoFrame> videoFrame);
		virtual ~VideoFrameBaseWidget();

	protected:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void wheelEvent(QWheelEvent* event);

		// Methods
		//
	protected:
		std::shared_ptr<CVideoFrame>& videoFrame();

		void setVideoFrame(std::shared_ptr<CVideoFrame>& videoFrame);

		VideoFrameView* videoFrameView();

		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

		// Properties
		//
	public:
		double zoom() const;
		void setZoom(double value, int horzScrollValue = -1, int vertScrollValue = -1);

		// Data
		//
	private:
		// Interface data
		//
		QPoint mousePos;		// Keeps mouse pos during different actions like scrolling etc
		int horzScrollBarValue;	// Horizintal scroll bar value in mousePressEvent -- midButton
		int vertScrollBarValue;	// Vertical scroll bar value in mousePressEvent -- midButton

		VideoFrameView* m_pVideoFrameView;
	};

//	//
//	//
//	// VideoFrameWidget -- QScrollArea with VideoFrameView viewport
//	//
//	//
//	class VFRAME30LIBSHARED_EXPORT VideoFrameWidget : public VideoFrameBaseWidget
//	{
//		Q_OBJECT

//		friend VideoFrameWidgetAgent;

//	private:
//		VideoFrameWidget();
//	public:
//		VideoFrameWidget(VideoFrameManager& vfManager, const QString& videoFrameStrID);
//		virtual ~VideoFrameWidget();

//	protected:
//		virtual void mousePressEvent(QMouseEvent* event) override;
//		virtual void mouseReleaseEvent(QMouseEvent* event) override;

//		virtual void mouseMoveEvent(QMouseEvent* event) override;
//		virtual void wheelEvent(QWheelEvent* event);

//		// Methods
//		//
//	protected:
//		std::shared_ptr<const CVideoFrame>& videoFrame();
//		VideoFrameView* videoFrameView();
//		VideoFrameManager& videoFrameManager();

//		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

//	public:
//		bool setCurrentVideoFrame(const QString& strID, bool repaint, bool addHistory = true);

//		// VideoFrames switch history methods
//		//
//	public:
//		bool canSwitchBack() const;
//		bool canSwitchForward() const;

//		void resetHistory();

//		void switchBack();
//		void switchForward();

//	protected:
//		bool addHistoryItem(const QString& oldStrID, const QString& newStrID, double zoom, int horzScrollValue, int vertScrollValue);

//	signals:
//		void historyChanged(bool canSwitchBack, bool canSwitchForward);

//		// Properties
//		//
//	public:
//		double zoom() const;
//		void setZoom(double value, int horzScrollValue = -1, int vertScrollValue = -1);

//		// Data
//		//
//	private:
//		// Interface data
//		//
//		QPoint mousePos;		// Keeps mouse pos during different actions like scrolling etc
//		int horzScrollBarValue;	// Horizintal scroll bar value in mousePressEvent -- midButton
//		int vertScrollBarValue;	// Vertical scroll bar value in mousePressEvent -- midButton

//		VideoFrameView* m_pVideoFrameView;

//		// VideoFrames Data
//		//
//		VideoFrameManager& m_refVideoFrameManager;

//		// Frame switch history
//		//
//		std::list<FrameHistoryItem> backHistory;
//		std::list<FrameHistoryItem> forwardHistory;
//	};

}


