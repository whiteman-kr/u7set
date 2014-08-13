#ifndef VIDEOFRAMEDIAG_H
#define VIDEOFRAMEDIAG_H

#include "VideoFrame.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT CVideoFrameDiag : public CVideoFrame
	{
		Q_OBJECT

	public:
		CVideoFrameDiag(void);
		virtual ~CVideoFrameDiag(void);
	};

}

#endif // VIDEOFRAMEDIAG_H