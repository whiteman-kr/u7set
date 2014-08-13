#include "Stable.h"
#include "VideoFrameWidgetAgent.h"
#include "VideoFrameWidget.h"
#include "VideoFrameAgent.h"

namespace VFrame30
{
//	VideoFrameWidgetAgent::VideoFrameWidgetAgent(VideoFrameBaseWidget* pVideoFrameWidget) :
//		m_pVideoFrameWidget(pVideoFrameWidget)
//	{
//		assert(pVideoFrameWidget != nullptr);
//		//int metaTypeID = qScriptRegisterMetaType<VideoFrameAgent*>(m_pScriptEngine, VFrame30::VideoFrameAgent::videoFrameAgentToScriptValue, VFrame30::VideoFrameAgent::videoFrameAgentFromScriptValue);
//		//qDebug() << metaTypeID;
//	}

//	VideoFrameWidgetAgent::~VideoFrameWidgetAgent()
//	{
//	}

//	bool VideoFrameWidgetAgent::isValide() const
//	{
//		if (m_pVideoFrameWidget == nullptr)
//		{
//			assert(m_pVideoFrameWidget != nullptr);
//			return false;
//		}

//		return true;
//	}

//	double VideoFrameWidgetAgent::zoom() const
//	{
//		if (isValide() == false)
//		{
//			return 0;
//		}

//		return m_pVideoFrameWidget->zoom();
//	}

//	void VideoFrameWidgetAgent::setZoom(double value)
//	{
//		if (isValide() == false)
//		{
//			return;
//		}

//		m_pVideoFrameWidget->setZoom(value);
//		return;
//	}

//	QScriptValue VideoFrameWidgetAgent::currentVideoFrame() const
//	{
//		if (isValide() == false)
//		{
//			return QScriptValue();
//		}

//		auto videoFrame = m_pVideoFrameWidget->videoFrame();
//		VideoFrameAgent* pVideoFrameAgent = new VideoFrameAgent(nullptr, videoFrame);
		
//		return engine()->newQObject(pVideoFrameAgent,  QScriptEngine::ScriptOwnership);
//	}

//	void VideoFrameWidgetAgent::setCurrentVideoFrame(const QScriptValue& videoFrame)
//	{
//		if (isValide() == false || videoFrame.isError() == true)
//		{
//			return;
//		}

//		if (videoFrame.isString() == true)
//		{
//			m_pVideoFrameWidget->setCurrentVideoFrame(videoFrame.toString(), true);
//		}
		
//		return;
//	}
}
