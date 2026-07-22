#include "EditSchemaWidget.h"

#include "F2KeyForSchemaItem.h"
#include "GlobalMessanger.h"
#include "ProjectDefaults.h"
#include "SchemaFindDialog.h"
#include "SchemaItemPropertiesDialog.h"
#include "SchemaLayersDialog.h"
#include "SchemaPropertiesDialog.h"
#include "Settings.h"
#include "SignalPropertiesDialog.h"

#include <HardwareLib/LmDescription.h>
#include <VFrame30/ActuatorHeader.h>
#include <VFrame30/SchemaDetails.h>
#include <VFrame30/SchemaItemActuator.h>
#include <VFrame30/SchemaItemAfb.h>
#include <VFrame30/SchemaItemBus.h>
#include <VFrame30/SchemaItemConnection.h>
#include <VFrame30/SchemaItemConst.h>
#include <VFrame30/SchemaItemDiagValue.h>
#include <VFrame30/SchemaItemImage.h>
#include <VFrame30/SchemaItemImageValue.h>
#include <VFrame30/SchemaItemIndicator.h>
#include <VFrame30/SchemaItemLine.h>
#include <VFrame30/SchemaItemLineEdit.h>
#include <VFrame30/SchemaItemLink.h>
#include <VFrame30/SchemaItemLoopback.h>
#include <VFrame30/SchemaItemPath.h>
#include <VFrame30/SchemaItemPushButton.h>
#include <VFrame30/SchemaItemRect.h>
#include <VFrame30/SchemaItemSignal.h>
#include <VFrame30/SchemaItemSlider.h>
#include <VFrame30/SchemaItemTerminator.h>
#include <VFrame30/SchemaItemUfb.h>
#include <VFrame30/SchemaItemValue.h>
#include <VFrame30/SchemaItemVduImage.h>
#include <VFrame30/SchemaItemVduImageValue.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/SchemaItemVduTrend.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaLayer.h>
#include <VFrame30/Session.h>
#include <VFrame30/UfbSchema.h>

#include "../AppSignalLib/Bus.h"
#include "../AppSignalSetProvider.h"
#include "../EditEngine/EditEngine.h"
#include "../Forms/ChooseActuatorDialog.h"
#include "../Forms/ChooseAfbDialog.h"
#include "../Forms/ChooseUfbDialog.h"
#include "../Forms/ComparePropertyObjectDialog.h"


const EditSchemaWidget::MouseStateCursor EditSchemaWidget::m_mouseStateCursor[] = {
	{MouseState::Scrolling, Qt::CursorShape::ArrowCursor},
	{MouseState::Selection, Qt::CursorShape::CrossCursor},
	{MouseState::Moving, Qt::CursorShape::SizeAllCursor},
	{MouseState::SizingTopLeft, Qt::CursorShape::SizeFDiagCursor},
	{MouseState::SizingTop, Qt::CursorShape::SizeVerCursor},
	{MouseState::SizingTopRight, Qt::CursorShape::SizeBDiagCursor},
	{MouseState::SizingRight, Qt::CursorShape::SizeHorCursor},
	{MouseState::SizingBottomRight, Qt::CursorShape::SizeFDiagCursor},
	{MouseState::SizingBottom, Qt::CursorShape::SizeVerCursor},
	{MouseState::SizingBottomLeft, Qt::CursorShape::SizeBDiagCursor},
	{MouseState::SizingLeft, Qt::CursorShape::SizeHorCursor},
	{MouseState::MovingStartLinePoint, Qt::CursorShape::SizeAllCursor},
	{MouseState::MovingEndLinePoint, Qt::CursorShape::SizeAllCursor},
	{MouseState::MovingHorizontalEdge, Qt::CursorShape::SplitVCursor},
	{MouseState::MovingVerticalEdge, Qt::CursorShape::SplitHCursor},
	{MouseState::MovingConnectionLinePoint, Qt::CursorShape::SizeAllCursor},
};

const EditSchemaWidget::SizeActionToMouseCursor EditSchemaWidget::m_sizeActionToMouseCursor[] = {
	{SchemaItemAction::ChangeSizeTopLeft, MouseState::SizingTopLeft, Qt::SizeFDiagCursor},
	{SchemaItemAction::ChangeSizeTop, MouseState::SizingTop, Qt::SizeVerCursor},
	{SchemaItemAction::ChangeSizeTopRight, MouseState::SizingTopRight, Qt::SizeBDiagCursor},
	{SchemaItemAction::ChangeSizeRight, MouseState::SizingRight, Qt::SizeHorCursor},
	{SchemaItemAction::ChangeSizeBottomRight, MouseState::SizingBottomRight, Qt::SizeFDiagCursor},
	{SchemaItemAction::ChangeSizeBottom, MouseState::SizingBottom, Qt::SizeVerCursor},
	{SchemaItemAction::ChangeSizeBottomLeft, MouseState::SizingBottomLeft, Qt::SizeBDiagCursor},
	{SchemaItemAction::ChangeSizeLeft, MouseState::SizingLeft, Qt::SizeHorCursor}};


//
// SchemaItemsClipboard
//
const char* SchemaItemClipboardData::mimeType = "application/x-radiyschemaset";


//
//
// EditSchemaWidget
//
//
EditSchemaWidget::EditSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
								   const DbFileInfo& fileInfo,
								   DbController* dbController,
								   AppSignalSetProvider* signalSetProvider,
								   QWidget* parent) :
	VFrame30::BaseSchemaWidget(schema, new EditSchemaView{signalSetProvider, schema}, parent),
	m_fileInfo(fileInfo),
	m_dbcontroller(dbController),
	m_initialSchemaId(schema->schemaId())
{
	assert(schema != nullptr);
	assert(dbController);

	createActions();

	// Left Button Down
	//
	m_mouseLeftDownStateAction.emplace_back(MouseState::None,
											std::bind(&EditSchemaWidget::mouseLeftDown_None, this, std::placeholders::_1));
	m_mouseLeftDownStateAction.emplace_back(
		MouseState::AddSchemaPosLineStartPoint,
		std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosLineStartPoint, this, std::placeholders::_1));
	m_mouseLeftDownStateAction.emplace_back(
		MouseState::AddSchemaPosRectStartPoint,
		std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosRectStartPoint, this, std::placeholders::_1));
	m_mouseLeftDownStateAction.emplace_back(
		MouseState::AddSchemaPosConnectionStartPoint,
		std::bind(&EditSchemaWidget::mouseLeftDown_AddSchemaPosConnectionStartPoint, this, std::placeholders::_1));

	// Left Button Up
	//
	m_mouseLeftUpStateAction.emplace_back(MouseState::Selection,
										  std::bind(&EditSchemaWidget::mouseLeftUp_Selection, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::Moving,
										  std::bind(&EditSchemaWidget::mouseLeftUp_Moving, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingTopLeft,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingTop,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingTopRight,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingRight,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingBottomRight,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingBottom,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingBottomLeft,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::SizingLeft,
										  std::bind(&EditSchemaWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::MovingStartLinePoint,
										  std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::MovingEndLinePoint,
										  std::bind(&EditSchemaWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::AddSchemaPosLineEndPoint,
										  std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosLineEndPoint, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::AddSchemaPosRectEndPoint,
										  std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosRectEndPoint, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(
		MouseState::AddSchemaPosConnectionNextPoint,
		std::bind(&EditSchemaWidget::mouseLeftUp_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::MovingHorizontalEdge,
										  std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::MovingVerticalEdge,
										  std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1));
	m_mouseLeftUpStateAction.emplace_back(MouseState::MovingConnectionLinePoint,
										  std::bind(&EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1));

	// Mouse Move
	//
	m_mouseMoveStateAction.emplace_back(MouseState::Scrolling,
										std::bind(&EditSchemaWidget::mouseMove_Scrolling, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::Selection,
										std::bind(&EditSchemaWidget::mouseMove_Selection, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::Moving, std::bind(&EditSchemaWidget::mouseMove_Moving, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingTopLeft,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingTop,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingTopRight,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingRight,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingBottomRight,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingBottom,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingBottomLeft,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::SizingLeft,
										std::bind(&EditSchemaWidget::mouseMove_SizingRect, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::MovingStartLinePoint,
										std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::MovingEndLinePoint,
										std::bind(&EditSchemaWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::AddSchemaPosLineEndPoint,
										std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosLineEndPoint, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::AddSchemaPosRectEndPoint,
										std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosRectEndPoint, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(
		MouseState::AddSchemaPosConnectionNextPoint,
		std::bind(&EditSchemaWidget::mouseMove_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::MovingHorizontalEdge,
										std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::MovingVerticalEdge,
										std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1));
	m_mouseMoveStateAction.emplace_back(MouseState::MovingConnectionLinePoint,
										std::bind(&EditSchemaWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1));

	// Mouse Right Button Down
	//
	// m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemaWidget::mouseRightDown_None, this,
	// std::placeholders::_1)));
	m_mouseRightDownStateAction.emplace_back(
		MouseState::AddSchemaPosConnectionNextPoint,
		std::bind(&EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint, this, std::placeholders::_1));
	m_mouseRightDownStateAction.emplace_back(MouseState::MovingConnectionLinePoint,
											 std::bind(&EditSchemaWidget::mouseRightDown_MovingEdgesOrVertex, this, std::placeholders::_1));

	// Mouse Right Button Up
	//
	m_mouseRightUpStateAction.emplace_back(MouseState::None, std::bind(&EditSchemaWidget::mouseRightUp_None, this, std::placeholders::_1));

	// Init Session Variables
	//
	schemaView()->session().setProject(dbController->currentProject().projectName());
	schemaView()->session().setUsername(dbController->currentUser().username());
	schemaView()->session().setHost(QHostInfo::localHostName());

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

	clipboardDataChanged(); // Enable m_editPasteAction if somthing in clipborad

	return;
}

EditSchemaWidget::~EditSchemaWidget() {}

void EditSchemaWidget::createActions()
{
	// Escape Button Pressed
	//
	m_escapeAction = new QAction(tr("Escape"), this);
	m_escapeAction->setEnabled(true);
	m_escapeAction->setMenuRole(QAction::NoRole);
	m_escapeAction->setShortcut(QKeySequence(Qt::Key_Escape));
	connect(m_escapeAction, &QAction::triggered, this, &EditSchemaWidget::escapeKey);
	addAction(m_escapeAction);

	// F2 Button Pressed
	//
	m_f2Action = new QAction(tr("Edit AppSignalID(s)"), this);
	m_f2Action->setEnabled(true);
	m_f2Action->setMenuRole(QAction::NoRole);
	m_f2Action->setShortcut(QKeySequence(Qt::Key_F2));
	m_f2Action->setShortcutVisibleInContextMenu(true);
	connect(m_f2Action, &QAction::triggered, this, &EditSchemaWidget::f2Key);
	addAction(m_f2Action);

	// Info Mode Action
	//
	m_infoModeAction = new QAction(tr("Info"), this);
	m_infoModeAction->setToolTip(tr("Show/Hide information like items' label"));
	m_infoModeAction->setEnabled(true);
	m_infoModeAction->setCheckable(true);
	m_infoModeAction->setChecked(theSettings.isInfoMode());
	m_infoModeAction->setMenuRole(QAction::NoRole);
	m_infoModeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
	m_infoModeAction->setShortcutVisibleInContextMenu(true);
	connect(m_infoModeAction,
			&QAction::toggled,
			this,
			[this](bool checked)
			{
				theSettings.setInfoMode(checked);
				editSchemaView()->update();
			});
	addAction(m_infoModeAction);

	//
	// File
	//
	m_detachWindow = new QAction(tr("Detach/Attach Window"), this);
	m_detachWindow->setStatusTip(tr("Detach/attach window..."));
	m_detachWindow->setEnabled(true);
	m_detachWindow->setShortcut(Qt::ALT | Qt::Key_D);
	m_detachWindow->setShortcutVisibleInContextMenu(true);
	connect(m_detachWindow, &QAction::triggered, this, &EditSchemaWidget::detachOrAttachWindow);
	addAction(m_detachWindow);

	m_fileCheckInAction = new QAction(tr("Check In"), this);
	m_fileCheckInAction->setStatusTip(tr("Check In changes..."));
	m_fileCheckInAction->setIcon(QIcon{":/Images/Images/SchemaCheckIn.svg"});
	m_fileCheckInAction->setEnabled(false);
	connect(m_fileCheckInAction, &QAction::triggered, this, &EditSchemaWidget::checkInFile);

	m_fileCheckOutAction = new QAction(tr("Check Out"), this);
	m_fileCheckOutAction->setStatusTip(tr("Check Out for edit..."));
	m_fileCheckOutAction->setIcon(QIcon{":/Images/Images/SchemaCheckOut.svg"});
	m_fileCheckOutAction->setEnabled(false);
	connect(m_fileCheckOutAction, &QAction::triggered, this, &EditSchemaWidget::checkOutFile);

	m_fileUndoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_fileUndoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_fileUndoChangesAction->setIcon(QIcon{":/Images/Images/SchemaUndo.svg"});
	m_fileUndoChangesAction->setEnabled(false);
	connect(m_fileUndoChangesAction, &QAction::triggered, this, &EditSchemaWidget::undoChangesFile);

	m_fileSeparatorAction0 = new QAction(this);
	m_fileSeparatorAction0->setSeparator(true);

	m_fileSaveAction = new QAction(tr("Save"), this);
	m_fileSaveAction->setStatusTip(tr("Save current changes..."));
	m_fileSaveAction->setEnabled(false);
	m_fileSaveAction->setShortcuts(QKeySequence::Save);
	m_fileSaveAction->setShortcutVisibleInContextMenu(true);
	connect(m_fileSaveAction, &QAction::triggered, this, &EditSchemaWidget::saveWorkcopy);
	addAction(m_fileSaveAction);

	m_fileExportToPdfAction = new QAction(tr("Export to PDF"), this);
	m_fileExportToPdfAction->setStatusTip(tr("Export schema to PDF..."));
	m_fileExportToPdfAction->setEnabled(true);
	connect(m_fileExportToPdfAction, &QAction::triggered, this, &EditSchemaWidget::exportToPdf);
	addAction(m_fileExportToPdfAction);

	m_fileSeparatorAction1 = new QAction(this);
	m_fileSeparatorAction1->setSeparator(true);

	m_fileExportAction = new QAction(tr("Export File..."), this);
	m_fileExportAction->setStatusTip(tr("Export file to disk"));
	m_fileExportAction->setIcon(QIcon{":/Images/Images/SchemaUpload.svg"});
	m_fileExportAction->setEnabled(true);
	connect(m_fileExportAction, &QAction::triggered, this, &EditSchemaWidget::getCurrentWorkcopy);

	m_fileImportAction = new QAction(tr("Import file..."), this);
	m_fileImportAction->setStatusTip(tr("Import file from disk to project"));
	m_fileImportAction->setIcon(QIcon{":/Images/Images/SchemaDownload.svg"});
	m_fileImportAction->setEnabled(false);
	connect(m_fileImportAction, &QAction::triggered, this, &EditSchemaWidget::setCurrentWorkcopy);

	m_fileSeparatorAction2 = new QAction(this);
	m_fileSeparatorAction2->setSeparator(true);

	m_filePropertiesAction = new QAction(tr("Properties..."), this);
	m_filePropertiesAction->setStatusTip(tr("Edit schema properties"));
	m_filePropertiesAction->setEnabled(true);
	connect(m_filePropertiesAction, &QAction::triggered, this, &EditSchemaWidget::schemaProperties);

	m_fileSeparatorAction3 = new QAction(this);
	m_fileSeparatorAction3->setSeparator(true);

	m_fileCloseAction = new QAction(tr("Close"), this);
	m_fileCloseAction->setStatusTip(tr("Close file"));
	m_fileCloseAction->setIcon(QIcon{":/Images/Images/CloseButtonBlack.svg"});
	m_fileCloseAction->setEnabled(true);
	m_fileCloseAction->setShortcuts(QKeySequence::Close);
	m_fileCloseAction->setShortcutVisibleInContextMenu(true);
	connect(m_fileCloseAction,
			&QAction::triggered,
			[this](bool)
			{
				emit closeTab(this);
			});
	addAction(m_fileCloseAction);

	//
	// Add Item
	//
	m_addLineAction = new QAction(tr("Line"), this);
	m_addLineAction->setEnabled(true);
	m_addLineAction->setIcon(QIcon(":/Images/Images/SchemaLine.svg"));
	m_addLineAction->setToolTip(tr("Static Line\nType: SchemaItemLine"));
	connect(m_addLineAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemLine>(schema()->unit()));
			});

	m_addPathAction = new QAction(tr("Path"), this);
	m_addPathAction->setEnabled(true);
	m_addPathAction->setIcon(QIcon(":/Images/Images/SchemaPath.svg"));
	m_addPathAction->setToolTip(tr("Static Path\nType: SchemaItemPath"));
	connect(m_addPathAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemPath>(schema()->unit()));
			});

	m_addRectAction = new QAction(tr("Rect"), this);
	m_addRectAction->setEnabled(true);
	m_addRectAction->setIcon(QIcon(":/Images/Images/SchemaRect.svg"));
	m_addRectAction->setToolTip(tr("Static Rect\nType: SchemaItemRect"));
	connect(m_addRectAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemRect>(schema()->unit()));
			});

	m_addTextAction = new QAction(tr("Text"), this);
	m_addTextAction->setEnabled(true);
	m_addTextAction->setIcon(QIcon(":/Images/Images/SchemaText.svg"));
	m_addTextAction->setToolTip(tr("Static Rect\nType: SchemaItemRect"));
	connect(m_addTextAction,
			&QAction::triggered,
			[this](bool)
			{
				auto text = std::make_shared<VFrame30::SchemaItemRect>(schema()->unit());
				text->setText(QLatin1String("Text"));
				text->setFill(false);
				text->setDrawRect(false);
				addItem(text);
			});

	m_addImageAction = new QAction(tr("Image"), this);
	m_addImageAction->setEnabled(true);
	m_addImageAction->setIcon(QIcon(":/Images/Images/SchemaItemImage.svg"));
	m_addImageAction->setToolTip(tr("Static Image\nType: SchemaItemImage"));
	connect(m_addImageAction,
			&QAction::triggered,
			[this](bool)
			{
				auto image = std::make_shared<VFrame30::SchemaItemImage>(schema()->unit());
				addItem(image);
			});

	//	m_addFrameAction = new QAction(tr("Frame"), this);
	//	m_addFrameAction->setEnabled(true);
	//	m_addFrameAction->setIcon(QIcon(":/Images/Images/SchemaItemFrame.svg"));
	// 	m_addFrameAction->setToolTip(tr("Static Frame\nType: SchemaItemFrame"));
	//	connect(m_addFrameAction, &QAction::triggered,
	//			[this](bool)
	//			{
	//				addItem(std::make_shared<VFrame30::SchemaItemFrame>(schema()->unit()));
	//			});

	// ----------------------------------------
	m_addLinkAction = new QAction(tr("Link"), this);
	m_addLinkAction->setEnabled(true);
	m_addLinkAction->setIcon(QIcon(":/Images/Images/SchemaLink.svg"));
	m_addLinkAction->setToolTip(tr("Link\nType: SchemaItemLink"));
	connect(m_addLinkAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemLink>(schema()->unit()));
			});

	m_addInputSignalAction = new QAction(tr("Input"), this);
	m_addInputSignalAction->setEnabled(true);
	m_addInputSignalAction->setIcon(QIcon(":/Images/Images/SchemaInputSignal.svg"));
	m_addInputSignalAction->setToolTip(tr("Input\nType: SchemaItemInput"));
	connect(m_addInputSignalAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemInput>(schema()->unit());
				addItem(item);
			});

	m_addInOutSignalAction = new QAction(tr("In/Out"), this);
	m_addInOutSignalAction->setEnabled(true);
	m_addInOutSignalAction->setIcon(QIcon(":/Images/Images/SchemaInOutSignal.svg"));
	m_addInOutSignalAction->setToolTip(tr("In/Out\nType: SchemaItemInOut"));
	connect(m_addInOutSignalAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemInOut>(schema()->unit());
				addItem(item);
			});

	m_addOutputSignalAction = new QAction(tr("Output"), this);
	m_addOutputSignalAction->setEnabled(true);
	m_addOutputSignalAction->setIcon(QIcon(":/Images/Images/SchemaOutputSignal.svg"));
	m_addOutputSignalAction->setToolTip(tr("Output\nType: SchemaItemOutput"));
	connect(m_addOutputSignalAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemOutput>(schema()->unit());
				addItem(item);
			});

	m_addConstantAction = new QAction(tr("Constant"), this);
	m_addConstantAction->setEnabled(true);
	m_addConstantAction->setIcon(QIcon(":/Images/Images/SchemaConstant.svg"));
	m_addConstantAction->setToolTip(tr("Constant\nType: SchemaItemConst"));
	connect(m_addConstantAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemConst>(schema()->unit()));
			});

	m_addTerminatorAction = new QAction(tr("Terminator"), this);
	m_addTerminatorAction->setEnabled(true);
	m_addTerminatorAction->setIcon(QIcon(":/Images/Images/SchemaTerminator.svg"));
	m_addTerminatorAction->setToolTip(tr("Terminator\nType: SchemaItemTerminator"));
	connect(m_addTerminatorAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemTerminator>(schema()->unit()));
			});

	// ----------------------------------------
	m_addAfbAction = new QAction(tr("App Functional Block"), this);
	m_addAfbAction->setEnabled(true);
	m_addAfbAction->setIcon(QIcon(":/Images/Images/SchemaFblElement.svg"));
	m_addAfbAction->setToolTip(tr("App Functional Block\nType: SchemaItemAfb"));
	connect(m_addAfbAction, &QAction::triggered, this, &EditSchemaWidget::addAfbElement);

	m_addUfbAction = new QAction(tr("User Functional Block"), this);
	m_addUfbAction->setEnabled(true);
	m_addUfbAction->setIcon(QIcon(":/Images/Images/SchemaUfbElement.svg"));
	m_addUfbAction->setToolTip(tr("User Functional Block\nType: SchemaItemUfb"));
	connect(m_addUfbAction, &QAction::triggered, this, &EditSchemaWidget::addUfbElement);

	m_addActuatorAction = new QAction(tr("Actuator"), this);
	m_addActuatorAction->setEnabled(true);
	m_addActuatorAction->setIcon(QIcon(":/Images/Images/SchemaActuatorElement.svg"));
	m_addActuatorAction->setToolTip(tr("Actuator\nType: SchemaItemActuator"));
	connect(m_addActuatorAction, &QAction::triggered, this, &EditSchemaWidget::addActuatorElement);

	// ----------------------------------------
	m_addConnection = new QAction(tr("Connection"), this);
	m_addConnection->setEnabled(true);
	m_addConnection->setIcon(QIcon(":/Images/Images/SchemaTransmitter.svg"));
	m_addConnection->setToolTip(tr("Connection\nType: SchemaItemTransmitter/SchemaItemReceiver"));
	connect(m_addConnection, &QAction::triggered, this, &EditSchemaWidget::addConnection);

	m_addTransmitter = new QAction(tr("Transmitter"), this);
	m_addTransmitter->setEnabled(true);
	m_addTransmitter->setIcon(QIcon(":/Images/Images/SchemaTransmitter.svg"));
	m_addTransmitter->setToolTip(tr("Transmitter\nType: SchemaItemTransmitter"));
	connect(m_addTransmitter, &QAction::triggered, this, &EditSchemaWidget::addTransmitter);

	m_addReceiver = new QAction(tr("Receiver"), this);
	m_addReceiver->setEnabled(true);
	m_addReceiver->setIcon(QIcon(":/Images/Images/SchemaReceiver.svg"));
	m_addReceiver->setToolTip(tr("Receiver\nType: SchemaItemReceiver"));
	connect(m_addReceiver, &QAction::triggered, this, &EditSchemaWidget::addReceiver);

	// ----------------------------------------
	m_addLoopback = new QAction(tr("Loopback"), this);
	m_addLoopback->setEnabled(true);
	m_addLoopback->setIcon(QIcon(":/Images/Images/SchemaLoopbackSource.svg"));
	m_addLoopback->setToolTip(tr("Loopback Source/Target\nType: SchemaItemLoopbackSource/SchemaItemLoopbackTarget"));
	connect(m_addLoopback, &QAction::triggered, this, &EditSchemaWidget::addLoopback);

	m_addLoopbackSource = new QAction(tr("Loopback Source"), this);
	m_addLoopbackSource->setEnabled(true);
	m_addLoopbackSource->setIcon(QIcon(":/Images/Images/SchemaLoopbackSource.svg"));
	m_addLoopbackSource->setToolTip(tr("Loopback Source\nType: SchemaItemLoopbackSource"));
	connect(m_addLoopbackSource, &QAction::triggered, this, &EditSchemaWidget::addLoopbackSource);

	m_addLoopbackTarget = new QAction(tr("Loopback Target"), this);
	m_addLoopbackTarget->setEnabled(true);
	m_addLoopbackTarget->setIcon(QIcon(":/Images/Images/SchemaLoopbackTarget.svg"));
	m_addLoopbackTarget->setToolTip(tr("Loopback Target\nType: SchemaItemLoopbackTarget"));
	connect(m_addLoopbackTarget, &QAction::triggered, this, &EditSchemaWidget::addLoopbackTarget);

	// ----------------------------------------

	m_addBus = new QAction(tr("Bus Composer/Extractor"), this);
	m_addBus->setEnabled(true);
	m_addBus->setIcon(QIcon(":/Images/Images/SchemaBusComposer.svg"));
	m_addBus->setToolTip(tr("Bus Composer/Extractor\nType: SchemaItemBusComposer/SchemaItemBusExtractor"));
	connect(m_addBus, &QAction::triggered, this, &EditSchemaWidget::addBus);

	m_addValueAction = new QAction(tr("Value"), this);
	m_addValueAction->setEnabled(true);
	m_addValueAction->setIcon(QIcon(":/Images/Images/SchemaItemValue.svg"));
	m_addValueAction->setToolTip(tr("Value\nType: SchemaItemValue"));
	connect(m_addValueAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemValue>(schema()->unit());
				addItem(item);
			});

	m_addImageValueAction = new QAction(tr("Image Value"), this);
	m_addImageValueAction->setEnabled(true);
	m_addImageValueAction->setIcon(QIcon(":/Images/Images/SchemaItemImageValue.svg"));
	m_addImageValueAction->setToolTip(tr("Image Value\nType: SchemaItemImageValue"));
	connect(m_addImageValueAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemImageValue>(schema()->unit());
				addItem(item);
			});

	m_addPushButtonAction = new QAction(tr("PushButton"), this);
	m_addPushButtonAction->setEnabled(true);
	m_addPushButtonAction->setIcon(QIcon(":/Images/Images/SchemaItemPushButton.svg"));
	m_addPushButtonAction->setToolTip(tr("PushButton\nType: SchemaItemPushButton"));
	connect(m_addPushButtonAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemPushButton>(schema()->unit());
				addItem(item);
			});

	m_addLineEditAction = new QAction(tr("LineEdit"), this);
	m_addLineEditAction->setEnabled(true);
	m_addLineEditAction->setIcon(QIcon(":/Images/Images/SchemaItemLineEdit.svg"));
	m_addLineEditAction->setToolTip(tr("LineEdit\nType: SchemaItemLineEdit"));
	connect(m_addLineEditAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemLineEdit>(schema()->unit());
				addItem(item);
			});

	m_addSliderAction = new QAction(tr("Slider"), this);
	m_addSliderAction->setEnabled(true);
	m_addSliderAction->setIcon(QIcon(":/Images/Images/SchemaItemSlider.svg"));
	m_addSliderAction->setToolTip(tr("Slider\nType: SchemaItemSlider"));
	connect(m_addSliderAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemSlider>(schema()->unit());
				addItem(item);
			});

	m_addIndicatorAction = new QAction(tr("Indicator"), this);
	m_addIndicatorAction->setEnabled(true);
	m_addIndicatorAction->setIcon(QIcon(":/Images/Images/SchemaItemIndicator.svg"));
	m_addIndicatorAction->setToolTip(tr("Indicator\nType: SchemaItemIndicator"));
	connect(m_addIndicatorAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemIndicator>(schema()->unit());
				addItem(item);
			});

	// ----------------------------------------

	m_addDiagSignalAction = new QAction(tr("DiagValue"), this);
	m_addDiagSignalAction->setEnabled(true);
	m_addDiagSignalAction->setIcon(QIcon(":/Images/Images/SchemaItemDiagValue.svg"));
	m_addDiagSignalAction->setToolTip(tr("DiagValue\nType: SchemaItemDiagValue"));
	connect(m_addDiagSignalAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemDiagValue>(schema()->unit());
				addItem(item);
			});

	// VDU items
	//
	m_addVduLineAction = new QAction(tr("VduLine"), this);
	m_addVduLineAction->setEnabled(true);
	m_addVduLineAction->setIcon(QIcon(":/Images/Images/SchemaLine.svg"));
	connect(m_addVduLineAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemVduLine>(schema()->unit()));
			});

	m_addVduRectAction = new QAction(tr("VduRect"), this);
	m_addVduRectAction->setEnabled(true);
	m_addVduRectAction->setIcon(QIcon(":/Images/Images/SchemaRect.svg"));
	connect(m_addVduRectAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemVduRect>(schema()->unit()));
			});

	m_addVduImageAction = new QAction(tr("VduImage"), this);
	m_addVduImageAction->setEnabled(true);
	m_addVduImageAction->setIcon(QIcon(":/Images/Images/SchemaItemImage.svg"));
	connect(m_addVduImageAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemVduImage>(schema()->unit()));
			});

	m_addVduValueAction = new QAction(tr("VduValue"), this);
	m_addVduValueAction->setEnabled(true);
	m_addVduValueAction->setIcon(QIcon(":/Images/Images/SchemaItemValue.svg"));
	connect(m_addVduValueAction,
			&QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemaItemVduValue>(schema()->unit()));
			});

	m_addVduImageValueAction = new QAction(tr("Image Value"), this);
	m_addVduImageValueAction->setEnabled(true);
	m_addVduImageValueAction->setIcon(QIcon(":/Images/Images/SchemaItemImageValue.svg"));
	m_addVduImageValueAction->setToolTip(tr("Image Value\nType: SchemaItemImageValue"));
	connect(m_addVduImageValueAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemVduImageValue>(schema()->unit());
				addItem(item);
			});

	m_addVduTrendAction = new QAction(tr("Trend"), this);
	m_addVduTrendAction->setEnabled(true);
	m_addVduTrendAction->setIcon(QIcon(":/Images/Images/SchemaItemTrend.svg"));
	connect(m_addVduTrendAction,
			&QAction::triggered,
			[this](bool)
			{
				auto item = std::make_shared<VFrame30::SchemaItemVduTrend>(schema()->unit());
				addItem(item);
			});

	//
	// Edit
	//

	// Edit->Undo
	//
	m_undoAction = new QAction(tr("Undo"), this);
	m_undoAction->setEnabled(false);
	m_undoAction->setShortcuts(QKeySequence::Undo);
	m_undoAction->setShortcutVisibleInContextMenu(true);
	connect(m_undoAction, &QAction::triggered, this, &EditSchemaWidget::undo);
	addAction(m_undoAction);

	// Edit->Redo
	//
	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setEnabled(false);
	m_redoAction->setShortcuts(QKeySequence::Redo);
	m_redoAction->setShortcutVisibleInContextMenu(true);
	connect(m_redoAction, &QAction::triggered, this, &EditSchemaWidget::redo);
	addAction(m_redoAction);

	// ------------------------------------
	//
	m_editSeparatorAction0 = new QAction(this);
	m_editSeparatorAction0->setSeparator(true);

	// Edit->Select All
	//
	m_selectAllAction = new QAction(tr("Select All"), this);
	m_selectAllAction->setEnabled(true);
	m_selectAllAction->setShortcuts(QKeySequence::SelectAll);
	m_selectAllAction->setShortcutVisibleInContextMenu(true);
	connect(m_selectAllAction, &QAction::triggered, this, &EditSchemaWidget::selectAll);
	addAction(m_selectAllAction);

	// ------------------------------------
	//
	m_editSeparatorAction1 = new QAction(this);
	m_editSeparatorAction1->setSeparator(true);

	// Edit->Cut
	//
	m_editCutAction = new QAction(tr("Cut"), this);
	m_editCutAction->setEnabled(false);
	m_editCutAction->setShortcuts(QKeySequence::Cut);
	m_editCutAction->setShortcutVisibleInContextMenu(true);
	connect(m_editCutAction, &QAction::triggered, this, &EditSchemaWidget::editCut);
	addAction(m_editCutAction);

	// Edit->Copy
	//
	m_editCopyAction = new QAction(tr("Copy"), this);
	m_editCopyAction->setEnabled(false);
	m_editCopyAction->setShortcuts(QKeySequence::Copy);
	m_editCopyAction->setShortcutVisibleInContextMenu(true);
	connect(m_editCopyAction, &QAction::triggered, this, &EditSchemaWidget::editCopy);
	addAction(m_editCopyAction);

	// Edit->Paste
	//
	m_editPasteAction = new QAction(tr("Paste"), this);
	m_editPasteAction->setEnabled(false);
	m_editPasteAction->setShortcuts(QKeySequence::Paste);
	m_editPasteAction->setShortcutVisibleInContextMenu(true);
	connect(m_editPasteAction, &QAction::triggered, this, &EditSchemaWidget::editPaste);
	addAction(m_editPasteAction);

	// ------------------------------------
	//
	m_editSeparatorAction2 = new QAction(this);
	m_editSeparatorAction2->setSeparator(true);

	// Edit->Delete
	//
	m_deleteAction = new QAction(tr("Delete"), this);
	m_deleteAction->setEnabled(false);
	m_deleteAction->setMenuRole(QAction::NoRole);
	m_deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
	m_deleteAction->setShortcutVisibleInContextMenu(true);
	connect(m_deleteAction, &QAction::triggered, this, &EditSchemaWidget::deleteKey);
	addAction(m_deleteAction);

	// ------------------------------------
	//
	m_editSeparatorAction3 = new QAction(this);
	m_editSeparatorAction3->setSeparator(true);

	// Edit->Properties
	//
	m_propertiesAction = new QAction(tr("Properties..."), this);
	m_propertiesAction->setEnabled(true);
	m_propertiesAction->setMenuRole(QAction::NoRole);
	m_propertiesAction->setShortcut(QKeySequence(tr("Alt+Return")));
	m_propertiesAction->setShortcutVisibleInContextMenu(true);
	// Shortcuts Alt+Return and Alt+Numeric Enter are different,
	// Look for real call of EditSchemaWidget::properties in keyPressEvent!
	//
	connect(m_propertiesAction, &QAction::triggered, this, &EditSchemaWidget::properties);
	addAction(m_propertiesAction);

	//
	// Size And Pos (align)
	//

	// Size/Pos->Same Width
	//
	m_sameWidthAction = new QAction(tr("Same Width"), this);
	m_sameWidthAction->setEnabled(false);
	m_sameWidthAction->setShortcut(Qt::ALT | Qt::Key_W);
	m_sameWidthAction->setShortcutVisibleInContextMenu(true);
	connect(m_sameWidthAction, &QAction::triggered, this, &EditSchemaWidget::sameWidth);
	addAction(m_sameWidthAction);

	// Size/Pos->Same Height
	//
	m_sameHeightAction = new QAction(tr("Same Height"), this);
	m_sameHeightAction->setEnabled(false);
	m_sameHeightAction->setShortcut(Qt::ALT | Qt::Key_H);
	m_sameHeightAction->setShortcutVisibleInContextMenu(true);
	connect(m_sameHeightAction, &QAction::triggered, this, &EditSchemaWidget::sameHeight);
	addAction(m_sameHeightAction);

	// Size/Pos->Same Size
	//
	m_sameSizeAction = new QAction(tr("Same Size"), this);
	m_sameSizeAction->setEnabled(false);
	connect(m_sameSizeAction, &QAction::triggered, this, &EditSchemaWidget::sameSize);
	addAction(m_sameSizeAction);

	// ------------------------------------
	//
	m_sizeAndPosSeparatorAction0 = new QAction(this);
	m_sizeAndPosSeparatorAction0->setSeparator(true);

	// Size/Pos->Align Left
	//
	m_alignLeftAction = new QAction(tr("Align Left"), this);
	m_alignLeftAction->setEnabled(false);
	connect(m_alignLeftAction, &QAction::triggered, this, &EditSchemaWidget::alignLeft);
	addAction(m_alignLeftAction);

	// Size/Pos->Align Right
	//
	m_alignRightAction = new QAction(tr("Align Right"), this);
	m_alignRightAction->setEnabled(false);
	connect(m_alignRightAction, &QAction::triggered, this, &EditSchemaWidget::alignRight);
	addAction(m_alignRightAction);

	// Size/Pos->Align Top
	//
	m_alignTopAction = new QAction(tr("Align Top"), this);
	m_alignTopAction->setEnabled(false);
	connect(m_alignTopAction, &QAction::triggered, this, &EditSchemaWidget::alignTop);
	addAction(m_alignTopAction);

	// Size/Pos->Align Bottom
	//
	m_alignBottomAction = new QAction(tr("Align Bottom"), this);
	m_alignBottomAction->setEnabled(false);
	connect(m_alignBottomAction, &QAction::triggered, this, &EditSchemaWidget::alignBottom);
	addAction(m_alignBottomAction);

	//
	// Items Order
	//

	// Items Order->Bring to Front
	//
	m_bringToFrontAction = new QAction(tr("Bring to Front"), this);
	m_bringToFrontAction->setEnabled(false);
	m_bringToFrontAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Home));
	m_bringToFrontAction->setShortcutVisibleInContextMenu(true);
	connect(m_bringToFrontAction, &QAction::triggered, this, &EditSchemaWidget::bringToFront);
	addAction(m_bringToFrontAction);

	// Items Order->Bring Forward
	//
	m_bringForwardAction = new QAction(tr("Bring Forward"), this);
	m_bringForwardAction->setEnabled(false);
	m_bringForwardAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
	m_bringForwardAction->setShortcutVisibleInContextMenu(true);
	connect(m_bringForwardAction, &QAction::triggered, this, &EditSchemaWidget::bringForward);
	addAction(m_bringForwardAction);

	// Items Order->Send to Back
	//
	m_sendToBackAction = new QAction(tr("Send to Back"), this);
	m_sendToBackAction->setEnabled(false);
	m_sendToBackAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_End));
	m_sendToBackAction->setShortcutVisibleInContextMenu(true);
	connect(m_sendToBackAction, &QAction::triggered, this, &EditSchemaWidget::sendToBack);
	addAction(m_sendToBackAction);

	// Items Order->Send Backward
	//
	m_sendBackwardAction = new QAction(tr("Send Backward"), this);
	m_sendBackwardAction->setEnabled(false);
	m_sendBackwardAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
	m_sendBackwardAction->setShortcutVisibleInContextMenu(true);
	connect(m_sendBackwardAction, &QAction::triggered, this, &EditSchemaWidget::sendBackward);
	addAction(m_sendBackwardAction);

	//
	// Transform Into
	//
	// m_transformAction = new QAction(tr("Transform Into"), this);
	// m_transformAction->setEnabled(true);

	// Transform Into->Input
	//
	m_transformIntoInputAction = new QAction(tr("Input"), this);
	m_transformIntoInputAction->setIcon(QIcon(":/Images/Images/SchemaInputSignal.svg"));
	connect(m_transformIntoInputAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoInput);

	// Transform Into->In/Out
	//
	m_transformIntoInOutAction = new QAction(tr("In/Out"), this);
	m_transformIntoInOutAction->setIcon(QIcon(":/Images/Images/SchemaInOutSignal.svg"));
	connect(m_transformIntoInOutAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoInOut);

	// Transform Into->Output
	//
	m_transformIntoOutputAction = new QAction(tr("Output"), this);
	m_transformIntoOutputAction->setIcon(QIcon(":/Images/Images/SchemaOutputSignal.svg"));
	connect(m_transformIntoOutputAction, &QAction::triggered, this, &EditSchemaWidget::transformIntoOutput);

	//
	// View
	//

	// View->ZoomIn, creating of these actions was moved to VFrame30::BaseSchemaWidget
	//
	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setIcon(QIcon{":/Images/Images/ZoomIn.svg"});
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	m_zoomInAction->setShortcutVisibleInContextMenu(true);
	connect(m_zoomInAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomIn);
	addAction(m_zoomInAction);

	// View->ZoomOut
	//
	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setIcon(QIcon{":/Images/Images/ZoomOut.svg"});
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	m_zoomOutAction->setShortcutVisibleInContextMenu(true);
	connect(m_zoomOutAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomOut);
	addAction(m_zoomOutAction);

	// View->Zoom100
	//
	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Asterisk));
	m_zoom100Action->setShortcutVisibleInContextMenu(true);
	connect(m_zoom100Action, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoom100);
	addAction(m_zoom100Action);

	// View->Fit to Screen
	//
	m_zoomFitToScreenAction = new QAction(tr("Fit to Screen"), this);
	m_zoomFitToScreenAction->setIcon(QIcon{":/Images/Images/ZoomFitToScreen.svg"});
	m_zoomFitToScreenAction->setEnabled(true);
	// m_zoomFitToScreenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Slash)); It is comment/uncommnet shortcut ((
	m_zoomFitToScreenAction->setShortcutVisibleInContextMenu(true);
	connect(m_zoomFitToScreenAction, &QAction::triggered, this, &VFrame30::BaseSchemaWidget::zoomToFit);
	addAction(m_zoomFitToScreenAction);

	// ------------------------------------
	//
	m_viewSeparatorAction0 = new QAction(this);
	m_viewSeparatorAction0->setSeparator(true);

	// View->SnapToGrid
	//
	m_snapToGridAction = new QAction(tr("Snap To Grid"), this);
	m_snapToGridAction->setEnabled(true);
	m_snapToGridAction->setCheckable(true);
	m_snapToGridAction->setChecked(snapToGrid());
	connect(m_snapToGridAction, &QAction::toggled, this, &EditSchemaWidget::snapToGridToggled);


	// High Level Menu
	//
	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);


	// Edit->Properties
	//
	m_layersAction = new QAction(tr("Layers..."), this);
	m_layersAction->setEnabled(true);
	// m_layersAction->setMenuRole(QAction::NoRole);
	// m_layersAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter));
	// m_layersAction->setShortcutVisibleInContextMenu(true);
	connect(m_layersAction, &QAction::triggered, this, &EditSchemaWidget::layers);
	addAction(m_layersAction);

	m_compareDiffAction = new QAction(tr("Item Diffs..."), this);
	m_compareDiffAction->setEnabled(true);
	connect(m_compareDiffAction, &QAction::triggered, this, &EditSchemaWidget::compareSchemaItem);
	// addAction(m_compareDiffAction);

	// Comment
	//
	m_toggleCommentAction = new QAction(tr("Comment/Uncomment"), this);
	m_toggleCommentAction->setEnabled(false);
	m_toggleCommentAction->setShortcut(Qt::CTRL | Qt::Key_Slash);
	m_toggleCommentAction->setShortcutVisibleInContextMenu(true);
	connect(m_toggleCommentAction, &QAction::triggered, this, &EditSchemaWidget::toggleComment);
	addAction(m_toggleCommentAction);

	// Lock/Unlock
	//
	m_lockAction = new QAction(tr("Lock/Unlock"), this);
	m_lockAction->setEnabled(false);
	m_lockAction->setShortcut(Qt::CTRL | Qt::Key_L);
	m_lockAction->setShortcutVisibleInContextMenu(true);
	connect(m_lockAction, &QAction::triggered, this, &EditSchemaWidget::toggleLock);
	addAction(m_lockAction);

	// Find
	//
	m_findAction = new QAction(tr("Find..."), this);
	m_findAction->setEnabled(true);
	m_findAction->setShortcut(QKeySequence::Find);
	m_findAction->setShortcutVisibleInContextMenu(true);
	connect(m_findAction, &QAction::triggered, this, &EditSchemaWidget::find);
	addAction(m_findAction);

	m_findNextAction = new QAction(tr("Find Next"), this);
	m_findNextAction->setEnabled(true);
	m_findNextAction->setShortcut(QKeySequence::FindNext);
	m_findNextAction->setShortcutVisibleInContextMenu(true);
	connect(m_findNextAction,
			&QAction::triggered,
			this,
			[this]()
			{
				findNext(theSettings.m_findSchemaItemCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			});
	addAction(m_findNextAction);

	m_findPrevAction = new QAction(tr("Find Previous"), this);
	m_findPrevAction->setEnabled(true);
	m_findPrevAction->setShortcut(QKeySequence::FindPrevious);
	m_findPrevAction->setShortcutVisibleInContextMenu(true);
	connect(m_findPrevAction,
			&QAction::triggered,
			this,
			[this]()
			{
				findPrev(theSettings.m_findSchemaItemCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			});

	addAction(m_findPrevAction);

	// Other
	//
	m_addAppSignalAction = new QAction(tr("AddAppSignal"), this);
	m_addAppSignalAction->setShortcut(Qt::ALT | Qt::Key_N);
	m_addAppSignalAction->setShortcutVisibleInContextMenu(true);
	m_addAppSignalAction->setEnabled(false);
	connect(m_addAppSignalAction, &QAction::triggered, this, &EditSchemaWidget::addNewAppSignalSelected);
	addAction(m_addAppSignalAction);

	m_appSignalPropertiesAction = new QAction(tr("AppSignalProperties"), this);
	m_appSignalPropertiesAction->setShortcut(Qt::ALT | Qt::Key_S);
	m_appSignalPropertiesAction->setShortcutVisibleInContextMenu(true);
	m_appSignalPropertiesAction->setEnabled(false);
	connect(m_appSignalPropertiesAction, &QAction::triggered, this, &EditSchemaWidget::appSignalsSelectedProperties);
	addAction(m_appSignalPropertiesAction);

	//
	// Create Sub Menus
	//
	m_fileSubMenu = new QMenu(tr("File"), this);

	m_fileSubMenu->addAction(m_detachWindow);
	m_fileSubMenu->addSeparator();
	m_fileSubMenu->addAction(m_fileCheckOutAction);
	m_fileSubMenu->addAction(m_fileCheckInAction);
	m_fileSubMenu->addAction(m_fileUndoChangesAction);
	m_fileSubMenu->addAction(m_fileSeparatorAction0);
	m_fileSubMenu->addAction(m_fileSaveAction);
	m_fileSubMenu->addAction(m_fileExportToPdfAction);
	m_fileSubMenu->addAction(m_fileSeparatorAction1);
	m_fileSubMenu->addAction(m_fileExportAction);
	m_fileSubMenu->addAction(m_fileImportAction);
	m_fileSubMenu->addAction(m_fileSeparatorAction2);
	m_fileSubMenu->addAction(m_filePropertiesAction);
	m_fileSubMenu->addAction(m_fileSeparatorAction3);
	m_fileSubMenu->addAction(m_fileCloseAction);

	m_addSubMenu = new QMenu(tr("Add Item"), this);

	if (schema()->isLogicSchema() == true)
	{
		fillActionsForLogicSchema(m_addSubMenu);
	}

	if (schema()->isUfbSchema() == true)
	{
		fillActionsForUfbSchema(m_addSubMenu);
	}

	if (schema()->isMonitorSchema() == true)
	{
		fillActionsForMonitorSchema(m_addSubMenu);
	}

	if (schema()->isTuningSchema() == true)
	{
		fillActionsForTuningSchema(m_addSubMenu);
	}

	if (schema()->isDiagSchema() == true)
	{
		fillActionsForDiagSchema(m_addSubMenu);
	}

	if (schema()->isVduSchema() == true)
	{
		fillActionsForVduSchema(m_addSubMenu);
	}

	if (schema()->isActuatorSchema() == true)
	{
		fillActionsForActuatorSchema(m_addSubMenu);
	}

	m_editSubMenu = new QMenu(tr("Edit"), this);
	m_editSubMenu->addAction(m_undoAction);
	m_editSubMenu->addAction(m_redoAction);
	m_editSubMenu->addAction(m_editSeparatorAction0);
	m_editSubMenu->addAction(m_selectAllAction);
	m_editSubMenu->addAction(m_editSeparatorAction1);
	m_editSubMenu->addAction(m_editCutAction);
	m_editSubMenu->addAction(m_editCopyAction);
	m_editSubMenu->addAction(m_editPasteAction);
	m_editSubMenu->addAction(m_editSeparatorAction2);
	m_editSubMenu->addAction(m_deleteAction);

	m_alignSubMenu = new QMenu(tr("Align"), this);
	m_alignSubMenu->addAction(m_sameWidthAction);
	m_alignSubMenu->addAction(m_sameHeightAction);
	m_alignSubMenu->addAction(m_sameSizeAction);
	m_alignSubMenu->addAction(m_sizeAndPosSeparatorAction0);
	m_alignSubMenu->addAction(m_alignLeftAction);
	m_alignSubMenu->addAction(m_alignRightAction);
	m_alignSubMenu->addAction(m_alignTopAction);
	m_alignSubMenu->addAction(m_alignBottomAction);

	m_orderSubMenu = new QMenu(tr("Order"), this);
	m_orderSubMenu->addAction(m_bringToFrontAction);
	m_orderSubMenu->addAction(m_bringForwardAction);
	m_orderSubMenu->addAction(m_sendBackwardAction);
	m_orderSubMenu->addAction(m_sendToBackAction);

	m_transformSubMenu = new QMenu("Transform Into ", this);
	m_transformSubMenu->addAction(m_transformIntoInputAction);
	m_transformSubMenu->addAction(m_transformIntoInOutAction);
	m_transformSubMenu->addAction(m_transformIntoOutputAction);

	m_viewSubMenu = new QMenu(tr("View"), this);
	m_viewSubMenu->addAction(m_zoomInAction);
	m_viewSubMenu->addAction(m_zoomOutAction);
	m_viewSubMenu->addAction(m_zoom100Action);
	m_viewSubMenu->addAction(m_zoomFitToScreenAction);

	if (schema()->unit() == SchemaUnit::Display)
	{
		m_viewSubMenu->addAction(m_viewSeparatorAction0);
		m_viewSubMenu->addAction(m_snapToGridAction);
	}

	return;
}

void EditSchemaWidget::updateFileActions()
{
	// Version Control enable/disable items
	//
	m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);
	m_fileCheckInAction->setEnabled(readOnly() == false && fileInfo().state() == E::VcsState::CheckedOut);
	m_fileCheckOutAction->setEnabled(readOnly() == true && fileInfo().state() == E::VcsState::CheckedIn);
	m_fileUndoChangesAction->setEnabled(readOnly() == false && fileInfo().state() == E::VcsState::CheckedOut);
	m_fileExportAction->setEnabled(true);
	m_fileImportAction->setEnabled(readOnly() == false && fileInfo().state() == E::VcsState::CheckedOut);

	return;
}

void EditSchemaWidget::fillActionsForLogicSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addLinkAction);
	widget->addAction(m_addInputSignalAction);
	widget->addAction(m_addInOutSignalAction);
	widget->addAction(m_addOutputSignalAction);
	widget->addAction(m_addConstantAction);
	widget->addAction(m_addTerminatorAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addAfbAction);
	widget->addAction(m_addUfbAction);
	widget->addAction(m_addActuatorAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addConnection);
	widget->addAction(m_addLoopback);
	widget->addAction(m_addBus);

	return;
}

void EditSchemaWidget::fillActionsForUfbSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addLinkAction);
	widget->addAction(m_addInputSignalAction);
	widget->addAction(m_addOutputSignalAction);
	widget->addAction(m_addConstantAction);
	widget->addAction(m_addTerminatorAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addAfbAction);
	widget->addAction(m_addActuatorAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addLoopback);
	widget->addAction(m_addBus);

	return;
}

void EditSchemaWidget::fillActionsForMonitorSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addValueAction);
	widget->addAction(m_addImageValueAction);
	widget->addAction(m_addPushButtonAction);
	widget->addAction(m_addLineEditAction);
	widget->addAction(m_addSliderAction);
	widget->addAction(m_addIndicatorAction);

	return;
}

void EditSchemaWidget::fillActionsForTuningSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addValueAction);
	widget->addAction(m_addImageValueAction);
	widget->addAction(m_addPushButtonAction);
	widget->addAction(m_addLineEditAction);
	widget->addAction(m_addSliderAction);

	return;
}

void EditSchemaWidget::fillActionsForDiagSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addDiagSignalAction);

	return;
}

void EditSchemaWidget::fillActionsForVduSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addVduLineAction);
	widget->addAction(m_addVduRectAction);
	widget->addAction(m_addVduImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addVduValueAction);
	widget->addAction(m_addVduImageValueAction);
	widget->addAction(m_addVduTrendAction);

	return;
}

void EditSchemaWidget::fillActionsForActuatorSchema(QWidget* widget)
{
	assert(widget);

	widget->addAction(m_addLineAction);
	widget->addAction(m_addRectAction);
	widget->addAction(m_addPathAction);
	widget->addAction(m_addTextAction);
	widget->addAction(m_addImageAction);

	auto separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addLinkAction);
	widget->addAction(m_addInputSignalAction);
	widget->addAction(m_addInOutSignalAction);
	widget->addAction(m_addOutputSignalAction);
	widget->addAction(m_addConstantAction);
	widget->addAction(m_addTerminatorAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addAfbAction);
	widget->addAction(m_addUfbAction);

	separator = new QAction(widget);
	separator->setSeparator(true);
	widget->addAction(separator);

	widget->addAction(m_addLoopback);
	widget->addAction(m_addBus);

	return;
}


bool EditSchemaWidget::event(QEvent* event)
{
	// Show tool tip
	//
	if (event->type() == QEvent::ToolTip)
	{
		QHelpEvent* he = static_cast<QHelpEvent*>(event);

		// Get item under cursor
		//
		QPointF docPoint = widgetPointToDocument(he->pos(), false);
		SchemaItemPtr itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr)
		{
			QString toolTip = itemUnderPoint->toolTipText(this->logicalDpiX(), this->logicalDpiY(), this->devicePixelRatioF());
			setToolTip(toolTip);
		}
		else
		{
			setToolTip("");
		}

		return VFrame30::BaseSchemaWidget::event(event);
	}

	return VFrame30::BaseSchemaWidget::event(event);
}

void EditSchemaWidget::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
	case Qt::Key_Left:
		onLeftKey(e);
		return;
	case Qt::Key_Right:
		onRightKey(e);
		return;
	case Qt::Key_Up:
		onUpKey(e);
		return;
	case Qt::Key_Down:
		onDownKey(e);
		return;
	}

	// Show properties dialog
	//
	if ((e->modifiers().testFlag(Qt::AltModifier) == true && // Alt + numeric keypad Enter
		 e->modifiers().testFlag(Qt::KeypadModifier) == true && e->key() == Qt::Key_Enter) ||
		(e->modifiers().testFlag(Qt::AltModifier) == true && // Alt + Enter
		 e->key() == Qt::Key_Return))
	{
		properties();
	}

	// This will update if Moving item in progress and we try to move connection links
	//
	if (editSchemaView()->m_editConnectionLines.empty() == false)
	{
		if (e->key() == Qt::Key_Space)
		{
			editSchemaView()->m_doNotMoveConnectionLines = !editSchemaView()->m_doNotMoveConnectionLines;
		}

		bool ctrlIsPressed = e->modifiers() & Qt::ControlModifier;

		if (ctrlIsPressed != m_ctrlWasPressed || e->key() == Qt::Key_Space)
		{
			m_ctrlWasPressed = ctrlIsPressed;

			editSchemaView()->update();
		}

		setFocus(); // As alt could be pressed and MainMenu activated
		e->ignore();
		return;
	}

	BaseSchemaWidget::keyPressEvent(e);

	return;
}

void EditSchemaWidget::keyReleaseEvent(QKeyEvent* event)
{
	// This will update if Moving item in progress and we try to move connection links
	//
	if (editSchemaView()->m_editConnectionLines.empty() == false)
	{
		bool ctrlIsPressed = event->modifiers() & Qt::ControlModifier;

		if (ctrlIsPressed != m_ctrlWasPressed)
		{
			m_ctrlWasPressed = ctrlIsPressed;
			editSchemaView()->update();
		}

		setFocus(); // As alt could be pressed and MainMenu activated
		event->accept();
		return;
	}

	BaseSchemaWidget::keyReleaseEvent(event);

	return;
}


// Set corresponding to the current situation and user actions context menu
//
void EditSchemaWidget::setCorrespondingContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	return;
}

void EditSchemaWidget::mousePressEvent(QMouseEvent* event)
{
	grabKeyboard();

	BaseSchemaWidget::mousePressEvent(event);

	if (event->isAccepted() == true)
	{
		return;
	}

	if (event->button() == Qt::LeftButton)
	{
		for (auto msa = m_mouseLeftDownStateAction.begin(); msa != m_mouseLeftDownStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		setMouseCursor(event->pos());

		event->accept();
		return;
	}

	if (event->button() == Qt::RightButton)
	{
		for (auto msa = m_mouseRightDownStateAction.begin(); msa != m_mouseRightDownStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		if (mouseState() != MouseState::None)
		{
			event->accept();
			resetAction();
		}
		else
		{
			event->ignore();
		}

		return;
	}

	event->ignore();
	return;
}

void EditSchemaWidget::mouseReleaseEvent(QMouseEvent* event)
{
	releaseKeyboard();

	if (event->button() == Qt::LeftButton)
	{
		for (auto msa = m_mouseLeftUpStateAction.begin(); msa != m_mouseLeftUpStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		setMouseCursor(event->pos());

		event->accept();
		return;
	}

	if (event->button() == Qt::RightButton)
	{
		for (auto msa = m_mouseRightUpStateAction.begin(); msa != m_mouseRightUpStateAction.end(); ++msa)
		{
			if (msa->mouseState == mouseState())
			{
				msa->action(event);
				setMouseCursor(event->pos());

				event->accept();
				return;
			}
		}

		// DefaultAction
		//
		// setMouseCursor(event->pos());

		// event->accept();
		// return;
	}


	setMouseCursor(event->pos());

	// unsetCursor();
	event->ignore();
}

void EditSchemaWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (mouseState() == MouseState::MovingConnectionLinePoint || mouseState() == MouseState::AddSchemaPosConnectionNextPoint)
	{
		// It accidental double clicking, ignore it
		//
		event->ignore();
		return;
	}

	setMouseState(MouseState::None);

	if (event->button() == Qt::LeftButton && selectedItems().empty() == false)
	{
		properties();
	}

	event->accept();
	return;
}

void EditSchemaWidget::mouseMoveEvent(QMouseEvent* event)
{
	BaseSchemaWidget::mouseMoveEvent(event);
	if (event->isAccepted() == true)
	{
		return;
	}

	for (const MouseStateAction& msa : m_mouseMoveStateAction)
	{
		if (msa.mouseState == mouseState())
		{
			msa.action(event);

			setMouseCursor(event->pos());
			event->accept();
			return;
		}
	}

	// Default Action
	//
	setMouseCursor(event->pos());

	event->ignore();
	return;
}

void EditSchemaWidget::mouseLeftDown_None(QMouseEvent* me)
{
	bool ctrlIsPressed = me->modifiers().testFlag(Qt::ControlModifier);
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	if (shiftIsPressed == false)
	{
		QPointF docPoint = widgetPointToDocument(me->pos(), false);

		if (selectedItems().size() == 1)
		{
			int movingEdgePointIndex = 0;
			auto selectedItem = selectedItems()[0];

			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

			if (dynamic_cast<VFrame30::IPosRect*>(selectedItem.get()) != nullptr)
			{
				auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor),
											   std::end(m_sizeActionToMouseCursor),
											   [&possibleAction](const SizeActionToMouseCursor& item)
											   {
												   return item.action == possibleAction;
											   });

				if (findResult != std::end(m_sizeActionToMouseCursor))
				{
					auto itemPosRotatable = dynamic_cast<const VFrame30::PosRectRotatable*>(selectedItem.get());
					bool rotated = itemPosRotatable != nullptr && itemPosRotatable->angle() != 0;

					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					if (rotated == true)
					{
						auto rotatePoint = itemPosRotatable->rotationPointInDocPt();

						QTransform transform;
						transform.translate(rotatePoint.x(), rotatePoint.y());
						transform.rotate(-itemPosRotatable->angle());
						transform.translate(-rotatePoint.x(), -rotatePoint.y());

						docPoint = transform.map(docPoint);
					}

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(findResult->mouseState);

					setMouseCursor(me->pos());
					editSchemaView()->update();
					return;
				}
			}

			if (dynamic_cast<VFrame30::IPosLine*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == SchemaItemAction::MoveStartLinePoint)
				{
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingStartLinePoint);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveEndLinePoint)
				{
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemaView()->m_editStartDocPt = docPoint;
					editSchemaView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingEndLinePoint);

					setMouseCursor(me->pos());
					editSchemaView()->update();

					return;
				}
			}

			if (dynamic_cast<VFrame30::IPosConnection*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == SchemaItemAction::MoveHorizontalEdge)
				{
					assert(movingEdgePointIndex != -1);

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditEdge);

					ecl.setEditEdgeIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

					setMouseState(MouseState::MovingHorizontalEdge);
					setMouseCursor(me->pos());

					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveVerticalEdge)
				{
					assert(movingEdgePointIndex != -1);

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditEdge);

					ecl.setEditEdgeIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

					setMouseState(MouseState::MovingVerticalEdge);
					setMouseCursor(me->pos());

					editSchemaView()->update();

					return;
				}

				if (possibleAction == SchemaItemAction::MoveConnectionLinePoint)
				{
					assert(movingEdgePointIndex != -1);

					EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem),
										   EditConnectionLine::EditMode::EditPoint);

					ecl.setEditPointIndex(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(selectedItem), movingEdgePointIndex);

					editSchemaView()->m_editConnectionLines.clear();
					editSchemaView()->m_editConnectionLines.push_back(ecl);

					setMouseState(MouseState::MovingConnectionLinePoint);
					setMouseCursor(me->pos());

					editSchemaView()->update();

					return;
				}
			}
		}

		// Create proposed connection for FblItem pins.
		//
		for (auto lines = editSchemaView()->m_autoFblItemConnection.getPropositions(); const auto& line : lines)
		{
			if (line.addButtonRect.contains(docPoint) == true)
			{
				// Save selection for mo convenient control.
				// We don't want to select just created link, we want to keep selection so we can create other
				// links in the next mouse click.
				//
				auto selection = selectedItems();

				createProposedAfbLink(std::vector{line});

				editSchemaView()->setSelectedItems(selection);
				editSchemaView()->update();
				return;
			}
		}

		// --
		//
		for (auto si : selectedItems())
		{
			int movingEdgePointIndex = 0;
			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(si.get(), docPoint, &movingEdgePointIndex);

			if (possibleAction == SchemaItemAction::MoveItem && editSchemaView()->selectedItems().size() > 1 &&
				si->isType<VFrame30::IPosConnection>() == true)
			{
				// This SchemaItemAction::MoveItem is for @MOveBar@ of IPosConnection, exclude it for MULTIPLE selection
				//
				possibleAction = SchemaItemAction::NoAction;
			}

			if (possibleAction == SchemaItemAction::MoveItem || possibleAction == SchemaItemAction::MoveConnectionLinePoint ||
				possibleAction == SchemaItemAction::MoveHorizontalEdge || possibleAction == SchemaItemAction::MoveVerticalEdge)
			{
				docPoint = widgetPointToDocument(me->pos(), snapToGrid());

				editSchemaView()->m_editStartDocPt = docPoint;
				editSchemaView()->m_editEndDocPt = docPoint;

				setMouseState(MouseState::Moving);

				setMouseCursor(me->pos());
				editSchemaView()->update();
				return;
			}
		}

		// If mouse is over pin, start drawing SchemaItemLink.
		//
		{
			std::vector<VFrame30::AfbPin> itemPins;
			itemPins.reserve(64);

			double gridSize = schema()->gridSize();
			double pinGridStep = static_cast<double>(schema()->pinGridStep());

			for (const SchemaItemPtr& item : activeLayer()->items())
			{
				VFrame30::FblItemRect* fbRect = dynamic_cast<VFrame30::FblItemRect*>(item.get());
				VFrame30::SchemaItemLink* link = dynamic_cast<VFrame30::SchemaItemLink*>(item.get());

				if (fbRect != nullptr &&
					std::find(selectedItems().begin(), selectedItems().end(), item) ==
						selectedItems().end()) // Item is not selected, as in this case it can be resized or moved by control bars
				{
					const std::vector<VFrame30::AfbPin>& inputs = fbRect->inputs();
					const std::vector<VFrame30::AfbPin>& outputs = fbRect->outputs();

					itemPins.clear();
					itemPins.insert(itemPins.end(), inputs.begin(), inputs.end());
					itemPins.insert(itemPins.end(), outputs.begin(), outputs.end());

					for (const VFrame30::AfbPin& pin : itemPins)
					{
						QRectF pinArea = {pin.x() - gridSize * pinGridStep / 2,
										  pin.y() - gridSize * pinGridStep / 2,
										  gridSize * pinGridStep,
										  gridSize * pinGridStep};

						if (pinArea.contains(docPoint) == true)
						{
							addItem(std::make_shared<VFrame30::SchemaItemLink>(schema()->unit()));

							mouseLeftDown_AddSchemaPosConnectionStartPoint(me);

							return;
						}
					}

					continue;
				}

				if (link != nullptr)
				{
					continue;
				}
			}
		}

		// Start moving.
		//
		auto itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr &&
			(itemUnderPoint->isLocked() == false || (itemUnderPoint->isLocked() == true && ctrlIsPressed == true)))
		{
			if (std::find(selectedItems().begin(), selectedItems().end(), itemUnderPoint) != selectedItems().end()) {}
			else
			{
				editSchemaView()->clearSelection();
				editSchemaView()->setSelectedItem(itemUnderPoint);
			}

			docPoint = widgetPointToDocument(me->pos(), snapToGrid());

			editSchemaView()->m_editStartDocPt = docPoint;
			editSchemaView()->m_editEndDocPt = docPoint;

			setMouseState(MouseState::Moving);

			setMouseCursor(me->pos());
			editSchemaView()->update();
			return;
		}
	}

	if (shiftIsPressed == false)
	{
		editSchemaView()->clearSelection();
	}

	// Selection item or area
	//
	editSchemaView()->m_mouseSelectionStartPoint = widgetPointToDocument(me->pos(), false);
	editSchemaView()->m_mouseSelectionEndPoint = editSchemaView()->m_mouseSelectionStartPoint;

	editSchemaView()->m_mouseSelectionStartPointForUpdate = me->pos();
	editSchemaView()->m_mouseSelectionEndPointForUpdate = editSchemaView()->m_mouseSelectionStartPointForUpdate;

	setMouseState(MouseState::Selection);

	editSchemaView()->update();

	setMouseCursor(me->pos());

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosLineStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		resetAction();
		return;
	}

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setStartXDocPt(docPoint.x());
	itemPos->setStartYDocPt(docPoint.y());

	setMouseState(MouseState::AddSchemaPosLineEndPoint);

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosRectStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setLeftDocPt(docPoint.x());
	itemPos->setTopDocPt(docPoint.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	itemPos->setWidthDocPt(minWidth);
	itemPos->setHeightDocPt(minHeight);

	editSchemaView()->m_addRectStartPoint = docPoint;
	editSchemaView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemaView()->m_addRectEndPoint.setX(itemPos->topDocPt() + itemPos->heightDocPt());

	setMouseState(MouseState::AddSchemaPosRectEndPoint);

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseLeftDown_AddSchemaPosConnectionStartPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// magnet point to pin
	//
	docPoint = magnetPointToPin(docPoint);

	EditConnectionLine ecl(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(editSchemaView()->m_newItem),
						   EditConnectionLine::AddToEnd);

	ecl.addBasePoint(docPoint);
	ecl.addExtensionPoint(docPoint);

	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_editConnectionLines.push_back(ecl);

	setMouseState(MouseState::AddSchemaPosConnectionNextPoint);

	return;
}

void EditSchemaWidget::mouseLeftUp_Selection(QMouseEvent* me)
{
	m_nextSelectionFromLeft = {}; // Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	// ���������� ����� �������� ������ � ���������� �������,
	// �������� �� � selectedItem
	//
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	// ����� ���� ����� ��� ���� ��� �����, �� � �������� ��������� ��� ��������,
	// ������� � ��������� ��� �������� �� ���������
	//
	if (shiftIsPressed == false)
	{
		editSchemaView()->clearSelection();
	}

	editSchemaView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);

	// ��������� ���������� ��������� ��� �����
	//
	QRectF pageSelectionArea =
		QRectF(editSchemaView()->m_mouseSelectionStartPoint, editSchemaView()->m_mouseSelectionEndPoint).normalized();

	// ����� ��������� ������ pageSelectionArea
	//
	auto activeLayer = editSchemaView()->activeLayer();

	// ���� ���� ��� �������� �����, �� ��������� �������� ������ �������� ��������
	//
	if (editSchemaView()->m_mouseSelectionStartPoint == editSchemaView()->m_mouseSelectionEndPoint)
	{
		auto item = activeLayer->getItemUnderPoint(pageSelectionArea.topLeft());

		if (item != nullptr)
		{
			// ���� ����� ������� ��� ���� � ������, �� ������� ��� �� ������ ����������
			//
			bool wasDeleted = editSchemaView()->removeFromSelection(item);

			if (wasDeleted == false)
			{
				// This item was not selected, so just select it
				//
				editSchemaView()->addSelection(item);
			}
		}
	}
	else
	{
		// ������������� ������ ��������� , ������������ (��� �������������)
		// ��� �������� ��������� � �������������
		//
		auto itemsInRect = activeLayer->getItemListInRectangle(pageSelectionArea);

		for (auto item = itemsInRect.begin(); item != itemsInRect.end(); ++item)
		{
			// ���� ����� ������� ��� ���� � ������, �� ������� ��� �� ������ ����������
			//
			auto findResult = std::find(selectedItems().begin(), selectedItems().end(), *item);

			if (findResult != selectedItems().end())
			{
				editSchemaView()->removeFromSelection(*item, false); // do emit SectionChanged manually, after the loop
			}
			else
			{
				editSchemaView()->addSelection(*item, false);        // do emit SectionChanged manually, after the loopB
			}
		}

		emit selectionChanged();
	}

	// --
	//
	editSchemaView()->m_mouseSelectionStartPoint = QPoint();
	editSchemaView()->m_mouseSelectionEndPoint = QPoint();

	editSchemaView()->m_mouseSelectionStartPointForUpdate = QPoint();
	editSchemaView()->m_mouseSelectionEndPointForUpdate = QPoint();

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_Moving(QMouseEvent* event)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() != true);
		return;
	}

	const auto& selected = selectedItems();

	QPointF mouseMovingStartPointIn = editSchemaView()->m_editStartDocPt;
	QPointF mouseMovingEndPointIn = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_editEndDocPt = mouseMovingEndPointIn;

	// If Alt is pressed then moving in one dirrection horz or vert
	//
	if (bool altIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier); altIsPressed)
	{
		QPointF offset = mouseMovingEndPointIn - editSchemaView()->m_editStartDocPt;

		if (std::abs(offset.rx()) > std::abs(offset.ry()))
		{
			mouseMovingEndPointIn.setY(editSchemaView()->m_editStartDocPt.ry());
		}
		else
		{
			mouseMovingEndPointIn.setX(editSchemaView()->m_editStartDocPt.rx());
		}
	}

	// --
	//
	double xdif = mouseMovingEndPointIn.x() - mouseMovingStartPointIn.x();
	double ydif = mouseMovingEndPointIn.y() - mouseMovingStartPointIn.y();

	if (std::abs(xdif) < 0.0000001 && std::abs(ydif) < 0.0000001)
	{
		// SchemaItem's have not changed positions
		//
		resetAction();
		return;
	}

	if (bool ctrlIsPressed = event->modifiers().testFlag(Qt::ControlModifier); ctrlIsPressed == false)
	{
		// Move items
		//
		std::vector<SchemaItemPtr> itemsForMove;
		itemsForMove.reserve(selected.size());

		for (auto& item : selected)
		{
			if (item->isLocked() == false)
			{
				itemsForMove.push_back(item);
			}
		}

		if (itemsForMove.empty() == false)
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runMoveItem(xdif, ydif, itemsForMove, snapToGrid());
				finishMoveAfbsConnectionLinks();

				m_editEngine->endBatch();
			}
		}
	}
	else
	{
		// Copy SchemaItems and move copied items
		//
		std::vector<SchemaItemPtr> newItems;

		DbController* dbc = db();
		auto s = schema();

		std::for_each(selected.begin(),
					  selected.end(),
					  [xdif, ydif, &newItems, dbc, s](const SchemaItemPtr& si)
					  {
						  QByteArray data;
						  bool result = si->saveToByteArray(&data);

						  if (result == false || data.isEmpty() == true)
						  {
							  assert(result == true);
							  assert(data.isEmpty() == false);
							  return;
						  }

						  SchemaItemPtr newItem = VFrame30::SchemaItem::Create(data);

						  if (newItem == nullptr)
						  {
							  assert(newItem != nullptr);
							  return;
						  }

						  newItem->setNewGuid();

						  {
							  int counterValue = 0;
							  bool nextValRes = dbc->nextCounterValue(&counterValue);
							  if (nextValRes == false)
							  {
								  return;
							  }

							  newItem->setLabel(s->schemaId() + "_" + QString::number(counterValue));
						  }

						  newItem->moveItem(xdif, ydif);

						  newItems.push_back(newItem);
						  return;
					  });

		m_editEngine->runAddItem(newItems, editSchemaView()->activeLayer());

		// Apply default values, and undo, so user can choose between a clear copy or a copy with applied default values.
		//
		bool defaultsWereSet = setDefaultItemProperties(newItems);
		if (defaultsWereSet == true)
		{
			// Undo one time that user can select what to take, a clear item copy or modified with default properties.
			//
			undo();
		}
	}

	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_SizingRect(QMouseEvent* event)
{
	if (mouseState() != MouseState::SizingTopLeft && mouseState() != MouseState::SizingTop && mouseState() != MouseState::SizingTopRight &&
		mouseState() != MouseState::SizingRight && mouseState() != MouseState::SizingBottomRight &&
		mouseState() != MouseState::SizingBottom && mouseState() != MouseState::SizingBottomLeft && mouseState() != MouseState::SizingLeft)
	{
		return;
	}

	if (editSchemaView()->m_editStartDocPt.isNull() == true || editSchemaView()->m_editEndDocPt.isNull() == true)
	{
		assert(editSchemaView()->m_editStartDocPt.isNull() == false);
		assert(editSchemaView()->m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();
	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	QPointF mouseSizingStartPointDocPt = editSchemaView()->m_editStartDocPt;
	QPointF mouseSizingEndPointDocPt = widgetPointToDocument(event->pos(), snapToGrid());

	auto itemPosRotatable = dynamic_cast<const VFrame30::PosRectRotatable*>(si.get());
	bool rotated = itemPosRotatable != nullptr && itemPosRotatable->angle() != 0;

	if (rotated == true)
	{
		auto rotatePoint = itemPosRotatable->rotationPointInDocPt();

		QTransform transform;
		transform.translate(rotatePoint.x(), rotatePoint.y());
		transform.rotate(-itemPosRotatable->angle());
		transform.translate(-rotatePoint.x(), -rotatePoint.y());

		mouseSizingEndPointDocPt = transform.map(mouseSizingEndPointDocPt);
	}

	double xdif = mouseSizingEndPointDocPt.x() - mouseSizingStartPointDocPt.x();
	double ydif = mouseSizingEndPointDocPt.y() - mouseSizingStartPointDocPt.y();

	// Get new rect
	//
	QRectF newItemRect = editSchemaView()->sizingRectItem(xdif, ydif, itemPos);
	newItemRect = newItemRect.normalized();

	// --
	//
	std::vector<VFrame30::SchemaPoint> itemPoints;

	itemPoints.push_back(VFrame30::SchemaPoint(newItemRect.topLeft()));
	itemPoints.push_back(VFrame30::SchemaPoint(newItemRect.bottomRight()));

	if (bool ok = m_editEngine->startBatch(); ok == true)
	{
		m_editEngine->runSetPoints(itemPoints, si, true);
		finishMoveAfbsConnectionLinks();

		m_editEngine->endBatch();
	}

	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_MovingLinePoint(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingStartLinePoint && mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();
	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	std::vector<VFrame30::SchemaPoint> points(2);

	QPointF spt = editSchemaView()->m_editStartDocPt;
	QPointF ept = widgetPointToDocument(event->pos(), snapToGrid());

	double xdif = ept.x() - spt.x();
	double ydif = ept.y() - spt.y();

	if (std::abs(xdif) < 0.000001 && std::abs(ydif) < 0.000001)
	{
		// There is no real moving
		//
		resetAction();
		return;
	}

	if (mouseState() == MouseState::MovingStartLinePoint)
	{
		points[0] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->startXDocPt() + xdif, itemPos->startYDocPt() + ydif));
		points[1] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->endXDocPt(), itemPos->endYDocPt()));
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		points[0] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->startXDocPt(), itemPos->startYDocPt()));
		points[1] = static_cast<VFrame30::SchemaPoint>(QPointF(itemPos->endXDocPt() + xdif, itemPos->endYDocPt() + ydif));
	}

	m_editEngine->runSetPoints(points, si, true);

	//--
	//
	resetAction();
	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosLineEndPoint(QMouseEvent* event)
{
	assert(editSchemaView()->m_newItem != nullptr);

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemaView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setEndXDocPt(docPoint.x());
	itemPos->setEndYDocPt(docPoint.y());

	if (std::abs(itemPos->startXDocPt() - itemPos->endXDocPt()) < 0.000001 &&
		std::abs(itemPos->startYDocPt() - itemPos->endYDocPt()) < 0.000001)
	{
		// The line is empty
		//
		update();
	}
	else
	{
		// Add item to the active layer
		//
		auto newItem = editSchemaView()->m_newItem;

		runAddItem(newItem, editSchemaView()->activeLayer());
		/*bool defaultsWereSet = */ setDefaultItemProperties(std::list{newItem});
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosRectEndPoint(QMouseEvent* event)
{
	assert(editSchemaView()->m_newItem != nullptr);

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemaView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemaView()->m_addRectStartPoint;
	QPointF ep = editSchemaView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());
	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	if (itemPos->widthDocPt() < minWidth)
	{
		itemPos->setWidthDocPt(minWidth);
	}
	if (itemPos->heightDocPt() < minHeight)
	{
		itemPos->setHeightDocPt(minHeight);
	}

	if (itemPos->widthDocPt() < 0.000001 && itemPos->heightDocPt() < 0.000001)
	{
		// The rect is empty
		//
		update();
	}
	else
	{
		// �������� ������� � �������� ����
		//
		auto newItem = editSchemaView()->m_newItem;
		runAddItem(newItem, editSchemaView()->activeLayer());

		/*bool defaultsWereSet = */ setDefaultItemProperties(std::list{newItem});
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_AddSchemaPosConnectionNextPoint(QMouseEvent* e)
{
	if (editSchemaView()->m_newItem == nullptr || editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);
		resetAction();
		return;
	}

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	// Add the last point, where cursor is now to ALL
	//
	mouseRightDown_AddSchemaPosConnectionNextPoint(e);

	const EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();
	ecl.setPointToItem(std::dynamic_pointer_cast<VFrame30::PosConnectionImpl>(editSchemaView()->m_newItem));

	if (itemPos->GetPointList().size() >= 2)
	{
		// ���� ����� �������� ��� ���������� �� ����� �� ��������, �� ��������� ��� ��� (��� ���) �����
		//
		auto points = itemPos->GetPointList();

		VFrame30::SchemaPoint startPoint = points.front();
		VFrame30::SchemaPoint endPoint = points.back();

		// ����� ��������� ������� ����� �� ������ startPoint, endPoint
		//
		QUuid startItemGuid = QUuid();       // ����� ���������� �� �������� ����� ������� ����� ����������� � ������ ����� ����� �����
		bool startPointAddedToOther = false; // ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)
		bool endPointAddedToOther = false;   // ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)


		std::list<SchemaItemPtr> linksUnderStartPoint =
			activeLayer()->getItemListUnderPoint(QPointF(startPoint.X, startPoint.Y),
												 editSchemaView()->m_newItem->metaObject()->className());

		std::list<SchemaItemPtr> linksUnderEndPoint =
			activeLayer()->getItemListUnderPoint(QPointF(endPoint.X, endPoint.Y), editSchemaView()->m_newItem->metaObject()->className());

		SchemaItemPtr linkUnderStartPoint = linksUnderStartPoint.size() == 1 ? linksUnderStartPoint.front() : SchemaItemPtr();
		SchemaItemPtr linkUnderEndPoint = linksUnderEndPoint.size() == 1 ? linksUnderEndPoint.front() : SchemaItemPtr();

		SchemaItemPtr fblRectUnderStartPoint = activeLayer()->findPinUnderPoint(startPoint, schema()->gridSize(), schema()->pinGridStep());

		SchemaItemPtr fblRectUnderEndPoint = activeLayer()->findPinUnderPoint(endPoint, schema()->gridSize(), schema()->pinGridStep());

		if (linkUnderStartPoint != nullptr && fblRectUnderStartPoint.get() == nullptr) // Start point is not on BlItemRect pin
		{
			assert(linkUnderStartPoint->metaObject()->className() == editSchemaView()->m_newItem->metaObject()->className());

			// ��� ����� �� ��������, ���� ����� schemaItemStartPoint ����� �� ������ ��� ��������� ����� schemaItemStartPoint,
			// �� ���������� schemaItemStartPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IPosConnection*>(linkUnderStartPoint.get()) != nullptr);

			VFrame30::IPosConnection* existingItemPos = dynamic_cast<VFrame30::IPosConnection*>(linkUnderStartPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - startPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - startPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ��������� ����� ������ �����
				//
				startItemGuid =
					linkUnderStartPoint
						->guid(); // ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����, ��� �� �� ���������� ������
				startPointAddedToOther = true;

				// --
				points.reverse();
				existingItemPoints.pop_front();
				points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());
				points.reverse(); // ���� ����� ����������� �� ��������� �����, �� ���� Recerse ����� �����

				std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
				newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

				m_editEngine->runSetPoints(newPoints, linkUnderStartPoint, true);
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - startPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - startPoint.Y) < 0.000001)
				{
					// ������ ����� ����� ����� ����� �� ��������� ����� ������������ �����
					//
					startItemGuid = linkUnderStartPoint->guid(); // ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����,
																 // ��� �� �� ���������� ������
					startPointAddedToOther = true;

					// --
					points.pop_front();

					existingItemPoints.insert(existingItemPoints.end(), points.begin(), points.end());
					points.clear();
					points.assign(existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderStartPoint, true);
				}
			}
		}

		// --
		//
		if (linkUnderEndPoint != nullptr && fblRectUnderEndPoint.get() == nullptr && // End point is not on BlItemRect pin
			linkUnderEndPoint->guid() != startItemGuid) // ������� �� ������ ���� � ���������� ������� ����������� � ���� �� �����
		{
			// ������ ���� ����� �� ��� ��������
			//
			assert(linkUnderEndPoint->metaObject()->className() == editSchemaView()->m_newItem->metaObject()->className());

			// ��� ����� �� ��������, ���� ����� schemaItemEndPoint ����� �� ������ ��� ��������� ����� schemaItemEndPoint,
			// �� ���������� schemaItemEndPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IPosConnection*>(linkUnderEndPoint.get()) != nullptr);
			VFrame30::IPosConnection* existingItemPos = dynamic_cast<VFrame30::IPosConnection*>(linkUnderEndPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - endPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - endPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ������ ����� ������ �����
				//
				endPointAddedToOther = true;

				if (startPointAddedToOther == true) // ����� ����� ��� ���� ��������� ��������� schemaItemStartPoint
				{
					// ����������� ���� �������� - schemaItemStartPoint � schemaItemEndPoint
					//
					existingItemPoints.pop_front();
					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					m_editEngine->runDeleteItem(linkUnderStartPoint, activeLayer());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
				}
				else
				{
					// � �������� schemaItemEndPoint �������� points
					//
					existingItemPoints.pop_front();

					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
					newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

					m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
				}
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - endPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - endPoint.Y) < 0.000001)
				{
					// ��������� ����� ����� ����� ����� �� ��������� ����� ������������ �����
					//
					endPointAddedToOther = true;

					if (startPointAddedToOther == true) // ����� ����� ��� ���� ��������� ��������� schemaItemStartPoint
					{
						// ����������� ���� �������� - schemaItemStartPoint � schemaItemEndPoint
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						m_editEngine->runDeleteItem(linkUnderStartPoint, activeLayer());

						std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
						newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
					}
					else
					{
						// � �������� schemaItemEndPoint �������� points
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						std::vector<VFrame30::SchemaPoint> newPoints(points.begin(), points.end());
						newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);

						m_editEngine->runSetPoints(newPoints, linkUnderEndPoint, true);
					}
				}
			}
		}

		if (startPointAddedToOther == false && endPointAddedToOther == false)
		{
			if (itemPos->GetPointList().size() > 2 ||
				(itemPos->GetPointList().size() == 2 && itemPos->GetPointList().front() != itemPos->GetPointList().back()))
			{
				const std::list<VFrame30::SchemaPoint>& pointList = itemPos->GetPointList();

				std::list<VFrame30::SchemaPoint> newPoints = EditConnectionLine::removeUnwantedPoints(pointList);

				itemPos->SetPointList(newPoints);
				assert(itemPos->GetPointList().size() >= 2);

				auto newItem = editSchemaView()->m_newItem;
				runAddItem(newItem, activeLayer());

				/*bool defaultsWereSet = */ setDefaultItemProperties(std::list{newItem});
			}
		}
	}

	resetAction();

	return;
}

void EditSchemaWidget::mouseLeftUp_MovingEdgeOrVertex(QMouseEvent*)
{
	if (mouseState() != MouseState::MovingHorizontalEdge && mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		return;
	}

	if (selectedItems().size() != 1 || editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

		resetAction();
		return;
	}

	auto si = selectedItems().front();
	assert(si != nullptr);

	VFrame30::IPosConnection* itemPos = dynamic_cast<VFrame30::IPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	// Check if the real change vertex or edge has been done
	//
	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	if (ecl.mode() == EditConnectionLine::AddToBegin || ecl.mode() == EditConnectionLine::AddToEnd)
	{
		QPointF lastExtPt = ecl.lastExtensionPoint();

		ecl.moveExtensionPointsToBasePoints();
		ecl.addExtensionPoint(lastExtPt);
	}

	auto basePoints = ecl.basePoints();

	std::list<VFrame30::SchemaPoint> newPoints = {basePoints.begin(), basePoints.end()};
	const std::list<VFrame30::SchemaPoint>& itemPoints = itemPos->GetPointList();

	if (newPoints == itemPoints)
	{
		// Nothing has changed, do not exec a command
		//
		resetAction();
		return;
	}

	if (ecl.mode() == EditConnectionLine::AddToBegin || ecl.mode() == EditConnectionLine::AddToEnd)
	{
		newPoints = EditConnectionLine::removeUnwantedPoints(newPoints);
	}

	std::vector<VFrame30::SchemaPoint> setPoints(newPoints.begin(), newPoints.end());
	m_editEngine->runSetPoints(setPoints, si, true);

	resetAction();
	return;
}

void EditSchemaWidget::mouseMove_Scrolling(QMouseEvent*)
{
	// To Do
	assert(false);
	return;
}

void EditSchemaWidget::mouseMove_Selection(QMouseEvent* me)
{
	QRect oldRect{editSchemaView()->m_mouseSelectionStartPointForUpdate, editSchemaView()->m_mouseSelectionEndPointForUpdate};

	editSchemaView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);
	editSchemaView()->m_mouseSelectionEndPointForUpdate = me->pos();

	QRect newRect{editSchemaView()->m_mouseSelectionStartPointForUpdate, editSchemaView()->m_mouseSelectionEndPointForUpdate};
	newRect = newRect.normalized();
	newRect = newRect.united(oldRect);

	// --
	//
	QRect updateRect{editSchemaView()->mapFrom(this, newRect.topLeft()), editSchemaView()->mapFrom(this, newRect.bottomRight())};

	updateRect.adjust(-2, -2, 2, 2);

	editSchemaView()->update(updateRect);

	return;
}

void EditSchemaWidget::mouseMove_Moving(QMouseEvent* me)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() == false);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	// If Alt is pressed then moving in one dirrection horz or vert
	//
	if (bool altIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier); altIsPressed)
	{
		QPointF offset = editSchemaView()->m_editEndDocPt - editSchemaView()->m_editStartDocPt;

		if (std::abs(offset.rx()) > std::abs(offset.ry()))
		{
			editSchemaView()->m_editEndDocPt.setY(editSchemaView()->m_editStartDocPt.ry());
		}
		else
		{
			editSchemaView()->m_editEndDocPt.setX(editSchemaView()->m_editStartDocPt.rx());
		}
	}

	// Move links along item
	//
	QPointF offset = editSchemaView()->m_editEndDocPt - editSchemaView()->m_editStartDocPt;

	moveAfbsConnectionLinks(offset, mouseState());

	// --
	//
	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_SizingRect(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(selectedItems().front().get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	auto itemPosRotatable = dynamic_cast<const VFrame30::PosRectRotatable*>(selectedItems().front().get());
	bool rotated = itemPosRotatable != nullptr && itemPosRotatable->angle() != 0;

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());
	if (rotated == true)
	{
		auto rotatePoint = itemPosRotatable->rotationPointInDocPt();

		QTransform transform;
		transform.translate(rotatePoint.x(), rotatePoint.y());
		transform.rotate(-itemPosRotatable->angle());
		transform.translate(-rotatePoint.x(), -rotatePoint.y());

		editSchemaView()->m_editEndDocPt = transform.map(editSchemaView()->m_editEndDocPt);
	}

	// Get possible links offset
	//
	double xdif = editSchemaView()->m_editEndDocPt.x() - editSchemaView()->m_editStartDocPt.x();
	double ydif = editSchemaView()->m_editEndDocPt.y() - editSchemaView()->m_editStartDocPt.y();

	QRectF currentRect(itemPos->leftDocPt(), itemPos->topDocPt(), itemPos->widthDocPt(), itemPos->heightDocPt());

	QRectF newRect = editSchemaView()->sizingRectItem(xdif, ydif, itemPos);

	switch (mouseState())
	{
	case MouseState::SizingTop:
		xdif = 0;
		ydif = newRect.top() - currentRect.top();
		break;
	case MouseState::SizingTopRight:
		xdif = newRect.right() - currentRect.right();
		ydif = newRect.top() - currentRect.top();
		break;
	case MouseState::SizingRight:
		xdif = newRect.right() - currentRect.right();
		ydif = 0;
		break;
	case MouseState::SizingBottomRight:
		xdif = newRect.right() - currentRect.right();
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingBottom:
		xdif = 0;
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingBottomLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = newRect.bottom() - currentRect.bottom();
		break;
	case MouseState::SizingLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = 0;
		break;
	case MouseState::SizingTopLeft:
		xdif = newRect.left() - currentRect.left();
		ydif = newRect.top() - currentRect.top();
		break;
	default:
		assert(false);
	}

	// Move links
	//
	moveAfbsConnectionLinks(QPointF(xdif, ydif), mouseState());

	// --
	//
	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_MovingLinePoint(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	auto si = selectedItems().front();

	if (dynamic_cast<VFrame30::IPosLine*>(si.get()) == nullptr)
	{
		assert(dynamic_cast<VFrame30::IPosLine*>(si.get()) != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemaView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());
	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosLineEndPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosLine* itemPos = dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setEndXDocPt(docPoint.x());
	itemPos->setEndYDocPt(docPoint.y());

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosRectEndPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemaView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemaView()->m_addRectStartPoint;
	QPointF ep = editSchemaView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());


	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(schema()->gridSize(), schema()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(), schema()->pinGridStep());

	if (itemPos->widthDocPt() < minWidth)
	{
		itemPos->setWidthDocPt(minWidth);
	}
	if (itemPos->heightDocPt() < minHeight)
	{
		itemPos->setHeightDocPt(minHeight);
	}

	editSchemaView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemaView()->m_addRectEndPoint.setY(itemPos->topDocPt() + itemPos->heightDocPt());

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseMove_AddSchemaPosConnectionNextPoint(QMouseEvent* event)
{
	if (editSchemaView()->m_newItem == nullptr)
	{
		assert(editSchemaView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	// magnet point to pin
	//
	docPoint = magnetPointToPin(docPoint);

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		movePosConnectionEndPoint(editSchemaView()->m_newItem, &ecl, docPoint);
	}

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseMove_MovingEdgesOrVertex(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingHorizontalEdge && mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		resetAction();
		return;
	}

	if (selectedItems().size() != 1 || editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1);
		assert(editSchemaView()->m_editConnectionLines.size() != 1);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	// --
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		ecl.modifyEdge(docPoint.y());
		break;
	case MouseState::MovingVerticalEdge:
		ecl.modifyEdge(docPoint.x());
		break;
	case MouseState::MovingConnectionLinePoint:
		{
			switch (ecl.mode())
			{
			case EditConnectionLine::EditMode::EditPoint:
				docPoint = magnetPointToPin(docPoint);
				ecl.modifyPoint(docPoint);
				break;

			case EditConnectionLine::EditMode::AddToBegin:
			case EditConnectionLine::EditMode::AddToEnd:
				docPoint = magnetPointToPin(docPoint);
				movePosConnectionEndPoint(selectedItems().front(), &ecl, docPoint);
				break;

			default:
				assert(false);
			}
		}
		break;
	default:
		assert(false);
	}

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightDown_None(QMouseEvent*)
{
	// CURRENTLY THIS ACTION IS DISABLED IN CONSTRUCTOR, ADD IT TO THE RightClickPress array
	//
	// To Do from old project
	//
	return;
}

void EditSchemaWidget::mouseRightDown_AddSchemaPosConnectionNextPoint(QMouseEvent* /*event*/)
{
	if (editSchemaView()->m_newItem == nullptr || editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(editSchemaView()->m_newItem != nullptr);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

		resetAction();
		return;
	}

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		assert(ecl.extensionPoints().empty() == false);

		QPointF lastExtPt = ecl.lastExtensionPoint();

		ecl.moveExtensionPointsToBasePoints();
		ecl.addExtensionPoint(lastExtPt);
	}

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightDown_MovingEdgesOrVertex(QMouseEvent* event)
{
	if (selectedItems().size() != 1 || editSchemaView()->m_editConnectionLines.size() != 1)
	{
		assert(selectedItems().size() == 1);
		assert(editSchemaView()->m_editConnectionLines.size() == 1);

		resetAction();
		return;
	}

	EditConnectionLine& ecl = editSchemaView()->m_editConnectionLines.front();

	if (ecl.mode() != EditConnectionLine::EditMode::EditPoint && ecl.mode() != EditConnectionLine::EditMode::AddToBegin &&
		ecl.mode() != EditConnectionLine::EditMode::AddToEnd)
	{
		assert(false);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	switch (ecl.mode())
	{
	case EditConnectionLine::EditMode::EditPoint:
		ecl.addPointAndSwitchMode(docPoint);
		break;

	case EditConnectionLine::EditMode::AddToBegin:
	case EditConnectionLine::EditMode::AddToEnd:
		{
			assert(ecl.extensionPoints().empty() == false);
			QPointF lastExtPt = ecl.lastExtensionPoint();

			ecl.moveExtensionPointsToBasePoints();
			ecl.addExtensionPoint(lastExtPt);
		}
		break;
	default:
		assert(false);
		resetAction();
	}

	// --
	//
	editSchemaView()->update();

	return;
}

void EditSchemaWidget::mouseRightUp_None(QMouseEvent* event)
{
	QPointF docPoint = widgetPointToDocument(event->pos(), false);

	// Proposed links.
	//
	{
		auto proposedLinks = editSchemaView()->m_autoFblItemConnection.getPropositions();

		bool clickOnAddRectArea = std::any_of(proposedLinks.begin(),
											  proposedLinks.end(),
											  [&docPoint](const auto& link)
											  {
												  return link.addButtonRect.contains(docPoint);
											  });
		if (clickOnAddRectArea == true)
		{
			return;
		}
	}

	// Selected one item, check if we hit in it, on item, control bars, etc
	//
	if (selectedItems().size() == 1)
	{
		int movingEdgePointIndex = 0;
		auto selectedItem = selectedItems()[0];

		SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

		if (possibleAction != SchemaItemAction::NoAction)
		{
			resetAction();
			return;
		}
	}

	// Check if we hit on any item
	//
	auto item = editSchemaView()->activeLayer()->getItemUnderPoint(docPoint);

	if (item != nullptr)
	{
		bool itemIsAlreadySelected = editSchemaView()->isItemSelected(item);

		if (itemIsAlreadySelected == false)
		{
			editSchemaView()->setSelectedItem(item);
			resetAction();
		}
	}
	else
	{
		editSchemaView()->clearSelection();
		resetAction();
	}

	return;
}

DbController* EditSchemaWidget::dbcontroller()
{
	return m_dbcontroller;
}

DbController* EditSchemaWidget::db()
{
	return m_dbcontroller;
}

EditSchemaView* EditSchemaWidget::editSchemaView()
{
	EditSchemaView* sw = dynamic_cast<EditSchemaView*>(schemaView());
	assert(sw != nullptr);
	return sw;
}

const EditSchemaView* EditSchemaWidget::editSchemaView() const
{
	const EditSchemaView* sw = dynamic_cast<const EditSchemaView*>(schemaView());
	assert(sw != nullptr);
	return sw;
}

bool EditSchemaWidget::isLogicSchema() const
{
	return schema()->isLogicSchema();
}

bool EditSchemaWidget::isUfbSchema() const
{
	return schema()->isUfbSchema();
}

bool EditSchemaWidget::isMonitorSchema() const
{
	return schema()->isMonitorSchema();
}

bool EditSchemaWidget::isTuningSchema() const
{
	return schema()->isTuningSchema();
}

bool EditSchemaWidget::isDiagSchema() const
{
	return schema()->isDiagSchema();
}

bool EditSchemaWidget::isVduSchema() const
{
	return schema()->isVduSchema();
}

bool EditSchemaWidget::isActuatorSchema() const
{
	return schema()->isActuatorSchema();
}

std::shared_ptr<VFrame30::LogicSchema> EditSchemaWidget::logicSchema()
{
	std::shared_ptr<VFrame30::LogicSchema> logicSchema = std::dynamic_pointer_cast<VFrame30::LogicSchema>(schemaSharedPtr());
	return logicSchema;
}

const std::shared_ptr<VFrame30::LogicSchema> EditSchemaWidget::logicSchema() const
{
	const std::shared_ptr<VFrame30::LogicSchema> logicSchema = std::dynamic_pointer_cast<VFrame30::LogicSchema>(schemaSharedPtr());
	return logicSchema;
}

const std::vector<SchemaItemPtr>& EditSchemaWidget::selectedItems() const
{
	return editSchemaView()->m_selectedItems;
}

std::vector<SchemaItemPtr> EditSchemaWidget::selectedNonLockedItems() const
{
	return editSchemaView()->selectedNonLockedItems();
}

std::shared_ptr<VFrame30::SchemaLayer> EditSchemaWidget::activeLayer()
{
	return editSchemaView()->activeLayer();
}


QPointF EditSchemaWidget::widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const
{
	QPointF result = BaseSchemaWidget::widgetPointToDocument(widgetPoint);

	if (snapToGrid == true)
	{
		QPointF snapped = this->snapToGrid(result);
		return snapped;
	}

	return result;
}

QPointF EditSchemaWidget::snapToGrid(QPointF pt) const
{
	return VFrame30::snapToGrid(pt, schema()->gridSize());
}

VFrame30::SchemaPoint EditSchemaWidget::snapToGrid(VFrame30::SchemaPoint pt) const
{
	return VFrame30::SchemaPoint{VFrame30::snapToGrid(pt.X, pt.Y, schema()->gridSize())};
}

bool EditSchemaWidget::updateAfbsForSchema()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Update Afb list
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs;

	bool ok = loadAfbsDescriptions(&afbs);
	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemFbs(afbs, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update AFB schema items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 AFB(s) are updated according to the latest AFB description.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}

bool EditSchemaWidget::updateUfbsForSchema()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Get Ufb list
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

	bool ok = loadUfbSchemas(&ufbs);

	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	// Update
	//
	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemUfb(ufbs, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update UFB schema items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 UFB(s) are updated according to the latest UFB schemas.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}

bool EditSchemaWidget::updateBussesForSchema()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QApplication::processEvents();

	// Get Bus list
	//
	std::vector<AppSignalLib::Bus> busses;

	bool ok = F2KeyForSchemaItem::loadBusses(db(), &busses, this);

	if (ok == false)
	{
		QApplication::restoreOverrideCursor();
		return false;
	}

	// Update
	//
	QString errorMessage;
	int updatedItemCount = 0;

	ok = schema()->updateAllSchemaItemBusses(busses, &updatedItemCount, &errorMessage);

	QApplication::restoreOverrideCursor();

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("Update Bus items error: ") + errorMessage);
		return false;
	}

	if (updatedItemCount != 0)
	{
		setModified();

		QMessageBox msgBox(this);
		msgBox.setWindowTitle(qApp->applicationName());
		msgBox.setText(tr("%1 Bus(s) are updated according to the latest Bus descriptions.").arg(updatedItemCount));
		msgBox.setInformativeText("Please, check input/output pins and parameters.\nClose schema without saving to discard changes.");
		msgBox.exec();
	}

	return true;
}


void EditSchemaWidget::addItem(SchemaItemPtr newItem)
{
	if (newItem == nullptr)
	{
		assert(newItem != nullptr);
		return;
	}

	editSchemaView()->m_newItem = newItem;

	// Set label to it
	//
	{
		int counterValue = 0;
		bool nextValRes = db()->nextCounterValue(&counterValue);
		if (nextValRes == false)
		{
			return;
		}

		newItem->setLabel(schema()->schemaId() + "_" + QString::number(counterValue));
	}

	// --
	//
	bool posInterfaceFound = false;

	// ����������� ������� � ������������� ISchemaPosLine
	//
	if (dynamic_cast<VFrame30::IPosLine*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosLineStartPoint);
	}

	// ����������� ������� � ������������� ISchemaPosRect
	//
	if (dynamic_cast<VFrame30::IPosRect*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosRectStartPoint);

		VFrame30::IPosRect* itemPos = dynamic_cast<VFrame30::IPosRect*>(newItem.get());
		std::ignore = itemPos->minimumPossibleHeightDocPt(schema()->gridSize(),
														  schema()->pinGridStep()); // chachedGridSize and pinStep will be initialized here
	}

	// ����������� ������� � ������������� ISchemaPosConnection
	//
	if (dynamic_cast<VFrame30::IPosConnection*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemaPosConnectionStartPoint);
	}

	if (posInterfaceFound == true)
	{
		// �������� �������� ������, ������ �������� ��������� �����
		//
		setMouseCursor(mapFromGlobal(QCursor::pos()));
	}
	else
	{
		setMouseState(MouseState::None);
		assert(posInterfaceFound != false);
	}

	return;
}

void EditSchemaWidget::runAddItem(SchemaItemPtr item, std::shared_ptr<VFrame30::SchemaLayer> layer)
{
	std::list<SchemaItemPtr> list{item};
	return runAddItem(list, layer);
}

void EditSchemaWidget::runAddItem(const std::list<SchemaItemPtr>& items, std::shared_ptr<VFrame30::SchemaLayer> layer)
{
	bool ok = m_editEngine->runAddItem(items, layer);
	if (ok == false)
	{
		return;
	}

	return;
}

bool EditSchemaWidget::setDefaultItemProperties(const auto& items)
{
	// Update properties to default values
	//
	const auto& pd = ProjectDefaults::instance();

	bool canByApplied = std::any_of(items.cbegin(),
									items.cend(),
									[&pd](const SchemaItemPtr& item)
									{
										return pd.hasSection(item->type());
									});

	if (canByApplied == false)
	{
		return false;
	}

	m_editEngine->runApplyDefaultProperty(pd, std::vector<SchemaItemPtr>{items.begin(), items.end()});

	return true;
}

void EditSchemaWidget::setMouseCursor(QPoint mousePos)
{
	for (size_t i = 0; i < sizeof(m_mouseStateCursor) / sizeof(m_mouseStateCursor[0]); i++)
	{
		if (mouseState() == m_mouseStateCursor[i].mouseState)
		{
			setCursor(m_mouseStateCursor[i].cursorShape);
			return;
		}
	}

	int movingEdgePointIndex = -1;

	// Setting cursor specific cases
	//
	if (mouseState() == MouseState::None)
	{
		// Convert pixels to document points
		//
		QPointF docPos = widgetPointToDocument(mousePos, false);

		// Create proposed connections for FblItem pins.
		//
		auto proposedLinks = editSchemaView()->m_autoFblItemConnection.getPropositions();

		bool inRect = std::any_of(proposedLinks.begin(),
								  proposedLinks.end(),
								  [&docPos](const auto& link)
								  {
									  return link.addButtonRect.contains(docPos);
								  });
		if (inRect == true)
		{
			setCursor(Qt::PointingHandCursor);
			return;
		}

		// --
		//
		if (selectedItems().empty() == true)
		{
			SchemaItemPtr itemUnderPoint = editSchemaView()->activeLayer()->getItemUnderPoint(docPos);

			if (itemUnderPoint != nullptr &&
				editSchemaView()->getPossibleAction(itemUnderPoint.get(), docPos, &movingEdgePointIndex) == SchemaItemAction::MoveItem)
			{
				setCursor(Qt::SizeAllCursor);
				return;
			}
		}

		for (SchemaItemPtr si : editSchemaView()->selectedItems())
		{
			SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(si.get(), docPos, &movingEdgePointIndex);

			if (possibleAction == SchemaItemAction::MoveItem && editSchemaView()->selectedItems().size() > 1 &&
				si->isType<VFrame30::IPosConnection>() == true)
			{
				// This SchemaItemAction::MoveItem is for @MOveBar@, exclude it for MULTIPLE selection
				//
				possibleAction = SchemaItemAction::NoAction;
			}

			if (possibleAction != SchemaItemAction::NoAction)
			{
				// Changing size, is possible for only one selected object
				//
				if (editSchemaView()->selectedItems().size() == 1)
				{
					auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor),
												   std::end(m_sizeActionToMouseCursor),
												   [&possibleAction](const SizeActionToMouseCursor& c) -> bool
												   {
													   return c.action == possibleAction;
												   });

					if (findResult != std::end(m_sizeActionToMouseCursor))
					{
						// qDebug() << Q_FUNC_INFO << static_cast<int>(findResult->cursorShape);
						setCursor(findResult->cursorShape);
						return;
					}
				}

				// Changing any vertexes or ribs of ConnectionLine possible only
				// for one item
				//
				if (si->isType<VFrame30::IPosConnection>() == true && editSchemaView()->selectedItems().size() > 1)
				{
					if (possibleAction == SchemaItemAction::MoveConnectionLinePoint ||
						possibleAction == SchemaItemAction::MoveHorizontalEdge || possibleAction == SchemaItemAction::MoveVerticalEdge)
					{
						possibleAction = SchemaItemAction::MoveItem;
					}
				}

				// --
				//
				switch (possibleAction)
				{
				case SchemaItemAction::MoveItem:
					setCursor(Qt::SizeAllCursor);
					return;
				case SchemaItemAction::MoveStartLinePoint:
					setCursor(Qt::CrossCursor);
					return;
				case SchemaItemAction::MoveEndLinePoint:
					setCursor(Qt::CrossCursor);
					return;
				case SchemaItemAction::MoveHorizontalEdge:
					setCursor(Qt::SplitVCursor);
					return;
				case SchemaItemAction::MoveVerticalEdge:
					setCursor(Qt::SplitHCursor);
					return;
				case SchemaItemAction::MoveConnectionLinePoint:
					setCursor(Qt::CrossCursor);
					return;
				default:
					void();
				}
			}
		}

		// --
		//
		setCursor(Qt::ArrowCursor);
		return;
	}

	// --
	//
	if (dynamic_cast<VFrame30::IPosLine*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		setCursor(Qt::CursorShape::CrossCursor);
		return;
	}

	if (dynamic_cast<VFrame30::IPosRect*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		setCursor(Qt::CursorShape::CrossCursor);
		return;
	}

	if (dynamic_cast<VFrame30::IPosConnection*>(editSchemaView()->m_newItem.get()) != nullptr)
	{
		setCursor(Qt::CursorShape::CrossCursor);
		return;
	}

	setCursor(Qt::ArrowCursor);
	return;
}

QPointF EditSchemaWidget::magnetPointToPin(QPointF docPoint)
{
	double gridSize = schema()->gridSize();
	double pinGridStep = static_cast<double>(schema()->pinGridStep());

	// Find "magnet" points, it can be any pin or link
	// Detect if docPoint closer then any pin to gridSize * pinGridStep / 2
	// If so, stick docPoint to this pin
	//
	std::vector<VFrame30::AfbPin> itemPins;
	itemPins.reserve(64);

	for (const SchemaItemPtr& item : activeLayer()->items())
	{
		VFrame30::FblItemRect* fblItemRect = dynamic_cast<VFrame30::FblItemRect*>(item.get());

		if (fblItemRect != nullptr)
		{
			fblItemRect->SetConnectionsPos(schema()->gridSize(), schema()->pinGridStep());

			const std::vector<VFrame30::AfbPin>& inputs = fblItemRect->inputs();
			const std::vector<VFrame30::AfbPin>& outputs = fblItemRect->outputs();

			itemPins.clear();
			itemPins.insert(itemPins.end(), inputs.begin(), inputs.end());
			itemPins.insert(itemPins.end(), outputs.begin(), outputs.end());

			for (const VFrame30::AfbPin& pin : itemPins)
			{
				QRectF pinArea = {pin.x() - gridSize * pinGridStep / 2,
								  pin.y() - gridSize * pinGridStep / 2,
								  gridSize * pinGridStep,
								  gridSize * pinGridStep};

				if (pinArea.contains(docPoint) == true)
				{
					return QPointF(pin.x(), pin.y());
				}
			}

			continue;
		}

		// To do, magnet to link
		//
	}

	return docPoint;
}

void EditSchemaWidget::movePosConnectionEndPoint([[maybe_unused]] SchemaItemPtr schemaItem, EditConnectionLine* ecl, QPointF toPoint)
{
	assert(schemaItem);
	assert(ecl);

	ecl->moveEndPointPos(activeLayer(), toPoint, EditConnectionLine::Auto, schema()->gridSize());

	return;
}

void EditSchemaWidget::addConnectionLinePoint(SchemaItemPtr schemaItem, QPointF docPoint)
{
	VFrame30::IPosConnection* posItem = schemaItem->toType<VFrame30::IPosConnection>();

	if (posItem == nullptr)
	{
		Q_ASSERT(posItem);
		return;
	}

	int movingEdgePointIndex = 0;
	SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(schemaItem.get(), docPoint, &movingEdgePointIndex);

	std::list<VFrame30::SchemaPoint> points = posItem->GetPointList();
	if (static_cast<size_t>(movingEdgePointIndex) >= points.size())
	{
		Q_ASSERT(false);
		return;
	}

	VFrame30::SchemaPoint newPoint = *std::next(points.begin(), movingEdgePointIndex);

	if (possibleAction == SchemaItemAction::MoveHorizontalEdge)
	{
		newPoint.X = docPoint.x();
	}
	if (possibleAction == SchemaItemAction::MoveVerticalEdge)
	{
		newPoint.Y = docPoint.y();
	}

	if (snapToGrid() == true)
	{
		newPoint = snapToGrid(newPoint);
	}

	points.insert(std::next(points.begin(), movingEdgePointIndex + 1), newPoint);

	// --
	//
	std::vector<VFrame30::SchemaPoint> v{points.begin(), points.end()};
	m_editEngine->runSetPoints(v, schemaItem, true);

	return;
}

void EditSchemaWidget::removeConnectionLinePoint(SchemaItemPtr schemaItem, size_t pointIndex)
{
	VFrame30::IPosConnection* posItem = schemaItem->toType<VFrame30::IPosConnection>();
	if (posItem == nullptr)
	{
		Q_ASSERT(posItem);
		return;
	}

	std::list<VFrame30::SchemaPoint> points = posItem->GetPointList();

	if (pointIndex >= points.size())
	{
		Q_ASSERT(false);
		return;
	}

	points.erase(std::next(points.begin(), pointIndex));

	// --
	//
	std::vector<VFrame30::SchemaPoint> v{points.begin(), points.end()};
	m_editEngine->runSetPoints(v, schemaItem, true);

	return;
}

void EditSchemaWidget::initMoveAfbsConnectionLinks(MouseState mouseState)
{
	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_doNotMoveConnectionLines = false;

	// Go over all selected items pins, and add data to m_editConnectionLines
	//
	std::vector<SchemaItemPtr> selected = selectedNonLockedItems();
	std::multiset<std::shared_ptr<VFrame30::SchemaItemLink>> commonLinks;

	for (const SchemaItemPtr& item : selected)
	{
		if (item->isFblItemRect() == false)
		{
			continue;
		}

		VFrame30::FblItemRect* fblItemRect = item->toFblItemRect();
		assert(fblItemRect);

		// Get links with end on pins
		//
		double gridSize = schema()->gridSize();
		int pinGridStep = schema()->pinGridStep();

		fblItemRect->SetConnectionsPos(gridSize, pinGridStep); // Calc pins' positions

		const std::vector<VFrame30::AfbPin>& inputs = fblItemRect->inputs();
		const std::vector<VFrame30::AfbPin>& outputs = fblItemRect->outputs();

		std::vector<VFrame30::AfbPin> inOuts;
		inOuts.reserve(inputs.size() + outputs.size());

		switch (mouseState)
		{
		case MouseState::Moving:
		case MouseState::SizingTop:
		case MouseState::SizingTopRight:
		case MouseState::SizingTopLeft:
			inOuts.insert(inOuts.end(), inputs.begin(), inputs.end());
			inOuts.insert(inOuts.end(), outputs.begin(), outputs.end());
			break;

		case MouseState::SizingRight:
		case MouseState::SizingBottomRight:
			inOuts.insert(inOuts.end(), outputs.begin(), outputs.end());
			break;

		case MouseState::SizingLeft:
		case MouseState::SizingBottomLeft:
			inOuts.insert(inOuts.end(), inputs.begin(), inputs.end());
			break;
		default:
			break;
		}

		for (const VFrame30::AfbPin& pin : inOuts)
		{
			VFrame30::SchemaPoint pinPos = pin.point();

			std::list<SchemaItemPtr> linksUnderPin = activeLayer()->getItemListUnderPoint(pinPos, "VFrame30::SchemaItemLink");

			// Check if pin on the Start or End point
			//
			for (SchemaItemPtr foundLinkItem : linksUnderPin)
			{
				std::shared_ptr<VFrame30::SchemaItemLink> link = std::dynamic_pointer_cast<VFrame30::SchemaItemLink>(foundLinkItem);
				assert(link);

				// If this link in selected items, slip it
				//
				auto foundInSelectedIt = std::find(selected.begin(), selected.end(), foundLinkItem);
				if (foundInSelectedIt != selected.end())
				{
					continue;
				}

				// Get end points of found link and check if they are on pin
				//
				VFrame30::SchemaPoint ptBegin = link->GetPointList().front();
				VFrame30::SchemaPoint ptEnd = link->GetPointList().back();

				if (pinPos == ptBegin)
				{
					EditConnectionLine ecl(link, EditConnectionLine::MoveToPin);
					ecl.moveToPin_init(link, pin.dirrection(), ptBegin);

					editSchemaView()->m_editConnectionLines.push_back(ecl);

					commonLinks.insert(link);
				}

				if (pinPos == ptEnd)
				{
					EditConnectionLine ecl(link, EditConnectionLine::MoveToPin);
					ecl.moveToPin_init(link, pin.dirrection(), ptEnd);

					editSchemaView()->m_editConnectionLines.push_back(ecl);

					commonLinks.insert(link);
				}
			}
		}
	}

	// Check if there is EditConnectionLine which is going to be moved from both sides
	// If [SIGNAL1] and [SIGNAL2] are selected, the select their common links, and remove it from editSchemaView()->m_editConnectionLines
	//
	// [SIGNAL1]-+---------------+-[SIGNAL2]
	//
	for (std::shared_ptr<VFrame30::SchemaItemLink> cl : commonLinks)
	{
		size_t useCount = commonLinks.count(cl);

		if (useCount > 1)
		{
			auto it = std::find_if(editSchemaView()->m_editConnectionLines.begin(),
								   editSchemaView()->m_editConnectionLines.end(),
								   [cl](const EditConnectionLine& ecl)
								   {
									   return ecl.moveToPin_schemaItem() == cl;
								   });

			if (it != editSchemaView()->m_editConnectionLines.end())
			{
				it->moveToPin_setMoveWholeLink();

				// Remove all other occurrences of Link in m_editConnectionLines
				//
				auto removeIt = std::remove_if(++it,
											   editSchemaView()->m_editConnectionLines.end(),
											   [cl](const EditConnectionLine& ecl)
											   {
												   return ecl.moveToPin_schemaItem() == cl;
											   });

				editSchemaView()->m_editConnectionLines.erase(removeIt, editSchemaView()->m_editConnectionLines.end());
			}
		}
	}

	return;
}

void EditSchemaWidget::moveAfbsConnectionLinks(QPointF offset, MouseState mouseState)
{
	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		QPointF eclOffset;

		switch (mouseState)
		{
		case MouseState::Moving:
			eclOffset = offset;
			break;

		case MouseState::SizingTop:
			eclOffset.ry() = offset.y();
			break;

		case MouseState::SizingBottom:
			assert(false);
			return;

		case MouseState::SizingTopLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset = offset;
			}
			else
			{
				eclOffset.ry() = offset.y();
			}
			break;

		case MouseState::SizingLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.rx() = offset.x();
			}
			else
			{
				assert(false); // Don't add outputs in this case
			}
			break;

		case MouseState::SizingBottomLeft:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.rx() = offset.x();
			}
			else
			{
				assert(false); // Don't add outputs in this case
			}
			break;

		case MouseState::SizingTopRight:
			if (ecl.moveToPin_isInput() == true)
			{
				eclOffset.ry() = offset.y();
			}
			else
			{
				eclOffset = offset;
			}
			break;

		case MouseState::SizingRight:
			if (ecl.moveToPin_isInput() == true)
			{
				assert(false); // Don't add outputs in this case
			}
			else
			{
				eclOffset.rx() = offset.x();
			}
			break;

		case MouseState::SizingBottomRight:
			if (ecl.moveToPin_isInput() == true)
			{
				assert(false); // Don't add outputs in this case
			}
			else
			{
				eclOffset.rx() = offset.x();
			}
			break;

		default:
			assert(false);
		}

		ecl.moveToPin_offset(activeLayer(), eclOffset, schema()->gridSize());
	}

	return;
}

void EditSchemaWidget::finishMoveAfbsConnectionLinks()
{
	setFocus(); // As alt could be pressed and MainMeu activated

	bool ctrlIsPressed = QApplication::queryKeyboardModifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == true || editSchemaView()->m_doNotMoveConnectionLines == true)
	{
		editSchemaView()->m_editConnectionLines.clear();
		return;
	}

	std::vector<std::vector<VFrame30::SchemaPoint>> commandPoints;
	std::vector<SchemaItemPtr> commandItems;

	for (EditConnectionLine& ecl : editSchemaView()->m_editConnectionLines)
	{
		ecl.moveExtensionPointsToBasePoints();
		std::vector<QPointF> points = ecl.points();

		std::list<VFrame30::SchemaPoint> uniquePoints(points.begin(), points.end());
		uniquePoints.unique();

		if (uniquePoints.size() == 1)
		{
			// It can happen theat connection line is zero length and all points except the first one were filtered,
			// but still we expect line to have at least two points.
			//
			uniquePoints.push_back(uniquePoints.front());
		}

		uniquePoints = EditConnectionLine::removeUnwantedPoints(uniquePoints);

		std::vector<VFrame30::SchemaPoint> resultPoinst(uniquePoints.begin(), uniquePoints.end());

		commandPoints.push_back(resultPoinst);
		commandItems.push_back(ecl.moveToPin_schemaItem());
	}

	if (commandPoints.empty() == false)
	{
		assert(commandPoints.size() == commandItems.size());

		m_editEngine->runSetPoints(commandPoints, commandItems, false);

		editSchemaView()->m_editConnectionLines.clear();
	}

	return;
}

void EditSchemaWidget::createProposedAfbLink(const std::vector<AutoFblConnectionProposition>& links)
{
	std::list<SchemaItemPtr> addItems;

	for (const auto& link : links)
	{
		VFrame30::SchemaPoint from = link.from;
		VFrame30::SchemaPoint to = link.to;

		if (std::abs(from.X - to.X) < 0.000001 && std::abs(from.Y - to.Y) < 0.000001)
		{
			continue;
		}

		std::shared_ptr<VFrame30::SchemaItemLink> linkItem = std::make_shared<VFrame30::SchemaItemLink>(schema()->unit());
		linkItem->AddPoint(from.X, from.Y);
		linkItem->AddPoint(from.X, from.Y);

		EditConnectionLine ecl{linkItem, EditConnectionLine::AddToEnd};

		ecl.addExtensionPoint(to);
		ecl.moveEndPointPos(activeLayer(), to, EditConnectionLine::Auto, schema()->gridSize());
		ecl.moveExtensionPointsToBasePoints();

		ecl.setPointToItem(linkItem);
		linkItem->RemoveSamePoints();

		if (auto pl = linkItem->GetPointList(); pl.size() < 1)
		{
			Q_ASSERT(pl.size() > 2);
			continue;
		}

		addItems.push_back(linkItem);
	}

	runAddItem(addItems, activeLayer());
	/*bool defaultsWereSet = */ setDefaultItemProperties(addItems);

	return;
}

bool EditSchemaWidget::loadAfbsDescriptions(std::vector<std::shared_ptr<Afb::AfbElement>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	if (isLogicSchema() == false && isUfbSchema() == false && isActuatorSchema() == false)
	{
		// this function is not applicable
		//
		return false;
	}

	QString LmDescriptionFile;

	{
		auto lmp = schema()->propertyByCaption(VFrame30::PropertyNames::lmDescriptionFile);
		if (lmp != nullptr)
		{
			LmDescriptionFile = lmp->value().toString();
		}
		else
		{
			return false;
		}
	}

	if (LmDescriptionFile.isEmpty() == true)
	{
		QString errorMsg = tr("Scheme property LmDescription is empty. It must contain LogicModuleDescription filename.");
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	std::vector<DbFileInfo> fileList;

	bool result = db()->getFileList(&fileList, DbDir::AfblDir, LmDescriptionFile, true, this);
	if (result == false)
	{
		return false;
	}

	if (fileList.empty() == true)
	{
		QString errorMsg = tr("Cannot find file %1.").arg(LmDescriptionFile);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	// Get description file from the DB
	//
	std::shared_ptr<DbFile> file;
	result = db()->getLatestVersion(fileList[0], &file, this);
	if (result == false)
	{
		return false;
	}

	// Parse file
	//
	LmDescription lm;
	QString parseErrorMessage;

	result = lm.load(file->data(), &parseErrorMessage);

	if (result == false)
	{
		QString errorMsg = tr("Cannot parse file %1. Error message: %2").arg(LmDescriptionFile).arg(parseErrorMessage);
		QMessageBox::critical(this, qApp->applicationName(), errorMsg);
		return false;
	}

	// Get the AFBs and return them
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs = lm.afbElements();

	std::swap(*out, afbs);

	return true;
}

bool EditSchemaWidget::loadUfbSchemas(std::vector<std::shared_ptr<VFrame30::UfbSchema>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	out->clear();

	// Get User Functional Block List
	//
	DbFileTree filesTree;

	bool ok = db()->getFileListTree(&filesTree, DbDir::UfblDir, "%", true, this);
	if (ok == false)
	{
		return false;
	}

	auto fileList = filesTree.toVectorIf(
		[](const DbFileInfo& file)
		{
			return file.fileName().endsWith(QString(".") + File::UfbFileExtension, Qt::CaseInsensitive) == true && file.isFolder() == false;
		});


	// Get UFBs where LmDescriptionFile same with this schema
	//
	std::vector<DbFileInfo> filteredFileList;
	filteredFileList.reserve(fileList.size());

	if (schema()->isLogicSchema() == true || schema()->isActuatorSchema() == true)
	{
		QString schemaLmDescriptionFile;
		auto lmp = schema()->propertyByCaption(VFrame30::PropertyNames::lmDescriptionFile);
		if (lmp != nullptr)
		{
			schemaLmDescriptionFile = lmp->value().toString();
		}

		for (const auto& fi : fileList)
		{
			VFrame30::SchemaDetails details(fi.details());

			if (details.m_lmDescriptionFile == schemaLmDescriptionFile)
			{
				filteredFileList.push_back(fi);
			}
		}
	}
	else
	{
		filteredFileList = fileList;
	}

	// Get UFBs latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db()->getLatestVersion(filteredFileList, &files, this);

	if (ok == false)
	{
		return false;
	}

	// Parse files, create actual UFBs
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;
	ufbs.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true || f->action() == E::VcsItemAction::Deleted)
		{
			continue;
		}

		std::shared_ptr<VFrame30::Schema> s = VFrame30::Schema::Create(f->data());

		if (s == nullptr)
		{
			assert(s);
			continue;
		}

		if (s->isUfbSchema() == false)
		{
			assert(s->isUfbSchema() == true);
			continue;
		}

		std::shared_ptr<VFrame30::UfbSchema> u = std::dynamic_pointer_cast<VFrame30::UfbSchema>(s);
		if (u == nullptr)
		{
			assert(u);
			continue;
		}

		ufbs.push_back(u);

		qDebug() << "EditSchemaWidget::loadUfbSchemas(): Ufb: " << u->schemaId() << ", UfbVersion " << u->version();
	}

	std::swap(ufbs, *out);

	return true;
}

bool EditSchemaWidget::loadActuatorHeaders(std::vector<std::shared_ptr<VFrame30::ActuatorHeader>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	DbFileTree filesTree;

	bool ok = db()->getFileListTree(&filesTree, DbDir::ActuatorsDir, "%", true, this);
	if (ok == false)
	{
		return false;
	}

	auto fileList = filesTree.toVectorIf(
		[](const DbFileInfo& file)
		{
			return file.fileName().endsWith(QString(".") + File::ActuatorHeaderFileExtension, Qt::CaseInsensitive) == true &&
				   file.isFolder() == false;
		});

	// Get UFBs latest version from the DB
	//
	std::vector<std::shared_ptr<DbFile>> files;

	ok = db()->getLatestVersion(fileList, &files, this);
	if (ok == false)
	{
		return false;
	}

	// Parse files, create actual ActuatorHeaders
	//
	std::vector<std::shared_ptr<VFrame30::ActuatorHeader>> actuatorHeaders;
	actuatorHeaders.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->deleted() == true || f->action() == E::VcsItemAction::Deleted)
		{
			continue;
		}

		auto ah = VFrame30::ActuatorHeader::Create(f->data());
		if (ah == nullptr)
		{
			assert(ah);
			continue;
		}

		actuatorHeaders.push_back(ah);
	}

	*out = std::move(actuatorHeaders);
	return true;
}

void EditSchemaWidget::resetAction()
{
	setMouseState(MouseState::None);
	editSchemaView()->m_newItem.reset();

	editSchemaView()->m_editConnectionLines.clear();
	editSchemaView()->m_mouseSelectionStartPoint = QPoint();
	editSchemaView()->m_mouseSelectionEndPoint = QPoint();
	editSchemaView()->m_mouseSelectionStartPointForUpdate = QPoint();
	editSchemaView()->m_mouseSelectionEndPointForUpdate = QPoint();
	editSchemaView()->m_editStartDocPt = QPointF();
	editSchemaView()->m_editEndDocPt = QPointF();

	setMouseCursor(mapFromGlobal(QCursor::pos()));

	editSchemaView()->update();

	return;
}

void EditSchemaWidget::clearSelection()
{
	m_nextSelectionFromLeft = {}; // Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	editSchemaView()->clearSelection();
}

void EditSchemaWidget::layers()
{
	SchemaLayersDialog schemaLayersDialog(editSchemaView(), this);

	if (schemaLayersDialog.exec() == QDialog::Accepted) {}

	update();
	return;
}

void EditSchemaWidget::setActiveLayer(QString name)
{
	if (schema() == nullptr)
	{
		return;
	}

	if (activeLayer() != nullptr && activeLayer()->name() == name)
	{
		return;
	}

	for (auto layers = schema()->layers(); auto layer : layers)
	{
		if (layer->name() == name)
		{
			editSchemaView()->setActiveLayer(layer);
			editSchemaView()->clearSelection();
			editSchemaView()->update();
			break;
		}
	}

	return;
}

void EditSchemaWidget::contextMenu(const QPoint& pos)
{
	if (mouseState() == MouseState::AddSchemaPosConnectionNextPoint || mouseState() == MouseState::MovingConnectionLinePoint)
	{
		// Don't show context menu, because it's right click for adding next point to connection line
		//
		return;
	}

	// If there is some action, don't show context menu
	//
	if (mouseState() != MouseState::None)
	{
		resetAction();
		setMouseCursor(pos);

		return;
	}

	// Context menu for proposed link.
	//
	const QPointF docPoint = widgetPointToDocument(pos, false);

	{
		auto proposedLinks = editSchemaView()->m_autoFblItemConnection.getPropositions();

		bool clickOnAddRectArea = std::any_of(proposedLinks.begin(),
											  proposedLinks.end(),
											  [&docPoint](const auto& link)
											  {
												  return link.addButtonRect.contains(docPoint);
											  });
		if (clickOnAddRectArea == true)
		{
			QAction action{proposedLinks.size() == 1 ? tr("Add") : tr("Add all %1").arg(proposedLinks.size())};
			QList<QAction*> al{&action};

			auto menuResult = QMenu::exec(al, mapToGlobal(pos));
			if (menuResult != nullptr)
			{
				createProposedAfbLink(proposedLinks);
			}

			return;
		}
	}

	// All selected are signals?
	//
	bool allSelectedAreSignals = selectedItems().empty() == true ? false : true;
	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			allSelectedAreSignals = false;
			break;
		}
	}

	// Selected one ConnectionLine, and click on it's edge
	//
	bool selectedOneConnectionLine = selectedItems().size() == 1 && selectedItems().front()->isType<VFrame30::PosConnectionImpl>();
	bool possibleAddVertexToConnLine = false;
	bool possibleDeleteVertexOnConnLine = false;

	int movingEdgePointIndex = 0;

	if (selectedOneConnectionLine == true)
	{
		// Check if possible to add vertex to connection line
		//
		auto selectedItem = selectedItems()[0];
		SchemaItemAction possibleAction = editSchemaView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

		possibleAddVertexToConnLine =
			possibleAction == SchemaItemAction::MoveHorizontalEdge || possibleAction == SchemaItemAction::MoveVerticalEdge;


		// Can remove end points or point on the flat line
		// ----x----- ok
		//
		VFrame30::IPosConnection* itemConnection = selectedItem->toType<VFrame30::IPosConnection>();
		Q_ASSERT(itemConnection);

		if (possibleAction == SchemaItemAction::MoveConnectionLinePoint)
		{
			std::list<VFrame30::SchemaPoint> points = itemConnection->GetPointList();
			std::vector<VFrame30::SchemaPoint> vp{points.begin(), points.end()};

			if (points.size() > 2)
			{
				if (movingEdgePointIndex == 0 || movingEdgePointIndex == static_cast<int>(points.size() - 1))
				{
					// This is start/end point, can be deleted
					//
					possibleDeleteVertexOnConnLine = true;
				}
				else
				{
					// Is this point on the horz or vert line?
					//
					if (movingEdgePointIndex >= 1 && movingEdgePointIndex < static_cast<int>(vp.size() - 1))
					{
						// Is this point on vert or horz line?
						//
						if ((std::abs(vp[movingEdgePointIndex - 1].X - vp[movingEdgePointIndex].X) < 0.000001 &&
							 (std::abs(vp[movingEdgePointIndex + 1].X - vp[movingEdgePointIndex].X) < 0.000001)) ||
							(std::abs(vp[movingEdgePointIndex - 1].Y - vp[movingEdgePointIndex].Y) < 0.000001 &&
							 (std::abs(vp[movingEdgePointIndex + 1].Y - vp[movingEdgePointIndex].Y) < 0.000001)))
						{
							possibleDeleteVertexOnConnLine = true;
						}
					}
				}
			}
		}
	}


	// Disable some actions in ReadOnly mode
	//
	m_addSubMenu->setDisabled(readOnly());
	updateFileActions();

	m_propertiesAction->setDisabled(editSchemaView()->selectedItems().empty());

	// Compose menu
	//
	QMenu menu{this};

	menu.addMenu(m_fileSubMenu);

	menu.addSeparator();
	menu.addMenu(m_addSubMenu);

	menu.addSeparator();
	menu.addMenu(m_viewSubMenu);

	menu.addMenu(m_editSubMenu);
	menu.addMenu(m_alignSubMenu);
	menu.addMenu(m_orderSubMenu);

	// --
	//
	QList<QAction*> actions;
	if (selectedOneConnectionLine == true)
	{
		Q_ASSERT(selectedItems().size() == 1);

		actions << new QAction{tr("Add Vertex"), &menu};
		actions.back()->setEnabled(possibleAddVertexToConnLine);
		connect(actions.back(),
				&QAction::triggered,
				this,
				[docPoint, this]()
				{
					this->addConnectionLinePoint(selectedItems()[0], docPoint);
				});

		actions << new QAction{tr("Delete Vertex"), &menu};
		actions.back()->setEnabled(possibleDeleteVertexOnConnLine);
		connect(actions.back(),
				&QAction::triggered,
				this,
				[movingEdgePointIndex, this]()
				{
					this->removeConnectionLinePoint(selectedItems()[0], movingEdgePointIndex);
				});

		actions << new QAction{&menu};
		actions.back()->setSeparator(true);
	}

	if (allSelectedAreSignals == true)
	{
		menu.addMenu(m_transformSubMenu);
	}

	// "Selection" -> Select by item type or Afb
	//
	QMenu selectionSubMenu{tr("Selection")};
	if (selectedItems().empty() == false)
	{
		menu.addMenu(&selectionSubMenu);

		std::set<std::pair<QString, QString>> itemTypes; // First class name, second text for menu
		std::set<QString> afbTypes;                      // AfbElement caption

		for (const auto& si : selectedItems())
		{
			QString typeText = QString{si->metaObject()->className()}.replace("VFrame30::SchemaItem", "");
			itemTypes.insert({si->metaObject()->className(), typeText});

			if (si->isSchemaItemAfb() == true)
			{
				afbTypes.insert({si->toSchemaItemAfb()->afbElement().caption()});
			}
		}

		QList<QAction*> typeActions;

		for (const auto& [typeStr, text] : itemTypes)
		{
			QAction* typeAction = new QAction{text, &selectionSubMenu};
			typeActions << typeAction;

			connect(typeAction,
					&QAction::triggered,
					this,
					[typeStr, this]()
					{
						std::list<SchemaItemPtr> selection;
						for (const auto& si : selectedItems())
						{
							if (si->metaObject()->className() == typeStr)
							{
								selection.push_back(si);
							}
						}

						editSchemaView()->setSelectedItems(selection);
						editSchemaView()->update();
					});
		}

		QAction* separator = new QAction{"", &selectionSubMenu};
		separator->setSeparator(true);
		typeActions << separator;

		for (const auto& afbCaption : afbTypes)
		{
			QAction* typeAction = new QAction{afbCaption, &selectionSubMenu};
			typeActions << typeAction;

			connect(typeAction,
					&QAction::triggered,
					this,
					[afbCaption, this]()
					{
						std::list<SchemaItemPtr> selection;
						for (const auto& si : selectedItems())
						{
							if (si->isSchemaItemAfb() == true && si->toSchemaItemAfb()->afbElement().caption() == afbCaption)
							{
								selection.push_back(si);
							}
						}

						editSchemaView()->setSelectedItems(selection);
						editSchemaView()->update();
					});
		}

		selectionSubMenu.addActions(typeActions);
	}

	// Signal properties
	//
	std::set<QString> signalStrIds; // QSet for unique strIds

	if (selectedItems().empty() == false)
	{
		for (auto item : selectedItems())
		{
			auto appSignalProp = item->propertyByCaption(VFrame30::PropertyNames::appSignalIDs);
			if (appSignalProp != nullptr)
			{
				static const QRegularExpression re{"\\s+"};
				auto appSignals = appSignalProp->value().toString().split(re, Qt::SkipEmptyParts);

				std::copy(appSignals.begin(), appSignals.end(), std::inserter(signalStrIds, signalStrIds.end()));
			}
		}

		if (signalStrIds.empty() == false)
		{
			QAction* signalSeparator = new QAction(tr("Signals"), &menu);
			signalSeparator->setSeparator(true);
			actions << signalSeparator;

			for (QString s : signalStrIds)
			{
				QAction* signalAction = new QAction(s, &menu);
				if (signalStrIds.size() == 1) // If not 1, then this shortcut will be added to "All Signals %1 Properties..."
				{
					signalAction->setShortcut(Qt::ALT | Qt::Key_S);
					signalAction->setShortcutVisibleInContextMenu(true);
				}

				connect(signalAction,
						&QAction::triggered,
						[s, this](bool)
						{
							QStringList sl;
							sl << s;
							this->appSignalsProperties(sl);
						});

				actions << signalAction;
			}

			if (signalStrIds.size() > 1)
			{
				QAction* allSignals = new QAction(tr("All Signals %1 Properties...").arg(signalStrIds.size()), &menu);
				allSignals->setShortcut(Qt::ALT | Qt::Key_S);
				allSignals->setShortcutVisibleInContextMenu(true);
				connect(allSignals, &QAction::triggered, this, &EditSchemaWidget::appSignalsSelectedProperties);

				actions << allSignals;
			}
		}
	}

	// Add new Application Logic signal
	//
	if (isLogicSchema() == true)
	{
		if (selectedItems().size() == 1)
		{
			SchemaItemPtr selected = selectedItems().front();

			auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selected.get());

			if (itemSignal != nullptr)
			{
				QAction* addSignal = new QAction(tr("Add New App Signal..."), &menu);
				addSignal->setShortcut(Qt::ALT | Qt::Key_N);
				addSignal->setShortcutVisibleInContextMenu(true);

				// Highlight this menu item if it was selected last time
				//
				QFont f = addSignal->font();
				f.setBold(m_lastSelectedAddSignal);
				m_lastSelectedAddSignal = false;

				addSignal->setFont(f);

				// --
				//
				connect(addSignal,
						&QAction::triggered,
						[this, selected](bool)
						{
							this->addNewAppSignal(selected);
							m_lastSelectedAddSignal = true;
						});

				actions << addSignal;
			}
			else
			{
				// it is not VFrame30::SchemaItemSignal
				//
			}
		}
	}

	// --
	//
	QAction* separatorCommentFind = new QAction(&menu);
	separatorCommentFind->setSeparator(true);

	actions << separatorCommentFind;
	actions << m_toggleCommentAction;
	actions << m_lockAction;
	actions << m_findAction;

	// Layer, Item property etc
	//
	actions << m_separatorAction0;
	actions << m_layersAction;

	if (compareWidget() == true)
	{
		actions << m_compareDiffAction;
	}

	actions << m_propertiesAction;

	menu.addActions(actions);

	menu.exec(mapToGlobal(pos));
	return;
}

void EditSchemaWidget::snapToGridToggled(bool state)
{
	setSnapToGrid(state);
	return;
}

void EditSchemaWidget::exportToPdf()
{
	assert(schema());

	static QString path{"."};
	QString fileName = QFileDialog::getSaveFileName(this,
													"Export schema to PDF",
													path + QDir::separator() + schema()->schemaId() + ".pdf",
													"PDF (*.pdf);;All files (*.*)");

	if (fileName.isEmpty())
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	qDebug() << "Export schema " << schema()->caption() << " " << schema()->schemaId() << " to PDF, " << fileName;

	editSchemaView()->exportToPdf(fileName, theSettings.infoMode());

	return;
}


void EditSchemaWidget::appSignalsSelectedProperties()
{
	return appSignalsProperties(QStringList{});
}

void EditSchemaWidget::appSignalsProperties(QStringList strIds)
{
	static const auto reSplitLines = QRegularExpression("\\s+");

	if (strIds.isEmpty() == true && selectedItems().empty() == false)
	{
		// Get AppSignals from SchemaItems
		//
		std::set<QString> appSignalSet; // set for unique strIds

		for (const auto& item : selectedItems())
		{
			auto prop = item->propertyByCaption(VFrame30::PropertyNames::appSignalIDs);

			if (prop != nullptr)
			{
				QStringList list = prop->value().toString().split(reSplitLines, Qt::SkipEmptyParts);
				std::copy(list.begin(), list.end(), std::inserter(appSignalSet, appSignalSet.end()));
			}
		}

		std::copy(appSignalSet.begin(), appSignalSet.end(), std::back_inserter(strIds));
	}

	if (strIds.isEmpty() == true)
	{
		return;
	}

	// ::editApplicationSignals -- returns vector of pairs or empty vector if edit was cancelled
	//		first: previous AppSignalID
	//		second: new AppSignalID
	//
	std::vector<std::pair<QString, QString>> result = SignalPropertiesDialog::editApplicationSignals(strIds, db(), this);

	if (result.empty() == true)
	{
		// Edit signal properties cancelled
		//
		return;
	}

	std::map<QString, QString> newIdsMap;
	auto isChanged = [](const auto& p)
	{
		return p.first != p.second;
	};

	for (const auto& [k, v] : result | std::views::filter(isChanged))
	{
		newIdsMap[k] = v;
	}

	// in editApplicationSignals AppSignalIds could be changed,
	// update them in selected items, apparently this function was called for selected items.
	//

	// !!! Make a copy of selected items !!!!!
	// As in this loop runSetProperty is called and selection vector is changed,
	// and this loop will crash if it is not a copy
	//
	std::vector<SchemaItemPtr> selected = selectedItems();

	for (auto item : selected)
	{
		auto prop = item->propertyByCaption(VFrame30::PropertyNames::appSignalIDs);

		if (prop != nullptr)
		{
			QStringList signalList = prop->value().toString().split(reSplitLines, Qt::KeepEmptyParts);
			bool itemsSignalsWereChanged = false;

			for (QString& appSignalId : signalList)
			{
				auto foundInChanged = newIdsMap.find(appSignalId);

				if (foundInChanged != newIdsMap.end())
				{
					// AppSignalIdWasChanged
					//
					appSignalId = foundInChanged->second; // appSignalId is a reference
					itemsSignalsWereChanged = true;
				}
			}

			if (itemsSignalsWereChanged == true)
			{
				QString oneStringIds = signalList.join(QChar::LineFeed);
				SchemaItemPtr itemPtrCopy(item);
				m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), itemPtrCopy);
			}
		}
	}

	return;
}

void EditSchemaWidget::addNewAppSignalSelected()
{
	if (isLogicSchema() == false || selectedItems().size() != 1)
	{
		return;
	}

	SchemaItemPtr selected = selectedItems().front();
	auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selected.get());

	if (itemSignal == nullptr)
	{
		// it is not VFrame30::SchemaItemSignal
		//
		return;
	}

	return addNewAppSignal(selected);
}

void EditSchemaWidget::addNewAppSignal(SchemaItemPtr schemaItem)
{
	if (isLogicSchema() == false || schemaItem == nullptr || dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) == nullptr)
	{
		Q_ASSERT(isLogicSchema() == false);
		Q_ASSERT(schemaItem);
		Q_ASSERT(dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get()) != nullptr);
		return;
	}

	QStringList equipmentIdList = logicSchema()->equipmentIdList();
	if (equipmentIdList.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Cannot create Application Signal as schema property EquipmentIDs is empty."));
		return;
	}

	const VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get());

	QStringList itemsAppSignals = signalItem->appSignalIdList();

	if (itemsAppSignals.size() == 1 &&
		(itemsAppSignals[0] == QLatin1String("#OUT_STRID") || // Not good, subject to change. must get default signals value from somewhere
		 itemsAppSignals[0] == QLatin1String("#IN_STRID") || itemsAppSignals[0] == QLatin1String("#APPSIGNALID")))
	{
		// This is just created signal item
		//
		itemsAppSignals.clear(); // clear - means generate new AppSignalIds
	}

	m_createSignalDialogOptions.init(schema()->schemaId(), schema()->caption(), equipmentIdList, itemsAppSignals);

	QStringList signalsIds = CreateSignalDialog::showDialog(db(), &m_createSignalDialogOptions, this);

	if (signalsIds.isEmpty() == false)
	{
		// Set value
		//
		QString oneStringIds;
		for (QString s : signalsIds)
		{
			oneStringIds += s + QChar::LineFeed;
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), schemaItem);
	}

	//--------------------------------------------------
	//	QStringList equipmentIdList = logicSchema()->equipmentIdList();
	//	if (equipmentIdList.isEmpty() == true)
	//	{
	//		QMessageBox::critical(this, qAppName(), tr("Cannot create Application Signal as schema property EquipmentIDs is empty."));
	//		return;
	//	}

	//	const VFrame30::SchemaItemSignal* signalItem = dynamic_cast<VFrame30::SchemaItemSignal*>(schemaItem.get());
	//	QStringList itemsAppSignals = signalItem->appSignalIdList();

	//	if (itemsAppSignals.size() == 1 &&
	//		(itemsAppSignals[0] == QLatin1String("#OUT_STRID")) ||
	//		(itemsAppSignals[0] == QLatin1String("#IN_STRID")) ||
	//		(itemsAppSignals[0] == QLatin1String("#APPSIGNALID")))
	//	{
	//	}
	//	else
	//	{
	//		m_createSignalOptions.appSignalIdList = itemsAppSignals;
	//	}

	//	int counterValue = 0;
	//	bool nextValRes = db()->nextCounterValue(&counterValue);
	//	if (nextValRes == false)
	//	{
	//		return;
	//	}

	//	m_createSignalOptions.lmEquipmentIdList = equipmentIdList;

	//	QStringList signalsIds = SignalsTabPage::createSignal(db(),
	//														  counterValue,
	//														  schema()->schemaId(),
	//														  schema()->caption(),
	//														  &m_createSignalOptions,
	//														  this);

	//	if (signalsIds.isEmpty() == false)
	//	{
	//		// Set value
	//		//
	//		QString oneStringIds;
	//		for (QString s : signalsIds)
	//		{
	//			oneStringIds += s + QChar::LineFeed;
	//		}

	//		m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(oneStringIds), schemaItem);
	//	}

	return;
}

void EditSchemaWidget::escapeKey()
{
	if (mouseState() != MouseState::None)
	{
		resetAction();
	}
	else
	{
		editSchemaView()->clearSelection();
	}

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::f2Key()
{
	if (mouseState() != MouseState::None)
	{
		return;
	}

	const std::vector<SchemaItemPtr>& selected = selectedItems();
	if (selected.size() != 1)
	{
		return;
	}

	SchemaItemPtr item = selected.at(0);
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	F2KeyForSchemaItem f2KeyForSchemaItem{db(), m_editEngine, editSchemaView(), this};
	f2KeyForSchemaItem.show(item);
	return;
}

void EditSchemaWidget::deleteKey()
{
	auto items = editSchemaView()->selectedNonLockedItems();

	m_editEngine->runDeleteItem(items, activeLayer());

	return;
}

void EditSchemaWidget::undo()
{
	m_nextSelectionFromLeft = {}; // Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	m_editEngine->undo();

	if (m_schemaPropertiesDialog != nullptr && m_schemaPropertiesDialog->isVisible())
	{
		m_schemaPropertiesDialog->setSchema(schemaSharedPtr());
	}
}

void EditSchemaWidget::redo()
{
	m_nextSelectionFromLeft = {}; // Reset state for slection next items via Alt + Arrow Keys
	m_nextSelectionFromRight = {};

	m_editEngine->redo();

	if (m_schemaPropertiesDialog != nullptr && m_schemaPropertiesDialog->isVisible())
	{
		m_schemaPropertiesDialog->setSchema(schemaSharedPtr());
	}
}

void EditSchemaWidget::editEngineStateChanged(bool canUndo, bool canRedo)
{
	if (m_undoAction == nullptr || m_redoAction == nullptr)
	{
		assert(m_undoAction);
		assert(m_redoAction);
		return;
	}

	m_undoAction->setEnabled(canUndo);
	m_redoAction->setEnabled(canRedo);

	return;
}

void EditSchemaWidget::modifiedChangedSlot(bool modified)
{
	m_fileSaveAction->setEnabled(modified);
	emit modifiedChanged(modified);
	return;
}

void EditSchemaWidget::selectAll()
{
	editSchemaView()->clearSelection();

	std::vector<SchemaItemPtr> items;
	items.assign(editSchemaView()->activeLayer()->items().begin(), editSchemaView()->activeLayer()->items().end());

	editSchemaView()->setSelectedItems(items);

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::selectItem(SchemaItemPtr item)
{
	editSchemaView()->clearSelection();

	std::vector<SchemaItemPtr> items;
	items.push_back(item);
	editSchemaView()->setSelectedItems(items);

	editSchemaView()->update();
	return;
}

void EditSchemaWidget::selectItems(std::vector<SchemaItemPtr> items)
{
	editSchemaView()->clearSelection();
	editSchemaView()->setSelectedItems(items);
	editSchemaView()->update();
	return;
}

void EditSchemaWidget::editCut()
{
	// Cut schema item(s) to clipboard
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	const std::vector<SchemaItemPtr> selected = selectedNonLockedItems();

	// Save to protobuf message
	//
	::Proto::EnvelopeSet message;
	for (SchemaItemPtr si : selected)
	{
		::Proto::Envelope* protoSchemaItem = message.add_items();
		si->Save(protoSchemaItem);
	}

	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		assert(ok);
		return;
	}

	// Delete items, it a cut operation
	//
	m_editEngine->runDeleteItem(selected, editSchemaView()->activeLayer());

	// Set data to clipboard
	//
	QByteArray ba(dataString.data(), static_cast<int>(dataString.size()));

	if (ba.isEmpty() == false)
	{
		QClipboard* clipboard = QApplication::clipboard();

		QMimeData* mime = new QMimeData();
		mime->setData(SchemaItemClipboardData::mimeType, ba);

		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	return;
}


void EditSchemaWidget::editCopy()
{
	// Copy schema item(s) to clipboard
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	const std::vector<SchemaItemPtr>& selected = selectedItems();

	// Save to protobuf message
	//
	::Proto::EnvelopeSet message;
	message.mutable_items()->Reserve(static_cast<int>(selected.size()));
	for (SchemaItemPtr si : selected)
	{
		::Proto::Envelope* protoSchemaItem = message.add_items();
		si->Save(protoSchemaItem);
	}

	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		assert(ok);
		return;
	}

	// Set data to clipboard
	//
	QByteArray ba(dataString.data(), static_cast<int>(dataString.size()));

	if (ba.isEmpty() == false)
	{
		QClipboard* clipboard = QApplication::clipboard();

		QMimeData* mime = new QMimeData();
		mime->setData(SchemaItemClipboardData::mimeType, ba);

		// If selected one image some additional text copy can happen
		//
		if (selectedItems().size() == 1)
		{
			SchemaItemPtr si = selectedItems().front();

			// If selected one image, then copy to clipboard it also as image
			//
			if (si->isType<VFrame30::SchemaItemImage>() == true)
			{
				VFrame30::SchemaItemImage* imageItem = si->toType<VFrame30::SchemaItemImage>();
				mime->setImageData(imageItem->image());
			}

			// If selected one SchemaItemSignal, then copy to clipboard AppSignalID
			//
			if (VFrame30::SchemaItemSignal* itemSignal = si->toType<VFrame30::SchemaItemSignal>(); itemSignal != nullptr)
			{
				mime->setText(itemSignal->appSignalIds());
			}

			// If selected one SchemaItemRect, then copy to clipboard rect text
			//
			if (VFrame30::SchemaItemRect* itemRect = si->toType<VFrame30::SchemaItemRect>();
				itemRect != nullptr && itemRect->text().isEmpty() == false)
			{
				mime->setText(itemRect->text());
			}

			// If selected one SchemaItemRect, then copy to clipboard rect text
			//
			if (VFrame30::SchemaItemLoopback* itemLoopback = si->toType<VFrame30::SchemaItemLoopback>(); itemLoopback != nullptr)
			{
				mime->setText(itemLoopback->loopbackId());
			}

			// If selected one SchemaItemLoopback, then copy to clipboard loopbackId
			//
			if (VFrame30::SchemaItemLoopback* itemLoopback = si->toType<VFrame30::SchemaItemLoopback>(); itemLoopback != nullptr)
			{
				mime->setText(itemLoopback->loopbackId());
			}

			// If selected one SchemaItemTransmitter, then copy to clipboard connectionId
			//
			if (VFrame30::SchemaItemTransmitter* itemTransmitter = si->toType<VFrame30::SchemaItemTransmitter>();
				itemTransmitter != nullptr)
			{
				mime->setText(itemTransmitter->connectionIds());
			}

			// If selected one SchemaItemReceiver, then copy to clipboard appSignalIds
			//
			if (VFrame30::SchemaItemReceiver* itemReceiver = si->toType<VFrame30::SchemaItemReceiver>(); itemReceiver != nullptr)
			{
				mime->setText(itemReceiver->appSignalIds());
			}

			// If selected one SchemaItemBus, then copy to clipboard busTypeId
			//
			if (VFrame30::SchemaItemBus* itemBus = si->toType<VFrame30::SchemaItemBus>(); itemBus != nullptr)
			{
				mime->setText(itemBus->busTypeId());
			}
		}

		// --
		//
		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	// if selected one item, and it has some text data (like InputSchemaItem has appSignalID),
	// then copy this data to clipboard as text
	//

	return;
}

void EditSchemaWidget::editPaste()
{
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		return;
	}

	// Paste schema items
	//
	if (mimeData->hasFormat(SchemaItemClipboardData::mimeType) == true)
	{
		QByteArray cbData = mimeData->data(SchemaItemClipboardData::mimeType);

		::Proto::EnvelopeSet message;
		bool ok = message.ParseFromArray(cbData.constData(), static_cast<int>(cbData.size()));

		if (ok == false)
		{
			QMessageBox::critical(this,
								  qApp->applicationName(),
								  tr("Clipboard has Schema Items, but it seems that data corrupted or data has incompatible format."));
			return;
		}

		std::list<SchemaItemPtr> itemList;
		std::list<SchemaItemPtr> itemsToRemove;

		bool schemaItemAfbIsPresent = false;
		bool schemaItemUfbIsPresent = false;
		bool schemaItemBusIsPresent = false;
		bool schemaItemConnectionIsPresent = false;
		bool schemaItemInOutIsPresent = false;

		for (int i = 0; i < message.items_size(); i++)
		{
			const ::Proto::Envelope& schemaItemMessage = message.items(i);

			SchemaItemPtr schemaItem = VFrame30::SchemaItem::Create(schemaItemMessage);

			if (schemaItem == nullptr)
			{
				Q_ASSERT(schemaItem);
				continue;
			}

			// --
			//
			if (schemaItem->itemUnit() != schema()->unit())
			{
				continue; // No transform, the problem is: we need to transform pos, height, points pos
						  // it's all ok, BUT also we need to transform lineWeight, TextSize and maybe something else
						  // and these properties can vary from item to item.
			}

			// --
			//
			schemaItem->setNewGuid();
			itemList.push_back(schemaItem);

			// Ensure item is visible after paste
			//
			for (SchemaItemPtr& item : itemList)
			{
				VFrame30::ISchemaItemPropertiesPos* pos = item.get();
				VFrame30::PosRectRotatable* rotatableItem = item->toType<VFrame30::PosRectRotatable>();

				if (rotatableItem != nullptr)
				{
					std::array<QPointF, 21> pointsForEnsuringVisible{
						QPointF{schema()->docWidthRegional() / 2.0, schema()->docHeightRegional() / 2.0},
						QPointF{.0, .0},
						QPointF{schema()->docWidthRegional(), .0},
						QPointF{schema()->docWidthRegional(), schema()->docHeightRegional()},
						QPointF{.0, schema()->docHeightRegional()},
						QPointF{schema()->docWidthRegional() / 2.0, .0},
						QPointF{schema()->docWidthRegional(), schema()->docHeightRegional() / 2.0},
						QPointF{schema()->docWidthRegional() / 2.0, schema()->docHeightRegional()},
						QPointF{.0, schema()->docHeightRegional() / 2.0},
						QPointF{.0, -pos->height()},
						QPointF{-pos->width(), .0},
						QPointF{.0, +pos->height()},
						QPointF{+pos->width(), .0},
						QPointF{schema()->docWidthRegional(), -pos->height()},
						QPointF{schema()->docWidthRegional() - pos->width(), .0},
						QPointF{schema()->docWidthRegional(), +pos->height()},
						QPointF{schema()->docWidthRegional() - pos->width(), schema()->docHeightRegional()},
						QPointF{schema()->docWidthRegional(), schema()->docHeightRegional() - pos->height()},
						QPointF{schema()->docWidthRegional() - pos->width(), schema()->docHeightRegional()},
						QPointF{.0, schema()->docHeightRegional() - pos->height()},
						QPointF{-pos->width(), schema()->docHeightRegional()}};

					for (const QPointF& pointForApply : pointsForEnsuringVisible)
					{
						bool isVisible = rotatableItem->isIntersectRect(0, 0, schema()->docWidth(), schema()->docHeight());
						if (isVisible == true)
						{
							break;
						}

						pos->setLeft(pointForApply.x());
						pos->setTop(pointForApply.y());
					}

					bool isVisible = rotatableItem->isIntersectRect(0, 0, schema()->docWidth(), schema()->docHeight());
					if (isVisible == false)
					{
						// I give up, just remove it.
						//
						itemsToRemove.push_back(item);
					}

					continue;
				}

				// This is not rotatable item here everything is simple.
				//
				if (pos->left() + pos->width() > schema()->docWidthRegional())
				{
					pos->setLeft(schema()->docWidthRegional() - pos->width());
				}

				if (pos->left() < 0)
				{
					pos->setLeft(0);
				}

				if (pos->top() + pos->height() > schema()->docHeightRegional())
				{
					pos->setTop(schema()->docHeightRegional() - pos->height());
				}

				if (pos->top() < 0)
				{
					pos->setTop(0);
				}
			}

			// --
			//
			schemaItemAfbIsPresent |= schemaItem->isSchemaItemAfb();
			schemaItemUfbIsPresent |= schemaItem->isType<VFrame30::SchemaItemUfb>();
			schemaItemBusIsPresent |= schemaItem->isType<VFrame30::SchemaItemBus>();
			schemaItemConnectionIsPresent |= schemaItem->isType<VFrame30::SchemaItemConnection>();
			schemaItemInOutIsPresent |= schemaItem->isType<VFrame30::SchemaItemInOut>();

			{
				// set label to it
				//
				int counterValue = 0;
				bool nextValRes = db()->nextCounterValue(&counterValue);
				if (nextValRes == false)
				{
					return;
				}

				schemaItem->setLabel(schema()->schemaId() + "_" + QString::number(counterValue));
			}
		}

		if (itemsToRemove.empty() == false)
		{
			for (SchemaItemPtr& item : itemsToRemove)
			{
				itemList.remove(item);
			}

			QMessageBox::critical(this, qAppName(), tr("Some items cannot correctly be placed and were not added to the schema."));
		}

		if (itemList.empty() == false)
		{
			if (schema()->isUfbSchema() == true &&
				(schemaItemUfbIsPresent == true || schemaItemConnectionIsPresent == true || schemaItemInOutIsPresent == true))
			{
				QMessageBox::critical(this,
									  qAppName(),
									  tr("Adding In/Outs, Transmitters/Receivers, User Functional Blocks to UFB Schema is impossible."));
				return;
			}

			// Add items
			//
			m_editEngine->runAddItem(itemList, editSchemaView()->activeLayer());

			bool defaultsWereSet = setDefaultItemProperties(itemList);
			if (defaultsWereSet == true)
			{
				// Undo one time that user can select what to take, a clear item copy or modified with default properties.
				//
				undo();
			}
		}

		// If new items has different afb/ufb description version
		// then they will be updated to the current version
		//
		if (schemaItemAfbIsPresent == true)
		{
			updateAfbsForSchema();
		}

		if (schemaItemUfbIsPresent == true)
		{
			updateUfbsForSchema();
		}

		if (schemaItemBusIsPresent == true)
		{
			updateBussesForSchema();
		}

		return;
	}

	// Specific pastes
	//
	const std::vector<SchemaItemPtr>& selected = editSchemaView()->selectedItems();

	// --
	//
	if (selected.empty() == true)
	{
		return;
	}

	// Paste image to selected SchemaItemImage
	//
	if (mimeData->hasImage() == true)
	{
		bool allItemsAreImages = true;

		for (SchemaItemPtr item : selected)
		{
			VFrame30::SchemaItemImage* imageItem = dynamic_cast<VFrame30::SchemaItemImage*>(item.get());

			if (imageItem == nullptr)
			{
				allItemsAreImages = false;
				break;
			}
		}

		if (allItemsAreImages == true)
		{
			QImage image = qvariant_cast<QImage>(mimeData->imageData());

			for (SchemaItemPtr item : selected)
			{
				VFrame30::SchemaItemImage* imageItem = dynamic_cast<VFrame30::SchemaItemImage*>(item.get());
				Q_ASSERT(imageItem);

				m_editEngine->runSetProperty(VFrame30::PropertyNames::image, QVariant(image), selected);
			}

			return;
		}
	}

	// All other items receives only text
	//
	if (mimeData->hasText() == false)
	{
		return;
	}

	// Paste text to SchemaItemConst
	//
	{
		bool allItemsAreConsts = true;

		bool okInteger = false;
		bool okFloat = false;
		bool okDiscrete = false;

		int constInt = mimeData->text().toInt(&okInteger);
		double constFloat = mimeData->text().toDouble(&okFloat);
		int constDiscrete = mimeData->text().toInt(&okDiscrete);

		std::vector<SchemaItemPtr> constIntItems;
		constIntItems.reserve(selected.size());

		std::vector<SchemaItemPtr> constFloatItems;
		constFloatItems.reserve(selected.size());

		std::vector<SchemaItemPtr> constDiscreteItems;
		constDiscreteItems.reserve(selected.size());

		for (SchemaItemPtr item : selected)
		{
			VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());

			if (constItem == nullptr)
			{
				allItemsAreConsts = false;
				break;
			}

			switch (constItem->type())
			{
			case VFrame30::SchemaItemConst::ConstType::IntegerType:
				if (okInteger == true)
				{
					constIntItems.push_back(item);
				}
				break;

			case VFrame30::SchemaItemConst::ConstType::FloatType:
				if (okFloat == true)
				{
					constFloatItems.push_back(item);
				}
				break;

			case VFrame30::SchemaItemConst::ConstType::Discrete:
				if (okDiscrete == true)
				{
					constDiscreteItems.push_back(item);
				}
				break;

			default:
				assert(false);
				allItemsAreConsts = false;
			}
		}

		if (allItemsAreConsts == true)
		{
			if (okInteger == true && constIntItems.empty() == false)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::valueInteger, QVariant(constInt), constIntItems);
			}

			if (okFloat == true && constFloatItems.empty() == false)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::valueFloat, QVariant(constFloat), constFloatItems);
			}

			if (okDiscrete == true && constDiscreteItems.empty() == false)
			{
				m_editEngine->runSetProperty(VFrame30::PropertyNames::valueDiscrete, QVariant(constDiscrete), constDiscreteItems);
			}

			return;
		}
	}

	// Paste text to SchemaItemRect
	//
	{
		bool allItemsAreRects = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemRect*>(item.get()) == nullptr)
			{
				allItemsAreRects = false;
				break;
			}
		}

		if (allItemsAreRects == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::text, QVariant(mimeData->text()), selected);
			return;
		}
	}

	// Paste appSignalID to SchemaItemSignal
	//
	{
		bool allItemsAreSignals = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) == nullptr)
			{
				allItemsAreSignals = false;
				break;
			}
		}

		if (allItemsAreSignals == true && mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(mimeData->text()), selected);
			return;
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemReceiver
	//
	{
		bool allItemsAreReceivers = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) == nullptr)
			{
				allItemsAreReceivers = false;
				break;
			}
		}

		if (allItemsAreReceivers == true && mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(mimeData->text()), selected);
			return;
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemLoopback
	//
	{
		bool allItemsAreLoopbacks = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get()) == nullptr)
			{
				allItemsAreLoopbacks = false;
				break;
			}
		}

		if (allItemsAreLoopbacks == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::loopbackId, QVariant(mimeData->text()), selected);
			F2KeyForSchemaItem::m_lastUsedLoopbackId = mimeData->text();
			return;
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemValue
	//
	{
		bool allItemsAreValues = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemValue*>(item.get()) == nullptr)
			{
				allItemsAreValues = false;
				break;
			}
		}

		if (allItemsAreValues == true && mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(mimeData->text()), selected);
			return;
		}
	}

	// Paste appSignalID to VFrame30::SchemaItemImageValue
	//
	{
		bool allItemsAreImageValues = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get()) == nullptr)
			{
				allItemsAreImageValues = false;
				break;
			}
		}

		if (allItemsAreImageValues == true && mimeData->text().startsWith('#') == true)
		{
			m_editEngine->runSetProperty(VFrame30::PropertyNames::appSignalIDs, QVariant(mimeData->text()), selected);
			return;
		}
	}

	return;
}

void EditSchemaWidget::schemaProperties()
{
	if (m_schemaPropertiesDialog == nullptr)
	{
		m_schemaPropertiesDialog = new SchemaPropertiesDialog(m_editEngine, db(), this);
	}

	m_schemaPropertiesDialog->setSchema(schemaSharedPtr());
	m_schemaPropertiesDialog->show();
	return;
}

void EditSchemaWidget::properties()
{
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemaItemPropertiesDialog(m_editEngine, db(), this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemaView()->selectedItems());
	m_itemsPropertiesDialog->setReadOnly(m_editEngine->readOnly());

	m_itemsPropertiesDialog->show();
	m_itemsPropertiesDialog->ensureVisible();

	m_itemsPropertiesDialog->raise();
	m_itemsPropertiesDialog->activateWindow();

	return;
}

void EditSchemaWidget::compareSchemaItem()
{
	if (editSchemaView()->m_compareWidget == false || editSchemaView()->m_compareSourceSchema == nullptr ||
		editSchemaView()->m_compareTargetSchema == nullptr)
	{
		assert(editSchemaView()->m_compareWidget == true);
		assert(editSchemaView()->m_compareSourceSchema);
		assert(editSchemaView()->m_compareTargetSchema);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	SchemaItemPtr selectedItem = selectedItems().front();
	if (selectedItem == nullptr)
	{
		assert(selectedItem);
		return;
	}

	SchemaItemPtr sourceItem = editSchemaView()->m_compareSourceSchema->getItemById(selectedItem->guid());
	SchemaItemPtr targetItem = editSchemaView()->m_compareTargetSchema->getItemById(selectedItem->guid());

	if (sourceItem == nullptr || targetItem == nullptr)
	{
		return;
	}

	DbChangesetObject dbObject; // Fake object, need to fill only name

	QString title;

	if (selectedItem->label().isEmpty() == false)
	{
		title = selectedItem->metaObject()->className() + QString(" ") + selectedItem->label();
		title = title.remove("VFrame30::SchemaItem");
	}
	else
	{
		title = selectedItem->metaObject()->className();
		title = title.remove("VFrame30::SchemaItem");
	}
	dbObject.setName(title);

	CompareData cd = CompareData();
	ComparePropertyObjectDialog::showDialog(dbObject, cd, sourceItem, targetItem, this);

	return;
}

void EditSchemaWidget::selectionChanged()
{
	// Properties dialog
	//
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemaItemPropertiesDialog(m_editEngine, db(), this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemaView()->selectedItems());
	m_itemsPropertiesDialog->setReadOnly(m_editEngine->readOnly());
	m_itemsPropertiesDialog->ensureVisible();

	const std::vector<SchemaItemPtr>& selected = selectedItems();
	auto selectedNotLocked = selectedNonLockedItems();

	// Edit Menu
	//
	m_deleteAction->setEnabled(selectedNotLocked.empty() == false && readOnly() == false);
	m_editCutAction->setEnabled(selectedNotLocked.empty() == false && readOnly() == false);
	m_editCopyAction->setEnabled(selected.empty() == false);

	// Align
	//
	bool allowAlign = selectedNotLocked.size() >= 2 && readOnly() == false;

	m_alignLeftAction->setEnabled(allowAlign);
	m_alignRightAction->setEnabled(allowAlign);
	m_alignTopAction->setEnabled(allowAlign);
	m_alignBottomAction->setEnabled(allowAlign);

	// Size
	//
	std::vector<SchemaItemPtr> selectedFiltered;

	for (SchemaItemPtr item : selectedNotLocked)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr || dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	bool allowSize = selectedFiltered.size() >= 2 && readOnly() == false;

	m_sameWidthAction->setEnabled(allowSize);
	m_sameHeightAction->setEnabled(allowSize);
	m_sameSizeAction->setEnabled(allowSize);

	// Order
	//
	bool allowSetOrder = selectedNotLocked.empty() == false && readOnly() == false;

	m_bringToFrontAction->setEnabled(allowSetOrder);
	m_bringForwardAction->setEnabled(allowSetOrder);
	m_sendToBackAction->setEnabled(allowSetOrder);
	m_sendBackwardAction->setEnabled(allowSetOrder);

	// Comment Action
	//
	bool hasFblItems = std::any_of(selected.begin(),
								   selected.end(),
								   [](const auto& item)
								   {
									   return item->isFblItem();
								   });

	m_toggleCommentAction->setEnabled(hasFblItems && readOnly() == false);

	// Lock action
	//
	m_lockAction->setEnabled(selected.empty() == false && readOnly() == false);

	// Compare SchemaItem
	//
	m_compareDiffAction->setEnabled(selected.size() == 1 && isCompareWidget() == true);

	// --
	//
	clipboardDataChanged();

	// AppSignal properties
	//
	{
		std::set<QString> selectedSignalStrIds; // Set for unique strIds

		for (const auto& item : selectedItems())
		{
			auto appSignalProp = item->propertyByCaption(VFrame30::PropertyNames::appSignalIDs);
			if (appSignalProp != nullptr)
			{
				static const QRegularExpression re{"\\s+"};
				auto appSignals = appSignalProp->value().toString().split(re, Qt::SkipEmptyParts);

				std::copy(appSignals.begin(), appSignals.end(), std::inserter(selectedSignalStrIds, selectedSignalStrIds.end()));
			}
		}

		m_appSignalPropertiesAction->setEnabled(selectedSignalStrIds.empty() == false);
	}

	// Add new Application Logic signal
	//
	bool enableAddAppSignal = false;

	if (isLogicSchema() == true)
	{
		if (selectedItems().size() == 1)
		{
			SchemaItemPtr selectedFrontItem = selectedItems().front();

			auto itemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(selectedFrontItem.get());

			if (itemSignal != nullptr)
			{
				enableAddAppSignal = true;
			}
			else
			{
				// it is not VFrame30::SchemaItemSignal
				//
			}
		}
	}

	m_addAppSignalAction->setEnabled(enableAddAppSignal);

	return;
}

void EditSchemaWidget::clipboardDataChanged()
{
	if (readOnly() == true)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	// if same SchemaItems in the clipboard
	//
	QStringList hasFormats = mimeData->formats();

	for (auto f : hasFormats)
	{
		if (f == SchemaItemClipboardData::mimeType)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// Specific items cases
	//
	const std::vector<SchemaItemPtr>& selected = editSchemaView()->selectedItems();

	if (selected.empty() == true)
	{
		m_editPasteAction->setEnabled(false);
		return;
	}

	// if SchemaItemImage is(are) selected and Image is in the clipboard
	//
	bool allItemsAreImages = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemImage*>(item.get()) == nullptr)
		{
			allItemsAreImages = false;
			break;
		}
	}

	if (allItemsAreImages == true && mimeData->hasImage() == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// All Items are SchemaItemConsts
	//
	bool allItemsAreConsts = true;

	bool okInteger = false;
	bool okFloat = false;

	mimeData->text().toInt(&okInteger);
	mimeData->text().toDouble(&okFloat);

	std::vector<SchemaItemPtr> constIntItems;
	constIntItems.reserve(selected.size());

	std::vector<SchemaItemPtr> constFloatItems;
	constFloatItems.reserve(selected.size());

	std::vector<SchemaItemPtr> constDiscreteItems;
	constDiscreteItems.reserve(selected.size());

	for (SchemaItemPtr item : selected)
	{
		VFrame30::SchemaItemConst* constItem = dynamic_cast<VFrame30::SchemaItemConst*>(item.get());

		if (constItem == nullptr)
		{
			allItemsAreConsts = false;
			break;
		}

		switch (constItem->type())
		{
		case VFrame30::SchemaItemConst::ConstType::IntegerType:
			if (okInteger == true)
			{
				constIntItems.push_back(item);
			}
			break;

		case VFrame30::SchemaItemConst::ConstType::FloatType:
			if (okFloat == true)
			{
				constFloatItems.push_back(item);
			}
			break;
		case VFrame30::SchemaItemConst::ConstType::Discrete:
			if (okInteger == true)
			{
				constDiscreteItems.push_back(item);
			}
			break;

		default:
			assert(false);
			allItemsAreConsts = false;
		}
	}

	if (allItemsAreConsts == true)
	{
		if ((okInteger == true && constIntItems.empty() == false) || (okFloat == true && constFloatItems.empty() == false) ||
			(okInteger == true && constDiscreteItems.empty() == false))
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// if SchemaItemRect is selected and Text is in the clipboard
	//
	bool allItemsAreRects = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemRect*>(item.get()) == nullptr)
		{
			allItemsAreRects = false;
			break;
		}
	}

	if (allItemsAreRects == true && mimeData->hasText() == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemSignal is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreSignals = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemSignal*>(item.get()) == nullptr)
		{
			allItemsAreSignals = false;
			break;
		}
	}

	if (allItemsAreSignals == true && mimeData->hasText() == true && mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemReceiver is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreReceivers = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get()) == nullptr)
		{
			allItemsAreReceivers = false;
			break;
		}
	}

	if (allItemsAreReceivers == true && mimeData->hasText() == true && mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemLoopback is selected and Text is in the clipboard
	//
	bool allItemsAreLoopbacks = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemLoopback*>(item.get()) == nullptr)
		{
			allItemsAreLoopbacks = false;
			break;
		}
	}

	if (allItemsAreLoopbacks == true && mimeData->hasText() == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemValue is selected and AppSignalID is in the clipboard
	//
	bool allItemsAreValues = true;
	for (SchemaItemPtr item : selected)
	{
		if (dynamic_cast<VFrame30::SchemaItemValue*>(item.get()) == nullptr)
		{
			allItemsAreValues = false;
			break;
		}
	}

	if (allItemsAreValues == true && mimeData->hasText() == true && mimeData->text().startsWith('#') == true)
	{
		m_editPasteAction->setEnabled(true);
		return;
	}

	// if Any SchemaItemImageValue is selected and AppSignalID is in the clipboard
	//
	{
		bool allItemsAreImageValues = true;
		for (SchemaItemPtr item : selected)
		{
			if (dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get()) == nullptr)
			{
				allItemsAreImageValues = false;
				break;
			}
		}

		if (allItemsAreImageValues == true && mimeData->hasText() == true && mimeData->text().startsWith('#') == true)
		{
			m_editPasteAction->setEnabled(true);
			return;
		}
	}

	// --
	//
	m_editPasteAction->setEnabled(false);

	return;
}

void EditSchemaWidget::addConnection()
{
	QMenu menu{this};
	menu.addAction(m_addTransmitter);
	menu.addAction(m_addReceiver);

	menu.exec(QCursor::pos());
}

void EditSchemaWidget::addTransmitter()
{
	if (isLogicSchema() == false)
	{
		return;
	}

	auto schemaItem = std::make_shared<VFrame30::SchemaItemTransmitter>(schema()->unit());

	F2KeyForSchemaItem f2KeyForSchemaItem{db(), m_editEngine, editSchemaView(), this};

	bool ok = f2KeyForSchemaItem.f2KeyForTransmitter(schemaItem, *schema()->toLogicSchema(), false);
	if (ok == false)
	{
		return;
	}

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addReceiver()
{
	if (isLogicSchema() == false)
	{
		return;
	}

	auto schemaItem = std::make_shared<VFrame30::SchemaItemReceiver>(schema()->unit());

	F2KeyForSchemaItem f2KeyForSchemaItem{db(), m_editEngine, editSchemaView(), this};

	bool ok = f2KeyForSchemaItem.f2KeyForReceiver(schemaItem, *schema()->toLogicSchema(), false);
	if (ok == false)
	{
		return;
	}

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addLoopback()
{
	QMenu menu{this};
	menu.addAction(m_addLoopbackSource);
	menu.addAction(m_addLoopbackTarget);

	menu.exec(QCursor::pos());
}

void EditSchemaWidget::addLoopbackSource()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemLoopbackSource>(schema()->unit());

	QString loopbackId = QString("LBID_%1").arg(db()->nextCounterValue());
	F2KeyForSchemaItem::m_lastUsedLoopbackId = loopbackId;

	schemaItem->setLoopbackId(loopbackId);

	addItem(schemaItem);
	return;
}

void EditSchemaWidget::addLoopbackTarget()
{
	auto schemaItem = std::make_shared<VFrame30::SchemaItemLoopbackTarget>(schema()->unit());

	if (F2KeyForSchemaItem::m_lastUsedLoopbackId.isEmpty() == false)
	{
		schemaItem->setLoopbackId(F2KeyForSchemaItem::m_lastUsedLoopbackId);
	}

	addItem(schemaItem);
}

void EditSchemaWidget::addAfbElement()
{
	// Get Afb descriptions
	//
	std::vector<std::shared_ptr<Afb::AfbElement>> afbs;
	bool ok = loadAfbsDescriptions(&afbs);

	if (ok == false)
	{
		QMessageBox::critical(this, QObject::tr("Error"), tr("Cannot load AFB descriptions! Check LmDescriptionFile."));
		return;
	}

	// --
	//
	ChooseAfbDialog* dialog = new ChooseAfbDialog(afbs, this);

	if (dialog->exec() == QDialog::Accepted)
	{
		int index = dialog->index();

		if (index < 0 || static_cast<size_t>(index) >= afbs.size())
		{
			assert(false);
			return;
		}

		std::shared_ptr<Afb::AfbElement> afb = afbs[index];

		QString errorMsg;
		auto newSchemaItem = std::make_shared<VFrame30::SchemaItemAfb>(schema()->unit(), *(afb.get()), &errorMsg);

		if (errorMsg.isEmpty() == false)
		{
			QMessageBox::critical(this, QObject::tr("Error"), errorMsg);
			return;
		}

		if (newSchemaItem->isPackedLogic() == true)
		{
			// Assign new PackedLogicId
			//
			QString packedLogicId = QString("%1%2").arg(newSchemaItem->packedLogic().idPrefix).arg(db()->nextCounterValue());
			newSchemaItem->setPackedLogicId(packedLogicId);
		}

		addItem(newSchemaItem);
	}

	return;
}

void EditSchemaWidget::addUfbElement()
{
	// Get Schemas
	//
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> ufbs;

	bool ok = loadUfbSchemas(&ufbs);
	if (ok == false)
	{
		return;
	}

	// Choose User Functional Block
	//
	ChooseUfbDialog dialog(ufbs, this);

	if (dialog.exec() == QDialog::Accepted)
	{
		std::shared_ptr<VFrame30::UfbSchema> ufb = dialog.result();
		assert(ufb);

		qDebug() << "UserFunctionalBlock selected " << ufb->caption();

		QString errorMsg;
		addItem(std::make_shared<VFrame30::SchemaItemUfb>(schema()->unit(), *ufb, &errorMsg));

		if (errorMsg.isEmpty() == false)
		{
			QMessageBox::critical(this, QObject::tr("Error"), errorMsg);
		}
	}

	return;
}

void EditSchemaWidget::addActuatorElement()
{
	std::vector<std::shared_ptr<VFrame30::ActuatorHeader>> actuatorHeaders;

	bool ok = loadActuatorHeaders(&actuatorHeaders);
	if (ok == false)
	{
		return;
	}

	ChooseActuatorDialog dialog{actuatorHeaders, this};

	if (dialog.exec() == QDialog::Accepted)
	{
		auto actuatorHeader = dialog.result();
		assert(actuatorHeader);

		addItem(std::make_shared<VFrame30::SchemaItemActuator>(schema()->unit(), *actuatorHeader));
	}

	return;
}

void EditSchemaWidget::addBus()
{
	QMenu menu{this};
	QMenu* composerMenu = menu.addMenu(QIcon(":/Images/Images/SchemaBusComposer.svg"), tr("Bus Composer"));
	QMenu* extractorMenu = menu.addMenu(QIcon(":/Images/Images/SchemaBusExtractor.svg"), tr("Bus Extractor"));

	std::vector<AppSignalLib::Bus> busses;

	bool ok = F2KeyForSchemaItem::loadBusses(db(), &busses, this);
	if (ok == false)
	{
		return;
	}

	for (const auto& bus : busses)
	{
		QString caption = QString("%1").arg(bus.busTypeId());

		QAction* composerAction = new QAction(caption, composerMenu);
		composerAction->setData(bus.busTypeId());
		composerMenu->addAction(composerAction);

		QAction* extractorAction = new QAction(caption, extractorMenu);
		extractorAction->setData(bus.busTypeId());
		extractorMenu->addAction(extractorAction);
	}


	QAction* triggeredAction = menu.exec(QCursor::pos());
	if (triggeredAction == nullptr)
	{
		return;
	}

	std::shared_ptr<VFrame30::SchemaItemBus> schemaItem;

	if (qobject_cast<QMenu*>(triggeredAction->parent()) == composerMenu)
	{
		schemaItem = std::make_shared<VFrame30::SchemaItemBusComposer>(schema()->unit());
	}
	else
	{
		schemaItem = std::make_shared<VFrame30::SchemaItemBusExtractor>(schema()->unit());
	}

	QString selectedBusId = triggeredAction->data().toString();
	for (const AppSignalLib::Bus& bus : busses)
	{
		if (bus.busTypeId() == selectedBusId)
		{
			schemaItem->setBusType(bus);

			addItem(schemaItem);
			return;
		}
	}

	return;
}

void EditSchemaWidget::onLeftKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextLeftItem({});
		return;
	}

	// Move selected items to left
	//
	auto selected = selectedNonLockedItems();
	if (selected.empty() == true)
	{
		return;
	}

	std::vector<SchemaItemPtr> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = snapToGrid() ? -schemaView()->schema()->gridSize() : -1;

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(dif, 0), MouseState::Moving);
		{
			if (bool ok = m_editEngine->startBatch(); ok == true)
			{
				m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
				finishMoveAfbsConnectionLinks();

				m_editEngine->endBatch();
			}
		}
	}

	return;
}

void EditSchemaWidget::onRightKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextRightItem({});
		return;
	}

	// Move selected items to right
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	std::vector<SchemaItemPtr> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = snapToGrid() ? schemaView()->schema()->gridSize() : 1;

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(dif, 0), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch(); ok == true)
		{
			m_editEngine->runMoveItem(dif, 0, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

void EditSchemaWidget::onUpKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextUpItem();
		return;
	}

	// --
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	std::vector<SchemaItemPtr> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = snapToGrid() ? -schemaView()->schema()->gridSize() : -1;

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(0, dif), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch(); ok == true)
		{
			m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

void EditSchemaWidget::onDownKey(QKeyEvent* e)
{
	if (selectedItems().empty() == false && e->modifiers().testFlag(Qt::AltModifier) == true)
	{
		selectNextDownItem();
		return;
	}

	// --
	//
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	std::vector<SchemaItemPtr> itemsForMove;
	itemsForMove.reserve(selected.size());

	for (auto& item : selected)
	{
		if (item->isLocked() == false)
		{
			itemsForMove.push_back(item);
		}
	}

	if (itemsForMove.empty() == false)
	{
		double dif = snapToGrid() ? schemaView()->schema()->gridSize() : 1;

		initMoveAfbsConnectionLinks(MouseState::Moving);
		moveAfbsConnectionLinks(QPointF(0, dif), MouseState::Moving);

		if (bool ok = m_editEngine->startBatch(); ok == true)
		{
			m_editEngine->runMoveItem(0, dif, itemsForMove, snapToGrid());
			finishMoveAfbsConnectionLinks();

			m_editEngine->endBatch();
		}
	}

	return;
}

bool EditSchemaWidget::selectNextLeftItem(NextSelectionItem switchToLeftItem)
{
	m_nextSelectionFromLeft = {};
	m_nextSelectionFromRight = {};

	if (selectedItems().empty() == true)
	{
		return false;
	}

	SchemaItemPtr selectedItem;

	if (switchToLeftItem.isNull() == false)
	{
		selectedItem = switchToLeftItem.schemaItem;
	}
	else
	{
		selectedItem = selectedItems().front();
	}

	// Get selected item pos - left, top + midle heigh
	//
	QPointF point;

	if (selectedItem->isType<VFrame30::PosConnectionImpl>() == true || selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			VFrame30::SchemaPoint firstPoint = selectedConnectionPoints.front();
			VFrame30::SchemaPoint lastPoint = selectedConnectionPoints.back();

			if (firstPoint.X < lastPoint.X)
			{
				point = {firstPoint.X, firstPoint.Y};
			}
			else
			{
				point = {lastPoint.X, lastPoint.Y};
			}
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
	{
		if (auto fblItem = posRectImpl->toFblItem(); fblItem != nullptr && fblItem->hasInputs() == true)
		{
			VFrame30::AfbPin pin;

			if (switchToLeftItem.isNull() == false && switchToLeftItem.isFblItemRect() == true)
			{
				if (switchToLeftItem.pinIndex >= 0 && switchToLeftItem.pinIndex < fblItem->inputsCount())
				{
					pin = fblItem->inputs()[switchToLeftItem.pinIndex];
					m_nextSelectionFromLeft = {selectedItem, switchToLeftItem.pinIndex};
				}
				else
				{
					return false;
				}
			}
			else
			{
				pin = fblItem->inputs().front();
				m_nextSelectionFromLeft = {selectedItem, 0};
			}

			point = {pin.x(), pin.y()};
		}
		else
		{
			// If it does not have pins, then take left center point
			//
			point = {posRectImpl->leftDocPt(), posRectImpl->topDocPt() + posRectImpl->heightDocPt() / 2.0};
		}
	}

	//  --
	//
	const double VertFactor = 2.0;
	point.setY(point.y() * VertFactor);

	// Decay all other items to points
	//
	double minDistance = std::numeric_limits<double>::max();
	SchemaItemPtr minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, VertFactor](double x, double y, auto schemaItem)
	{
		QPointF p = {x, y * VertFactor};

		if (point.x() - p.x() > -0.000001)
		{
			double d = QLineF(point, p).length();

			if (d < minDistance)
			{
				minDistance = d;
				minDistanceItem = schemaItem;
			}
		}

		return;
	};


	for (const SchemaItemPtr& item : editSchemaView()->activeLayer()->items())
	{
		if (item == selectedItem || item == m_nextSelectionFromLeft.schemaItem || item == m_nextSelectionFromRight.schemaItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
		{
			if (item->isFblItem() == true)
			{
				const VFrame30::FblItem* fblItem = item->toFblItem();
				Q_ASSERT(fblItem);

				//				for (const VFrame30::AfbPin& pin : fblItem->inputs())
				//				{
				//					calcDistance(pin.x(), pin.y(), item);
				//				}

				for (const VFrame30::AfbPin& pin : fblItem->outputs())
				{
					calcDistance(pin.x(), pin.y(), item);
				}
			}

			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(), posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (rc.left() < point.x() || rc.right() < point.x())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

		if (item->isType<VFrame30::PosConnectionImpl>() == true || item->isType<VFrame30::PosLineImpl>() == true)
		{
			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

			if (points.size() >= 2)
			{
				const VFrame30::SchemaPoint& firstPoint = points.front();
				const VFrame30::SchemaPoint& lastPoint = points.back();

				// At least one point must be at left OR
				// it is a vertical line on point.x
				//
				if ((firstPoint.X < point.x() || lastPoint.X < point.x()) ||
					(std::abs(firstPoint.X - lastPoint.X) <= 0.000001 && std::abs(firstPoint.X - point.x()) <= 0.000001) ||
					(std::abs(firstPoint.X - point.x()) <= 0.000001 && std::abs(firstPoint.Y * VertFactor - point.y()) <= 0.000001) ||
					(std::abs(lastPoint.X - point.x()) <= 0.000001 && std::abs(lastPoint.Y * VertFactor - point.y()) <= 0.000001))
				{
					calcDistance(firstPoint.X, firstPoint.Y, item);
					calcDistance(lastPoint.X, lastPoint.Y, item);
				}
			}
			else
			{
				Q_ASSERT(points.size() >= 2);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem != nullptr)
	{
		selectItem(minDistanceItem);
		return true;
	}

	return false;
}

bool EditSchemaWidget::selectNextRightItem(NextSelectionItem switchToRightItem)
{
	m_nextSelectionFromLeft = {};
	m_nextSelectionFromRight = {};

	if (selectedItems().empty() == true)
	{
		return false;
	}

	SchemaItemPtr selectedItem;

	if (switchToRightItem.isNull() == false)
	{
		selectedItem = switchToRightItem.schemaItem;
	}
	else
	{
		selectedItem = selectedItems().front();
	}

	// Get selected item pos - right, top + midle heigh
	//
	QPointF point;

	if (selectedItem->isType<VFrame30::PosConnectionImpl>() == true || selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			VFrame30::SchemaPoint firstPoint = selectedConnectionPoints.front();
			VFrame30::SchemaPoint lastPoint = selectedConnectionPoints.back();

			if (firstPoint.X < lastPoint.X)
			{
				point = {lastPoint.X, lastPoint.Y};
			}
			else
			{
				point = {firstPoint.X, firstPoint.Y};
			}
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
	{
		if (auto fblItem = posRectImpl->toFblItem(); fblItem != nullptr && fblItem->hasOutputs() == true)
		{
			VFrame30::AfbPin pin;

			if (switchToRightItem.isNull() == false && switchToRightItem.isFblItemRect() == true)
			{
				if (switchToRightItem.pinIndex >= 0 && switchToRightItem.pinIndex < fblItem->outputsCount())
				{
					pin = fblItem->outputs()[switchToRightItem.pinIndex];
					m_nextSelectionFromRight = {selectedItem, switchToRightItem.pinIndex};
				}
				else
				{
					return false;
				}
			}
			else
			{
				pin = fblItem->outputs().front();
				m_nextSelectionFromRight = {selectedItem, 0};
			}

			point = {pin.x(), pin.y()};

			m_nextSelectionFromLeft = {};
		}
		else
		{
			// If it does not have pins, then take left center point
			//
			point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt(), posRectImpl->topDocPt() + posRectImpl->heightDocPt() / 2.0};
		}
	}

	//  --
	//
	const double VertFactor = 2.0;
	point.setY(point.y() * VertFactor);

	// Decay all other items to points
	//
	double minDistance = std::numeric_limits<double>::max();
	SchemaItemPtr minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, VertFactor](double x, double y, auto schemaItem)
	{
		QPointF p = {x, y * VertFactor};

		if (point.x() - 0.000001 <= p.x())
		{
			double d = QLineF(point, p).length();

			if (d < minDistance)
			{
				minDistance = d;
				minDistanceItem = schemaItem;
			}
		}

		return;
	};


	for (const SchemaItemPtr& item : editSchemaView()->activeLayer()->items())
	{
		if (item == selectedItem || item == m_nextSelectionFromLeft.schemaItem || item == m_nextSelectionFromRight.schemaItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
		{
			if (item->isFblItem() == true)
			{
				const VFrame30::FblItem* fblItem = item->toFblItem();
				Q_ASSERT(fblItem);

				for (const VFrame30::AfbPin& pin : fblItem->inputs())
				{
					calcDistance(pin.x(), pin.y(), item);
				}

				//				for (const VFrame30::AfbPin& pin : fblItem->outputs())
				//				{
				//					calcDistance(pin.x(), pin.y(), item);
				//				}
			}

			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(), posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (point.x() < rc.left() || point.x() < rc.right())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

		if (item->isType<VFrame30::PosConnectionImpl>() == true || item->isType<VFrame30::PosLineImpl>() == true)
		{
			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

			if (points.size() >= 2)
			{
				const VFrame30::SchemaPoint& firstPoint = points.front();
				const VFrame30::SchemaPoint& lastPoint = points.back();

				// At least one point must be at right OR
				// it is a vertical line on point.x
				//
				if ((point.x() < firstPoint.X || point.x() < lastPoint.X) ||
					(std::abs(firstPoint.X - lastPoint.X) <= 0.000001 && std::abs(firstPoint.X - point.x()) <= 0.000001) ||
					(std::abs(firstPoint.X - point.x()) <= 0.000001 && std::abs(firstPoint.Y * VertFactor - point.y()) <= 0.000001) ||
					(std::abs(lastPoint.X - point.x()) <= 0.000001 && std::abs(lastPoint.Y * VertFactor - point.y()) <= 0.000001))
				{
					calcDistance(firstPoint.X, firstPoint.Y, item);
					calcDistance(lastPoint.X, lastPoint.Y, item);
				}
			}
			else
			{
				Q_ASSERT(points.size() >= 2);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem != nullptr)
	{
		selectItem(minDistanceItem);
		return true;
	}

	return false;
}

void EditSchemaWidget::selectNextUpItem()
{
	// If Alt is pressed then select the next left item
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	SchemaItemPtr selectedItem = selectedItems().front();

	// Get selected item pos
	//
	QPointF point;
	bool pointInitialized = false;

	if (m_nextSelectionFromRight.isNull() == false && m_nextSelectionFromRight.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromRight;
		ni.pinIndex--;

		pointInitialized = selectNextRightItem(ni);

		if (pointInitialized == true)
		{
			// selectNextRightItem(ni) has made selection;
			//
			return;
		}
	}

	if (m_nextSelectionFromLeft.isNull() == false && m_nextSelectionFromLeft.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromLeft;
		ni.pinIndex--;

		pointInitialized = selectNextLeftItem(ni);

		if (pointInitialized == true)
		{
			// selectNextLeftItem(ni) has made selection;
			//
			return;
		}
	}


	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>(); pointInitialized == false && posRectImpl != nullptr)
	{
		point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt() / 2.0, posRectImpl->topDocPt()};
		pointInitialized = true;
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// Get center of line
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = {line.center().x(), std::max(firstPoint.Y, lastPoint.Y)};
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosConnectionImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// FOR NOW JUST TAKE CENTER BY RECT END POINTS
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = line.center();
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false)
	{
		Q_ASSERT(pointInitialized);
		return;
	}

	//  --
	const double HorzFactor = 1.5;
	point.setX(point.x() * HorzFactor);

	// Decay all other items to points
	//
	double minDistance = std::numeric_limits<double>::max();
	SchemaItemPtr minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, HorzFactor](double x, double y, auto schemaItem)
	{
		QPointF p = {x * HorzFactor, y};

		if (point.y() - p.y() > -0.000001)
		{
			double d = QLineF(point, p).length();

			if (d < minDistance)
			{
				minDistance = d;
				minDistanceItem = schemaItem;
			}
		}

		return;
	};


	for (const SchemaItemPtr& item : editSchemaView()->activeLayer()->items())
	{
		if (item == selectedItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
		{
			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(), posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (rc.top() < point.y() || rc.bottom() < point.y())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}

		//		if (item->isType<VFrame30::PosLineImpl>() == true)
		//		{
		//			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

		//			if (points.size() >= 2)
		//			{
		//				const VFrame30::SchemaPoint& firstPoint = points.front();
		//				const VFrame30::SchemaPoint& lastPoint = points.back();

		//				QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};
		//				calcDistance(line.center().x(), line.center().y(), item);
		//			}
		//			else
		//			{
		//				Q_ASSERT(points.size() >= 2);
		//			}

		//			continue;
		//		}

		//		if (item->isType<VFrame30::PosConnectionImpl>() == true)
		//		{
		//			std::vector<VFrame30::SchemaPoint> points = item->getPointList();

		//			if (points.size() >= 2)
		//			{
		//				VFrame30::SchemaPoint prevPoint = points[0];

		//				for (int i = 1; i < points.size(); i++)
		//				{
		//					const VFrame30::SchemaPoint& firstPoint = prevPoint;
		//					const VFrame30::SchemaPoint& lastPoint = points[i];

		//					QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};
		//					calcDistance(line.center().x(), line.center().y(), item);

		//					prevPoint = points[i];
		//				}
		//			}
		//			else
		//			{
		//				Q_ASSERT(points.size() >= 2);
		//			}

		//			continue;
		//		}
	}

	// Get the closest item
	//
	if (minDistanceItem != nullptr)
	{
		selectItem(minDistanceItem);
	}

	return;
}

void EditSchemaWidget::selectNextDownItem()
{
	// If Alt is pressed then select the next left item
	//
	if (selectedItems().empty() == true)
	{
		return;
	}

	SchemaItemPtr selectedItem = selectedItems().front();

	// Get selected item pos
	//
	QPointF point;
	bool pointInitialized = false;

	if (m_nextSelectionFromRight.isNull() == false && m_nextSelectionFromRight.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromRight;
		ni.pinIndex++;

		pointInitialized = selectNextRightItem(ni);

		if (pointInitialized == true)
		{
			// selectNextRightItem(ni) has made selection;
			//
			return;
		}
	}

	if (m_nextSelectionFromLeft.isNull() == false && m_nextSelectionFromLeft.isFblItemRect() == true)
	{
		NextSelectionItem ni = m_nextSelectionFromLeft;
		ni.pinIndex++;

		pointInitialized = selectNextLeftItem(ni);

		if (pointInitialized == true)
		{
			// selectNextLeftItem(ni) has made selection;
			//
			return;
		}
	}

	if (auto posRectImpl = selectedItem->toType<VFrame30::PosRectImpl>(); pointInitialized == false && posRectImpl != nullptr)
	{
		point = {posRectImpl->leftDocPt() + posRectImpl->widthDocPt() / 2.0, posRectImpl->topDocPt() + posRectImpl->heightDocPt()};
		pointInitialized = true;
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosLineImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// Get center of line
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = {line.center().x(), std::min(firstPoint.Y, lastPoint.Y)};
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false && selectedItem->isType<VFrame30::PosConnectionImpl>() == true)
	{
		std::vector<VFrame30::SchemaPoint> selectedConnectionPoints = selectedItem->getPointList();

		if (selectedConnectionPoints.size() >= 2)
		{
			// FOR NOW JUST TAKE CENTER BY RECT END POINTS
			//
			const VFrame30::SchemaPoint& firstPoint = selectedConnectionPoints.front();
			const VFrame30::SchemaPoint& lastPoint = selectedConnectionPoints.back();

			QLineF line = {firstPoint.X, firstPoint.Y, lastPoint.X, lastPoint.Y};

			point = line.center();
			pointInitialized = true;
		}
		else
		{
			Q_ASSERT(selectedConnectionPoints.size() >= 2);
		}
	}

	if (pointInitialized == false)
	{
		Q_ASSERT(pointInitialized);
		return;
	}

	//  --
	const double HorzFactor = 1.5;
	point.setX(point.x() * HorzFactor);

	// Decay all other items to points
	//
	double minDistance = std::numeric_limits<double>::max();
	SchemaItemPtr minDistanceItem;

	auto calcDistance = [&minDistance, &minDistanceItem, &point, HorzFactor](double x, double y, auto schemaItem)
	{
		QPointF p = {x * HorzFactor, y};

		if (point.y() - 0.000001 <= p.y())
		{
			double d = QLineF(point, p).length();

			if (d < minDistance)
			{
				minDistance = d;
				minDistanceItem = schemaItem;
			}
		}

		return;
	};


	for (const SchemaItemPtr& item : editSchemaView()->activeLayer()->items())
	{
		if (item == selectedItem)
		{
			continue;
		}

		if (auto posRectImpl = item->toType<VFrame30::PosRectImpl>(); posRectImpl != nullptr)
		{
			QRectF rc = {posRectImpl->leftDocPt(), posRectImpl->topDocPt(), posRectImpl->widthDocPt(), posRectImpl->heightDocPt()};

			QPointF center = rc.center();

			if (point.y() < rc.top() || point.y() < rc.bottom())
			{
				calcDistance(center.x(), rc.top(), item);
				calcDistance(rc.right(), rc.top(), item);
				calcDistance(rc.right(), center.y(), item);
				calcDistance(rc.right(), rc.bottom(), item);
				calcDistance(center.x(), rc.bottom(), item);
				calcDistance(rc.left(), rc.top(), item);
				calcDistance(rc.left(), rc.bottom(), item);
				calcDistance(rc.left(), center.y(), item);
			}

			continue;
		}
	}

	// Get the closest item
	//
	if (minDistanceItem != nullptr)
	{
		selectItem(minDistanceItem);
	}

	return;
}

void EditSchemaWidget::sameWidth()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<SchemaItemPtr> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr || dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selectedFiltered.at(0);

	// find the most left and most right points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint leftPoint = firstItemPoints[0];
	VFrame30::SchemaPoint rightPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < leftPoint.X)
		{
			leftPoint = pt;
		}

		if (pt.X > rightPoint.X)
		{
			rightPoint = pt;
		}
	}

	double width = rightPoint.X - leftPoint.X;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].X < points[1].X)
			{
				points[1].X = points[0].X + width;
			}
			else
			{
				points[0].X = points[1].X + width;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].X = points[0].X + width;
			}
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::sameHeight()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<SchemaItemPtr> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr || dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selectedFiltered.at(0);

	// find the top and bottom points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint topPoint = firstItemPoints[0];
	VFrame30::SchemaPoint bottomPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y < topPoint.Y)
		{
			topPoint = pt;
		}

		if (pt.Y > bottomPoint.Y)
		{
			bottomPoint = pt;
		}
	}

	double height = bottomPoint.Y - topPoint.Y;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].Y < points[1].Y)
			{
				points[1].Y = points[0].Y + height;
			}
			else
			{
				points[0].Y = points[1].Y + height;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].Y = points[0].Y + height;
			}
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::sameSize()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	// Same Width/Height works only for usual lines and rectangles,filter connection line
	//
	std::vector<SchemaItemPtr> selectedFiltered;

	for (auto item : selected)
	{
		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr || dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			selectedFiltered.push_back(item);
		}
	}

	if (selectedFiltered.size() < 2)
	{
		assert(selectedFiltered.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selectedFiltered.at(0);

	// find the most left and most right points
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint leftPoint = firstItemPoints[0];
	VFrame30::SchemaPoint rightPoint = firstItemPoints[0];

	VFrame30::SchemaPoint topPoint = firstItemPoints[0];
	VFrame30::SchemaPoint bottomPoint = firstItemPoints[0];

	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < leftPoint.X)
		{
			leftPoint = pt;
		}

		if (pt.X > rightPoint.X)
		{
			rightPoint = pt;
		}

		if (pt.Y < topPoint.Y)
		{
			topPoint = pt;
		}

		if (pt.Y > bottomPoint.Y)
		{
			bottomPoint = pt;
		}
	}

	double width = rightPoint.X - leftPoint.X;
	double height = bottomPoint.Y - topPoint.Y;

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selectedFiltered)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		if (dynamic_cast<VFrame30::PosLineImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points[0].X < points[1].X)
			{
				points[1].X = points[0].X + width;
			}
			else
			{
				points[0].X = points[1].X + width;
			}

			if (points[0].Y < points[1].Y)
			{
				points[1].Y = points[0].Y + height;
			}
			else
			{
				points[0].Y = points[1].Y + height;
			}
		}

		if (dynamic_cast<VFrame30::PosRectImpl*>(item.get()) != nullptr)
		{
			assert(points.size() == 2);

			if (points.size() == 2)
			{
				points[1].X = points[0].X + width;
			}

			if (points.size() == 2)
			{
				points[1].Y = points[0].Y + height;
			}
		}

		// --
		//
		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selectedFiltered, true);

	return;
}

void EditSchemaWidget::alignLeft()
{
	auto selected = selectedNonLockedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selected.at(0);

	// find the most left point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostLeftPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X < firstItemMostLeftPoint.X)
		{
			firstItemMostLeftPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most left point for the item
		//
		VFrame30::SchemaPoint itemMostLeftPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.X < itemMostLeftPoint.X)
			{
				itemMostLeftPoint = pt;
			}
		}

		double diff = firstItemMostLeftPoint.X - itemMostLeftPoint.X;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.X += diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignRight()
{
	auto selected = selectedNonLockedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selected.at(0);

	// find the most right point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostRightPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.X > firstItemMostRightPoint.X)
		{
			firstItemMostRightPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most right point for the item
		//
		VFrame30::SchemaPoint itemMostRightPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.X > itemMostRightPoint.X)
			{
				itemMostRightPoint = pt;
			}
		}

		double diff = itemMostRightPoint.X - firstItemMostRightPoint.X;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.X -= diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignTop()
{
	auto selected = selectedNonLockedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selected.at(0);

	// find the most top point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostTopPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y < firstItemMostTopPoint.Y)
		{
			firstItemMostTopPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most top point for the item
		//
		VFrame30::SchemaPoint itemMostTopPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.Y < itemMostTopPoint.Y)
			{
				itemMostTopPoint = pt;
			}
		}

		double diff = firstItemMostTopPoint.Y - itemMostTopPoint.Y;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.Y += diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::alignBottom()
{
	auto selected = selectedNonLockedItems();

	if (selected.size() < 2)
	{
		assert(selected.size() >= 2);
		return;
	}

	SchemaItemPtr firstItem = selected.at(0);

	// find the most bottom point
	//
	std::vector<VFrame30::SchemaPoint> firstItemPoints = firstItem->getPointList();

	if (firstItemPoints.empty() == true)
	{
		assert(firstItemPoints.empty() == false);
		return;
	}

	VFrame30::SchemaPoint firstItemMostBottomPoint = firstItemPoints[0];
	for (const VFrame30::SchemaPoint& pt : firstItemPoints)
	{
		if (pt.Y > firstItemMostBottomPoint.Y)
		{
			firstItemMostBottomPoint = pt;
		}
	}

	// get new points for all items
	//
	std::vector<std::vector<VFrame30::SchemaPoint>> newPoints;
	for (SchemaItemPtr item : selected)
	{
		std::vector<VFrame30::SchemaPoint> points = item->getPointList();
		assert(points.empty() == false);

		// find the most bottom point for the item
		//
		VFrame30::SchemaPoint itemMostBottomPoint = points[0];
		for (const VFrame30::SchemaPoint& pt : points)
		{
			if (pt.Y > itemMostBottomPoint.Y)
			{
				itemMostBottomPoint = pt;
			}
		}

		double diff = itemMostBottomPoint.Y - firstItemMostBottomPoint.Y;

		for (VFrame30::SchemaPoint& pt : points)
		{
			pt.Y -= diff;
		}

		newPoints.push_back(points);
	}

	m_editEngine->runSetPoints(newPoints, selected, true);

	return;
}

void EditSchemaWidget::bringToFront()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::BringToFront, selected, activeLayer());
}

void EditSchemaWidget::bringForward()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::BringForward, selected, activeLayer());
}

void EditSchemaWidget::sendToBack()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::SendToBack, selected, activeLayer());
}

void EditSchemaWidget::sendBackward()
{
	auto selected = selectedNonLockedItems();

	if (selected.empty() == true)
	{
		return;
	}

	m_editEngine->runSetOrder(EditEngine::SetOrder::SendBackward, selected, activeLayer());
}

void EditSchemaWidget::transformIntoInput()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<SchemaItemPtr> selected = selectedItems();
	std::list<SchemaItemPtr> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoInput();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch(); ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();

		// Apply default values, and undo, so user can choose between a clear copy or a copy with applied default values.
		//
		bool defaultsWereSet = setDefaultItemProperties(newItems);
		if (defaultsWereSet == true)
		{
			// Undo one time that user can select what to take, a clear item copy or modified with default properties.
			//
			undo();
		}
	}

	return;
}

void EditSchemaWidget::transformIntoInOut()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<SchemaItemPtr> selected = selectedItems();
	std::list<SchemaItemPtr> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoInOut();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch(); ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();

		// Apply default values, and undo, so user can choose between a clear copy or a copy with applied default values.
		//
		bool defaultsWereSet = setDefaultItemProperties(newItems);
		if (defaultsWereSet == true)
		{
			// Undo one time that user can select what to take, a clear item copy or modified with default properties.
			//
			undo();
		}
	}

	return;
}

void EditSchemaWidget::transformIntoOutput()
{
	if (selectedItems().empty() == true)
	{
		return;
	}

	for (auto item : selectedItems())
	{
		if (item->isType<VFrame30::SchemaItemSignal>() == false)
		{
			assert(item->isType<VFrame30::SchemaItemSignal>() == true);
			return;
		}
	}

	const std::vector<SchemaItemPtr> selected = selectedItems();
	std::list<SchemaItemPtr> newItems;

	for (auto item : selected)
	{
		auto signalItem = item->toType<VFrame30::SchemaItemSignal>();
		assert(signalItem);

		auto transformedItem = signalItem->transformIntoOutput();
		assert(transformedItem);

		newItems.push_back(transformedItem);
	}

	if (bool ok = m_editEngine->startBatch(); ok == true)
	{
		m_editEngine->runDeleteItem(selected, activeLayer());
		m_editEngine->runAddItem(newItems, activeLayer());

		m_editEngine->endBatch();

		// Apply default values, and undo, so user can choose between a clear copy or a copy with applied default values.
		//
		bool defaultsWereSet = setDefaultItemProperties(newItems);
		if (defaultsWereSet == true)
		{
			// Undo one time that user can select what to take, a clear item copy or modified with default properties.
			//
			undo();
		}
	}

	return;
}

void EditSchemaWidget::toggleComment()
{
	qDebug() << "EditSchemaWidget::toggleComment()";

	if (selectedItems().empty() == true)
	{
		return;
	}

	// Only FblItems can be commented
	//
	bool hasCommented = false;
	bool hasUncommented = false;

	const auto& selected = selectedItems();

	for (auto& selItem : selected)
	{
		if (selItem->isFblItem() == true)
		{
			if (selItem->isCommented() == true)
			{
				hasCommented = true;
			}
			else
			{
				hasUncommented = true;
			}
		}
	}

	if (hasUncommented == true)
	{
		// Comment all
		//
		std::vector<SchemaItemPtr> items;
		items.reserve(selected.size());

		for (auto& selItem : selected)
		{
			if (selItem->isFblItem() == true)
			{
				items.push_back(selItem);
			}
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::commented, QVariant(true), items);
		return;
	}

	if (hasCommented == true)
	{
		// Uncomment all
		//
		std::vector<SchemaItemPtr> items;
		items.reserve(selected.size());

		for (auto& selItem : selected)
		{
			if (selItem->isFblItem() == true)
			{
				items.push_back(selItem);
			}
		}

		m_editEngine->runSetProperty(VFrame30::PropertyNames::commented, QVariant(false), items);
		return;
	}

	return;
}

void EditSchemaWidget::toggleLock()
{
	qDebug() << "EditSchemaWidget::toggleLock()";

	if (selectedItems().empty() == true)
	{
		return;
	}

	// Only FblItems can be commented
	//
	bool hasLocked = false;
	bool hasUnlocked = false;

	const auto& selected = selectedItems();

	for (auto& selItem : selected)
	{
		if (selItem->isLocked() == true)
		{
			hasLocked = true;
		}
		else
		{
			hasUnlocked = true;
		}
	}

	if (hasUnlocked == true)
	{
		// Lock all
		//
		m_editEngine->runSetProperty(VFrame30::PropertyNames::locked, QVariant(true), selected);
		return;
	}

	if (hasLocked == true)
	{
		// Unlock all
		//
		m_editEngine->runSetProperty(VFrame30::PropertyNames::locked, QVariant(false), selected);
		return;
	}

	return;
}

void EditSchemaWidget::find()
{
	bool replaceEndbled = readOnly() == false;

	if (m_findDialog != nullptr && m_findDialog->replaceEnabled() != replaceEndbled)
	{
		delete m_findDialog;
		m_findDialog = nullptr;
	}

	if (m_findDialog == nullptr)
	{
		m_findDialog = new SchemaFindDialog(replaceEndbled, this);

		connect(m_findDialog, &SchemaFindDialog::findPrev, this, &EditSchemaWidget::findPrev);
		connect(m_findDialog, &SchemaFindDialog::findNext, this, &EditSchemaWidget::findNext);

		connect(m_findDialog, &SchemaFindDialog::replaceAndFind, this, &EditSchemaWidget::replaceAndFind);
		connect(m_findDialog, &SchemaFindDialog::replaceAll, this, &EditSchemaWidget::replaceAll);
	}

	m_findDialog->show();
	m_findDialog->ensureVisible();

	m_findDialog->raise();
	m_findDialog->activateWindow();

	return;
}

void EditSchemaWidget::findNext(Qt::CaseSensitivity cs)
{
	if (m_findDialog == nullptr)
	{
		find();
		return;
	}

	QString searchText = m_findDialog->findText();

	if (searchText.isEmpty() == true)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();

		m_findDialog->setFocusToEditLine();
		return;
	}

	m_findDialog->updateCompleter();

	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->items().empty() == true)
	{
		clearSelection();
		return;
	}

	auto& selected = selectedItems(); // Keep reference!!!!

	// Get start iterator
	//
	auto searchStartIterator = layer->items().begin();
	if (selected.size() != 1)
	{
		searchStartIterator = layer->items().begin();
	}
	else
	{
		assert(selected.size() == 1);

		searchStartIterator = std::find(layer->items().begin(), layer->items().end(), selected.front());
		if (searchStartIterator == layer->items().end())
		{
			searchStartIterator = layer->items().begin();
		}
		else
		{
			searchStartIterator++;
		}
	}

	if (searchStartIterator == layer->items().end())
	{
		searchStartIterator = layer->items().begin();
	}

	// Search the text from the selected
	//
	for (auto it = searchStartIterator; it != layer->items().end(); ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->items().begin(); it != searchStartIterator; ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(searchText));

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();
	m_findDialog->setFocusToEditLine();

	return;
}

void EditSchemaWidget::findPrev(Qt::CaseSensitivity cs)
{
	if (m_findDialog == nullptr)
	{
		find();
		return;
	}

	QString searchText = m_findDialog->findText();

	if (searchText.isEmpty() == true)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();

		m_findDialog->setFocusToEditLine();
		return;
	}

	m_findDialog->updateCompleter();

	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->items().empty() == true)
	{
		clearSelection();
		return;
	}

	auto& selected = selectedItems(); // Keep reference!!!!

	// Get start iterator
	//
	auto searchStartIterator = layer->items().rbegin();
	if (selected.size() != 1)
	{
		searchStartIterator = layer->items().rbegin();
	}
	else
	{
		assert(selected.size() == 1);

		searchStartIterator = std::find(layer->items().rbegin(), layer->items().rend(), selected.front());
		if (searchStartIterator == layer->items().rend())
		{
			searchStartIterator = layer->items().rbegin();
		}
		else
		{
			searchStartIterator++;
		}
	}

	if (searchStartIterator == layer->items().rend())
	{
		searchStartIterator = layer->items().rbegin();
	}

	// Search the text from the selected
	//
	for (auto it = searchStartIterator; it != layer->items().rend(); ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Serach text from the beginning to selected
	//
	for (auto it = layer->items().rbegin(); it != searchStartIterator; ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(searchText, cs);

		if (result.empty() == false)
		{
			selectItem(item);
			m_findDialog->updateFoundInformation(item, result, searchText, cs);
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(searchText));

	m_findDialog->show();
	m_findDialog->raise();
	m_findDialog->activateWindow();
	m_findDialog->setFocusToEditLine();

	return;
}

int EditSchemaWidget::replace(SchemaItemPtr item, QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
	if (item == nullptr)
	{
		assert(item);
		return 0;
	}

	QByteArray oldState;
	item->saveToByteArray(&oldState);

	int replaceCount = item->replace(findText, replaceWith, cs);

	QByteArray newState;
	item->saveToByteArray(&newState);

	item->Load(oldState);

	if (replaceCount != 0)
	{
		m_editEngine->runSetObject(oldState, newState, item);
	}

	return replaceCount;
}

void EditSchemaWidget::replaceAndFind(QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->items().empty() == true)
	{
		clearSelection();
		return;
	}

	auto& selected = selectedItems(); // Keep reference!!!!

	// Get start iterator
	//
	auto searchStartIterator = layer->items().begin();

	if (selected.empty() == true)
	{
		searchStartIterator = layer->items().begin();
	}
	else
	{
		searchStartIterator = std::find(layer->items().begin(), layer->items().end(), selected.front());

		if (searchStartIterator == layer->items().end())
		{
			searchStartIterator = layer->items().begin();
		}
		else
		{
			// Replace text in selected item
			//
			replace(*searchStartIterator, findText, replaceWith, cs);
			searchStartIterator++;
		}
	}

	// Text in current selected item replaced, find and select next item
	//
	if (searchStartIterator == layer->items().end())
	{
		searchStartIterator = layer->items().begin();
	}

	for (auto it = searchStartIterator; it != layer->items().end(); ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(findText, cs);

		if (result.empty() == false)
		{
			selectItem(item);

			if (m_findDialog != nullptr)
			{
				m_findDialog->updateFoundInformation(item, result, findText, cs);
			}
			return;
		}
	}

	// Serach text from the beginning
	//
	for (auto it = layer->items().begin(); it != searchStartIterator; ++it)
	{
		SchemaItemPtr item = *it;

		auto result = item->searchTextByProps(findText, cs);

		if (result.empty() == false)
		{
			selectItem(item);

			if (m_findDialog != nullptr)
			{
				m_findDialog->updateFoundInformation(item, result, findText, cs);
			}
			return;
		}
	}

	// Text not found
	//
	clearSelection();

	QMessageBox::information(this, qApp->applicationName(), tr("Text <b>%1</b> not found.").arg(findText));

	if (m_findDialog != nullptr)
	{
		m_findDialog->show();
		m_findDialog->raise();
		m_findDialog->activateWindow();
		m_findDialog->setFocusToEditLine();
	}

	return;
}

void EditSchemaWidget::replaceAll(QString findText, QString replaceWith, Qt::CaseSensitivity cs)
{
	// Look for text
	//
	std::shared_ptr<VFrame30::SchemaLayer> layer = activeLayer();
	assert(layer);

	if (layer->items().empty() == true)
	{
		clearSelection();
		return;
	}

	// If there are selected items, then replace only in selected
	// else relace in all layer's items
	//
	std::vector<SchemaItemPtr> items = selectedItems();
	if (items.empty() == true)
	{
		items.assign(layer->items().begin(), layer->items().end());
	}

	// Replace
	//
	int count = 0;

	std::vector<SchemaItemPtr> replacedInItems;
	replacedInItems.reserve(items.size());

	for (SchemaItemPtr item : items)
	{
		int itemReplaces = replace(item, findText, replaceWith, cs);

		if (itemReplaces != 0)
		{
			count += itemReplaces;
			replacedInItems.push_back(item);
		}
	}

	if (count == 0)
	{
		QMessageBox::information(this, tr("Replace All Result"), tr("Text <b>%1</b> not found.").arg(findText));
	}
	else
	{
		selectItems(replacedInItems);
		QMessageBox::information(this,
								 tr("Replace All Result"),
								 tr("%1 replaced to %2 in %3 item(s).").arg(findText).arg(replaceWith).arg(replacedInItems.size()));
	}

	return;
}

void EditSchemaWidget::hideWorkDialogs()
{
	if (m_findDialog != nullptr)
	{
		m_findDialog->hide();
	}

	if (m_itemsPropertiesDialog != nullptr)
	{
		m_itemsPropertiesDialog->hide();
	}

	if (m_schemaPropertiesDialog != nullptr)
	{
		m_schemaPropertiesDialog = nullptr;
	}

	return;
}

MouseState EditSchemaWidget::mouseState() const
{
	return editSchemaView()->mouseState();
}

void EditSchemaWidget::setMouseState(MouseState state)
{
	editSchemaView()->setMouseState(state);

	if (state == MouseState::Moving || state == MouseState::SizingTopLeft || state == MouseState::SizingTop ||
		state == MouseState::SizingTopRight || state == MouseState::SizingRight || state == MouseState::SizingBottomRight ||
		state == MouseState::SizingBottom || state == MouseState::SizingBottomLeft || state == MouseState::SizingLeft)
	{
		initMoveAfbsConnectionLinks(state);
	}

	return;
}

const DbFileInfo& EditSchemaWidget::fileInfo() const
{
	return m_fileInfo;
}

void EditSchemaWidget::setFileInfo(const DbFileInfo& fi)
{
	m_fileInfo = fi;
}

bool EditSchemaWidget::snapToGrid() const
{
	return m_snapToGrid;
}

void EditSchemaWidget::setSnapToGrid(bool value)
{
	m_snapToGrid = value;
}

bool EditSchemaWidget::compareWidget() const
{
	return editSchemaView()->m_compareWidget;
}

bool EditSchemaWidget::isCompareWidget() const
{
	return editSchemaView()->m_compareWidget;
}

void EditSchemaWidget::setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target)
{
	editSchemaView()->m_compareWidget = value;

	if (value == true)
	{
		assert(source);
		assert(target);

		editSchemaView()->m_compareSourceSchema = source;
		editSchemaView()->m_compareTargetSchema = target;
	}

	return;
}

bool EditSchemaWidget::readOnly() const
{
	assert(m_editEngine);
	return m_editEngine->readOnly();
}

void EditSchemaWidget::setReadOnly(bool value)
{
	assert(m_editEngine);
	m_editEngine->setReadOnly(value);
}

bool EditSchemaWidget::modified() const
{
	assert(m_editEngine);
	return m_editEngine->modified();
}

void EditSchemaWidget::setModified()
{
	assert(m_editEngine);
	m_editEngine->setModified();
}

void EditSchemaWidget::resetModified()
{
	m_initialSchemaId = schema()->schemaId();

	assert(m_editEngine);
	m_editEngine->resetModified();
}

void EditSchemaWidget::resetEditEngine()
{
	assert(m_editEngine);
	m_editEngine->reset();
}

void EditSchemaWidget::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
{
	editSchemaView()->m_itemsActions = itemsActions;
}
