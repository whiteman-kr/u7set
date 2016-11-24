#pragma once

#include "../VFrame30/BaseSchemaWidget.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/SchemaView.h"
#include "../VFrame30/SchemaItem.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/UfbSchema.h"
#include "../lib/DbController.h"
#include "./EditEngine/EditEngine.h"

#define ControlBarSizeDisplay		10
#define ControlBarMm				mm2in(2.4)
#define ControlBar(_unit, _zoom)	((_unit == VFrame30::SchemaUnit::Display) ?	ControlBarSizeDisplay * (100.0 / _zoom) : ControlBarMm * (100.0 / _zoom))


enum class MouseState
{
	None,								// No state
	Scrolling,							// Scrolling with middle mouse button
	Selection,							// Selection items
	Moving,								// Moving Items
	SizingTopLeft,						// Resizing ISchemaItemPosRect
	SizingTop,							// Resizing ISchemaItemPosRect
	SizingTopRight,						// Resizing ISchemaItemPosRect
	SizingRight,						// Resizing ISchemaItemPosRect
	SizingBottomRight,					// Resizing ISchemaItemPosRect
	SizingBottom,						// Resizing ISchemaItemPosRect
	SizingBottomLeft,					// Resizing ISchemaItemPosRect
	SizingLeft,							// Resizing ISchemaItemPosRect
	MovingStartLinePoint,				// Moving point ISchemaItemPosLine.StartDocPt
	MovingEndLinePoint,					// Moving point ISchemaItemPosLine.EndDocPt
	AddSchemaPosLineStartPoint,			// Add item ISchemaPosLine
	AddSchemaPosLineEndPoint,			// Add item ISchemaPosLine
	AddSchemaPosRectStartPoint,			// Add item ISchemaPosRect
	AddSchemaPosRectEndPoint,			// Add item ISchemaPosRect
	AddSchemaPosConnectionStartPoint,	// Add item ISchemaPosConnectionLine
	AddSchemaPosConnectionNextPoint,	// Add item ISchemaPosConnectionLine
	MovingHorizontalEdge,				// Moving horizntal edge ISchemaPosConnectionLine
	MovingVerticalEdge,					// Moving vertical edge ISchemaPosConnectionLine
	MovingConnectionLinePoint			// Moving point ISchemaPosConnectionLine
};


// Possible action on SchemaItem
//
enum class SchemaItemAction
{
	NoAction,							// No Action
	MoveItem,							// Move Item
	ChangeSizeTopLeft,					// Change rectangle size (ISchemaPosRect)
	ChangeSizeTop,						// Change rectangle size (ISchemaPosRect)
	ChangeSizeTopRight,					// Change rectangle size (ISchemaPosRect)
	ChangeSizeRight,					// Change rectangle size (ISchemaPosRect)
	ChangeSizeBottomRight,				// Change rectangle size (ISchemaPosRect)
	ChangeSizeBottom,					// Change rectangle size (ISchemaPosRect)
	ChangeSizeBottomLeft,				// Change rectangle size (ISchemaPosRect)
	ChangeSizeLeft,						// Change rectangle size (ISchemaPosRect)
	MoveHorizontalEdge,					// Move horizontal edge (ISchemaPosConnection)
	MoveVerticalEdge,					// Move vertical edge  (ISchemaPosConnection)
	MoveStartLinePoint,					// Move start point (ISchemaPosLine)
	MoveEndLinePoint,					// Move end point (ISchemaPosLine)
	MoveConnectionLinePoint				// Move ConnectionLine point (ISchemaPosConnectionLine)
};

enum class CompareAction
{
	Unmodified,
	Modified,
	Added,
	Deleted
};


// Forward declarations
//
class EditSchemaWidget;
class SchemaPropertiesDialog;
class SchemaLayersDialog;
class SchemaItemPropertiesDialog;
class EditSchemaTabPage;
class SchemaFindDialog;


//
// SchemaItemsClipboard
//
struct SchemaItemClipboardData
{
	static const char* mimeType;	// = "application/x-schemaitem";
};

//
//
// EditSchemaView
//
//
class EditSchemaView : public VFrame30::SchemaView
{
	Q_OBJECT

public:
	explicit EditSchemaView(QWidget* parent = 0);
	explicit EditSchemaView(std::shared_ptr<VFrame30::Schema>& schema, QWidget* parent = nullptr);

	virtual ~EditSchemaView();


	// Painting
	//
protected:
	virtual void paintEvent(QPaintEvent*) override;

	void drawBuildIssues(VFrame30::CDrawParam* drawParam, QRectF clipRect);
	void drawRunOrder(VFrame30::CDrawParam* drawParam, QRectF clipRect);
	void drawSelection(QPainter* p);
	void drawNewItemOutline(QPainter* p, VFrame30::CDrawParam* drawParam);
	void drawSelectionArea(QPainter* p);
	void drawMovingItems(VFrame30::CDrawParam* drawParam);
	void drawRectSizing(VFrame30::CDrawParam* drawParam);
	void drawMovingLinePoint(VFrame30::CDrawParam* drawParam);
	void drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam);
	void drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect);

	void drawGrid(QPainter* p);

	// Some determine functions
	//
protected:
	SchemaItemAction getPossibleAction(VFrame30::SchemaItem* schemaItem, QPointF point, int* outMovingEdgePointIndex);

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
	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selectedItems() const;
	void setSelectedItems(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items);
	void setSelectedItems(const std::list<std::shared_ptr<VFrame30::SchemaItem>>& items);
	void setSelectedItem(const std::shared_ptr<VFrame30::SchemaItem>& item);
	void addSelection(const std::shared_ptr<VFrame30::SchemaItem>& item, bool emitSectionChanged = true);

	void clearSelection();
	bool removeFromSelection(const std::shared_ptr<VFrame30::SchemaItem>& item, bool emitSectionChanged = true);
	bool isItemSelected(const std::shared_ptr<VFrame30::SchemaItem>& item);

	// Data
	//
private:
	int m_activeLayer;
	MouseState m_mouseState;

protected:
	std::shared_ptr<VFrame30::SchemaItem> m_newItem;
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_selectedItems;

	std::map<QUuid, CompareAction> m_itemsActions;
	bool m_compareWidget = false;

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
	std::list<VFrame30::SchemaPoint> m_movingVertexPoints;

	//QRubberBand* m_rubberBand;				// Not don yet, on linux same CPU ussage for repainting everything and using QRubberBand
												// TO DO, test CPU Usage on Windows, if it has any advatages, move to using QRubberBand!!!!


	// Temporary data, can be changed in EditSchemaWidget
	//
	friend EditSchemaWidget;
};


//
//
// EditSchemaWidget
//
//
class EditSchemaWidget : public VFrame30::BaseSchemaWidget
{
	Q_OBJECT

private:
	EditSchemaWidget() = delete;

public:
	EditSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, const DbFileInfo& fileInfo, DbController* dbController) :
		VFrame30::BaseSchemaWidget(schema, new EditSchemaView(schema)),
		m_fileInfo(fileInfo),
		m_dbcontroller(dbController)
	{
		assert(schema != nullptr);
		assert(m_dbcontroller);

		createActions();

		// Left Button Down
		//
		m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseLeftDown_None, this, std::placeholders::_1)));
		m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosLineStartPoint, this, std::placeholders::_1)));
		m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosRectStartPoint, this, std::placeholders::_1)));
		m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionStartPoint, std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosConnectionStartPoint, this, std::placeholders::_1)));

		// Left Button Up
		//
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemaWidget::mouseLeftUp_Selection, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemaWidget::mouseLeftUp_Moving, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineEndPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosLineEndPoint, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectEndPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosRectEndPoint, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
		m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));

		// Moouse Mov
		//
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Scrolling, std::bind(&EditSchemaWidget::mouseMove_Scrolling, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemaWidget::mouseMove_Selection, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemaWidget::mouseMove_Moving, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosLineEndPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosLineEndPoint, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosRectEndPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosRectEndPoint, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
		m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));

		// Mouse Right Button Down
		//
		//m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseRightDown_None, this, std::placeholders::_1)));
		m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::AddSchemaPosConnectionNextPoint, std::bind(&EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1)));

		// Mouse Right Button Up
		//
		m_mouseRightUpStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseRightUp_None, this, std::placeholders::_1)));

		// --
		//
		connect(this, &QWidget::customContextMenuRequested, this, &EditSchemaWidget::contextMenu);
		setCorrespondingContextMenu();

		// Edit Engine
		//
		m_editEngine = new EditEngine::EditEngine(editSchemaView(), horizontalScrollBar(), verticalScrollBar(), this);

		connect(m_editEngine, &EditEngine::EditEngine::stateChanged, this, &EditSchemaWidget::editEngineStateChanged);
		connect(m_editEngine, &EditEngine::EditEngine::modifiedChanged, this, &EditSchemaWidget::modifiedChangedSlot);

		connect(editSchemaView(), &EditSchemaView::selectionChanged, this, &EditSchemaWidget::selectionChanged);

		// Clipboard
		//
		connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &EditSchemaWidget::clipboardDataChanged);

		return;
	}
	virtual ~EditSchemaWidget();
	
protected:
	void createActions();

	virtual void keyPressEvent(QKeyEvent* event) override;

	// Set corresponding to the current situation and user actions context menu
	//
	void setCorrespondingContextMenu();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	virtual void mouseMoveEvent(QMouseEvent* event) override;

	// Mouse Left Button Down
	//
	void mouseLeftDown_None(QMouseEvent* event);
	void mouseLeftDown_AddSchemaPosLineStartPoint(QMouseEvent* event);
	void mouseLeftDown_AddSchemaPosRectStartPoint(QMouseEvent* event);
	void mouseLeftDown_AddSchemaPosConnectionStartPoint(QMouseEvent* event);

	// Mouse Left Button Up
	//
	void mouseLeftUp_Selection(QMouseEvent* event);
	void mouseLeftUp_Moving(QMouseEvent* event);
	void mouseLeftUp_SizingRect(QMouseEvent* event);
	void mouseLeftUp_MovingLinePoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemaPosLineEndPoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemaPosRectEndPoint(QMouseEvent* event);
	void mouseLeftUp_AddSchemaPosConnectionNextPoint(QMouseEvent* event);
	void mouseLeftUp_MovingEdgeOrVertex(QMouseEvent* event);

	// Mouse Move
	//
	void mouseMove_Scrolling(QMouseEvent* event);
	void mouseMove_Selection(QMouseEvent* event);
	void mouseMove_Moving(QMouseEvent* event);
	void mouseMove_SizingRect(QMouseEvent* event);
	void mouseMove_MovingLinePoint(QMouseEvent* event);
	void mouseMove_AddSchemaPosLineEndPoint(QMouseEvent* event);
	void mouseMove_AddSchemaPosRectEndPoint(QMouseEvent* event);
	void mouseMove_AddSchemaPosConnectionNextPoint(QMouseEvent* event);
	void mouseMove_MovingEdgesOrVertex(QMouseEvent* event);

	// Mouse Right Button Down
	//
	void mouseRightDown_None(QMouseEvent* event);
	void mouseRightDown_AddSchemaPosConnectionNextPoint(QMouseEvent* event);

	// Mouse Right Button Up
	//
	void mouseRightUp_None(QMouseEvent* event);

	// Methods
	//
public:
	QPointF widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const;
	QPointF snapToGrid(QPointF pt) const;

	bool updateAfbsForSchema();
	bool updateUfbsForSchema();

protected:
	void addItem(std::shared_ptr<VFrame30::SchemaItem> newItem);

	void setMouseCursor(QPoint mousePos);

	QPointF magnetPointToPin(QPointF docPoint);

	std::vector<VFrame30::SchemaPoint> removeUnwantedPoints(const std::vector<VFrame30::SchemaPoint>& source) const;
	std::list<VFrame30::SchemaPoint> removeUnwantedPoints(const std::list<VFrame30::SchemaPoint>& source) const;

	bool loadAfbsDescriptions(std::vector<std::shared_ptr<Afb::AfbElement>>* out);
	bool loadUfbSchemas(std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out);

public:
	void resetAction();
	void clearSelection();

	// Signals
	//
signals:
	void closeTab(QWidget* tabWidget);		// Command to the owner to Close current tab
	void checkInFile();						// Command to the owner to CheckIn the file.
	void checkOutFile();					// Command to the owner to CheckOut the file.
	void undoChangesFile();					// Command to the owner to Undo the file in version control system, and reread last version.
	void saveWorkcopy();
	void getCurrentWorkcopy();				// Save current schema to a file
	void setCurrentWorkcopy();				// Load a schema from a file
	void modifiedChanged(bool modified);	// Command to the owner to change title

	// Slots
	//
protected slots:
	void contextMenu(const QPoint& pos);

	void exportToPdf();

	void signalsProperties(QStringList strIds);
	void addNewAppSignal(std::shared_ptr<VFrame30::SchemaItem> schemaItem);

	void escapeKey();
	void f2Key();
	void deleteKey();

	void undo();
	void redo();
	void editEngineStateChanged(bool canUndo, bool canRedo);

	void modifiedChangedSlot(bool modified);

	void selectAll();
	void selectItem(std::shared_ptr<VFrame30::SchemaItem> item);

	void editCut();
	void editCopy();
	void editPaste();

	void schemaProperties();
	void properties();
	void layers();
	void selectionChanged();

	void clipboardDataChanged();

	void addFblElement();			// Add Application Functional Block
	void addUfbElement();			// Add User Functional Block

	void onLeftKey();
	void onRightKey();
	void onUpKey();
	void onDownKey();

	void sameWidth();
	void sameHeight();
	void sameSize();

	void alignLeft();
	void alignRight();
	void alignTop();
	void alignBottom();

	void bringToFront();
	void bringForward();
	void sendToBack();
	void sendBackward();

	void toggleComment();

	void toggleLock();

	void find();
	void findNext();
	void findPrev();

	void hideWorkDialogs();

	// Properties
	//
public:
	DbController* dbcontroller();
	DbController* db();

	std::shared_ptr<VFrame30::SchemaLayer> activeLayer();

	const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& selectedItems() const;

	EditSchemaView* editSchemaView();
	const EditSchemaView* editSchemaView() const;

	bool isLogicSchema() const;
	bool isUfbSchema() const;

	std::shared_ptr<VFrame30::LogicSchema> logicSchema();
	const std::shared_ptr<VFrame30::LogicSchema> logicSchema() const;

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& fi);

	bool snapToGrid() const;
	void setSnapToGrid(bool value);

	bool compareWidget() const;
	void setCompareWidget(bool value);

	bool readOnly() const;
	void setReadOnly(bool value);

	bool modified() const;
	void setModified();
	void resetModified();

	void resetEditEngine();

	void setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions);

	// Data
	//
private:
	DbFileInfo m_fileInfo;
	DbController* m_dbcontroller = nullptr;

	bool m_snapToGrid = true;

	EditEngine::EditEngine* m_editEngine = nullptr;

	SchemaPropertiesDialog* m_schemaPropertiesDialog = nullptr;
	SchemaItemPropertiesDialog* m_itemsPropertiesDialog = nullptr;

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
		SchemaItemAction action;
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

	SchemaFindDialog* m_findDialog = nullptr;

	// Actions
	//
private:
	QAction* m_escapeAction = nullptr;
	QAction* m_f2Action = nullptr;	// Edit inputs/outputs signal strid

	QAction* m_infoModeAction = nullptr;

	//	Contexet Menu
	//
friend class EditSchemaTabPage;		// EditSchemaTabPage has toolbar, and it will contain some actions from this class

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
		QAction* m_fileExportToPdfAction = nullptr;
		QAction* m_fileSeparatorAction1 = nullptr;
		// ------------------------------
		QAction* m_fileExportAction = nullptr;
		QAction* m_fileImportAction = nullptr;
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
		QAction* m_addTextAction = nullptr;
		// ------------------------------
		QAction* m_addSeparatorAction0 = nullptr;
		QAction* m_addInputSignalAction = nullptr;
		QAction* m_addOutputSignalAction = nullptr;
		QAction* m_addInOutSignalAction = nullptr;
		QAction* m_addConstantAction = nullptr;
		QAction* m_addTerminatorAction = nullptr;
		QAction* m_addFblElementAction = nullptr;
		QAction* m_addLinkAction = nullptr;
		QAction* m_addTransmitter = nullptr;
		QAction* m_addReceiver = nullptr;
		QAction* m_addUfbAction = nullptr;

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

	// Size and Pos (Align)
	//
	QMenu* m_sizeAndPosMenu = nullptr;
	QAction* m_sizeAndPosAction = nullptr;
		QAction* m_sameWidthAction = nullptr;
		QAction* m_sameHeightAction = nullptr;
		QAction* m_sameSizeAction = nullptr;
		// ------------------------------
		QAction* m_sizeAndPosSeparatorAction0 = nullptr;
		QAction* m_alignLeftAction = nullptr;
		QAction* m_alignTopAction = nullptr;
		QAction* m_alignRightAction = nullptr;
		QAction* m_alignBottomAction = nullptr;

	// Order
	//
	QMenu* m_orderMenu = nullptr;
	QAction* m_orderAction = nullptr;
		QAction* m_bringToFrontAction = nullptr;
		QAction* m_bringForwardAction = nullptr;
		QAction* m_sendToBackAction = nullptr;
		QAction* m_sendBackwardAction = nullptr;

	// View
	//
	QMenu* m_viewMenu = nullptr;
	QAction* m_viewAction = nullptr;
		QAction* m_zoomInAction = nullptr;
		QAction* m_zoomOutAction = nullptr;
		QAction* m_zoom100Action = nullptr;
		// ------------------------------
		QAction* m_viewSeparatorAction0 = nullptr;
		QAction* m_snapToGridAction = nullptr;

	// Properties
	//
	QAction* m_separatorAction0 = nullptr;

	QAction* m_toggleCommentAction = nullptr;
	QAction* m_lockAction = nullptr;

	QAction* m_findAction = nullptr;
	QAction* m_findNextAction = nullptr;
	QAction* m_findPrevAction = nullptr;

	QAction* m_layersAction = nullptr;

	//QMenu* m_propertiesMenu = nullptr;
	//QAction* m_propertiesAction = nullptr;

	// --
	// End of ConextMenu
private:
};


class SchemaFindDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaFindDialog(QWidget* parent);

	QString findText() const;
	void setFocusToEditLine();

signals:
	void findPrev();
	void findNext();

public slots:
	void updateCompleter();

private:
	QLineEdit* m_lineEdit = nullptr;
	QPushButton* m_prevButton = nullptr;
	QPushButton* m_nextButton = nullptr;
};

