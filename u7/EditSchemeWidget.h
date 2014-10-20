#pragma once

#include "../VFrame30/SchemeView.h"
#include "../VFrame30/VideoItem.h"
#include "../VFrame30/FblItem.h"
#include "EditEngine/EditEngine.h"
#include "../include/DbController.h"
#include "SchemeItemPropertiesDialog.h"

#define GridSizeDisplay				5
#define GridSizeMm					mm2in(0.5)

#define ControlBarSizeDisplay		10
#define ControlBarMm				mm2in(2.4)
#define ControlBar(_unit, _zoom)	((_unit == VFrame30::SchemeUnit::Display) ?	ControlBarSizeDisplay * (100.0 / _zoom) : ControlBarMm * (100.0 / _zoom))

enum MouseState
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

// Possible action on CVideoItem
//
enum VideoItemAction
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

//
//
// EditVideoFrameView
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
	void drawNewItemOutline(QPainter* p);
	void drawSelectionArea(QPainter* p);
	void drawMovingItems(VFrame30::CDrawParam* drawParam);
	void drawRectSizing(VFrame30::CDrawParam* drawParam);
	void drawMovingLinePoint(VFrame30::CDrawParam* drawParam);
	void drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam);

	void drawGrid(QPainter* p);

	// Some determine functions
	//
protected:
	VideoItemAction getPossibleAction(VFrame30::VideoItem* videoItem, QPointF point, int* outMovingEdgePointIndex);

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
	const std::vector<std::shared_ptr<VFrame30::VideoItem>>& selectedItems() const;
	void setSelectedItems(const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items);
	void setSelectedItems(const std::list<std::shared_ptr<VFrame30::VideoItem>>& items);
	void setSelectedItem(const std::shared_ptr<VFrame30::VideoItem>& item);
	void addSelection(const std::shared_ptr<VFrame30::VideoItem>& item);

	void clearSelection();
	bool removeFromSelection(const std::shared_ptr<VFrame30::VideoItem>& item);
	bool isItemSelected(const std::shared_ptr<VFrame30::VideoItem>& item);

	// Data
	//
private:
	int m_activeLayer;
	MouseState m_mouseState;

protected:
	std::shared_ptr<VFrame30::VideoItem> m_newItem;
	std::vector<std::shared_ptr<VFrame30::VideoItem>> m_selectedItems;

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
	std::list<VFrame30::VideoItemPoint> m_movingVertexPoints;


	// Temporary data, can be changed in EditVideoFrameWidget
	//
	friend EditSchemeWidget;
};


//
//
// EditVideoFrameWidget
//
//
class EditSchemeWidget : public QScrollArea
{
	Q_OBJECT

private:
	EditSchemeWidget();			// deleted;

public:
	EditSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme, const DbFileInfo& fileInfo, DbController* dbController);
	virtual ~EditSchemeWidget();
	
protected:
	void createActions();

	// Set corresponding to the current situation and user actions context menu
	//
	void setCorrespondingContextMenu();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event);

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
	bool MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX = 0, int dpiY = 0);

	void addItem(std::shared_ptr<VFrame30::VideoItem> newItem);

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
	void getCurrentWorkcopy();				// Save current videoframe to a file
	void setCurrentWorkcopy();				// Load a videoframe from a file
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

	void zoomIn();
	void zoomOut();
	void zoom100();
	void selectAll();

	void properties();
	void selectionChanged();

	void addFblElement();

	// Properties
	//
public:
	DbController* dbcontroller();
	DbController* db();

	std::shared_ptr<VFrame30::Scheme>& scheme();
	std::shared_ptr<VFrame30::Scheme>& scheme() const;
	void setScheme(std::shared_ptr<VFrame30::Scheme>& scheme);

	std::shared_ptr<VFrame30::SchemeLayer> activeLayer();

	const std::vector<std::shared_ptr<VFrame30::VideoItem>>& selectedItems() const;

	EditSchemeView* schemeView();
	const EditSchemeView* schemeView() const;

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	double zoom() const;
	void setZoom(double value, int horzScrollValue = -1, int vertScrollValue = -1);

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

	bool m_snapToGrid;

	// Interface data
	//
	QPoint mousePos;			// Keeps mouse pos during different actions like scrolling etc
	int horzScrollBarValue;		// Horizintal scroll bar value in mousePressEvent -- midButton
	int vertScrollBarValue;		// Vertical scroll bar value in mousePressEvent -- midButton

	EditSchemeView* m_videoFrameView;
	EditEngine::EditEngine* m_editEngine;

	SchemeItemPropertiesDialog* m_propertiesDialog = nullptr;

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
		VideoItemAction action;
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
	QAction* m_escapeAction;

	//	Contexet Menu
	//
private:
	// File
	//
	QMenu* m_fileMenu;
	QAction* m_fileAction;
		QAction* m_fileCheckOutAction;
		QAction* m_fileCheckInAction;
		QAction* m_fileUndoChangesAction;
		// ------------------------------
		QAction* m_fileSeparatorAction0;
		QAction* m_fileSaveAction;
		QAction* m_fileSeparatorAction1;
		// ------------------------------
		QAction* m_fileGetWorkcopyAction;
		QAction* m_fileSetWorkcopyAction;
		// ------------------------------
		QAction* m_fileSeparatorAction2;
		QAction* m_filePropertiesAction;
		// ------------------------------
		QAction* m_fileSeparatorAction3;
		QAction* m_fileCloseAction;

	// Add Item
	//
	QMenu* m_addMenu;
	QAction* m_addAction;
		QAction* m_addLineAction;
		QAction* m_addConnectionLineAction;
		QAction* m_addRectAction;
		// ------------------------------
		QAction* m_addSeparatorAction0;
		QAction* m_addInputSignalAction;
		QAction* m_addOutputSignalAction;
		QAction* m_addFblElementAction;
		QAction* m_addLinkAction;

	// Edit
	//
	QMenu* m_editMenu;
	QAction* m_editAction;
		QAction* m_undoAction;
		QAction* m_redoAction;
		// ------------------------------
		QAction* m_editSeparatorAction0;
		QAction* m_selectAllAction;
		// ------------------------------
		QAction* m_editSeparatorAction1;
		QAction* m_editCutAction;
		QAction* m_editCopyAction;
		QAction* m_editPasteAction;
		// ------------------------------
		QAction* m_editSeparatorAction2;
		QAction* m_deleteAction;
		// ------------------------------
		QAction* m_editSeparatorAction3;
		QAction* m_propertiesAction;

	// View
	//
	QMenu* m_viewMenu;
	QAction* m_viewAction;
		QAction* m_zoomInAction;
		QAction* m_zoomOutAction;
		QAction* m_zoom100Action;
		// ------------------------------
		QAction* m_viewSeparatorAction0;
		QAction* m_snapToGridAction;

	// Properties
	//
	QAction* m_separatorAction0;
	//QMenu* m_propertiesMenu;
	//QAction* m_propertiesAction;

	// --
	// End of ConextMenu
};


