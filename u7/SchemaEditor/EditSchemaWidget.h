#pragma once
#include "../VFrame30/PosConnectionImpl.h"
#include "../VFrame30/BaseSchemaWidget.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/SchemaView.h"
#include "../VFrame30/SchemaItem.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/UfbSchema.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemBus.h"
#include "../DbLib/DbController.h"
#include "./EditEngine/EditEngine.h"
#include "SignalsTabPage.h"
#include "CreateSignalDialog.h"
#include "EditConnectionLine.h"
#include "EditSchemaView.h"
#include "EditSchemaTypes.h"


// Forward declarations
//
class EditSchemaWidget;
class SchemaPropertiesDialog;
class SchemaLayersDialog;
class SchemaItemPropertiesDialog;
class EditSchemaTabPage;
class SchemaFindDialog;
class AppSignalSetProvider;


//
// SchemaItemsClipboard
//
struct SchemaItemClipboardData
{
	static const char* mimeType;	// = "application/x-schemaitem";
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
	EditSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
					 const DbFileInfo& fileInfo,
					 DbController* dbController,
					 AppSignalSetProvider* signalSetProvider,
					 QWidget* parent);
	virtual ~EditSchemaWidget();
	
protected:
	void createActions();
	void updateFileActions();

	virtual bool event(QEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

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
	// WARNING, if you add another function in MouseRightUp, add in EditSchemaWidget::contextMenu(const QPoint& pos) exception for this MouseMode
	//
	void mouseRightDown_None(QMouseEvent* event);
	void mouseRightDown_AddSchemaPosConnectionNextPoint(QMouseEvent* event);
	void mouseRightDown_MovingEdgesOrVertex(QMouseEvent* event);

	// Mouse Right Button Up
	//
	void mouseRightUp_None(QMouseEvent* event);

	// Methods
	//
public:
	QPointF widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const;
	QPointF snapToGrid(QPointF pt) const;
	VFrame30::SchemaPoint snapToGrid(VFrame30::SchemaPoint pt) const;

	bool updateAfbsForSchema();
	bool updateUfbsForSchema();
	bool updateBussesForSchema();

protected:
	void addItem(SchemaItemPtr newItem);

	void setMouseCursor(QPoint mousePos);

	QPointF magnetPointToPin(QPointF docPoint);

	void movePosConnectionEndPoint(SchemaItemPtr schemaItem, EditConnectionLine* ecl, QPointF toPoint);
	void addConnectionLinePoint(SchemaItemPtr schemaItem, QPointF docPoint);
	void removeConnectionLinePoint(SchemaItemPtr schemaItem, size_t pointIndex);

	// Move ConnectionLinks withFblItemPects' pins
	//
	void initMoveAfbsConnectionLinks(MouseState mouseState);
	void moveAfbsConnectionLinks(QPointF offset, MouseState mouseState);
	void finishMoveAfbsConnectionLinks();

	// --
	//
	bool loadAfbsDescriptions(std::vector<std::shared_ptr<Afb::AfbElement>>* out);
	bool loadUfbSchemas(std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out);

public:
	static bool loadBusses(DbController* db, std::vector<VFrame30::Bus>* out, QWidget* parentWidget);

public:
	void resetAction();
	void clearSelection();

	// Signals
	//
signals:
	void detachOrAttachWindow();			// Command to the owner to attach or detach window from tab
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

	void snapToGridToggled(bool state);

	void exportToPdf();

	void appSignalsSelectedProperties();
	void appSignalsProperties(QStringList strIds);
	void addNewAppSignalSelected();
	void addNewAppSignal(SchemaItemPtr schemaItem);

	void escapeKey();

	void f2Key();
	void f2KeyForRect(SchemaItemPtr item);
	bool f2KeyForReceiver(SchemaItemPtr item, bool setViaEditEngine);
	bool f2KeyForTransmitter(SchemaItemPtr item, bool setViaEditEngine);
	void f2KeyForConst(SchemaItemPtr item);
	void f2KeyForSignal(SchemaItemPtr item);
	void f2KeyForLoopback(SchemaItemPtr item);
	void f2KeyForValue(SchemaItemPtr item);
	void f2KeyForImageValue(SchemaItemPtr item);
	void f2KeyForBus(SchemaItemPtr item);

	void deleteKey();

	void undo();
	void redo();
	void editEngineStateChanged(bool canUndo, bool canRedo);

	void modifiedChangedSlot(bool modified);

	void selectAll();
	void selectItem(SchemaItemPtr item);
	void selectItems(std::vector<SchemaItemPtr> items);

	void editCut();
	void editCopy();
	void editPaste();

	void schemaProperties();
	void properties();
	void layers();
	void compareSchemaItem();
	void selectionChanged();

	void clipboardDataChanged();

	void addTransmitter();
	void addReceiver();

	void addLoopbackSource();
	void addLoopbackTarget();

	void addAfbElement();			// Add Application Functional Block
	void addUfbElement();			// Add User Functional Block

	void addBusComposer();
	void addBusExtractor();
	void addBusItem(std::shared_ptr<VFrame30::SchemaItemBus> schemaItem);

	void onLeftKey(QKeyEvent* e);
	void onRightKey(QKeyEvent* e);
	void onUpKey(QKeyEvent* e);
	void onDownKey(QKeyEvent* e);

private:
	struct NextSelectionItem;
protected:
	bool selectNextLeftItem(NextSelectionItem switchToLeftItem);
	bool selectNextRightItem(NextSelectionItem switchToRightItem);
	void selectNextUpItem();
	void selectNextDownItem();

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

	void transformIntoInput();
	void transformIntoInOut();
	void transformIntoOutput();

	void toggleComment();

	void toggleLock();

	void find();
	void findNext(Qt::CaseSensitivity cs);
	void findPrev(Qt::CaseSensitivity cs);

	int replace(SchemaItemPtr item, QString findText, QString replaceWith, Qt::CaseSensitivity cs);
	void replaceAndFind(QString findText, QString replaceWith, Qt::CaseSensitivity cs);
	void replaceAll(QString findText, QString replaceWith, Qt::CaseSensitivity cs);

	void hideWorkDialogs();

	// Properties
	//
public:
	DbController* dbcontroller();
	DbController* db();

	std::shared_ptr<VFrame30::SchemaLayer> activeLayer();

	const std::vector<SchemaItemPtr>& selectedItems() const;
	std::vector<SchemaItemPtr> selectedNonLockedItems() const;

	EditSchemaView* editSchemaView();
	const EditSchemaView* editSchemaView() const;

	bool isLogicSchema() const;
	bool isUfbSchema() const;
	bool isMonitorSchema() const;
	bool isTuningSchema() const;

	std::shared_ptr<VFrame30::LogicSchema> logicSchema();
	const std::shared_ptr<VFrame30::LogicSchema> logicSchema() const;

	MouseState mouseState() const;
	void setMouseState(MouseState state);

	const DbFileInfo& fileInfo() const;
	void setFileInfo(const DbFileInfo& fi);

	bool snapToGrid() const;
	void setSnapToGrid(bool value);

	bool compareWidget() const;
	bool isCompareWidget() const;
	void setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target);

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

	CreatingSignalOptions m_createSignalOptions;
	CreatingSignalDialogOptions m_createSignalDialoOptions;

	// --
	//
	bool m_ctrlWasPressed = false;

	static QString m_lastUsedLoopbackId;

	QString m_initialSchemaId;

	// Actions
	//
private:
	QAction* m_escapeAction = nullptr;
	QAction* m_f2Action = nullptr;	// Edit inputs/outputs signal strid

	QAction* m_infoModeAction = nullptr;

	//	Contexet Menu
	//
friend class EditSchemaTabPage;		// EditSchemaTabPage has toolbar, and it will contain some actions from this class
friend class EditSchemaTabPageEx;	// EditSchemaTabPageEx has toolbar, and it will contain some actions from this class

private:
	// File
	//
	QMenu* m_fileMenu = nullptr;

	QAction* m_fileAction = nullptr;
		QAction* m_detachWindow = nullptr;
		// ------------------------------
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
		QAction* m_addImageAction = nullptr;
		QAction* m_addFrameAction = nullptr;
		// ------------------------------
		QAction* m_addSeparatorAction0 = nullptr;
		QAction* m_addLinkAction = nullptr;
		QAction* m_addInputSignalAction = nullptr;
		QAction* m_addInOutSignalAction = nullptr;
		QAction* m_addOutputSignalAction = nullptr;
		QAction* m_addConstantAction = nullptr;
		QAction* m_addTerminatorAction = nullptr;
		// ------------------------------
		QAction* m_addSeparatorAfb = nullptr;
		QAction* m_addAfbAction = nullptr;
		QAction* m_addUfbAction = nullptr;
		// ------------------------------
		QAction* m_addSeparatorConn = nullptr;
		QAction* m_addTransmitter = nullptr;
		QAction* m_addReceiver = nullptr;
		// ------------------------------
		QAction* m_addSeparatorLoop = nullptr;
		QAction* m_addLoopbackSource = nullptr;
		QAction* m_addLoopbackTarget = nullptr;
		// ------------------------------
		QAction* m_addSeparatorBus = nullptr;
		QAction* m_addBusComposer = nullptr;
		QAction* m_addBusExtractor = nullptr;

		QAction* m_addValueAction = nullptr;
		QAction* m_addImageValueAction = nullptr;
		QAction* m_addPushButtonAction = nullptr;
		QAction* m_addLineEditAction = nullptr;
		QAction* m_addIndicatorAction = nullptr;

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

	// Transform
	//
	QMenu* m_transformMenu = nullptr;
	QAction* m_transformAction = nullptr;
		QAction* m_transformIntoInputAction = nullptr;
		QAction* m_transformIntoInOutAction = nullptr;
		QAction* m_transformIntoOutputAction = nullptr;

	// View
	//
	QMenu* m_viewMenu = nullptr;
	QAction* m_viewAction = nullptr;
		QAction* m_zoomInAction = nullptr;
		QAction* m_zoomOutAction = nullptr;
		QAction* m_zoom100Action = nullptr;
		QAction* m_zoomFitToScreenAction = nullptr;
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
	QAction* m_compareDiffAction = nullptr;

	QAction* m_addAppSignalAction = nullptr;
	QAction* m_appSignalPropertiesAction = nullptr;

	// --
	// End of ConextMenu
private:

	bool m_lastSelectedAddSignal = false;

	// Selection of next schema item via Alt + arrow keys
	//
	struct NextSelectionItem
	{
		SchemaItemPtr schemaItem;
		int pinIndex = 0;

		bool isNull() const						{ return schemaItem == nullptr; }
		bool isFblItemRect() const				{ return dynamic_cast<VFrame30::FblItemRect*>(schemaItem.get()) != nullptr; }
		VFrame30::FblItemRect* toFblItemRect()	{ return dynamic_cast<VFrame30::FblItemRect*>(schemaItem.get()); }
	};

	NextSelectionItem m_nextSelectionFromLeft;
	NextSelectionItem m_nextSelectionFromRight;

	// --
	//
};


class SchemaFindDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SchemaFindDialog(QWidget* parent);
	virtual ~SchemaFindDialog();

	QString findText() const;
	void setFocusToEditLine();

	void ensureVisible();

signals:
	void findPrev(Qt::CaseSensitivity cs);
	void findNext(Qt::CaseSensitivity cs);

	void replaceAndFind(QString findText, QString replaceWith, Qt::CaseSensitivity cs);
	void replaceAll(QString findText, QString replaceWith, Qt::CaseSensitivity cs);

protected slots:
	void replaceAndFindPressed();
	void replaceAllPressed();

public slots:
	void updateCompleter();
	void updateFoundInformation(SchemaItemPtr item,
								const std::list<std::pair<QString, QString>>& foundProps,
								QString searchText,
								Qt::CaseSensitivity cs);

private:
	virtual void closeEvent(QCloseEvent* e);
	virtual void done(int r);

	void saveSettings();

private:
	QLineEdit* m_findTextEdit = nullptr;
	QLineEdit* m_replaceTextEdit = nullptr;

	QCheckBox* m_caseSensitiveCheckBox = nullptr;
	QTextEdit* m_findResult = nullptr;

	QPushButton* m_prevButton = nullptr;
	QPushButton* m_nextButton = nullptr;

	QPushButton* m_replaceButton = nullptr;
	QPushButton* m_replaceAllButton = nullptr;

};

