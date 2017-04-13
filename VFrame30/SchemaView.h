#pragma once

#include <QJSEngine>
#include "Session.h"
#include "../lib/Tuning/TuningController.h"


namespace VFrame30
{
	class Schema;
	class SchemaItem;
	class CDrawParam;

	class VFRAME30LIBSHARED_EXPORT SchemaView : public QWidget
	{
		Q_OBJECT

	public:
		explicit SchemaView(QWidget *parent = 0);
		explicit SchemaView(std::shared_ptr<Schema> schema, QWidget *parent = 0);

	protected:
		void updateControlWidgets(bool editMode);
		
		// Painting
		//
	protected:
		virtual void paintEvent(QPaintEvent*) override;
		void draw(CDrawParam& drawParam);

		void Ajust(QPainter* painter, double startX, double startY, double zoom) const;

	public:
		void exportToPdf(QString fileName) const;

		// Methods
	public:
		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

		std::shared_ptr<Schema>& schema();
		std::shared_ptr<Schema> schema() const;

		void setSchema(std::shared_ptr<Schema>& schema, bool repaint);

		Q_INVOKABLE void jsDebugOutput(QString str);
		
		// Events
		//
	protected:
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Signals
		//
	signals:
		void signal_schemaChanged(Schema* schema);

		// Properties
		//
	public:
		double zoom() const;
		void setZoom(double value, bool repaint = true, int dpiX = 0, int dpiY = 0);

		const Session& session() const;
		Session& session();

		const TuningController& tuningController() const;
		TuningController& tuningController();

		QJSEngine* jsEngine();

		// Data
		//
	private:
		std::shared_ptr<VFrame30::Schema> m_schema;
		double m_zoom = 100.0;

		Session m_session;
		TuningController m_tuningController;

		QJSEngine m_jsEngine;
	};
}


