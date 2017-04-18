#pragma once

//#ifdef Q_OS_WIN
//#ifndef NOMINMAX		// To resolve min/max conflic "windows.h macros vs std::min/max"
//	#define NOMINMAX
//#endif
//	#include <windows.h>
//#endif

#include "FontParam.h"
#include "Session.h"

class QPainter;
class QPaintDevice;
class QPixmap;
class AppSignalManager;

namespace VFrame30
{
	class Schema;
	class SchemaView;

	class VFRAME30LIBSHARED_EXPORT CDrawParam
	{
	public:
		CDrawParam(void) = delete;
		CDrawParam(QPainter* painter, Schema* schema, const SchemaView* view, double gridSize, int pinGridStep);
		virtual ~CDrawParam(void);

	public:
		QPainter* painter();
		QPaintDevice* device();

		const Schema* schema() const;

		const SchemaView* schemaView() const;
		SchemaView* schemaView();

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

		double cosmeticPenWidth() const;

		int dpiX();
		int dpiY();

		bool isEditMode() const;
		void setEditMode(bool value);

		bool isMonitorMode() const;
		void setMonitorMode(bool value);

		bool infoMode() const;
		void setInfoMode(bool value);

		AppSignalManager* appSignalManager();
		void setAppSignalManager(AppSignalManager* value);

		const Session& session() const;
		Session& session();

	private:
		QPainter* m_painter = nullptr;
		Schema* m_schema = nullptr;
		const SchemaView* m_schemaView = nullptr;
		AppSignalManager* m_appSignalmanager = nullptr;

		Session m_session;

		double m_controlBarSize = 0.0;
		double m_gridSize = 0.0;
		int m_pinGridStep = 0;
		bool m_isEditMode = true;
		bool m_infoMode = false;

		int m_dpiX = -1;
		int m_dpiY = -1;

		double m_cosmeticPenWidth = 0.0;
	};

	class VFRAME30LIBSHARED_EXPORT DrawHelper
	{
	public:
		static void drawText(QPainter* p,
							 const FontParam& font,
							 SchemaUnit unit,
							 const QString& str,
							 const QRectF& rect,
							 int flags,
							 QRectF* boundingRect = nullptr);
	};
}

