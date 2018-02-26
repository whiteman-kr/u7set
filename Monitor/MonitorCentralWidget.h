#ifndef MONITORCENTRALWIDGET_H
#define MONITORCENTRALWIDGET_H

#include "QTabWidget"
#include "MonitorSchemaWidget.h"
#include "MonitorSchemaManager.h"


class MonitorCentralWidget : public QTabWidget
{
	Q_OBJECT

public:
	MonitorCentralWidget(MonitorSchemaManager* schemaManager);
	~MonitorCentralWidget();

public:
	MonitorSchemaWidget* currentTab();

protected:
	int addSchemaTabPage(QString schemaId);

	// Signals
signals:
	void signal_actionCloseTabUpdated(bool allowed);
	void signal_schemaChanged(QString strId);
	void signal_historyChanged(bool enableBack, bool enableForward);

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

	void slot_selectSchemaForCurrentTab(QString schemaId);

protected slots:
	void slot_tabCloseRequested(int index);
	void slot_resetSchema();

	void slot_newSameTab(MonitorSchemaWidget* tabWidget);
	void slot_closeTab(MonitorSchemaWidget* tabWidget);

	void slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema);

	void slot_tabPageChanged(int index);

	// Data
	//
private:
	MonitorSchemaManager* m_schemaManager = nullptr;
};

#endif // MONITORCENTRALWIDGET_H
