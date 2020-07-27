#ifndef MONITORCENTRALWIDGET_H
#define MONITORCENTRALWIDGET_H

#include "MonitorSchemaWidget.h"
#include "MonitorSchemaManager.h"
#include "../lib/Ui/TabWidgetEx.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"


class MonitorCentralWidget : public TabWidgetEx
{
	Q_OBJECT

public:
	MonitorCentralWidget(MonitorSchemaManager* schemaManager,
						 VFrame30::AppSignalController* appSignalController,
						 VFrame30::TuningController* tuningController,
						 QWidget* parent);
	~MonitorCentralWidget();

public:
	MonitorSchemaWidget* currentTab();
	VFrame30::TuningController* tuningController();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	int addSchemaTabPage(QString schemaId, const QVariantHash& variables);

	// Signals
signals:
	void signal_tabPageChanged(bool schemaWidgetSelected);		// Emmited to enable/disable QActions dependo on current tab (schema/schemalist)
	void signal_actionCloseTabUpdated(bool allowed);
	void signal_schemaChanged(QString strId);
	void signal_historyChanged(bool enableBack, bool enableForward);

	// Slots
	//
public slots:
	void slot_schemaList();
	void slot_newSchemaTab(QString schemaId);
	void slot_newTab();
	void slot_closeCurrentTab();

	void slot_zoomIn();
	void slot_zoomOut();
	void slot_zoom100();
	void slot_zoomToFit();

	void slot_historyBack();
	void slot_historyForward();

	void slot_selectSchemaForCurrentTab(QString schemaId);

	void slot_signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void slot_signalInfo(QString signalId);

protected slots:
	void slot_tabCloseRequested(int index);
	void slot_resetSchema();

	void slot_newSameTab(MonitorSchemaWidget* tabWidget);
	void slot_closeTab(QWidget* tabWidget);

	void slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema);

	void slot_tabPageChanged(int index);

	// Data
	//
private:
	MonitorSchemaManager* m_schemaManager = nullptr;

	VFrame30::AppSignalController* m_appSignalController = nullptr;
	VFrame30::TuningController* m_tuningController = nullptr;

	int m_eventLoopTimerId = 0;				// We need to cathc event loop. Start timer, as we enter event loop timerEvent comes
	int m_eventLoopTimerCounter = 0;		// We need to cathc event loop. Start timer, as we enter event loop timerEvent comes
};

#endif // MONITORCENTRALWIDGET_H
