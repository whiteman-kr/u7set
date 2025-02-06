#pragma once
#include <VFrame30/BaseSchemaWidget.h>
#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/ISchemaViewHistory.h>
#include <VFrame30/Schema.h>
#include <VFrame30/SchemaManager.h>
#include <VFrame30/VFrame30Types.h>

namespace VFrame30
{
	struct SchemaHistoryItem
	{
		SchemaHistoryItem() = default;
		SchemaHistoryItem(const SchemaHistoryItem& src) = default;
		SchemaHistoryItem& operator= (const SchemaHistoryItem& src) = default;

		SchemaHistoryItem(std::shared_ptr<VFrame30::Schema> schema,
						  const QVariantHash& variables,
						  double zoom,
						  int horzScrollValue,
						  int vertScrollValue);

		std::shared_ptr<VFrame30::Schema> m_schema;
		QVariantHash m_variables;

		double m_zoom = 100.0;
		int m_horzScrollValue = 0;
		int m_vertScrollValue = 0;
	};


	class ClientSchemaWidget : public BaseSchemaWidget, public ISchemaViewHistory
	{
		Q_OBJECT

	public:
		ClientSchemaWidget() = delete;
		ClientSchemaWidget(ClientSchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema, VFrame30::SchemaManager* schemaManager, QWidget* parent);

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		std::vector<SchemaItemPtr> itemsUnderCursor(const QPoint& pos);

		// History functions, ISchemaViewHistory
		//
	public:
		[[nodiscard]] virtual bool canBackHistory() const override;
		[[nodiscard]] virtual bool canForwardHistory() const override;

		virtual void historyBack() override;
		virtual void historyForward() override;

		// End of ISchemaViewHistory
		//

		void resetHistory();
		void resetForwardHistory();

		void restoreState(const SchemaHistoryItem& historyState);
		[[nodiscard]] SchemaHistoryItem currentHistoryState() const;

		void emitHistoryChanged();

	public slots:
		// using BaseSchemaWidget::setSchema -- use ClientSchemaWidget::setSchema and it's overloads.
		//
		virtual void setSchema(QString schemaId, QStringList highlightIds, bool forceSchemaUpdate);

		virtual void setZoom(double zoom, bool repaint, int horzScrollValue = -1, int vertScrollValue = -1) override;

		// Signals
		//
	signals:
		void signal_schemaChanged(VFrame30::ClientSchemaWidget* widget, VFrame30::Schema* schema);
		void signal_historyChanged(bool enableBack, bool enableForward);

		// Properties
		//
	public:
		[[nodiscard]] QString schemaId() const;
		[[nodiscard]] QString caption() const;

		[[nodiscard]] VFrame30::SchemaManager* schemaManager();

		[[nodiscard]] VFrame30::ZoomMode zoomMode() const;
		void setZoomMode(VFrame30::ZoomMode zoomMode, bool repaint);

		[[nodiscard]] ClientSchemaView* clientSchemaView();
		[[nodiscard]] const ClientSchemaView* clientSchemaView() const;

		// Data
		//
	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;

		VFrame30::ZoomMode m_zoomMode = ZoomMode::Manual;

		QPoint m_dragStartPosition;							// For drag and drop

	protected:
		static constexpr size_t HistorySize = 15;
		std::list<SchemaHistoryItem> m_backHistory;
		std::list<SchemaHistoryItem> m_forwardHistory;
	};

}

