#pragma once

namespace VFrame30
{
	class Schema;
	class SchemaViewWidget;

	class BaseSchemaWidget : public QScrollArea
	{
		Q_OBJECT

	public:
		BaseSchemaWidget() = delete;
		BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaViewWidget* schemaView, QWidget* parent);
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
		[[nodiscard]] QPointF widgetPointToDocument(const QPoint& widgetPoint) const;

		// Slots
		//
	public slots:
		virtual void zoomIn();
		virtual void zoomOut();
		virtual void zoom100();
		virtual void zoomToFit();

	protected slots:
		void screenChanged(QScreen* screen);

		// Properties
		//
	public:
		VFrame30::Schema* schema();
		const VFrame30::Schema* schema() const;

		std::shared_ptr<VFrame30::Schema> schemaSharedPtr();
		std::shared_ptr<VFrame30::Schema> schemaSharedPtr() const;

		virtual void setSchema(std::shared_ptr<VFrame30::Schema> schema, bool repaint);

		SchemaViewWidget* schemaView();
		const SchemaViewWidget* schemaView() const;

		double zoom() const;
		virtual void setZoom(double zoom, bool repaint, int horzScrollValue = -1, int vertScrollValue = -1);

		// Data
		//
	protected:

	private:
		SchemaViewWidget* m_schemaView = nullptr;

		// Interface data
		//
		QPoint m_mousePos;					// Keeps mouse pos during different actions like scrolling etc
		int m_horzScrollBarValue = 0;		// Horizontal scroll bar value in mousePressEvent -- midButton
		int m_vertScrollBarValue = 0;		// Vertical scroll bar value in mousePressEvent -- midButton
	};

}

