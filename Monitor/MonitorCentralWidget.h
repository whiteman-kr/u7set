#ifndef MONITORCENTRALWIDGET_H
#define MONITORCENTRALWIDGET_H

#include "QTabWidget"
#include "MonitorSchemaWidget.h"

class SchemaManager;

class MonitorCentralWidget : public QTabWidget
{
public:
	MonitorCentralWidget(SchemaManager* schemaManager);
	~MonitorCentralWidget();

public:

protected:
	int addSchemaTabPage(QString schemaId);

	// Signals
signals:

	// Slots
	//
protected slots:
	void slot_tabCloseRequested(int index);
	void slot_resetSchema(QString startSchemaId);

	void slot_newSameTab(MonitorSchemaWidget* tabWidget);
	void slot_closeTab(MonitorSchemaWidget* tabWidget);

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;
};

#endif // MONITORCENTRALWIDGET_H
