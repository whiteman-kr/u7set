#ifndef BASESCHEMEWIDGET_H
#define BASESCHEMEWIDGET_H

namespace VFrame30
{
	class SchemaView;
	class Schema;

	class VFRAME30LIBSHARED_EXPORT BaseSchemeWidget : public QScrollArea
	{
		Q_OBJECT

	public:
		BaseSchemeWidget() = delete;
		BaseSchemeWidget(std::shared_ptr<VFrame30::Schema> scheme, SchemaView* schemeView);
		virtual ~BaseSchemeWidget();

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
		std::shared_ptr<VFrame30::Schema> scheme();
		const std::shared_ptr<VFrame30::Schema> scheme() const;
		void setScheme(std::shared_ptr<VFrame30::Schema> scheme);

		SchemaView* schemeView();
		const SchemaView* schemeView() const;

		double zoom() const;
		void setZoom(double zoom, int horzScrollValue = -1, int vertScrollValue = -1);

		// Data
		//
	protected:
		QAction* m_zoomInAction = nullptr;
		QAction* m_zoomOutAction = nullptr;
		QAction* m_zoom100Action = nullptr;

	private:
		SchemaView* m_schemeView = nullptr;

		// Interface data
		//
		QPoint m_mousePos;				// Keeps mouse pos during different actions like scrolling etc
		int m_horzScrollBarValue = 0;		// Horizintal scroll bar value in mousePressEvent -- midButton
		int m_vertScrollBarValue = 0;		// Vertical scroll bar value in mousePressEvent -- midButton
	};

}
#endif // BASESCHEMEWIDGET_H
