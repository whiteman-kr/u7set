#pragma once

#include "VideoFrame.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT CVideoFrameWiring : public CVideoFrame
	{
		Q_OBJECT

	public:
		CVideoFrameWiring(void);
		virtual ~CVideoFrameWiring(void);
	};
}


