#pragma once

#ifdef Q_OS_WIN
#ifndef NOMINMAX		// To resolve min/max conflic "windows.h macros vs std::min/max"
	#define NOMINMAX
#endif
	#include <windows.h>
#endif

#include <memory>

#include "FontParam.h"
#include "../include/TypesAndEnums.h"

class QPainter;
class QPaintDevice;
class QPixmap;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CDrawParam
	{
	private:
		CDrawParam(void);
	public:
		explicit CDrawParam(QPainter* painter);
		virtual ~CDrawParam(void);

	public:
		QPainter* painter();
		QPaintDevice* device();

		void Save() const;

		// Params for drawing
		//
	public:
		double GetMinFblGridSize() const;
		void SetMinFblGridSize(double val);

		double controlBarSize() const;
		void setControlBarSize(double value);

	private:
		QPainter* m_painter;
		double m_controlBarSize;
	};

	class VFRAME30LIBSHARED_EXPORT DrawHelper
	{
	public:
		static void DrawText(QPainter* p, const FontParam& font, SchemeUnit unit, const QString& str, const QRectF& rect);
	};
}

