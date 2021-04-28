#pragma once
#include "../../Builder/AppSignalSetProvider.h"
#include "../../VFrame30/Schema.h"
#include "../../VFrame30/SchemaView.h"
#include "../../VFrame30/PosRectImpl.h"
#include "../../VFrame30/AppSignalController.h"
#include "../../VFrame30/TuningController.h"
#include "EditSchemaTypes.h"
#include "EditConnectionLine.h"
#include "EditSchemaSignalProvider.h"

class EditSchemaWidget;

//
//
// EditSchemaView
//
//
class EditSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit EditSchemaView(AppSignalSetProvider* signalSetProvider, QWidget* parent = 0);
	explicit EditSchemaView(AppSignalSetProvider* signalSetProvider, std::shared_ptr<VFrame30::Schema> schema, QWidget* parent = nullptr);

	virtual ~EditSchemaView();

	// Painting
	//
protected:
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void paintEvent(QPaintEvent*) override;

	void drawBuildIssues(VFrame30::CDrawParam* drawParam, QRectF clipRect);
	void drawRunOrder(VFrame30::CDrawParam* drawParam, QRectF clipRect);
	void drawEditConnectionLineOutline(VFrame30::CDrawParam* drawParam);
	void drawNewItemOutline(QPainter* p, VFrame30::CDrawParam* drawParam);
	void drawSelectionArea(QPainter* p);
	void drawMovingItems(VFrame30::CDrawParam* drawParam);
	void drawRectSizing(VFrame30::CDrawParam* drawParam);
	void drawMovingLinePoint(VFrame30::CDrawParam* drawParam);
	void drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam);
	void drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect);

	void drawGrid(QPainter* p, const QRectF& clipRect);

	// Some determine functions
	//
protected:
	SchemaItemAction getPossibleAction(VFrame30::SchemaItem* schemaItem, QPointF point, int* outMovingEdgePointIndex);

	QRectF sizingRectItem(double xdif, double ydif, VFrame30::IPosRect* itemPos);

	// Signals
signals:
	void selectionChanged();

	// Properties
	//
public:
	// Layer props
	//
	QUuid activeLayerGuid() const;
	std::shared_ptr<VFrame30::SchemaLayer> activeLayer();
	void setActiveLayer(std::shared_ptr<VFrame30::SchemaLayer> layer);

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	// Selection
	//
	const std::vector<SchemaItemPtr>& selectedItems() const;
	std::vector<SchemaItemPtr> selectedNonLockedItems() const;

	void setSelectedItems(const std::vector<SchemaItemPtr>& items);
	void setSelectedItems(const std::list<SchemaItemPtr>& items);
	void setSelectedItem(const SchemaItemPtr& item);
	void addSelection(const SchemaItemPtr& item, bool emitSectionChanged = true);

	void clearSelection();
	bool removeFromSelection(const SchemaItemPtr& item, bool emitSectionChanged = true);
	bool isItemSelected(const SchemaItemPtr& item);

	// Data
	//
private:
	int m_activeLayer;
	MouseState m_mouseState;

	EditSchemaAppSignalProvider m_appSignalProvider;
	EditSchemaTuningSignalProvider m_tuningSignalProvider;

	VFrame30::AppSignalController m_appSignalController;
	VFrame30::TuningController m_tuningController;

	// Temporary data can be changed in EditSchemaWidget
	//
protected:
	SchemaItemPtr m_newItem;
	std::vector<SchemaItemPtr> m_selectedItems;

	std::map<QUuid, CompareAction> m_itemsActions;
	bool m_compareWidget = false;
	std::shared_ptr<VFrame30::Schema> m_compareSourceSchema;
	std::shared_ptr<VFrame30::Schema> m_compareTargetSchema;

	// Selection area variables
	//
	QPointF m_mouseSelectionStartPoint;				// Saved in DocPoints
	QPointF m_mouseSelectionEndPoint;				// Saved in DocPoints

	QPoint m_mouseSelectionStartPointForUpdate;		// Saved in WidgetPoints, just for update right region
	QPoint m_mouseSelectionEndPointForUpdate;		// Saved in WidgetPoints, just for update right region

	// Variables for performing some actions on object (moving, resizing, etc)
	//
	QPointF m_editStartDocPt;					// Start Point on some actions (moving, etc)
	QPointF m_editEndDocPt;						// End Poin on some actions
	QPointF m_addRectStartPoint;
	QPointF m_addRectEndPoint;

	// Variables for changing ConnectionLine
	//
	std::list<EditConnectionLine> m_editConnectionLines;	// Add new or edit PosConnectionImpl items
	bool m_doNotMoveConnectionLines = false;

	// For updating schema in timerEvent during build
	//
	int m_updateDuringBuildTimer = -1;
	Builder::BuildIssues::Counter m_lastSchemaIssues = {-1, -1};

	// Temporary data can be changed in EditSchemaWidget
	//
	friend EditSchemaWidget;
};
