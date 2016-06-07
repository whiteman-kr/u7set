#pragma once

#ifdef Q_OS_WIN
#ifndef NOMINMAX		// To resolve min/max conflic "windows.h macros vs std::min/max"
	#define NOMINMAX
#endif
	#include <windows.h>
#endif

#include <memory>

#include "FontParam.h"
#include "../lib/TypesAndEnums.h"

class QPainter;
class QPaintDevice;
class QPixmap;
class AppSignalManager;

namespace VFrame30
{
	class Schema;
}


namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CDrawParam
	{
	public:
		CDrawParam(void) = delete;
		CDrawParam(QPainter* painter, Schema* schema, double gridSize, int pinGridStep);
		virtual ~CDrawParam(void);

	public:
		QPainter* painter();
		QPaintDevice* device();

		const Schema* schema() const;

		void Save() const;

		// Params for drawing
		//
	public:
		double GetMinFblGridSize() const;
		void SetMinFblGridSize(double val);

		double controlBarSize() const;
		void setControlBarSize(double value);

		double gridSize() const;
		void setGridSize(double value);

		int pinGridStep() const;
		void setPinGridStep(int value);

		bool isEditMode() const;
		void setEditMode(bool value);

		bool isMonitorMode() const;
		void setMonitorMode(bool value);

		AppSignalManager* appSignalManager();
		void setAppSignalManager(AppSignalManager* value);

	private:
		QPainter* m_painter = nullptr;
		Schema* m_schema = nullptr;
		AppSignalManager* m_appSignalmanager = nullptr;

		double m_controlBarSize = 0.0;
		double m_gridSize = 0.0;
		int m_pinGridStep = 0;
		bool m_isEditMode = true;
	};

	class VFRAME30LIBSHARED_EXPORT DrawHelper
	{
	public:
		static void DrawText(QPainter* p,
							 const FontParam& font,
							 SchemaUnit unit,
							 const QString& str,
							 const QRectF& rect,
							 int flags);
	};
}

