#include "Stable.h"
#include "VideoFrameManager.h"
#include "VideoFrame.h"

namespace VFrame30
{

	VideoFrameManager::VideoFrameManager()
	{
	}

	VideoFrameManager::~VideoFrameManager()
	{
		mutex.lock();
		videoFrames.clear();
		mutex.unlock();
	}

	bool VideoFrameManager::load(const wchar_t* fileName)
	{
		if (fileName == nullptr)
		{
			assert(fileName);
			return false;
		}

		CVideoFrame* pVideoFrame = VFrame30::CVideoFrame::Create(fileName);
		if (pVideoFrame == nullptr)
		{
			return false;
		}

		QMutexLocker ml(&mutex);

		videoFrames[pVideoFrame->strID()] = std::shared_ptr<CVideoFrame>(pVideoFrame);
		return true;
	}

	bool VideoFrameManager::getVideoFrame(const QString& strID, std::shared_ptr<const CVideoFrame>* videoFrame) const
	{
		if (videoFrame == nullptr)
		{
			assert(videoFrame != nullptr);
			return false;
		}

		QMutexLocker ml(&mutex);

		auto it = videoFrames.find(strID);

		if (it == videoFrames.end())
		{
			// VideoFrame not found
			//
			qDebug() << "VideoFrameManager::getVideoFrame: Videoframe is not found. stdID = " << strID;
			return false;
		}

		*videoFrame = it->second;
		return true;
	}

}
