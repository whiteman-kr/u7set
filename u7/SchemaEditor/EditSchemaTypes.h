#pragma once


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
