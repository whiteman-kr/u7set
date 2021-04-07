#pragma once

#include "FontParam.h"
#include "Session.h"
#include "../lib/ClientBehavior.h"

class QPainter;
class QPaintDevice;
class QPixmap;

namespace VFrame30
{
	class Schema;
	class SchemaView;
	class ClientSchemaView;
	class AppSignalController;
	class TuningController;

	class CDrawParam
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

		const ClientSchemaView* clientSchemaView() const;	// Can be used only in Client mode (Monitor/Tuning/...)
		ClientSchemaView* clientSchemaView();

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

		int dpiX() const noexcept;
		int dpiY() const noexcept;

		double gridToDpiX(double pos) const noexcept;
		double gridToDpiY(double pos) const noexcept;
		QPointF gridToDpi(double x, double y) const noexcept;
		QPointF gridToDpi(const QPointF& pos) const noexcept;
		QRectF gridToDpi(const QRectF& rect) const noexcept;

		bool isEditMode() const noexcept;
		void setEditMode(bool value);

		bool isMonitorMode() const noexcept;
		void setMonitorMode(bool value);

		bool infoMode() const noexcept;
		void setInfoMode(bool value);

		bool blinkPhase() const noexcept;
		void setBlinkPhase(bool value);

		bool drawNotesLayer() const noexcept;
		void setDrawNotesLayer(bool value);

		AppSignalController* appSignalController() noexcept;
		void setAppSignalController(AppSignalController* value);

		TuningController* tuningController() noexcept;
		void setTuningController(TuningController* value);

		const Session& session() const noexcept;
		Session& session();

		const MonitorBehavior& monitorBehavor() const noexcept;
		const TuningClientBehavior& tuningClientBehavior() const;

		const QStringList& hightlightIds() const;
		void setHightlightIds(const QStringList& value);

	private:
		QPainter* m_painter = nullptr;
		Schema* m_schema = nullptr;
		const SchemaView* m_schemaView = nullptr;

		AppSignalController* m_appSignalController = nullptr;
		TuningController* m_tuningController = nullptr;

		Session m_session;

		QStringList m_highlightIds;

		double m_controlBarSize = 0.0;
		double m_gridSize = 0.0;
		int m_pinGridStep = 0;
		bool m_isEditMode = true;
		bool m_infoMode = false;
		bool m_blinkPhase = false;
		bool m_drawNotesLayer = true;

		mutable int m_dpiX = -1;
		mutable int m_dpiY = -1;

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

		// Draw using already stelectd font
		//
		static void drawText(QPainter* p,
							 SchemaUnit unit,
							 const QString& str,
							 const QRectF& rect,
							 int flags,
							 QRectF* boundingRect = nullptr);
	};
}

