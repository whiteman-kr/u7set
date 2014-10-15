#pragma once

namespace VFrame30
{
	class Scheme;

	class VFRAME30LIBSHARED_EXPORT VideoFrameManager
	{
	public:
		VideoFrameManager();
		virtual ~VideoFrameManager();

		// Methods
		//
	public:
		// Load VideoFrame and add it to videoFrames container, if such frame already exist in the container,
		// then reload it.
		// fileName: The VideoFrame file name.
		// Return: true if file loaded successfully
		//
		bool load(const wchar_t* fileName);

		// Get VideoFrame from container, videoframe must be loaded before.
		// VideoFrame may be just read, don't do any modification, becaouse it's NOT thread safe.
		//
		bool getVideoFrame(const QString& strID, std::shared_ptr<const Scheme>* videoFrame) const;

		// Data
		//
	private:
		mutable QMutex mutex;			// mutex used just for videoFrames map, any videoframe after getting
		// canniot be modified.

		std::map<QString, std::shared_ptr<Scheme>> videoFrames;
	};

}


