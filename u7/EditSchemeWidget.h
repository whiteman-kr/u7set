#pragma once

#include "../VFrame30/BaseSchemeWidget.h"
#include "../VFrame30/SchemeView.h"
#include "../VFrame30/SchemeItem.h"
#include "../VFrame30/FblItem.h"
#include "../include/DbController.h"

//#define GridSizeDisplay				5
//#define GridSizeMm					mm2in(1.0)
//#define GridSizeIn					0.04

#define ControlBarSizeDisplay		10
#define ControlBarMm				mm2in(2.4)
#define ControlBar(_unit, _zoom)	((_unit == VFrame30::SchemeUnit::Display) ?	ControlBarSizeDisplay * (100.0 / _zoom) : ControlBarMm * (100.0 / _zoom))

enum class MouseState
{
	None,								// No state
	Scrolling,							// Scrolling with middle mouse button
	Selection,							// Selection items
	Moving,								// Moving Items
	SizingTopLeft,						// Resizing ISchemeItemPosRect
	SizingTop,							// Resizing ISchemeItemPosRect
	SizingTopRight,						// Resizing ISchemeItemPosRect
	SizingRight,						// Resizing ISchemeItemPosRect
	SizingBottomRight,					// Resizing ISchemeItemPosRect
	SizingBottom,						// Resizing ISchemeItemPosRect
	SizingBottomLeft,					// Resizing ISchemeItemPosRect
	SizingLeft,							// Resizing ISchemeItemPosRect
	MovingStartLinePoint,				// Moving point ISchemeItemPosLine.StartDocPt
	MovingEndLinePoint,					// Moving point ISchemeItemPosLine.EndDocPt
	AddSchemePosLineStartPoint,			// Add item ISchemePosLine
	AddSchemePosLineEndPoint,			// Add item ISchemePosLine
	AddSchemePosRectStartPoint,			// Add item ISchemePosRect
	AddSchemePosRectEndPoint,			// Add item ISchemePosRect
	AddSchemePosConnectionStartPoint,	// Add item ISchemePosConnectionLine
	AddSchemePosConnectionNextPoint,	// Add item ISchemePosConnectionLine
	MovingHorizontalEdge,				// Moving horizntal edge ISchemePosConnectionLine
	MovingVerticalEdge,					// Moving vertical edge ISchemePosConnectionLine
	MovingConnectionLinePoint			// Moving point ISchemePosConnectionLine
};

// Possible action on SchemeItem
//
enum class SchemeItemAction
{
	NoAction,							// No Action
	MoveItem,							// Move Item
	ChangeSizeTopLeft,					// Change rectangle size (ISchemePosRect)
	ChangeSizeTop,						// Change rectangle size (ISchemePosRect)
	ChangeSizeTopRight,					// Change rectangle size (ISchemePosRect)
	ChangeSizeRight,					// Change rectangle size (ISchemePosRect)
	ChangeSizeBottomRight,				// Change rectangle size (ISchemePosRect)
	ChangeSizeBottom,					// Change rectangle size (ISchemePosRect)
	ChangeSizeBottomLeft,				// Change rectangle size (ISchemePosRect)
	ChangeSizeLeft,						// Change rectangle size (ISchemePosRect)
	MoveHorizontalEdge,					// Move horizontal edge (ISchemePosConnection)
	MoveVerticalEdge,					// Move vertical edge  (ISchemePosConnection)
	MoveStartLinePoint,					// Move start point (ISchemePosLine)
	MoveEndLinePoint,					// Move end point (ISchemePosLine)
	MoveConnectionLinePoint				// Move ConnectionLine point (ISchemePosConnectionLine)
};

class EditSchemeWidget;
class SchemePropertiesDialog;
class SchemeLayersDialog;
class SchemeItemPropertiesDialog;

namespace EditEngine
{
	class EditEngine;
}

//
//
// EditSchemeView
//
//
class EditSchemeView : public VFrame30::SchemeView
{
	Q_OBJECT

public:
	explicit EditSchemeView(QWidget* parent = 0);
	explicit EditSchemeView(std::shared_ptr<VFrame30::Scheme>& scheme, QWidget* parent = nullptr);

	virtual ~EditSchemeView();

	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

	void drawSelection(QPainter* p);
	void drawNewItemOutline(QPainter* p, VFrame30::CDrawParam* drawParam);
	void drawSelectionArea(QPainter* p);
	void drawMovingItems(VFrame30::CDrawParam* drawParam);
	void drawRectSizing(VFrame30::CDrawParam* drawParam);
	void drawMovingLinePoint(VFrame30::CDrawParam* drawParam);
	void drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam);

	void drawGrid(QPainter* p);

	// Some determine functions
	//
protected:
	SchemeItemAction getPossibleAction(VFrame30::SchemeItem* schemeItem, QPointF point, int* outMovingEdgePointIndex);

	// Signals
signals:
	void selectionChanged();

	// Properties
	//
public:
	// Layer props
	//
	QUuid activeLayerGuid() const;
	std::shared_ptr<VFrame30::SchemeLayer> activeLayer();
	void setActiveLayer(std::shared_ptr<VFrame30::SchemeLayer> layer);

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	// Selection
	//
	const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& selectedItems() const;
	void setSelectedItems(const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items);
	void setSelectedItems(const std::list<std::shared_ptr<VFrame30::SchemeItem>>& items);
	void setSelectedItem(const std::shared_ptr<VFrame30::SchemeItem>& item);
	void addSelection(const std::shared_ptr<VFrame30::SchemeItem>& item);

	void clearSelection();
	bool removeFromSelection(const std::shared_ptr<VFrame30::SchemeItem>& item);
	bool isItemSelected(const std::shared_ptr<VFrame30::SchemeItem>& item);

	// Data
	//
private:
	int m_activeLayer;
	MouseState m_mouseState;

protected:
	std::shared_ptr<VFrame30::SchemeItem> m_newItem;
	std::vector<std::shared_ptr<VFrame30::SchemeItem>> m_selectedItems;

	// Selection area variables
	//
	QPointF m_mouseSelectionStartPoint;
	QPointF m_mouseSelectionEndPoint;

	// Variables for performing some actions on object (moving, resizing, etc)
	//
	QPointF m_editStartDocPt;					// Start Point on some actions (moving, etc)
	QPointF m_editEndDocPt;						// End Poin on some actions
	QPointF m_addRectStartPoint;
	QPointF m_addRectEndPoint;

	// Variables for changing ConnectionLine
	//
	double m_editStartMovingEdge;				// Start pos fro moving edge
	double m_editEndMovingEdge;					// End pos for moving edge
	double m_editStartMovingEdgeX;				// Ќачальна€ координата дл€ перемещени€ вершины
	double m_editStartMovingEdgeY;				// Ќачальна€ координата дл€ перемещени€ вершины
	double m_editEndMovingEdgeX;				//  онечна€ координата дл€ перемещени€ грани
	double m_editEndMovingEdgeY;				//  онечна€ координата дл€ перемещени€ грани
	int m_movingEdgePointIndex;					// »ндекс точки при перемещении вершины или грани

												// ѕри перемещении вершины соединительно линии здесь
												// соххран€ютс€ точки (в отрисовке), и потом они
												// используютс€ при завершении (MouseUp) редактировани€.
	std::list<VFrame30::SchemePoint> m_movingVertexPoints;


	// Temporary data, can be changed in EditSchemeWidget
	//
	friend EditSchemeWidget;
};


//
//
// EditSchemeWidget
//
//
class EditSchemeWidget : public VFrame30::BaseSchemeWidget
{
	Q_OBJECT

private:
	EditSchemeWidget() = delete;

public:
	EditSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme, const DbFileInfo& fileInfo, DbController* dbController);
	virtual ~EditSchemeWidget();
	
protected:
	void createActions();

	virtual void keyPressEvent(QKeyEvent* event) override;

	// Set corresponding to the current situation and user actions context menu
	//
	void setCorrespondingContextMenu();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	virtual void mouseMoveEvent(QMouseEvent* event) override;

	// Mouse Left Button Down
	//
	void mouseLeftDown_None(QMouseEvent* event);
	void mouseLeftDown_AddSchemePosLineStartPoint(QMouseEvent* event);
	void mouseLeftDown_AddSchemePosRectStartPoint(QMouseEvent* event);
	void mouseLeftDown_AddSchemePosConnectionStartPoint(QMouseEvent* event);

	// Mouse Left Button Up
	//
	void mouseLeftUp_Selection(QMouseEvent* event);
	void mouseLeftUp_Moving(QMouseEvent* event);
	void mouseLeftUp_SizingRect(QMouseEvent* event);
	void mouseLeftUp_MovingLinePoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemePosLineEndPoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemePosRectEndPoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemePosConnectionNextPoint(QMouseEvent* event);
	void mouseLeftUp_MovingEdgeOrVertex(QMouseEvent* event);

	// Mouse Move
	//
	void mouseMove_Scrolling(QMouseEvent* event);
	void mouseMove_Selection(QMouseEvent* event);
	void mouseMove_Moving(QMouseEvent* event);
	void mouseMove_SizingRect(QMouseEvent* event);
	void mouseMove_MovingLinePoint(QMouseEvent* event);
	void mouseMove_AddSchemePosLineEndPoint(QMouseEvent* event);
	void mouseMove_AddSchemePosRectEndPoint(QMouseEvent* event);
	void mouseMove_AddSchemePosConnectionNextPoint(QMouseEvent* event);
	void mouseMove_MovingEdgesOrVertex(QMouseEvent* event);

	// Mouse Right Button Down
	//
	void mouseRightDown_None(QMouseEvent* event);
	void mouseRightDown_AddSchemePosConnectionNextPoint(QMouseEvent* event);

	// Mouse Right Button Up
	//
	void mouseRightUp_None(QMouseEvent* event);

	// Methods
	//
public:
	QPointF widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const;
	QPointF snapToGrid(QPointF pt) const;

protected:
	void addItem(std::shared_ptr<VFrame30::SchemeItem> newItem);

	void setMouseCursor(QPoint mousePos);

public:
	void resetAction();
	void clearSelection();

	// Signals
signals:
	void closeTab(QWidget* tabWidget);		// Command to the owner to Close current tab
	void checkInFile();						// Command to the owner to CheckIn the file.
	void checkOutFile();					// Command to the owner to CheckOut the file.
	void undoChangesFile();					// Command to the owner to Undo the file in version control system, and reread last version.
	void saveWorkcopy();
	void getCurrentWorkcopy();				// Save current scheme to a file
	void setCurrentWorkcopy();				// Load a scheme from a file
	void modifiedChanged(bool modified);	// Command to the owner to change title

	// Slots
	//
protected slots:
	void contextMenu(const QPoint& pos);

	void escapeKey();
	void deleteKey();

	void undo();
	void redo();
	void editEngineStateChanged(bool canUndo, bool canRedo);

	void modifiedChangedSlot(bool modified);

	void selectAll();

	void schemeProperties();
	void properties();
	void layers();
	void selectionChanged();

	void addFblElement();

	// Properties
	//
public:
	DbController* dbcontroller();
	DbController* db();

	std::shared_ptr<VFrame30::SchemeLayer> activeLayer();

	const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& selectedItems() const;

	EditSchemeView* editSchemeView();
	const EditSchemeView* editSchemeView() const;

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& fi);

	bool snapToGrid() const;
	void setSnapToGrid(bool value);

	bool readOnly() const;
	void setReadOnly(bool value);

	bool modified() const;
	void setModified();
	void resetModified();

	void resetEditEngine();

	// Data
	//
private:
	DbFileInfo m_fileInfo;
	DbController* m_dbcontroller = nullptr;

	bool m_snapToGrid = true;

	EditEngine::EditEngine* m_editEngine = nullptr;

	SchemePropertiesDialog* m_schemePropertiesDialog = nullptr;
	SchemeItemPropertiesDialog* m_itemsPropertiesDialog = nullptr;

	// Temporary and state variables
	//

	//Qt::MouseButtons m_mousePressedButtons;

	struct MouseStateCursor
	{
		MouseState mouseState;
		Qt::CursorShape cursorShape;
	};

	struct MouseStateAction
	{
		MouseStateAction(MouseState ms, std::function<void(QMouseEvent* event)> a) :
			mouseState(ms),
			action(a)
		{
		}
		MouseState mouseState;
		std::function<void(QMouseEvent* event)> action;
	};

	struct SizeActionToMouseCursor
	{
		SchemeItemAction action;
		MouseState mouseState;
		Qt::CursorShape cursorShape;
	};

	static const MouseStateCursor m_mouseStateCursor[];
	static const SizeActionToMouseCursor m_sizeActionToMouseCursor[];

	std::vector<MouseStateAction> m_mouseLeftDownStateAction;		// Initializend in constructor
	std::vector<MouseStateAction> m_mouseLeftUpStateAction;			// Initializend in constructor
	std::vector<MouseStateAction> m_mouseRightDownStateAction;		// Initializend in constructor
	std::vector<MouseStateAction> m_mouseRightUpStateAction;		// Initializend in constructor
	std::vector<MouseStateAction> m_mouseMoveStateAction;			// Initializend in constructor

	// Actions
	//
private:
	QAction* m_escapeAction = nullptr;

	//	Contexet Menu
	//
private:
	// File
	//
	QMenu* m_fileMenu = nullptr;
	QAction* m_fileAction = nullptr;
		QAction* m_fileCheckOutAction = nullptr;
		QAction* m_fileCheckInAction = nullptr;
		QAction* m_fileUndoChangesAction = nullptr;
		// ------------------------------
		QAction* m_fileSeparatorAction0 = nullptr;
		QAction* m_fileSaveAction = nullptr;
		QAction* m_fileSeparatorAction1 = nullptr;
		// ------------------------------
		QAction* m_fileGetWorkcopyAction = nullptr;
		QAction* m_fileSetWorkcopyAction = nullptr;
		// ------------------------------
		QAction* m_fileSeparatorAction2 = nullptr;
		QAction* m_filePropertiesAction = nullptr;
		// ------------------------------
		QAction* m_fileSeparatorAction3 = nullptr;
		QAction* m_fileCloseAction = nullptr;

	// Add Item
	//
	QMenu* m_addMenu = nullptr;
	QAction* m_addAction = nullptr;
		QAction* m_addLineAction = nullptr;
		QAction* m_addRectAction = nullptr;
		QAction* m_addPathAction = nullptr;
		// ------------------------------
		QAction* m_addSeparatorAction0 = nullptr;
		QAction* m_addInputSignalAction = nullptr;
		QAction* m_addOutputSignalAction = nullptr;
		QAction* m_addConstantAction = nullptr;
		QAction* m_addFblElementAction = nullptr;
		QAction* m_addLinkAction = nullptr;

	// Edit
	//
	QMenu* m_editMenu = nullptr;
	QAction* m_editAction = nullptr;
		QAction* m_undoAction = nullptr;
		QAction* m_redoAction = nullptr;
		// ------------------------------
		QAction* m_editSeparatorAction0 = nullptr;
		QAction* m_selectAllAction = nullptr;
		// ------------------------------
		QAction* m_editSeparatorAction1 = nullptr;
		QAction* m_editCutAction = nullptr;
		QAction* m_editCopyAction = nullptr;
		QAction* m_editPasteAction = nullptr;
		// ------------------------------
		QAction* m_editSeparatorAction2 = nullptr;
		QAction* m_deleteAction = nullptr;
		// ------------------------------
		QAction* m_editSeparatorAction3 = nullptr;
		QAction* m_propertiesAction = nullptr;

	// View
	//
	QMenu* m_viewMenu = nullptr;
	QAction* m_viewAction = nullptr;
		//QAction* m_zoomInAction = nullptr;		These actions were moved to VFrame30::BaseSchemeWidget
		//QAction* m_zoomOutAction = nullptr;
		//QAction* m_zoom100Action = nullptr;
		// ------------------------------
		QAction* m_viewSeparatorAction0 = nullptr;
		QAction* m_snapToGridAction = nullptr;

	// Properties
	//
	QAction* m_separatorAction0 = nullptr;
	QAction* m_layersAction = nullptr;
	//QMenu* m_propertiesMenu = nullptr;
	//QAction* m_propertiesAction = nullptr;

	// --
	// End of ConextMenu
};


