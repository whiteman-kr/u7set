#pragma once

#include "ITimeStats.h"
#include "FontParam.h"
#include "Session.h"
#include "VFrame30Types.h"
#include "VFrameTools.h"



#define VFRAME30_CACHE_DRAW_TEXT	// Use cached draw text, DrawHelper::drawTextCahed(...)


#define CONTROL_BAR_PX		10
#define CONTROL_BAR_MM		(VFrame30::mm2in(2.4))
#define CONTROL_BAR(_unit, _devicePixelRatio, _zoom)	((_unit == SchemaUnit::Display) ? CONTROL_BAR_PX * _devicePixelRatio * (100.0 / _zoom) : CONTROL_BAR_MM * (100.0 / _zoom))

class QPainter;
class QPaintDevice;
class QPixmap;

namespace Behavior
{
	class MonitorBehavior;
	class TuningClientBehavior;
} 

namespace VFrame30
{
	class Schema;
	class SchemaView;
	class ClientSchemaView;
	class TuningController;

	class CDrawParam final
	{
	public:
		CDrawParam(void) = delete;
		CDrawParam(QPainter* painter, const SchemaView* view, double gridSize, int pinGridStep, SchemaUnit schemaUnit);

	public:
		QPainter* painter();
		QPaintDevice* device();
		const QPaintDevice* device() const;

		const SchemaView* schemaView() const;
		SchemaView* schemaView();

		const ClientSchemaView* clientSchemaView() const;	// Can be used only in Client mode (Monitor/Tuning/...)
		ClientSchemaView* clientSchemaView();

		VFrame30::ITimeStats* timeStats();
		VFrame30::ITimeStats* timeStats() const;

		// Params for drawing
		//
	public:
		double GetMinFblGridSize() const;
		void SetMinFblGridSize(double val);

		SchemaUnit schemaUnit() const;

		double controlBarSize() const;
		void setControlBarSize(double value);

		double gridSize() const;
		void setGridSize(double value);

		int pinGridStep() const;
		void setPinGridStep(int value);

		double cosmeticPenWidth() const;

		double realDpiX() const noexcept;			// physicalDpiX * devicePixelRatio
		double realDpiY() const noexcept;			// physicalDpiY * devicePixelRatio
		double devicePixelRatio() const noexcept;

		static double realDpiX(QPainter* painter) noexcept;
		static double realDpiY(QPainter* painter) noexcept;

		double gridToDpiX(double pos) const noexcept;
		double gridToDpiY(double pos) const noexcept;
		QPointF gridToDpi(double x, double y) const noexcept;
		QPointF gridToDpi(const QPointF& pos) const noexcept;
		QRectF gridToDpi(const QRectF& rect) const noexcept;

		DrawMode drawMode() const noexcept;

		bool infoMode() const noexcept;
		void setInfoMode(bool value);

		bool pdfMode() const noexcept;
		void setPdfMode(bool value);

		bool blinkPhase() const noexcept;
		void setBlinkPhase(bool value);

		bool drawNotesLayer() const noexcept;
		void setDrawNotesLayer(bool value);

		const Session& session() const noexcept;
		Session& session();

		const Behavior::MonitorBehavior& monitorBehavior() const noexcept;
		const Behavior::TuningClientBehavior& tuningClientBehavior() const;

		const QStringList& highlightIds() const;
		void setHighlightIds(const QStringList& value);

	private:
		QPainter* m_painter = nullptr;
		const SchemaView* m_schemaView = nullptr;

		Session m_session;

		QStringList m_highlightIds;

		SchemaUnit m_schemaUnit = SchemaUnit::Inch;
		double m_controlBarSize = 0.0;
		double m_gridSize = 0.0;
		int m_pinGridStep = 0;
		bool m_infoMode = false;
		bool m_pdfMode = false;
		bool m_blinkPhase = false;
		bool m_drawNotesLayer = true;

		double m_cosmeticPenWidth = 0.0;
	};


	class DrawHelper
	{
	public:
		static void drawText(QPainter* p,
							 const FontParam& font,
							 SchemaUnit unit,
							 const QString& str,
							 const QRectF& rect,
							 int flags,
							 QRectF* boundingRect = nullptr);

		static void drawTextCahed(QPainter* p,
								  const FontParam& font,
								  SchemaUnit unit,
								  const QString& str,
								  const QRectF& rect,
								  int flags,
								  double zoom);

		// Draw using already stelectd font
		//
		static void drawText(QPainter* p,
							 SchemaUnit unit,
							 const QString& str,
							 const QRectF& rect,
							 int flags,
							 QRectF* boundingRect = nullptr);

		// Svg
		//
		static bool drawSvgCached(QPainter& painter, SchemaUnit unit, const QRectF& rect, const QString& svg, double zoom);
	};
}

