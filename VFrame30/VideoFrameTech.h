#ifndef VIDEOFRAMETECH_H
#define VIDEOFRAMETECH_H

#include "VideoFrame.h"

namespace VFrame30
{
	// Технологический видеокадр
	//
	class VFRAME30LIBSHARED_EXPORT CVideoFrameTech : public CVideoFrame
	{
		Q_OBJECT

	public:
		CVideoFrameTech(void);
		virtual ~CVideoFrameTech(void);
	};
}

#endif // VIDEOFRAMETECH_H
