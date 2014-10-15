#pragma once

#include "Scheme.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT LogicScheme : public Scheme
	{
		Q_OBJECT

	public:
		LogicScheme(void);
		virtual ~LogicScheme(void);

		// Methods
		//
	public:
		virtual void Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const override;
	};

}


