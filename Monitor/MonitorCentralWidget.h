#ifndef MONITORCENTRALWIDGET_H
#define MONITORCENTRALWIDGET_H

#include "QTabWidget"
#include "MonitorSchemaWidget.h"

class SchemaManager;

class MonitorCentralWidget : public QTabWidget
{
	Q_OBJECT

public:
	MonitorCentralWidget(SchemaManager* schemaManager);
	~MonitorCentralWidget();

public:

protected:
	int addSchemaTabPage(QString schemaId);

	// Signals
signals:
	void signal_actionCloseTabUpdated(bool allowed);

	// Slots
	//
public slots:
	void slot_newTab();
	void slot_closeCurrentTab();

	void slot_zoomIn();
	void slot_zoomOut();
	void slot_zoom100();

	void slot_historyBack();
	void slot_historyForward();


protected slots:
	void slot_tabCloseRequested(int index);
	void slot_resetSchema(QString startSchemaId);

	void slot_newSameTab(MonitorSchemaWidget* tabWidget);
	void slot_closeTab(MonitorSchemaWidget* tabWidget);

	void slot_schemaChanged(MonitorSchemaWidget* tabWidget, VFrame30::Schema* schema);

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;
};

#endif // MONITORCENTRALWIDGET_H
