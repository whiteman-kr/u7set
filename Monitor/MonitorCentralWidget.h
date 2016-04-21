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
	void addSchemaTabPage(QString schemaId);

	// Signals
signals:

	// Slots
	//
protected slots:
	void slot_tabCloseRequested(int index);

	void slot_resetSchema(QString startSchemaId);

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;
};

#endif // MONITORCENTRALWIDGET_H
