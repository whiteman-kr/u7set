#pragma once
#include <memory>
#include <QScrollArea>
#include "VFrame30Lib_global.h"

namespace VFrame30
{
	class Schema;
	class SchemaView;

	class VFRAME30LIBSHARED_EXPORT BaseSchemaWidget : public QScrollArea
	{
		Q_OBJECT

	public:
		BaseSchemaWidget() = delete;
		BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaView* schemaView);
		virtual ~BaseSchemaWidget();

	protected:
		void createActions();

		virtual void wheelEvent(QWheelEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Methods
		//
	public:
		QPointF widgetPointToDocument(const QPoint& widgetPoint) const;
		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* destDocPos, int dpiX = 0, int dpiY = 0);

		// Slots
		//
	public slots:
		void zoomIn();
		void zoomOut();
		void zoom100();

		// Properties
		//
	public:
		std::shared_ptr<VFrame30::Schema> schema();
		const std::shared_ptr<VFrame30::Schema> schema() const;

		virtual void setSchema(std::shared_ptr<VFrame30::Schema> schema, bool repaint);

		SchemaView* schemaView();
		const SchemaView* schemaView() const;

		double zoom() const;
		void setZoom(double zoom, bool repaint, int horzScrollValue = -1, int vertScrollValue = -1);

		// Data
		//
	protected:

	private:
		SchemaView* m_schemaView = nullptr;

		// Interface data
		//
		QPoint m_mousePos;					// Keeps mouse pos during different actions like scrolling etc
		int m_horzScrollBarValue = 0;		// Horizintal scroll bar value in mousePressEvent -- midButton
		int m_vertScrollBarValue = 0;		// Vertical scroll bar value in mousePressEvent -- midButton
	};

}

