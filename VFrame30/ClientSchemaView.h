#ifndef CLIENTSCHEMAVIEW_H
#define CLIENTSCHEMAVIEW_H

#include <memory>
#include <SchemaView.h>
#include <SchemaManager.h>
#include "SchemaItem.h"

class QPaintEvent;
class QTimerEvent;
class QMouseEvent;

namespace VFrame30
{

	class ClientSchemaView : public VFrame30::SchemaView
	{
		Q_OBJECT

	public:
		explicit ClientSchemaView(VFrame30::SchemaManager* schemaManager, QWidget* parent = nullptr);
		virtual ~ClientSchemaView();

	protected:
		virtual void paintEvent(QPaintEvent* event) override;
		virtual void timerEvent(QTimerEvent* event) override;

		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseReleaseEvent(QMouseEvent* event) override;

	protected slots:
		void startRepaintTimer();

	signals:
		void signal_setSchema(QString schemaId);

		// Public slots which are part of Script API
		//
	public slots:
		virtual void setSchema(QString schemaId) override;

		// Properties
		//
	public:
		virtual QString globalScript() const override;
		virtual QJSEngine* jsEngine() override;

	private:
		VFrame30::SchemaManager* m_schemaManager = nullptr;

		std::shared_ptr<SchemaItem> m_leftClickOverItem;
		QDateTime m_lastRepaintEventFired = QDateTime::currentDateTime();
	};


}

#endif // CLIENTSCHEMAVIEW_H
