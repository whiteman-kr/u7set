#pragma once

namespace VFrame30
{
	class SchemaView;
	class Schema;

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
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		// Methods
		//
	public:
		QPointF widgetPointToDocument(const QPoint& widgetPoint) const;
		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* destDocPos, int dpiX = 0, int dpiY = 0);

		// Slots
		//
	protected slots:
		void zoomIn();
		void zoomOut();
		void zoom100();

		// Properties
		//
	public:
		std::shared_ptr<VFrame30::Schema> schema();
		const std::shared_ptr<VFrame30::Schema> schema() const;
		void setSchema(std::shared_ptr<VFrame30::Schema> schema);

		SchemaView* schemaView();
		const SchemaView* schemaView() const;

		double zoom() const;
		void setZoom(double zoom, int horzScrollValue = -1, int vertScrollValue = -1);

		// Data
		//
	protected:
		QAction* m_zoomInAction = nullptr;
		QAction* m_zoomOutAction = nullptr;
		QAction* m_zoom100Action = nullptr;

	private:
		SchemaView* m_schemaView = nullptr;

		// Interface data
		//
		QPoint m_mousePos;				// Keeps mouse pos during different actions like scrolling etc
		int m_horzScrollBarValue = 0;		// Horizintal scroll bar value in mousePressEvent -- midButton
		int m_vertScrollBarValue = 0;		// Vertical scroll bar value in mousePressEvent -- midButton
	};

}

