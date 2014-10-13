#pragma once

#include "Scheme.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT CVideoFrameLogic : public Scheme
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


