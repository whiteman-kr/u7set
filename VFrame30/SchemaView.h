#pragma once

#include "Schema.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaView : public QWidget
	{
		Q_OBJECT

	public:
		explicit SchemaView(QWidget *parent = 0);
		explicit SchemaView(std::shared_ptr<VFrame30::Schema>& schema, QWidget *parent = 0);

	protected:
		void init();
		
		// Painting
		//
	protected:
		virtual void paintEvent(QPaintEvent*) override;

		void Ajust(QPainter* painter, double startX, double startY, double zoom) const;

	public:
		void exportToPdf(QString fileName) const;

		// Methods
	public:
		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

		std::shared_ptr<Schema>& schema();
		std::shared_ptr<Schema> schema() const;

		void setSchema(std::shared_ptr<Schema>& schema, bool repaint);
		
		// Events
		//
	protected:
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Properties
		//
	public:
		double zoom() const;
		void setZoom(double value, bool repaint = true, int dpiX = 0, int dpiY = 0);

		// Data
		//
	private:
		std::shared_ptr<VFrame30::Schema> m_schema;
		double m_zoom = 100.0;
	};
}


