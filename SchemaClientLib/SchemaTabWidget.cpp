#include "SchemaTabWidget.h"


// #include "MonitorSchemaManager.h"
// #include "MonitorAppSettings.h"
// #include "MonitorSchemaView.h"
// #include "../VFrame30/MonitorSchema.h"


namespace SchemaClientLib
{
	//
	// SchemaTabWidgetSignalSlot
	//
	SchemaTabWidgetSignalSlot::SchemaTabWidgetSignalSlot(QWidget* parent) :
		TabWidgetEx{parent}
	{
		return;
	}

	void SchemaTabWidgetSignalSlot::slot_newSchemaTab(QString schemaId)
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_newTab()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_closeCurrentTab()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_zoomIn()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_zoomOut()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_zoom100()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_zoomToFit()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_historyBack()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_historyForward()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_selectSchemaForCurrentTab(QString schemaId)
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_signalContextMenu(const QStringList signalList,
														   const QList<QMenu*>& customMenu)
	{
		Q_ASSERT(false);
	}
	void SchemaTabWidgetSignalSlot::slot_signalInfo(QString signalId)
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_export()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_tabCloseRequested(int index)
	{
		Q_ASSERT(false);
	}
	void SchemaTabWidgetSignalSlot::slot_resetSchema()
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_newSameTab(VFrame30::ClientSchemaWidget* tabWidget)
	{
		Q_ASSERT(false);
	}
	void SchemaTabWidgetSignalSlot::slot_closeTab(QWidget* tabWidget)
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget,
													   VFrame30::Schema* schema)
	{
		Q_ASSERT(false);
	}

	void SchemaTabWidgetSignalSlot::slot_tabPageChanged(int index)
	{
		Q_ASSERT(false);
	}

} // namespace SchemaClientLib
