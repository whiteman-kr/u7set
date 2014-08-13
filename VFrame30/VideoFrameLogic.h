#ifndef VIDEOFRAMELOGIC_H
#define VIDEOFRAMELOGIC_H

#include "VideoFrame.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT CVideoFrameLogic : public CVideoFrame
	{
		Q_OBJECT

	public:
		CVideoFrameLogic(void);
		virtual ~CVideoFrameLogic(void);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const override;
	};

}

#endif // VIDEOFRAMELOGIC_H
