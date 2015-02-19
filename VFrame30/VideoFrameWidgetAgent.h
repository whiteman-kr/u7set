#pragma once

#include <QObject>
//#include <../QtScript/QScriptable>

namespace VFrame30
{
	class VideoFrameBaseWidget;
	class VideoFrameAgent;

	// VideoFrameWiget Agent for scripts
	//
	class VideoFrameWidgetAgent : public QObject//, protected QScriptable
	{
		Q_OBJECT
//		Q_PROPERTY(double zoom READ zoom WRITE setZoom)
//		Q_PROPERTY(QScriptValue currentVideoFrame READ currentVideoFrame WRITE setCurrentVideoFrame)

//	private:
//		VideoFrameWidgetAgent();

//	public:
//		explicit VideoFrameWidgetAgent(VideoFrameBaseWidget* pVideoFrameWidget);
//		virtual ~VideoFrameWidgetAgent();

//		bool isValide() const;

//		// Properties
//		//
//		double zoom() const;
//		void setZoom(double value);

//		QScriptValue currentVideoFrame() const;
//		void setCurrentVideoFrame(const QScriptValue& videoFrame);
		
//		// Public methods for scripts
//		//
//	public slots:

//		// Data
//		//
//	private:
//		VideoFrameBaseWidget* m_pVideoFrameWidget;			// class doesn't own this object, just use it
	};

}


