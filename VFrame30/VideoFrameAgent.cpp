#include "Stable.h"
#include "VideoFrameAgent.h"
#include "VideoFrame.h"

namespace VFrame30
{
	VideoFrameAgent::VideoFrameAgent(QObject* pParent, const std::shared_ptr<const CVideoFrame>& videoFrame)
		: QObject(pParent), 
		m_videoFrame(videoFrame)
	{
		assert(videoFrame.get() != nullptr);
	}

	VideoFrameAgent::~VideoFrameAgent()
	{
	}

	QString VideoFrameAgent::strID()
	{
		return m_videoFrame->strID();
	}
}
