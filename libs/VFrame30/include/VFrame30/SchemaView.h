#pragma once
#include <VFrame30/Session.h>
#include <VFrame30/VFrame30Types.h>

#include <QWidget>
class QPainter;
class QPaintDevice;

namespace VFrame30
{
	class Schema;
	class SchemaItem;
	class CDrawParam;

	static constexpr double ZoomStep = 10.0;


	class SchemaView : public std::enable_shared_from_this<SchemaView>
	{
	protected:
		SchemaView();
		explicit SchemaView(std::shared_ptr<Schema> schema);

	public:
		virtual ~SchemaView() = default;

		// Methods
		//
	public:
		static void Ajust(QPainter* painter, SchemaUnit units, double startX, double startY, double zoom);
		static void Ajust(QPainter* painter,
						  double dpiX,
						  double dpiY,
						  double devicePixelRatioF,
						  SchemaUnit units,
						  double startX,
						  double startY,
						  double zoom);

		virtual DrawMode drawMode() const = 0;

		[[nodiscard]] double realDpiX(const QPaintDevice* device) const;
		[[nodiscard]] double realDpiY(const QPaintDevice* device) const;

		virtual void setSchema(std::shared_ptr<Schema> schema, bool repaint);
		void setSchemaInternal(std::shared_ptr<Schema> schema); // Use this when yoo do not need to update zoom, sliders, etc

		VFrame30::Schema* schema();
		const VFrame30::Schema* schema() const;

		std::shared_ptr<VFrame30::Schema> schemaSharedPtr();
		std::shared_ptr<VFrame30::Schema> schemaSharedPtr() const;

		// Properties
		//
	public:
		double zoom() const;

		const Session& session() const;
		Session& session();

		// Data
		//
	protected:
		std::shared_ptr<VFrame30::Schema> m_schema;

		double m_zoom = 100.0;

		Session m_session;
	};


	class SchemaViewWidget : public QWidget,
							 public SchemaView
	{
		Q_OBJECT

	public:
		explicit SchemaViewWidget(QWidget* parent = nullptr);
		explicit SchemaViewWidget(std::shared_ptr<Schema> schema, QWidget* parent = nullptr);

	public:
		void updateControlWidgets(bool editMode);
		void deleteControlWidgets();

		bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, double dpiX = 0, double dpiY = 0);

		void setSchema(std::shared_ptr<Schema> schema, bool repaint) override;

		// Painting
		//
		void draw(CDrawParam& drawParam, const QRectF& clipRect);

	protected:
		virtual void paintEvent(QPaintEvent*) override;

		// Events
		//
	protected:
		virtual void mouseMoveEvent(QMouseEvent* event) override;

	public:
		// Properties
		//
		double setZoom(double value, bool repaint = true);

		// Signals
		//
	signals:
		void signal_schemaChanged(VFrame30::Schema* schema);
	};
} // namespace VFrame30
