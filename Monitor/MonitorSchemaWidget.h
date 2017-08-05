#pragma once

#include "../VFrame30/BaseSchemaWidget.h"

class MonitorView;
class SchemaManager;
struct SchemaHistoryItem;

namespace VFrame30
{
	class SchemaItem;
}

//
//
// MonitorSchemaWidget
//
//
class MonitorSchemaWidget : public VFrame30::BaseSchemaWidget
{
	Q_OBJECT

private:
	MonitorSchemaWidget() = delete;

public:
	MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaManager* schemaManager);
	virtual ~MonitorSchemaWidget();

protected:
	void createActions();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

	std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsUnderCursor(const QPoint& pos);

	// Methods
	//
public:

	// History functions
	//
public:
	bool canBackHistory() const;
	bool canForwardHistory() const;

	void historyBack();
	void historyForward();

	void resetHistory();

	void restoreState(const SchemaHistoryItem& historyState);
	SchemaHistoryItem currentHistoryState() const;

	void emitHistoryChanged();

	// --
	//
protected:

private:

	// Signals
	//
signals:
	//void signal_newTab(MonitorSchemaWidget* tabWidget);			// Command to the owner to duplicate current tab
	//void signal_closeTab(MonitorSchemaWidget* tabWidget);		// Command to the owner to Close current tab

	void signal_schemaChanged(MonitorSchemaWidget* tabWidget, VFrame30::Schema* schema);

	void signal_historyChanged(bool enableBack, bool enableForward);

	// Slots
	//
public slots:
	void contextMenuRequested(const QPoint &pos);

	void signalContextMenu(const QStringList signalList);

	void signalInfo(QString appSignalId);

	void slot_setSchema(QString schemaId);

	// Properties
	//
public:
	QString schemaId() const;
	QString caption() const;

	MonitorView* monitorSchemaView();
	const MonitorView* monitorSchemaView() const;

	// Data
	//
private:
	SchemaManager* m_schemaManager = nullptr;

	// Actions
	//
private:
	QAction* m_newTabAction = nullptr;
	QAction* m_closeTabAction = nullptr;

	std::list<SchemaHistoryItem> m_backHistory;
	std::list<SchemaHistoryItem> m_forwardHistory;

	QPoint m_dragStartPosition;							// For drag and drop
};


struct SchemaHistoryItem
{
	//SchemaHistoryItem(QString schemaId, double zoom, int horzScrollValue, int vertScrollValue);

	QString m_schemaId;
	double m_zoom = 100.0;
	int m_horzScrollValue = 0;
	int m_vertScrollValue = 0;
};

