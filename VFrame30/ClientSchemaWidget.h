#ifndef CLIENTSCHEMAWIDGET_H
#define CLIENTSCHEMAWIDGET_H

#include "VFrame30Lib_global.h"
#include "BaseSchemaWidget.h"
#include "ClientSchemaView.h"
#include "Schema.h"
#include "SchemaManager.h"

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


	class VFRAME30LIBSHARED_EXPORT ClientSchemaWidget : public BaseSchemaWidget
	{
		Q_OBJECT

	private:
		ClientSchemaWidget() = delete;

	public:
		ClientSchemaWidget(SchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema, VFrame30::SchemaManager* schemaManager);
		virtual ~ClientSchemaWidget();

	protected:
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;

		std::vector<SchemaItemPtr> itemsUnderCursor(const QPoint& pos);

		// History functions
		//
	public:
		bool canBackHistory() const;
		bool canForwardHistory() const;

		void historyBack();
		void historyForward();

		void resetHistory();
		void resetForwardHistory();

		void restoreState(const SchemaHistoryItem& historyState);
		SchemaHistoryItem currentHistoryState() const;

		void emitHistoryChanged();

	public slots:
		virtual void setSchema(QString schemaId, QStringList highlightIds);

		// Signals
		//
	signals:
		void signal_schemaChanged(ClientSchemaWidget* widget, VFrame30::Schema* schema);
		void signal_historyChanged(bool enableBack, bool enableForward);

		// Properties
		//
	public:
		QString schemaId() const;
		QString caption() const;

		VFrame30::SchemaManager* schemaManager();

		ClientSchemaView* clientSchemaView();
		const ClientSchemaView* clientSchemaView() const;

		// Data
		//
	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;

		QPoint m_dragStartPosition;							// For drag and drop

	protected:
		std::list<SchemaHistoryItem> m_backHistory;
		std::list<SchemaHistoryItem> m_forwardHistory;
	};

}

#endif // CLIENTSCHEMAWIDGET_H
