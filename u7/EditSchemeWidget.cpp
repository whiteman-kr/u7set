#include "Stable.h"
#include "EditEngine/EditEngine.h"
#include "EditSchemeWidget.h"
#include "SchemePropertiesDialog.h"
#include "SchemeLayersDialog.h"
#include "SchemeItemPropertiesDialog.h"
#include "ChooseAfbDialog.h"
#include "../VFrame30/SchemeItemLine.h"
#include "../VFrame30/SchemeItemRect.h"
#include "../VFrame30/VideoItemConnectionLine.h"
#include "../VFrame30/VideoItemSignal.h"
#include "../VFrame30/VideoItemFblElement.h"
#include "../VFrame30/VideoItemLink.h"
#include "../VFrame30/SchemeItemConst.h"


const EditSchemeWidget::MouseStateCursor EditSchemeWidget::m_mouseStateCursor[] =
	{
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

const EditSchemeWidget::SizeActionToMouseCursor EditSchemeWidget::m_sizeActionToMouseCursor[] =
	{
		{VideoItemAction::ChangeSizeTopLeft, MouseState::SizingTopLeft, Qt::SizeFDiagCursor},
		{VideoItemAction::ChangeSizeTop, MouseState::SizingTop, Qt::SizeVerCursor},
		{VideoItemAction::ChangeSizeTopRight, MouseState::SizingTopRight, Qt::SizeBDiagCursor},
		{VideoItemAction::ChangeSizeRight, MouseState::SizingRight, Qt::SizeHorCursor},
		{VideoItemAction::ChangeSizeBottomRight, MouseState::SizingBottomRight, Qt::SizeFDiagCursor},
		{VideoItemAction::ChangeSizeBottom, MouseState::SizingBottom, Qt::SizeVerCursor},
		{VideoItemAction::ChangeSizeBottomLeft, MouseState::SizingBottomLeft, Qt::SizeBDiagCursor},
		{VideoItemAction::ChangeSizeLeft, MouseState::SizingLeft, Qt::SizeHorCursor}
	};


//
//
// EditVideoFrameView
//
//
EditSchemeView::EditSchemeView(QWidget* parent) :
	VFrame30::SchemeView(parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_editStartMovingEdge(0),
	m_editEndMovingEdge(0),
	m_editStartMovingEdgeX(0),
	m_editStartMovingEdgeY(0),
	m_editEndMovingEdgeX(0),
	m_editEndMovingEdgeY(0),
	m_movingEdgePointIndex(0)
{
}

EditSchemeView::EditSchemeView(std::shared_ptr<VFrame30::Scheme>& scheme, QWidget* parent)
	: VFrame30::SchemeView(scheme, parent),
	m_activeLayer(0),
	m_mouseState(MouseState::None),
	m_editStartMovingEdge(0),
	m_editEndMovingEdge(0),
	m_editStartMovingEdgeX(0),
	m_editStartMovingEdgeY(0),
	m_editEndMovingEdgeX(0),
	m_editEndMovingEdgeY(0),
	m_movingEdgePointIndex(0)
{
}

EditSchemeView::~EditSchemeView()
{
}

void EditSchemeView::paintEvent(QPaintEvent* pe)
{
	// Draw videoframe
	//
	VFrame30::SchemeView::paintEvent(pe);

	// Draw other -- selection, grid, outlines, rullers, etc
	//

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, scheme()->gridSize(), scheme()->pinGridStep());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0);

	// Draw VideoFrame
	//
	QRectF clipRect(0, 0, scheme()->docWidth(), scheme()->docHeight());

	drawParam.setControlBarSize(
		scheme()->unit() == VFrame30::SchemeUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

	// Draw selection
	//
	if (m_selectedItems.empty() == false)
	{
		VFrame30::SchemeItem::DrawSelection(&drawParam, m_selectedItems, m_selectedItems.size() == 1);
	}

	// Draw newItem outline
	//
	drawNewItemOutline(&p, &drawParam);

	// Draw selection bar
	//
	drawSelectionArea(&p);

	// Items are being moved drawing
	//
	drawMovingItems(&drawParam);

	// --
	//
	drawRectSizing(&drawParam);
	drawMovingLinePoint(&drawParam);
	drawMovingEdgesOrVertexConnectionLine(&drawParam);

	p.restore();

	// Draw grid performed in not ajusted painter
	//
	drawGrid(&p);

	// --
	//
	p.end();

	return;
}

void EditSchemeView::drawNewItemOutline(QPainter* p, VFrame30::CDrawParam* drawParam)
{
	if (m_newItem == nullptr)
	{
		return;
	}

	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw
	//
	std::vector<std::shared_ptr<VFrame30::SchemeItem>> outlines;
	outlines.push_back(m_newItem);

	VFrame30::SchemeItem::DrawOutline(drawParam, outlines);

	// Draw ruller for newItem
	//
	bool drawRullers = false;
	VFrame30::SchemePoint rullerPoint;

	bool posInterfaceFound = false;

	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemePosLineEndPoint)
		{
			return;
		}

		posInterfaceFound = true;

		VFrame30::IVideoItemPosLine* pos = dynamic_cast<VFrame30::IVideoItemPosLine*>(m_newItem.get());

		drawRullers = true;
		rullerPoint.X = pos->endXDocPt();
		rullerPoint.Y = pos->endYDocPt();
	}

	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemePosRectEndPoint)
		{
			return;
		}

		posInterfaceFound = true;
		VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(m_newItem.get());

		drawRullers = true;

		rullerPoint.X = itemPos->leftDocPt() + itemPos->widthDocPt();
		rullerPoint.Y = itemPos->topDocPt() + itemPos->heightDocPt();
	}

	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(m_newItem.get()) != nullptr)
	{
		if (mouseState() != MouseState::AddSchemePosConnectionStartPoint &&
			mouseState() != MouseState::AddSchemePosConnectionNextPoint)
		{
			return;
		}

		posInterfaceFound = true;
		VFrame30::IVideoItemPosConnection* pos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(m_newItem.get());

		const std::list<VFrame30::SchemePoint>& extPoints = pos->GetExtensionPoints();

		if (extPoints.empty() == false)
		{
			drawRullers = true;
			rullerPoint.X = extPoints.back().X;
			rullerPoint.Y = extPoints.back().Y;
		}
	}

	if (posInterfaceFound == false)
	{
		assert(posInterfaceFound == true);
		return;
	}

	if (drawRullers == true)
	{
		QPen outlinePen(Qt::blue);
		outlinePen.setWidth(0);

		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		p->setPen(outlinePen);

		p->drawLine(QPointF(rullerPoint.X, 0.0), QPointF(rullerPoint.X, scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerPoint.Y), QPointF(scheme()->docWidth(), rullerPoint.Y));

		p->setRenderHints(oldrenderhints);
	}

	return;
}

void EditSchemeView::drawSelectionArea(QPainter* p)
{
	QRectF r(m_mouseSelectionStartPoint, m_mouseSelectionEndPoint);

	QPen pen(QColor(0x33, 0x99, 0xFF, 0xE6));
	pen.setWidth(0);

	p->setPen(pen);
	p->setBrush(QColor(0x33, 0x99, 0xFF, 0x33));

	p->drawRect(r);

	return;
}

void EditSchemeView::drawMovingItems(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::Moving ||
		m_selectedItems.empty() == true)
	{
		return;
	}

	float xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	float ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	// Shift position
	//
	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif](std::shared_ptr<VFrame30::SchemeItem> si)
		{
			si->MoveItem(xdif, ydif);
		}
		);

	// Draw outline
	//
	VFrame30::SchemeItem::DrawOutline(drawParam, m_selectedItems);

	// Get bounding rect
	//
	double left = 0.0;
	double right = 0.0;
	double top = 0.0;
	double bottom = 0.0;

	for (auto it = std::begin(m_selectedItems); it != std::end(m_selectedItems); it++)
	{

		VFrame30::IPointList* ipoint = dynamic_cast<VFrame30::IPointList*>(it->get());
		assert(ipoint != nullptr);

		if (ipoint == nullptr)
		{
			continue;
		}

		std::vector<VFrame30::SchemePoint> points = ipoint->getPointList();

		for (size_t i = 0; i < points.size(); i++)
		{
			const VFrame30::SchemePoint& p = points[i];

			if (it == std::begin(m_selectedItems) && i == 0)
			{
				left = p.X;
				right = p.X;
				top = p.Y;
				bottom = p.Y;
				continue;
			}

			left = std::min(left, p.X);
			right = std::max(right, p.X);
			top = std::min(top, p.Y);
			bottom = std::max(bottom, p.Y);
		}
	}

	// Shift position back
	//
	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif](std::shared_ptr<VFrame30::SchemeItem> si)
		{
			si->MoveItem(-xdif, -ydif);
		}
		);

	// Draw rullers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);
	p->drawLine(QPointF(left, 0.0), QPointF(left, scheme()->docHeight()));
	p->drawLine(QPointF(right, 0.0), QPointF(right, scheme()->docHeight()));

	p->drawLine(QPointF(0.0, top), QPointF(scheme()->docWidth(), top));
	p->drawLine(QPointF(0.0, bottom), QPointF(scheme()->docWidth(), bottom));

	// --
	//
	p->setRenderHints(oldrenderhints);


	return;
}

void EditSchemeView::drawRectSizing(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::SizingTopLeft &&
		mouseState() != MouseState::SizingTop &&
		mouseState() != MouseState::SizingTopRight &&
		mouseState() != MouseState::SizingRight &&
		mouseState() != MouseState::SizingBottomRight &&
		mouseState() != MouseState::SizingBottom &&
		mouseState() != MouseState::SizingBottomLeft &&
		mouseState() != MouseState::SizingLeft)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true ||
		m_editEndDocPt.isNull() == true)
	{
		assert(m_editStartDocPt.isNull() == false);
		assert(m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	float xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	float ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(selectedItems().front().get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	auto si = selectedItems().front();

	// save old state
	//
	std::vector<VFrame30::SchemePoint> oldPos = si->getPointList();

	// set new pos
	//
	double x1 = itemPos->leftDocPt();
	double y1 = itemPos->topDocPt();
	double x2 = x1 + itemPos->widthDocPt();
	double y2 = y1 + itemPos->heightDocPt();

	double minWidth = itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		x1 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		break;
	default:
		assert(false);
		break;
	}

	itemPos->setLeftDocPt(std::min(x1, x2));
	itemPos->setTopDocPt(std::min(y1, y2));

	double width = std::max(std::abs(x2 - x1), itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep()));
	double height = std::max(std::abs(y2 - y1), itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep()));

	itemPos->setWidthDocPt(width);
	itemPos->setHeightDocPt(height);

	// Save result for drawing rullers
	//
	m_addRectStartPoint = VFrame30::SchemePoint(x1, y1);
	m_addRectEndPoint = VFrame30::SchemePoint(x2, y2);

	// Draw rullers by bounding rect
	//
	QPainter* p = drawParam->painter();

	QRectF rullerRect(m_addRectStartPoint, m_addRectEndPoint);

	QPen outlinePen(Qt::blue);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(scheme()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingTop:
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(scheme()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingTopRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.top()), QPointF(scheme()->docWidth(), rullerRect.top()));
		break;
	case MouseState::SizingRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), scheme()->docHeight()));
		break;
	case MouseState::SizingBottomRight:
		p->drawLine(QPointF(rullerRect.right(), 0.0), QPointF(rullerRect.right(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(scheme()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingBottom:
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(scheme()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingBottomLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerRect.bottom()), QPointF(scheme()->docWidth(), rullerRect.bottom()));
		break;
	case MouseState::SizingLeft:
		p->drawLine(QPointF(rullerRect.left(), 0.0), QPointF(rullerRect.left(), scheme()->docHeight()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);

	// Draw item outline
	//
	VFrame30::SchemeItem::DrawOutline(drawParam, m_selectedItems);

	// restore position
	//
	si->setPointList(oldPos);
	return;
}

void EditSchemeView::drawMovingLinePoint(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingStartLinePoint &&
		mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (m_editStartDocPt.isNull() == true ||
		m_editEndDocPt.isNull() == true)
	{
		assert(m_editStartDocPt.isNull() == false);
		assert(m_editEndDocPt.isNull() == false);
		return;
	}

	if (m_selectedItems.size() != 1)
	{
		assert(m_selectedItems.size() == 1);
		return;
	}

	auto si = m_selectedItems.front();
	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(m_selectedItems.front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	// Save current pos
	//

	auto oldPos = si->getPointList();

	// Set new pos
	//
	double xdif = m_editEndDocPt.x() - m_editStartDocPt.x();
	double ydif = m_editEndDocPt.y() - m_editStartDocPt.y();

	if (mouseState() == MouseState::MovingStartLinePoint)
	{
		itemPos->setStartXDocPt(itemPos->startXDocPt() + xdif);
		itemPos->setStartYDocPt(itemPos->startYDocPt() + ydif);
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		itemPos->setEndXDocPt(itemPos->endXDocPt() + xdif);
		itemPos->setEndYDocPt(itemPos->endYDocPt() + ydif);
	}

	// Draw rullers
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::MovingStartLinePoint:
		p->drawLine(QPointF(itemPos->startXDocPt(), 0.0), QPointF(itemPos->startXDocPt(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, itemPos->startYDocPt()), QPointF(scheme()->docWidth(), itemPos->startYDocPt()));
		break;
	case MouseState::MovingEndLinePoint:
		p->drawLine(QPointF(itemPos->endXDocPt(), 0.0), QPointF(itemPos->endXDocPt(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, itemPos->endYDocPt()), QPointF(scheme()->docWidth(), itemPos->endYDocPt()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);

	// Draw outline
	//
	VFrame30::SchemeItem::DrawOutline(drawParam, m_selectedItems);

	// Resotore points
	//
	si->setPointList(oldPos);

	return;
}

void EditSchemeView::drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		return;
	}

	if (m_movingEdgePointIndex == -1)
	{
		assert(m_movingEdgePointIndex != -1);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	std::list<VFrame30::SchemePoint> pointsList = itemPos->GetPointList();
	std::vector<VFrame30::SchemePoint> points(pointsList.begin(), pointsList.end());

	// Save position
	//
	auto oldPos = si->getPointList();

	if (m_movingEdgePointIndex < 0 || m_movingEdgePointIndex >= static_cast<int>(points.size()))
	{
		assert(m_movingEdgePointIndex >= 0);
		assert(m_movingEdgePointIndex < static_cast<int>(points.size()));
		return;
	}

	QPointF rullerPoint;

	// Calculate new position
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		{
			double diff = m_editEndMovingEdge - m_editStartMovingEdge;

			VFrame30::SchemePoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::SchemePoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::SchemePoint op = points[m_movingEdgePointIndex];
			op.Y += diff;

			points[m_movingEdgePointIndex] = op;

			//
			op = points[m_movingEdgePointIndex + 1];
			op.Y += diff;

			points[m_movingEdgePointIndex + 1] = op;

			rullerPoint.setY(op.Y);

			// Если по сторонам есть еще ГОРИЗОНАТЛЬНЫЕ линии то добавить точку,
			// что бы ребро не тянуло по диагонали соседние отрезки
			//
			if (m_movingEdgePointIndex + 2 < static_cast<int>(points.size()) &&
				std::abs(points[m_movingEdgePointIndex + 2].Y - oldEdgeEnd.Y) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex + 2, oldEdgeEnd);
			}

			if (m_movingEdgePointIndex - 1 >= 0 &&
				std::abs(points[m_movingEdgePointIndex - 1].Y - oldEdgeStart.Y) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex, oldEdgeStart);
			}
		}
		break;
	case MouseState::MovingVerticalEdge:
		{
			double diff = m_editEndMovingEdge - m_editStartMovingEdge;

			VFrame30::SchemePoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::SchemePoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::SchemePoint op = points[m_movingEdgePointIndex];
			op.X += diff;

			points[m_movingEdgePointIndex] = op;
			//
			op = points[m_movingEdgePointIndex + 1];
			op.X += diff;

			points[m_movingEdgePointIndex + 1] = op;

			rullerPoint.setX(op.X);

			// Если по сторонам есть еще ВЕРТИКАЛЬНЫЕ линии то добавить точку,
			// что бы ребро не тянуло по диагонали соседние отрезки
			//
			if (m_movingEdgePointIndex + 2 < static_cast<int>(points.size()) &&
				std::abs(points[m_movingEdgePointIndex + 2].X - oldEdgeEnd.X) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex + 2, oldEdgeEnd);
			}

			if (m_movingEdgePointIndex - 1 >= 0 &&
				std::abs(points[m_movingEdgePointIndex - 1].X - oldEdgeStart.X) < 0.000001)
			{
				points.insert(points.begin() + m_movingEdgePointIndex, oldEdgeStart);
			}
		}
		break;
	case MouseState::MovingConnectionLinePoint:
		{
			double diffX = m_editEndMovingEdgeX - m_editStartMovingEdgeX;
			double diffY = m_editEndMovingEdgeY - m_editStartMovingEdgeY;

			// Сдвинуть предыдущую точку
			//
			if (m_movingEdgePointIndex - 1 >= 0)
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::SchemePoint curPoint = points[index];

				while (index > 0 && sameDirrection == true)
				{
					VFrame30::SchemePoint prevPoint = points[index - 1];

					if (std::abs(prevPoint.X - curPoint.X) < std::abs(prevPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// Линия вертикальная
							prevPoint.X += diffX;
							wasHorz = false;
							wasVert = true;
						}
						else
						{
							sameDirrection = false;
						}
					}
					else
					{
						if (wasHorz == true)
						{
							// Линия горизонтальная
							prevPoint.Y += diffY;
							wasHorz = true;
							wasVert = false;
						}
						else
						{
							sameDirrection = false;
						}
					}

					if (sameDirrection == true)
					{
						curPoint = points[index - 1];
						points[index - 1] = prevPoint;
					}

					index--;
				}
			}

			// Сдвинуть следующую точку
			//
			if (m_movingEdgePointIndex + 1 < static_cast<int>(points.size()))
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::SchemePoint curPoint = points[index];

				while (index + 1 < static_cast<int>(points.size()) && sameDirrection == true)
				{
					VFrame30::SchemePoint nextPoint = points[index + 1];

					if (std::abs(nextPoint.X - curPoint.X) < std::abs(nextPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// Линия вертикальная
							//
							nextPoint.X += diffX;
							wasHorz = false;
							wasVert = true;
						}
						else
						{
							sameDirrection = false;
						}
					}
					else
					{
						if (wasHorz == true)
						{
							// Линия горизонтальная
							//
							nextPoint.Y += diffY;
							wasHorz = true;
							wasVert = false;
						}
						else
						{
							sameDirrection = false;
						}
					}

					if (sameDirrection == true)
					{
						curPoint = points[index + 1];
						points[index + 1] = nextPoint;
					}

					index++;
				}
			}

			// Сдвинуть перемещаемую вершину
			//
			VFrame30::SchemePoint pt = points[m_movingEdgePointIndex];

			pt.X += diffX;
			pt.Y += diffY;

			points[m_movingEdgePointIndex] = pt;

			rullerPoint.setX(pt.X);
			rullerPoint.setY(pt.Y);
		}
		break;

		default:
			void();
	}

	// Set calculated pos to SchemeItem
	//
	pointsList.assign(points.begin(), points.end());
	itemPos->SetPointList(pointsList);
	itemPos->RemoveSamePoints();

	// SavePoints to View, so later they will be used in MouseUp action to set new position
	//
	m_movingVertexPoints = itemPos->GetPointList();

	// Draw item outline
	//
	VFrame30::SchemeItem::DrawOutline(drawParam, m_selectedItems);

	// Draw rullers
	//
	QPainter* p = drawParam->painter();

	QPen outlinePen(Qt::blue);
	outlinePen.setWidth(0);

	QPainter::RenderHints oldrenderhints = p->renderHints();
	p->setRenderHint(QPainter::Antialiasing, false);

	p->setPen(outlinePen);

	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		p->drawLine(QPointF(0.0, rullerPoint.y()), QPointF(scheme()->docWidth(), rullerPoint.y()));
		break;
	case MouseState::MovingVerticalEdge:
		p->drawLine(QPointF(rullerPoint.x(), 0.0), QPointF(rullerPoint.x(), scheme()->docHeight()));
		break;
	case MouseState::MovingConnectionLinePoint:
		p->drawLine(QPointF(rullerPoint.x(), 0.0), QPointF(rullerPoint.x(), scheme()->docHeight()));
		p->drawLine(QPointF(0.0, rullerPoint.y()), QPointF(scheme()->docWidth(), rullerPoint.y()));
		break;
	default:
		assert(false);
		break;
	}
	p->setRenderHints(oldrenderhints);


	// Restore ald position
	//
	si->setPointList(oldPos);

	return;
}

void EditSchemeView::drawGrid(QPainter* p)
{
	assert(p);

	auto unit = scheme()->unit();

	double frameWidth = scheme()->docWidth();
	double frameHeight = scheme()->docHeight();

	double gridSize = scheme()->gridSize();

	double scale = zoom() / 100.0;

	// Thin out the grid
	//
	if (unit == VFrame30::SchemeUnit::Display)
	{
		while (gridSize * scale < 11.0)
		{
			gridSize *= 2;
		}
	}
	else
	{
		// Проредить грид, если точки находятся ближе чем 2 мм друг к другу
		//
		while (gridSize * scale < 2.6 / 25.4)
		{
			gridSize *= 2;
		}
	}

	// Calculate points count
	//
	int horzGridCount = (int)(frameWidth / gridSize);
	int vertGridCount = (int)(frameHeight / gridSize);

	// Drawing grid
	//
	p->setPen(QColor(0x00, 0x00, 0x80, 0xB4));
	QPointF pt;

	QRegion visiblePart = visibleRegion();

	double dpiX = unit == VFrame30::SchemeUnit::Display ? 1.0 : p->device()->logicalDpiX();
	double dpiY = unit == VFrame30::SchemeUnit::Display ? 1.0 : p->device()->logicalDpiY();

	for (int v = 0; v < vertGridCount; v++)
	{
		pt.setY(static_cast<double>(v + 1) * gridSize * dpiY * scale);

		for (int h = 0; h < horzGridCount; h++)
		{
			pt.setX(static_cast<double>(h + 1) * gridSize * dpiX * scale);

			if (visiblePart.contains(pt.toPoint()) == false)
			{
				continue;
			}

			p->drawPoint(pt);
		}
	}

	return;
}

VideoItemAction EditSchemeView::getPossibleAction(VFrame30::SchemeItem* videoItem, QPointF point, int* outMovingEdgePointIndex)
{
	if (videoItem == nullptr)
	{
		assert(videoItem != nullptr);
		return VideoItemAction::NoAction;
	}

	if (videoItem->itemUnit() != scheme()->unit())
	{
		assert(videoItem->itemUnit() == scheme()->unit());
		return VideoItemAction::NoAction;
	}

	if (outMovingEdgePointIndex != nullptr)
	{
		*outMovingEdgePointIndex = -1;
	}

	float controlBarSize = ControlBar(videoItem->itemUnit(), zoom());

	// Координаты schemeItem и point одинакового типа
	//
	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(videoItem) != nullptr)
	{
		VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(videoItem) ;

		// Если внутри прямоугольнка то SchemeItemAction.MoveItem
		//
		if (videoItem->IsIntersectPoint(point.x(), point.y()) == true)
		{
			return VideoItemAction::MoveItem;
		}

		// Проверка на захват управляющих прямоугольников ControlBarSizeIn
		//
		QRectF itemRectangle;		// Нормализированный прямоугольник

		itemRectangle.setX(itemPos->leftDocPt());
		itemRectangle.setY(itemPos->topDocPt());
		itemRectangle.setWidth(itemPos->widthDocPt());
		itemRectangle.setHeight(itemPos->heightDocPt());

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeTopLeft;
		}

		if (QRectF(itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2, itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeTop;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() - controlBarSize, controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeTopRight;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2, controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeRight;
		}

		if (QRectF(itemRectangle.right(), itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeBottomRight;
		}

		if (QRectF(itemRectangle.left() + itemRectangle.width() / 2 - controlBarSize / 2, itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeBottom;
		}

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() + itemRectangle.height(), controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeBottomLeft;
		}

		if (QRectF(itemRectangle.left() - controlBarSize, itemRectangle.top() + itemRectangle.height() / 2 - controlBarSize / 2, controlBarSize, controlBarSize).contains(point) == true)
		{
			return VideoItemAction::ChangeSizeLeft;
		}

		return VideoItemAction::NoAction;
	}

	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(videoItem) != nullptr)
	{
		VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(videoItem) ;

		// Проверка на захват управляющих прямоугольников ControlBarSizeIn
		//
		double x1 = itemPos->startXDocPt();
		double y1 = itemPos->startYDocPt();
		double x2 = itemPos->endXDocPt();
		double y2 = itemPos->endYDocPt();

		// Прямоугольники, за которые можно хвататься и изменять элемент
		//
		QRectF controlRectangles[2];

		controlRectangles[0] = QRectF(x1 - controlBarSize / 2, y1 - controlBarSize / 2, controlBarSize, controlBarSize);
		controlRectangles[1] =  QRectF(x2 - controlBarSize / 2, y2 - controlBarSize/ 2, controlBarSize, controlBarSize);

		if (controlRectangles[0].contains(point) == true)
		{
			return VideoItemAction::MoveStartLinePoint;
		}

		if (controlRectangles[1].contains(point) == true)
		{
			return VideoItemAction::MoveEndLinePoint;
		}

		// Если просто на линии, то SchemeItemAction.MoveItem
		//
		if (videoItem->IsIntersectPoint(point.x(), point.y()) == true)
		{
			return VideoItemAction::MoveItem;
		}

		return VideoItemAction::NoAction;
	}


	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoItem) != nullptr)
	{
		VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoItem) ;

		// Проверка на захват управляющих прямоугольников ControlBarSizeIn
		//
		std::list<VFrame30::SchemePoint> points = itemPos->GetPointList();

		int pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			QRectF controlRect(pt->X - controlBarSize / 2, pt->Y - controlBarSize / 2, controlBarSize, controlBarSize);

			if (controlRect.contains(point.x(), point.y()) == true)
			{
				*outMovingEdgePointIndex = pointIndex;
				return VideoItemAction::MoveConnectionLinePoint;
			}
		}

		// Проверка всех отрезков
		//
		VFrame30::SchemePoint lastPoint;

		pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			if (pt == points.begin())
			{
				lastPoint = *pt;
				continue;
			}

			// Нормализация линии
			//
			double x1 = std::min(lastPoint.X, pt->X);
			double y1 = std::min(lastPoint.Y, pt->Y);
			double x2 = std::max(lastPoint.X, pt->X);
			double y2 = std::max(lastPoint.Y, pt->Y);

			// Расширение линии
			//
			if (std::abs(x1 - x2) < std::abs(y1 - y2))
			{
				// Линия вертикальная
				//
				x1 -= controlBarSize / 4;
				x2 += controlBarSize / 4;

				if (point.x() >= x1 && point.x() <= x2 &&
					point.y() >= y1 && point.y() <= y2)
				{
					*outMovingEdgePointIndex = pointIndex - 1;
					return VideoItemAction::MoveVerticalEdge;
				}
			}
			else
			{
				// Линия горизонтальная
				//
				y1 -= controlBarSize / 4;
				y2 += controlBarSize / 4;

				if (point.x() >= x1 && point.x() <= x2 &&
					point.y() >= y1 && point.y() <= y2)
				{
					*outMovingEdgePointIndex = pointIndex - 1;
					return VideoItemAction::MoveHorizontalEdge;
				}
			}

			//--
			//
			lastPoint = *pt;
		}

		return VideoItemAction::NoAction;
	}

	assert(false);

	return VideoItemAction::NoAction;
}


QUuid EditSchemeView::activeLayerGuid() const
{
	if (m_activeLayer >= static_cast<int>(scheme()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(scheme()->Layers.size()));
		return QUuid();
	}

	return scheme()->Layers[m_activeLayer]->guid();
}

std::shared_ptr<VFrame30::SchemeLayer> EditSchemeView::activeLayer()
{
	if (m_activeLayer >= static_cast<int>(scheme()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(scheme()->Layers.size()));
		return std::make_shared<VFrame30::SchemeLayer>("Error", false);
	}

	return scheme()->Layers[m_activeLayer];
}

void EditSchemeView::setActiveLayer(std::shared_ptr<VFrame30::SchemeLayer> layer)
{
	for (int i = 0; i < static_cast<int>(scheme()->Layers.size()); i++)
	{
		if (scheme()->Layers[i] == layer)
		{
			m_activeLayer = i;
			return;
		}
	}

	// Layer was not found
	//
	assert(false);
	return;
}

MouseState EditSchemeView::mouseState() const
{
	return m_mouseState;
}

void EditSchemeView::setMouseState(MouseState state)
{
	m_mouseState = state;
}

const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& EditSchemeView::selectedItems() const
{
	return m_selectedItems;
}

void EditSchemeView::setSelectedItems(const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items)
{
	// Check if the selected items are the same, don't do anything and don't emit selectionCanged
	//
	if (items.size() == m_selectedItems.size())
	{
		bool differs = false;

		auto i = std::begin(items);
		for (auto s = std::begin(m_selectedItems); s != std::end(m_selectedItems) && differs == false; ++s, ++i)
		{
			if (*s != *i)
			{
				differs = true;
				break;
			}
		}

		if (differs == false)
		{
			return;
		}
	}

	// Set new selection
	//
	m_selectedItems = items;

	emit selectionChanged();
}

void EditSchemeView::setSelectedItems(const std::list<std::shared_ptr<VFrame30::SchemeItem>>& items)
{
	// Check if the selected items are the same, don't do anything and don't emit selectionCanged
	//
	if (items.size() == m_selectedItems.size())
	{
		bool differs = false;

		auto i = std::begin(items);
		for (auto s = std::begin(m_selectedItems); s != std::end(m_selectedItems) && differs == false; ++s, ++i)
		{
			if (*s != *i)
			{
				differs = true;
				break;
			}
		}

		if (differs == false)
		{
			return;
		}
	}

	// Set new selection
	//
	m_selectedItems.clear();
	m_selectedItems.insert(m_selectedItems.begin(), items.begin(), items.end());

	emit selectionChanged();
}

void EditSchemeView::setSelectedItem(const std::shared_ptr<VFrame30::SchemeItem>& item)
{
	if (m_selectedItems.size() == 1 && item == m_selectedItems.back())
	{
		return;
	}

	m_selectedItems.clear();
	m_selectedItems.push_back(item);

	emit selectionChanged();
}

void EditSchemeView::addSelection(const std::shared_ptr<VFrame30::SchemeItem>& item)
{
	auto fp = std::find(std::begin(m_selectedItems), std::end(m_selectedItems), item);

	if (fp == std::end(m_selectedItems))
	{
		m_selectedItems.push_back(item);
		emit selectionChanged();
	}

	return;
}

void EditSchemeView::clearSelection()
{
	if (m_selectedItems.empty() == true)
	{
		return;
	}

	m_selectedItems.clear();
	emit selectionChanged();
}

bool EditSchemeView::removeFromSelection(const std::shared_ptr<VFrame30::SchemeItem>& item)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);

	if (findResult != m_selectedItems.end())
	{
		m_selectedItems.erase(findResult);
		emit selectionChanged();

		// Was found and deleted
		//
		return true;
	}

	// Was not found in selection list
	//
	return false;
}

bool EditSchemeView::isItemSelected(const std::shared_ptr<VFrame30::SchemeItem>& item)
{
	auto findResult = std::find(m_selectedItems.begin(), m_selectedItems.end(), item);
	return findResult != m_selectedItems.end();
}


//
//
// EditVideoFrameWidget
//
//
EditSchemeWidget::EditSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme, const DbFileInfo& fileInfo, DbController* dbController) :
	VFrame30::BaseSchemeWidget(scheme, new EditSchemeView(scheme)),
	m_fileInfo(fileInfo),
	m_dbcontroller(dbController)
{
	assert(scheme != nullptr);
	assert(m_dbcontroller);

	createActions();

	// Left Button Down
	//
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemeWidget::mouseLeftDown_None, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineStartPoint, std::bind(&EditSchemeWidget::mouseLeftDown_AddSchemePosLineStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectStartPoint, std::bind(&EditSchemeWidget::mouseLeftDown_AddSchemePosRectStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionStartPoint, std::bind(&EditSchemeWidget::mouseLeftDown_AddSchemePosConnectionStartPoint, this, std::placeholders::_1)));

	// Left Button Up
	//
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemeWidget::mouseLeftUp_Selection, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemeWidget::mouseLeftUp_Moving, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemeWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemeWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemeWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineEndPoint, std::bind(&EditSchemeWidget::mouseLeftUp_AddSchemePosLineEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectEndPoint, std::bind(&EditSchemeWidget::mouseLeftUp_AddSchemePosRectEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditSchemeWidget::mouseLeftUp_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemeWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemeWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemeWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));

	// Moouse Mov
	//
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Scrolling, std::bind(&EditSchemeWidget::mouseMove_Scrolling, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditSchemeWidget::mouseMove_Selection, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditSchemeWidget::mouseMove_Moving, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditSchemeWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditSchemeWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditSchemeWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineEndPoint, std::bind(&EditSchemeWidget::mouseMove_AddSchemePosLineEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectEndPoint, std::bind(&EditSchemeWidget::mouseMove_AddSchemePosRectEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditSchemeWidget::mouseMove_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditSchemeWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditSchemeWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditSchemeWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));

	// Mouse Right Button Down
	//
	//m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditVideoFrameWidget::mouseRightDown_None, this, std::placeholders::_1)));
	m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditSchemeWidget::mouseRightDown_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));

	// Mouse Right Button Up
	//
	m_mouseRightUpStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditSchemeWidget::mouseRightUp_None, this, std::placeholders::_1)));

	// --
	//
	connect(this, &QWidget::customContextMenuRequested, this, &EditSchemeWidget::contextMenu);
	setCorrespondingContextMenu();

	// Edit Engine
	//
	m_editEngine = new EditEngine::EditEngine(editSchemeView(), horizontalScrollBar(), verticalScrollBar(), this);

	connect(m_editEngine, &EditEngine::EditEngine::stateChanged, this, &EditSchemeWidget::editEngineStateChanged);
	connect(m_editEngine, &EditEngine::EditEngine::modifiedChanged, this, &EditSchemeWidget::modifiedChangedSlot);

	connect(editSchemeView(), &EditSchemeView::selectionChanged, this, &EditSchemeWidget::selectionChanged);

	return;
}

EditSchemeWidget::~EditSchemeWidget()
{
}

void EditSchemeWidget::createActions()
{
	// Escape Button Pressed
	//
	m_escapeAction = new QAction(tr("Escape"), this);
	m_escapeAction->setEnabled(true);
	m_escapeAction->setMenuRole(QAction::NoRole);
	m_escapeAction->setShortcut(QKeySequence(Qt::Key_Escape));
	connect(m_escapeAction, &QAction::triggered, this, &EditSchemeWidget::escapeKey);
	addAction(m_escapeAction);

	//
	// File
	//
	m_fileAction = new QAction(tr("File"), this);
	m_fileAction->setEnabled(true);

	m_fileCheckInAction = new QAction(tr("Check In"), this);
	m_fileCheckInAction->setStatusTip(tr("Check In changes..."));
	m_fileCheckInAction->setEnabled(false);
	connect(m_fileCheckInAction, &QAction::triggered, this, &EditSchemeWidget::checkInFile);

	m_fileCheckOutAction = new QAction(tr("Check Out"), this);
	m_fileCheckOutAction->setStatusTip(tr("Check Out for edit..."));
	m_fileCheckOutAction->setEnabled(false);
	connect(m_fileCheckOutAction, &QAction::triggered, this, &EditSchemeWidget::checkOutFile);

	m_fileUndoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_fileUndoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_fileUndoChangesAction->setEnabled(false);
	connect(m_fileUndoChangesAction, &QAction::triggered, this, &EditSchemeWidget::undoChangesFile);

	m_fileSeparatorAction0 = new QAction(this);
	m_fileSeparatorAction0->setSeparator(true);

	m_fileSaveAction = new QAction(tr("Save"), this);
	m_fileSaveAction->setStatusTip(tr("Save current changes..."));
	m_fileSaveAction->setEnabled(false);
	m_fileSaveAction->setShortcut(QKeySequence::Save);
	connect(m_fileSaveAction, &QAction::triggered, this, &EditSchemeWidget::saveWorkcopy);
	addAction(m_fileSaveAction);

	m_fileSeparatorAction1 = new QAction(this);
	m_fileSeparatorAction1->setSeparator(true);

	m_fileGetWorkcopyAction = new QAction(tr("Get Workcopy..."), this);
	m_fileGetWorkcopyAction->setStatusTip(tr("Get file workcopy"));
	m_fileGetWorkcopyAction->setEnabled(true);
	connect(m_fileGetWorkcopyAction, &QAction::triggered, this, &EditSchemeWidget::getCurrentWorkcopy);

	m_fileSetWorkcopyAction = new QAction(tr("Set Workcopy..."), this);
	m_fileSetWorkcopyAction->setStatusTip(tr("Set file workcopy"));
	m_fileSetWorkcopyAction->setEnabled(false);
	connect(m_fileSetWorkcopyAction, &QAction::triggered, this, &EditSchemeWidget::setCurrentWorkcopy);

	m_fileSeparatorAction2 = new QAction(this);
	m_fileSeparatorAction2->setSeparator(true);

	m_filePropertiesAction = new QAction(tr("Properties..."), this);
	m_filePropertiesAction->setStatusTip(tr("Edit scheme properties"));
	m_filePropertiesAction->setEnabled(true);
	connect(m_filePropertiesAction, &QAction::triggered, this, &EditSchemeWidget::schemeProperties);

	m_fileSeparatorAction3 = new QAction(this);
	m_fileSeparatorAction3->setSeparator(true);

	m_fileCloseAction = new QAction(tr("Close"), this);
	m_fileCloseAction->setStatusTip(tr("Close file"));
	m_fileCloseAction->setEnabled(true);
	m_fileCloseAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_W));
	connect(m_fileCloseAction, &QAction::triggered, [this](bool) { emit closeTab(this); });
	addAction(m_fileCloseAction);

	//
	// Add Item
	//
	m_addAction = new QAction(tr("Add Item"), this);
	m_addAction->setEnabled(true);

	m_addLineAction = new QAction(tr("Line"), this);
	m_addLineAction->setEnabled(true);
	connect(m_addLineAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemeItemLine>(scheme()->unit()));
			});

	m_addConnectionLineAction = new QAction(tr("Connection Line"), this);
	m_addConnectionLineAction->setEnabled(true);
	connect(m_addConnectionLineAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::VideoItemConnectionLine>(scheme()->unit()));
			});

	m_addRectAction = new QAction(tr("Rect"), this);
	m_addRectAction->setEnabled(true);
	connect(m_addRectAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemeItemRect>(scheme()->unit()));
			});

	m_addSeparatorAction0 = new QAction(this);
	m_addSeparatorAction0->setSeparator(true);

	m_addInputSignalAction = new QAction(tr("Input"), this);
	m_addInputSignalAction->setEnabled(true);
	connect(m_addInputSignalAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::VideoItemInputSignal>(scheme()->unit()));
			});

	m_addOutputSignalAction = new QAction(tr("Output"), this);
	m_addOutputSignalAction->setEnabled(true);
	connect(m_addOutputSignalAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::VideoItemOutputSignal>(scheme()->unit()));
			});

	m_addConstantAction = new QAction(tr("Constant"), this);
	m_addConstantAction->setEnabled(true);
	connect(m_addConstantAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::SchemeItemConst>(scheme()->unit()));
			});

	m_addFblElementAction = new QAction(tr("FBL Element"), this);
	m_addFblElementAction->setEnabled(true);
	connect(m_addFblElementAction, &QAction::triggered, this, &EditSchemeWidget::addFblElement);

	m_addLinkAction = new QAction(tr("Link"), this);
	m_addLinkAction->setEnabled(true);
	connect(m_addLinkAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::VideoItemLink>(scheme()->unit()));
			});

	//
	// Edit
	//
	m_editAction = new QAction(tr("Edit"), this);
	m_editAction->setEnabled(true);

	// Edit->Undo
	//
	m_undoAction = new QAction(tr("Undo"), this);
	m_undoAction->setEnabled(false);
	m_undoAction->setShortcut(QKeySequence::Undo);
	connect(m_undoAction, &QAction::triggered, this, &EditSchemeWidget::undo);
	addAction(m_undoAction);

	// Edit->Redo
	//
	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setEnabled(false);
	m_redoAction->setShortcut(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &EditSchemeWidget::redo);
	addAction(m_redoAction);

	// ------------------------------------
	//
	m_editSeparatorAction0 = new QAction(this);
	m_editSeparatorAction0->setSeparator(true);

	// Edit->Select All
	//
	m_selectAllAction = new QAction(tr("Select All"), this);
	m_selectAllAction->setEnabled(true);
	m_selectAllAction->setShortcut(QKeySequence::SelectAll);
	connect(m_selectAllAction, &QAction::triggered, this, &EditSchemeWidget::selectAll);
	addAction(m_selectAllAction);

	// ------------------------------------
	//
	m_editSeparatorAction1 = new QAction(this);
	m_editSeparatorAction1->setSeparator(true);

	// Edit->Cut
	//
	m_editCutAction = new QAction(tr("Cut"), this);
	m_editCutAction->setEnabled(true);
	m_editCutAction->setShortcut(QKeySequence::Cut);
	//connect(m_editCutAction, &QAction::triggered, this, &EditVideoFrameWidget::___);
	addAction(m_editCutAction);

	// Edit->Copy
	//
	m_editCopyAction = new QAction(tr("Copy"), this);
	m_editCopyAction->setEnabled(true);
	m_editCopyAction->setShortcut(QKeySequence::Copy);
	//connect(m_editCopyAction, &QAction::triggered, this, &EditVideoFrameWidget::___);
	addAction(m_editCopyAction);

	// Edit->Paste
	//
	m_editPasteAction = new QAction(tr("Paste"), this);
	m_editPasteAction->setEnabled(true);
	m_editPasteAction->setShortcut(QKeySequence::Paste);
	//connect(m_editPasteAction, &QAction::triggered, this, &EditVideoFrameWidget::___);
	addAction(m_editPasteAction);

	// ------------------------------------
	//
	m_editSeparatorAction2 = new QAction(this);
	m_editSeparatorAction2->setSeparator(true);

	// Edit->Delete
	//
	m_deleteAction = new QAction(tr("Delete"), this);
	m_deleteAction->setEnabled(true);
	m_deleteAction->setMenuRole(QAction::NoRole);
	m_deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));
	connect(m_deleteAction, &QAction::triggered, this, &EditSchemeWidget::deleteKey);
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
	// Shortcuts Alt+Return and Alt+Numeric Enter are different,
	// Look for real call of EditSchemeWidget::properties in keyPressEvent!!!!
	//
	connect(m_propertiesAction, &QAction::triggered, this, &EditSchemeWidget::properties);
	addAction(m_propertiesAction);

	//
	// View
	//
	m_viewAction = new QAction(tr("View"), this);
	m_viewAction->setEnabled(true);

	// View->ZoomIn, creating of these actions was moved to VFrame30::BaseSchemeWidget
	//
//	m_zoomInAction = new QAction(tr("Zoom In"), this);
//	m_zoomInAction->setEnabled(true);
//	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
//	m_zoomInAction->setShortcutContext(Qt::ShortcutContext::WindowShortcut);
//	connect(m_zoomInAction, &QAction::triggered, this, &EditSchemeWidget::zoomIn);
//	addAction(m_zoomInAction);

//	// View->ZoomOut
//	//
//	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
//	m_zoomOutAction->setEnabled(true);
//	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
//	connect(m_zoomOutAction, &QAction::triggered, this, &EditSchemeWidget::zoomOut);
//	addAction(m_zoomOutAction);

//	// View->Zoom100
//	//
//	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
//	m_zoom100Action->setEnabled(true);
//	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
//	connect(m_zoom100Action, &QAction::triggered, this, &EditSchemeWidget::zoom100);
//	addAction(m_zoom100Action);

	// ------------------------------------
	//
	m_viewSeparatorAction0 = new QAction(this);
	m_viewSeparatorAction0->setSeparator(true);

	// View->SnapToGrid
	//
	m_snapToGridAction = new QAction(tr("Snap To Grid"), this);
	m_snapToGridAction->setEnabled(true);
	//connect(m_snapToGridAction, &QAction::triggered, this, &EditVideoFrameWidget::zoom100);


	// High Level Menu
	//
	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);


	// Edit->Properties
	//
	m_layersAction = new QAction(tr("Layers..."), this);
	m_layersAction->setEnabled(true);
	//m_layersAction->setMenuRole(QAction::NoRole);
	//m_layersAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter));
	connect(m_layersAction, &QAction::triggered, this, &EditSchemeWidget::layers);
	addAction(m_layersAction);

	//
	// Create Sub Menus
	//
	m_fileMenu = new QMenu(this);
	m_fileAction->setMenu(m_fileMenu);
		m_fileMenu->addAction(m_fileCheckOutAction);
		m_fileMenu->addAction(m_fileCheckInAction);
		m_fileMenu->addAction(m_fileUndoChangesAction);
		m_fileMenu->addAction(m_fileSeparatorAction0);
		m_fileMenu->addAction(m_fileSaveAction);
		m_fileMenu->addAction(m_fileSeparatorAction1);
		m_fileMenu->addAction(m_fileGetWorkcopyAction);
		m_fileMenu->addAction(m_fileSetWorkcopyAction);
		m_fileMenu->addAction(m_fileSeparatorAction2);
		m_fileMenu->addAction(m_filePropertiesAction);
		m_fileMenu->addAction(m_fileSeparatorAction3);
		m_fileMenu->addAction(m_fileCloseAction);

	m_addMenu = new QMenu(this);
	m_addAction->setMenu(m_addMenu);
		m_addMenu->addAction(m_addLineAction);
		m_addMenu->addAction(m_addConnectionLineAction);
		m_addMenu->addAction(m_addRectAction);
		m_addMenu->addAction(m_addSeparatorAction0);
		m_addMenu->addAction(m_addInputSignalAction);
		m_addMenu->addAction(m_addOutputSignalAction);
		m_addMenu->addAction(m_addConstantAction);
		m_addMenu->addAction(m_addFblElementAction);
		m_addMenu->addAction(m_addLinkAction);

	m_editMenu = new QMenu(this);
	m_editAction->setMenu(m_editMenu);
		m_editMenu->addAction(m_undoAction);
		m_editMenu->addAction(m_redoAction);
		m_editMenu->addAction(m_editSeparatorAction0);
		m_editMenu->addAction(m_selectAllAction);
		m_editMenu->addAction(m_editSeparatorAction1);
		m_editMenu->addAction(m_editCutAction);
		m_editMenu->addAction(m_editCopyAction);
		m_editMenu->addAction(m_editPasteAction);
		m_editMenu->addAction(m_editSeparatorAction2);
		m_editMenu->addAction(m_deleteAction);
		m_editMenu->addAction(m_editSeparatorAction3);
		m_editMenu->addAction(m_propertiesAction);


	m_viewMenu = new QMenu(this);
	m_viewAction->setMenu(m_viewMenu);
		m_viewMenu->addAction(m_zoomInAction);
		m_viewMenu->addAction(m_zoomOutAction);
		m_viewMenu->addAction(m_zoom100Action);
		m_viewMenu->addAction(m_viewSeparatorAction0);
		m_viewMenu->addAction(m_snapToGridAction);

	return;
}

void EditSchemeWidget::keyPressEvent(QKeyEvent* e)
{
	BaseSchemeWidget::keyPressEvent(e);

	// Show properties dialog
	//
	if ((e->modifiers().testFlag(Qt::AltModifier) == true &&		// Alt + numeric keypad Enter
		e->modifiers().testFlag(Qt::KeypadModifier) == true &&
		e->key() == Qt::Key_Enter) ||
		(e->modifiers().testFlag(Qt::AltModifier) == true &&		// Alt + Enter
		e->key() == Qt::Key_Return))
	{
		properties();
	}

}

// Set corresponding to the current situation and user actions context menu
//
void EditSchemeWidget::setCorrespondingContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	return;
}

void EditSchemeWidget::mousePressEvent(QMouseEvent* event)
{
	BaseSchemeWidget::mousePressEvent(event);
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

void EditSchemeWidget::mouseReleaseEvent(QMouseEvent* event)
{
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
		//setMouseCursor(event->pos());

		//event->accept();
		//return;
	}


	setMouseCursor(event->pos());

	//unsetCursor();
	event->ignore();
}

void EditSchemeWidget::mouseMoveEvent(QMouseEvent* event)
{
	BaseSchemeWidget::mouseMoveEvent(event);
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

void EditSchemeWidget::mouseLeftDown_None(QMouseEvent* me)
{
	if (me->modifiers().testFlag(Qt::ShiftModifier) == false)
	{
		QPointF docPoint = widgetPointToDocument(me->pos(), false);

		// Если выделен один объект, и клик на изменение рпазмеров этого объекта или изменение ребер, вершин и т.п.
		//
		if (selectedItems().size() == 1)
		{
			// Проверить установлена ли мышка на месте откудова можно начинанть изменять размер объекта
			//
			int movingEdgePointIndex = 0;
			auto selectedItem = selectedItems()[0];

			VideoItemAction possibleAction = editSchemeView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

			if (dynamic_cast<VFrame30::IVideoItemPosRect*>(selectedItem.get()) != nullptr)
			{
				auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor), std::end(m_sizeActionToMouseCursor),
					[&possibleAction](const SizeActionToMouseCursor& item)
					{
						return item.action == possibleAction;
					}
					);

				if (findResult != std::end(m_sizeActionToMouseCursor))
				{
					// Теперь получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartDocPt = docPoint;
					editSchemeView()->m_editEndDocPt = docPoint;

					setMouseState(findResult->mouseState);

					setMouseCursor(me->pos());
					editSchemeView()->update();
					return;
				}
			}

			// Проверять на изменение вершин линии
			//
			if (dynamic_cast<VFrame30::IVideoItemPosLine*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == VideoItemAction::MoveStartLinePoint)
				{
					// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartDocPt = docPoint;
					editSchemeView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingStartLinePoint);

					setMouseCursor(me->pos());
					editSchemeView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveEndLinePoint)
				{
					// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartDocPt = docPoint;
					editSchemeView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingEndLinePoint);

					setMouseCursor(me->pos());
					editSchemeView()->update();

					return;
				}
			}

			// Проверять на изменение вершин и ребер соединения
			//
			if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == VideoItemAction::MoveHorizontalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartMovingEdge = docPoint.y();
					editSchemeView()->m_editEndMovingEdge = docPoint.y();

					editSchemeView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingHorizontalEdge);

					setMouseCursor(me->pos());
					editSchemeView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveVerticalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartMovingEdge = docPoint.x();
					editSchemeView()->m_editEndMovingEdge = docPoint.x();

					editSchemeView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingVerticalEdge);

					setMouseCursor(me->pos());
					editSchemeView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveConnectionLinePoint)
				{
					assert(movingEdgePointIndex != -1);

					// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					editSchemeView()->m_editStartMovingEdgeX = docPoint.x();
					editSchemeView()->m_editStartMovingEdgeY = docPoint.y();

					editSchemeView()->m_editEndMovingEdgeX = docPoint.x();
					editSchemeView()->m_editEndMovingEdgeY = docPoint.y();

					editSchemeView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingConnectionLinePoint);

					setMouseCursor(me->pos());
					editSchemeView()->update();

					return;
				}
			}
		}

		// Проверить выделенные элементы, на возможность выполнения команды их перемещения
		//
		for (auto si = selectedItems().begin(); si != selectedItems().end(); ++si)
		{
			int movingEdgePointIndex = 0;
			VideoItemAction possibleAction = editSchemeView()->getPossibleAction(si->get(), docPoint, &movingEdgePointIndex);

			if (possibleAction == VideoItemAction::MoveItem)
			{
				// Теперь получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
				//
				docPoint = widgetPointToDocument(me->pos(), snapToGrid());

				editSchemeView()->m_editStartDocPt = docPoint;
				editSchemeView()->m_editEndDocPt = docPoint;

				setMouseState(MouseState::Moving);

				setMouseCursor(me->pos());
				editSchemeView()->update();
				return;
			}
		}

		// Если клик на поверхности которую можно перемещать (выделенный или просто объект)
		// то переход в режим перемещения объекта
		//
		auto itemUnderPoint = editSchemeView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr)
		{
			if (std::find(selectedItems().begin(), selectedItems().end(), itemUnderPoint) != selectedItems().end())
			{
				// Переход в режим перемещения всех выделенных элементов
				// SelectedItems уже заполнен
				//
			}
			else
			{
				// Переход в режим перемещения одного элемента
				//
				editSchemeView()->clearSelection();
				editSchemeView()->setSelectedItem(itemUnderPoint);
			}

			// Получить новые Xin и Yin привязанные к сетке, потомучто старые были для определения наличия элемента под мышой
			//
			docPoint = widgetPointToDocument(me->pos(), snapToGrid());

			editSchemeView()->m_editStartDocPt = docPoint;
			editSchemeView()->m_editEndDocPt = docPoint;

			setMouseState(MouseState::Moving);

			setMouseCursor(me->pos());
			editSchemeView()->update();
			return;
		}
	}

	// Если клик просто поверх элементиа то он выделяется и переходим в режим перемещения элемента
	//
	if (me->modifiers().testFlag(Qt::ShiftModifier) == false)
	{
		editSchemeView()->clearSelection();
	}

	// Выделение элемента или области
	//
	editSchemeView()->m_mouseSelectionStartPoint = widgetPointToDocument(me->pos(), false);
	editSchemeView()->m_mouseSelectionEndPoint = editSchemeView()->m_mouseSelectionStartPoint;

	setMouseState(MouseState::Selection);

	editSchemeView()->update();

	setMouseCursor(me->pos());

	return;
}

void EditSchemeWidget::mouseLeftDown_AddSchemePosLineStartPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);
		resetAction();
		return;
	}

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setStartXDocPt(docPoint.x());
	itemPos->setStartYDocPt(docPoint.y());

	setMouseState(MouseState::AddSchemePosLineEndPoint);

	return;
}

void EditSchemeWidget::mouseLeftDown_AddSchemePosRectStartPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setLeftDocPt(docPoint.x());
	itemPos->setTopDocPt(docPoint.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());

	itemPos->setWidthDocPt(minWidth);
	itemPos->setHeightDocPt(minHeight);

	editSchemeView()->m_addRectStartPoint = docPoint;
	editSchemeView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemeView()->m_addRectEndPoint.setX(itemPos->topDocPt() + itemPos->heightDocPt());

	setMouseState(MouseState::AddSchemePosRectEndPoint);

	editSchemeView()->update();

	return;
}

void EditSchemeWidget::mouseLeftDown_AddSchemePosConnectionStartPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());


	itemPos->DeleteAllPoints();

	itemPos->AddPoint(docPoint.x(), docPoint.y());		        // Сразу добавляется две точки
	itemPos->AddExtensionPoint(docPoint.x(), docPoint.y());

	// Проверить под кординатой нахождение пина
	//
	if (dynamic_cast<VFrame30::FblItem*>(editSchemeView()->m_newItem.get()) != nullptr)
	{
		// ??
		//VFrame30Ext.IFblItem fblItem = schemeView.newItem as VFrame30Ext.IFblItem;
	}

	setMouseState(MouseState::AddSchemePosConnectionNextPoint);

	return;
}

void EditSchemeWidget::mouseLeftUp_Selection(QMouseEvent* me)
{
	// Определить какие элеиенты попали в выделенную область,
	// добавить их в selectedItem
	//
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	// может быть такое что шифт был нажат, но в процессе выделения его отпстили,
	// поэтому и удаляются все элементы из выделения
	//
	if (shiftIsPressed == false)
	{
		editSchemeView()->clearSelection();
	}

	editSchemeView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);

	// Высислить координаты выделения для листа
	//
	QRectF pageSelectionArea = QRectF(editSchemeView()->m_mouseSelectionStartPoint, editSchemeView()->m_mouseSelectionEndPoint);

	// Поиск элементов внутри pageSelectionArea
	//
	auto activeLayer = editSchemeView()->activeLayer();

	// Если клик без движения мышки, то выделение касается только верхнего элемента
	//
	if (editSchemeView()->m_mouseSelectionStartPoint == editSchemeView()->m_mouseSelectionEndPoint)
	{
		auto item = activeLayer->getItemUnderPoint(pageSelectionArea.topLeft());

		if (item != nullptr)
		{
			// Если такой элемент уже есть в списке, то удалить его из списка выделенных
			//
			bool wasDeleted = editSchemeView()->removeFromSelection(item);

			if (wasDeleted == false)
			{
				// This item was not selected, so just select it
				//
				editSchemeView()->addSelection(item);
			}
		}
	}
	else
	{
		// Прямоугольная облась выделения , выделяеються (или развыделяются)
		// все элементы входязщие в прямоугольник
		//
		auto itemsInRect = activeLayer->getItemListInRectangle(pageSelectionArea);

		for (auto item = itemsInRect.begin(); item != itemsInRect.end(); ++item)
		{
			// Если такой элемент уже есть в списке, то удалить его из списка выделенных
			//
			auto findResult = std::find(selectedItems().begin(), selectedItems().end(), *item);

			if (findResult != selectedItems().end())
			{
				editSchemeView()->removeFromSelection(*item);
			}
			else
			{
				editSchemeView()->addSelection(*item);
			}
		}
	}

	// --
	//
	editSchemeView()->m_mouseSelectionStartPoint = QPoint();
	editSchemeView()->m_mouseSelectionEndPoint = QPoint();

	resetAction();

	return;
}

void EditSchemeWidget::mouseLeftUp_Moving(QMouseEvent* event)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() != true);
		return;
	}

	QPointF mouseMovingStartPointIn = editSchemeView()->m_editStartDocPt;
	QPointF mouseMovingEndPointIn = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemeView()->m_editEndDocPt = mouseMovingEndPointIn;

	float xdif = mouseMovingEndPointIn.x() - mouseMovingStartPointIn.x();
	float ydif = mouseMovingEndPointIn.y() - mouseMovingStartPointIn.y();

	if (std::abs(xdif) < 0.0000001 && std::abs(ydif) < 0.0000001)
	{
		// VideoItem's have not changed positions
		//
		resetAction();
		return;
	}

	bool ctrlIsPressed = event->modifiers().testFlag(Qt::ControlModifier);

	if (ctrlIsPressed == false)
	{
		// Move items
		//
		m_editEngine->runMoveItem(xdif, ydif, selectedItems(), snapToGrid());
	}
	else
	{
		// Copy VideoItems and move copied items
		//
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> newItems;

		std::for_each(selectedItems().begin(), selectedItems().end(),
			[xdif, ydif, &newItems](const std::shared_ptr<VFrame30::SchemeItem>& si)
			{
				QByteArray data;

				bool result = si->Save(data);

				if (result == false || data.isEmpty() == true)
				{
					assert(result == true);
					assert(data.isEmpty() == false);
					return;
				}

				VFrame30::SchemeItem* newItemRawPtr = VFrame30::SchemeItem::Create(data);
				if (newItemRawPtr == nullptr)
				{
					assert(newItemRawPtr != nullptr);
					return;
				}

				std::shared_ptr<VFrame30::SchemeItem> newItem(newItemRawPtr);

				newItem->setNewGuid();

				newItem->MoveItem(xdif, ydif);

				newItems.push_back(newItem);
				return;
			});

		m_editEngine->runAddItem(newItems, editSchemeView()->activeLayer());
	}

	resetAction();
	return;
}

void EditSchemeWidget::mouseLeftUp_SizingRect(QMouseEvent* event)
{
	if (mouseState() != MouseState::SizingTopLeft &&
		mouseState() != MouseState::SizingTop &&
		mouseState() != MouseState::SizingTopRight &&
		mouseState() != MouseState::SizingRight &&
		mouseState() != MouseState::SizingBottomRight &&
		mouseState() != MouseState::SizingBottom &&
		mouseState() != MouseState::SizingBottomLeft &&
		mouseState() != MouseState::SizingLeft)
	{
		return;
	}

	if (editSchemeView()->m_editStartDocPt.isNull() == true ||
		editSchemeView()->m_editEndDocPt.isNull() == true)
	{
		assert(editSchemeView()->m_editStartDocPt.isNull() == false);
		assert(editSchemeView()->m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	QPointF mouseSizingStartPointDocPt = editSchemeView()->m_editStartDocPt;
	QPointF mouseSizingEndPointDocPt = widgetPointToDocument(event->pos(), snapToGrid());

	auto si = selectedItems().front();
	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	double xdif = mouseSizingEndPointDocPt.x() - mouseSizingStartPointDocPt.x();
	double ydif = mouseSizingEndPointDocPt.y() - mouseSizingStartPointDocPt.y();

	// set new pos
	//
	double x1 = itemPos->leftDocPt();
	double y1 = itemPos->topDocPt();
	double x2 = x1 + itemPos->widthDocPt();
	double y2 = y1 + itemPos->heightDocPt();

	double minWidth = itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		x1 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y1
		{
			y1 = y2 - minHeight;
		}
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x2
		{
			x2 = x1 + minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		if (y2 - y1 < minHeight)	// y2
		{
			y2 = y1 + minHeight;
		}
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		if (x2 - x1 < minWidth)		// x1
		{
			x1 = x2 - minWidth;
		}
		break;
	default:
		assert(false);
		break;
	}

	// --
	//
	std::vector<VFrame30::SchemePoint> itemPoints;

	itemPoints.push_back(VFrame30::SchemePoint(std::min(x1, x2), std::min(y1, y2)));
	itemPoints.push_back(VFrame30::SchemePoint(std::min(x1, x2) + std::abs(x2 - x1), std::min(y1, y2) + std::abs(y2 - y1)));

	m_editEngine->runSetPoints(itemPoints, si);

	resetAction();
	return;
}

void EditSchemeWidget::mouseLeftUp_MovingLinePoint(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingStartLinePoint &&
		mouseState() != MouseState::MovingEndLinePoint)
	{
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	auto si = selectedItems().front();
	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(selectedItems().front().get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		return;
	}

	std::vector<VFrame30::SchemePoint> points(2);

	QPointF spt = editSchemeView()->m_editStartDocPt;
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
		points[0] = static_cast<VFrame30::SchemePoint>(QPointF(itemPos->startXDocPt() + xdif,itemPos->startYDocPt() + ydif));
		points[1] = static_cast<VFrame30::SchemePoint>(QPointF(itemPos->endXDocPt(), itemPos->endYDocPt()));
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		points[0] = static_cast<VFrame30::SchemePoint>(QPointF(itemPos->startXDocPt(),itemPos->startYDocPt()));
		points[1] = static_cast<VFrame30::SchemePoint>(QPointF(itemPos->endXDocPt() + xdif, itemPos->endYDocPt() + ydif));
	}

	m_editEngine->runSetPoints(points, si);

	//--
	//
	resetAction();
	return;
}

void EditSchemeWidget::mouseLeftUp_AddSchemePosLineEndPoint(QMouseEvent* event)
{
	assert(editSchemeView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemeView()->m_newItem.reset();
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
		m_editEngine->runAddItem(editSchemeView()->m_newItem, editSchemeView()->activeLayer());
	}

	resetAction();

	return;
}

void EditSchemeWidget::mouseLeftUp_AddSchemePosRectEndPoint(QMouseEvent* event)
{
	assert(editSchemeView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemeView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemeView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemeView()->m_addRectStartPoint;
	QPointF ep = editSchemeView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());
	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());

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
		// Добавить элемент в активный слой
		//
		m_editEngine->runAddItem(editSchemeView()->m_newItem, editSchemeView()->activeLayer());
	}

	resetAction();

	return;
}

void EditSchemeWidget::mouseLeftUp_AddSchemePosConnectionNextPoint(QMouseEvent*)
{
	assert(editSchemeView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		editSchemeView()->m_newItem.reset();
		return;
	}

	auto points = itemPos->GetPointList();

	if (points.size() >= 2)
	{
		itemPos->RemoveSamePoints();
		itemPos->DeleteAllExtensionPoints();

		// Если линия началась или окончилась на таком же элементе, то соединить эти две (или три) линии
		//
		VFrame30::SchemePoint startPoint = points.front();
		VFrame30::SchemePoint endPoint = points.back();

		// Поиск элементов которые лежат на точках startPoint, endPoint
		//
		QUuid startItemGuid = QUuid();			// Здесь сохранится ИД элемента линии которая будет подключенна к Первой точке новой линии
		bool startPointAddedToOther = false;	// Новый элемент был присоединен к существующему (конечные точки лежат на одной координте)
		bool endPointAddedToOther = false;		// Новый элемент был присоединен к существующему (конечные точки лежат на одной координте)

		auto schemeItemStartPoint = activeLayer()->getItemUnderPoint(QPointF(startPoint.X, startPoint.Y));
		auto schemeItemEndPoint = activeLayer()->getItemUnderPoint(QPointF(endPoint.X, endPoint.Y));

		if (schemeItemStartPoint != nullptr &&
			schemeItemStartPoint->metaObject()->className() == editSchemeView()->m_newItem->metaObject()->className())
		{
			// Это такой же эелемент, если точка schemeItemStartPoint лежит на первой или последней точке schemeItemStartPoint,
			// то объеденить schemeItemStartPoint и новый элемент
			//
			assert(dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemStartPoint.get()) != nullptr);

			VFrame30::IVideoItemPosConnection* existingItemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemStartPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - startPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - startPoint.Y) < 0.000001)
			{
				// Стартовая точка новой линии лежит на стартовой точке старой линии
				//
				startItemGuid = schemeItemStartPoint->guid();		// Сохранить, что бы потом не подключить к этой же линии и последнюю точку, что бы не получилось кольцо
				startPointAddedToOther = true;

				// --
				points.reverse();
				existingItemPoints.pop_front();
				points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());
				points.reverse();								// Если будет объединение по последней точке, то этот Recerse очень важен

				std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
				m_editEngine->runSetPoints(newPoints, schemeItemStartPoint);
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - startPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - startPoint.Y) < 0.000001)
				{
					// ПЕРВАЯ точка новой линии лежит на ПОСЛЕДНЕЙ точке существующей линии
					//
					startItemGuid = schemeItemStartPoint->guid();	// Сохранить, что бы потом не подключить к этой же линии и последнюю точку, что бы не получилось кольцо
					startPointAddedToOther = true;

					// --
					points.pop_front();

					existingItemPoints.insert(existingItemPoints.end(), points.begin(), points.end());
					points.clear();
					points.assign(existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemStartPoint);
				}
			}
		}

		// --
		//
		if (schemeItemEndPoint != nullptr &&
			schemeItemEndPoint->metaObject()->className() == editSchemeView()->m_newItem->metaObject()->className() &&	// Должен быть такой же тип элемента
			schemeItemEndPoint->guid() != startItemGuid)								// Элемент не должен быть в пердыдущем условии присоединен к ЭТОЙ ЖЕ линии
		{
			// Это такой же эелемент, если точка schemeItemEndPoint лежит на первой или последней точке schemeItemEndPoint,
			// то объеденить schemeItemEndPoint и новый элемент
			//
			assert(dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemEndPoint.get()) != nullptr);
			VFrame30::IVideoItemPosConnection* existingItemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemEndPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - endPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - endPoint.Y) < 0.000001)
			{
				// ПОСЛЕДНЯЯ точка новой линии лежит на Первой точке старой линии
				//
				endPointAddedToOther = true;

				if (startPointAddedToOther == true)	// Новая линия уже была поглащена элементом schemeItemStartPoint
				{
					// Объединения двух элемнтов - schemeItemStartPoint и schemeItemEndPoint
					//
					existingItemPoints.pop_front();
					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					m_editEngine->runDeleteItem(schemeItemStartPoint, activeLayer());

					std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
				}
				else
				{
					// К элементу schemeItemEndPoint добавить points
					//
					existingItemPoints.pop_front();

					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
				}
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - endPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - endPoint.Y) < 0.000001)
				{
					// ПОСЛЕДНЯЯ точка новой линии лежит на ПОСЛЕДНЕЙ точке существующей линии
					//
					endPointAddedToOther = true;

					if (startPointAddedToOther == true)	// Новая линия уже была поглащена элементом schemeItemStartPoint
					{
						// Объединения двух элемнтов - schemeItemStartPoint и schemeItemEndPoint
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						m_editEngine->runDeleteItem(schemeItemStartPoint, activeLayer());

						std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
						m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
					}
					else
					{
						// К элементу schemeItemEndPoint добавить points
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						std::vector<VFrame30::SchemePoint> newPoints(points.begin(), points.end());
						m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
					}
				}
			}
		}

		if (startPointAddedToOther == false && endPointAddedToOther == false)
		{
			m_editEngine->runAddItem(editSchemeView()->m_newItem, activeLayer());
		}
	}

	resetAction();

	return;
}

void EditSchemeWidget::mouseLeftUp_MovingEdgeOrVertex(QMouseEvent*)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		resetAction();
		return;
	}

	auto si = selectedItems().front();
	assert(si != nullptr);

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	if ((mouseState() == MouseState::MovingHorizontalEdge || mouseState() == MouseState::MovingVerticalEdge) &&
		std::abs(editSchemeView()->m_editEndMovingEdge - editSchemeView()->m_editStartMovingEdge) < 0.000001)
	{
		// изменения координат небыло, значит и не надо выполнять команду
		//
		resetAction();
		return;
	}

	if (mouseState() == MouseState::MovingConnectionLinePoint &&
		std::abs(editSchemeView()->m_editEndMovingEdgeX - editSchemeView()->m_editStartMovingEdgeX) < 0.000001 &&
		std::abs(editSchemeView()->m_editEndMovingEdgeY - editSchemeView()->m_editStartMovingEdgeY) < 0.000001)
	{
		// изменения координат небыло, значит и не надо выполнять команду
		//
		resetAction();
		return;
	}

	std::vector<VFrame30::SchemePoint> setPoints(editSchemeView()->m_movingVertexPoints.begin(), editSchemeView()->m_movingVertexPoints.end());
	m_editEngine->runSetPoints(setPoints, si);

	resetAction();
	return;
}

void EditSchemeWidget::mouseMove_Scrolling(QMouseEvent*)
{
	// To Do
	assert(false);
	return;
}

void EditSchemeWidget::mouseMove_Selection(QMouseEvent* me)
{
	// Выполнить перерисовку выделения.
	//
	editSchemeView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);
	editSchemeView()->update();

	return;
}

void EditSchemeWidget::mouseMove_Moving(QMouseEvent* me)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() == false);
		setMouseState(MouseState::None);
		return;
	}

	editSchemeView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	editSchemeView()->update();
	return;
}

void EditSchemeWidget::mouseMove_SizingRect(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	auto si = selectedItems().front();

	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(si.get()) == nullptr)
	{
		assert(dynamic_cast<VFrame30::IVideoItemPosRect*>(si.get()) != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemeView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	editSchemeView()->update();
	return;
}

void EditSchemeWidget::mouseMove_MovingLinePoint(QMouseEvent* me)
{
	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		setMouseState(MouseState::None);
		return;
	}

	auto si = selectedItems().front();

	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(si.get()) == nullptr)
	{
		assert(dynamic_cast<VFrame30::IVideoItemPosLine*>(si.get()) != nullptr);
		setMouseState(MouseState::None);
		return;
	}

	editSchemeView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());
	editSchemeView()->update();
	return;
}

void EditSchemeWidget::mouseMove_AddSchemePosLineEndPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(editSchemeView()->m_newItem.get());

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

	editSchemeView()->update();
	return;
}

void EditSchemeWidget::mouseMove_AddSchemePosRectEndPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	editSchemeView()->m_addRectEndPoint = docPoint;

	QPointF sp = editSchemeView()->m_addRectStartPoint;
	QPointF ep = editSchemeView()->m_addRectEndPoint;

	itemPos->setLeftDocPt(sp.x());
	itemPos->setTopDocPt(sp.y());


	itemPos->setWidthDocPt(ep.x() - sp.x());
	itemPos->setHeightDocPt(ep.y() - sp.y());

	double minWidth = itemPos->minimumPossibleWidthDocPt(scheme()->gridSize(), scheme()->pinGridStep());
	double minHeight = itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());

	if (itemPos->widthDocPt() < minWidth)
	{
		itemPos->setWidthDocPt(minWidth);
	}
	if (itemPos->heightDocPt() < minHeight)
	{
		itemPos->setHeightDocPt(minHeight);
	}

	editSchemeView()->m_addRectEndPoint.setX(itemPos->leftDocPt() + itemPos->widthDocPt());
	editSchemeView()->m_addRectEndPoint.setY(itemPos->topDocPt() + itemPos->heightDocPt());

	// --
	//
	editSchemeView()->update();

	return;
}

void EditSchemeWidget::mouseMove_AddSchemePosConnectionNextPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	auto points = itemPos->GetPointList();
	auto extPoints = itemPos->GetExtensionPoints();
	VFrame30::SchemePoint ptBase;

	if (points.empty() == true || extPoints.empty() == true)
	{
		assert(points.size() > 0);
		assert(extPoints.size() > 0);
		return;
	}

	if (points.size() + extPoints.size() < 2)
	{
		assert(points.size() + extPoints.size() >= 2);
		return;
	}

	if (extPoints.size() > 1)
	{
		ptBase = *std::prev(extPoints.end(), 2);		// extPoints[extPoints.Count - 2];
	}
	else
	{
		ptBase = points.back();							// ptBase = points[points.Count - 1];
	}

	QLineF v;
	v.setP1(ptBase);
	v.setP2(docPoint);

	// Выровнять угол до кратного 90 градусам значению, прием к ближаейшему
	//
	double angle = std::floor(v.angle() / 90.0 + 0.5) * 90.0;


	// сделать новый, горизонтальный вектор, и повернуть его на угол
	//
	QLineF v2;
	v2.setP1(ptBase);
	v2.setP2(QPointF(ptBase.X + v.length(), ptBase.Y));
	v2.setAngle(angle);

	QPointF newPoint = v2.p2();

	if (snapToGrid() == true)
	{
		newPoint = snapToGrid(newPoint);
	}

	itemPos->DeleteLastExtensionPoint();
	itemPos->AddExtensionPoint(newPoint.x(), newPoint.y());

	editSchemeView()->update();

	// Найти точки к торым может "прилипнуть" край.... смотри некоторую реализацию этого в старом проекте
	//

	return;
}

void EditSchemeWidget::mouseMove_MovingEdgesOrVertex(QMouseEvent* event)
{
	if (mouseState() != MouseState::MovingHorizontalEdge &&
		mouseState() != MouseState::MovingVerticalEdge &&
		mouseState() != MouseState::MovingConnectionLinePoint)
	{
		assert(false);
		resetAction();
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		resetAction();
		return;
	}

	auto si = selectedItems().front();
	assert(si != nullptr);

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(si.get());
	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		editSchemeView()->m_editEndMovingEdge = docPoint.y();
		break;
	case MouseState::MovingVerticalEdge:
		editSchemeView()->m_editEndMovingEdge = docPoint.x();
		break;
	case MouseState::MovingConnectionLinePoint:
		editSchemeView()->m_editEndMovingEdgeX = docPoint.x();
		editSchemeView()->m_editEndMovingEdgeY = docPoint.y();
		break;
	default:
		assert(false);
	}

	editSchemeView()->update();

	return;
}

void EditSchemeWidget::mouseRightDown_None(QMouseEvent*)
{
	// CURRENTLY THIS ACTION IS DISABLED IN CONSTRUCTOR, ADD IT TO THE RightClickPress array
	//
	// To Do from old project
	//
	return;
}

void EditSchemeWidget::mouseRightDown_AddSchemePosConnectionNextPoint(QMouseEvent* event)
{
	if (editSchemeView()->m_newItem == nullptr)
	{
		assert(editSchemeView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(editSchemeView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	//QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	auto extPoints = itemPos->GetExtensionPoints();

	if (extPoints.empty() == true)
	{
		assert(extPoints.size() > 0);
		return;
	}

	auto lastExtPt = extPoints.back();

	itemPos->AddPoint(lastExtPt.X, lastExtPt.Y);

	itemPos->DeleteAllExtensionPoints();
	itemPos->AddExtensionPoint(lastExtPt.X, lastExtPt.Y);

	// --
	//
	editSchemeView()->update();

	return;
}

void EditSchemeWidget::mouseRightUp_None(QMouseEvent* event)
{
	QPointF docPoint = widgetPointToDocument(event->pos(), false);

	auto item = editSchemeView()->activeLayer()->getItemUnderPoint(docPoint);

	if (item == nullptr)
	{
		editSchemeView()->clearSelection();
		resetAction();
	}
	else
	{
		bool itemIsAlreadySelected = editSchemeView()->isItemSelected(item);

		if (itemIsAlreadySelected == false)
		{
			editSchemeView()->setSelectedItem(item);
			resetAction();
		}
	}

	return;
}

DbController* EditSchemeWidget::dbcontroller()
{
	return m_dbcontroller;
}

DbController* EditSchemeWidget::db()
{
	return m_dbcontroller;
}

EditSchemeView* EditSchemeWidget::editSchemeView()
{
	EditSchemeView* sw = dynamic_cast<EditSchemeView*>(schemeView());
	assert(sw != nullptr);
	return sw;
}

const EditSchemeView* EditSchemeWidget::editSchemeView() const
{
	const EditSchemeView* sw = dynamic_cast<const EditSchemeView*>(schemeView());
	assert(sw != nullptr);
	return sw;
}

const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& EditSchemeWidget::selectedItems() const
{
	return editSchemeView()->m_selectedItems;
}

std::shared_ptr<VFrame30::SchemeLayer> EditSchemeWidget::activeLayer()
{
	return editSchemeView()->activeLayer();
}


QPointF EditSchemeWidget::widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const
{
	QPointF result = BaseSchemeWidget::widgetPointToDocument(widgetPoint);

	if (snapToGrid == true)
	{
		QPointF snapped = this->snapToGrid(result);
		return snapped;
	}

	return result;
}

QPointF EditSchemeWidget::snapToGrid(QPointF pt) const
{
	double gridSize = scheme()->gridSize();

	QPointF result = CUtils::snapToGrid(pt, gridSize);
	return result;
}

void EditSchemeWidget::addItem(std::shared_ptr<VFrame30::SchemeItem> newItem)
{
	if (newItem == nullptr)
	{
		assert(newItem != nullptr);
		return;
	}

	editSchemeView()->m_newItem = newItem;

	bool posInterfaceFound = false;

	// Добавляется элемент с расположением ISchemePosLine
	//
	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosLineStartPoint);
	}

	// Добавляется элемент с расположением ISchemePosRect
	//
	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosRectStartPoint);

		VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(newItem.get());
		itemPos->minimumPossibleHeightDocPt(scheme()->gridSize(), scheme()->pinGridStep());		// chachedGridSize and pinStep will be initialized here
	}

	// Добавляется элемент с расположением ISchemePosConnection
	//
	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosConnectionStartPoint);
	}

	if (posInterfaceFound == true)
	{
		// Интефейс элемента найден, дальше изменить состояние мышки
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

void EditSchemeWidget::setMouseCursor(QPoint mousePos)
{
	setCursor(QCursor(Qt::CursorShape::ArrowCursor));

	for (size_t i = 0; i < sizeof(m_mouseStateCursor) / sizeof(m_mouseStateCursor[0]); i++)
	{
		if (mouseState() == m_mouseStateCursor[i].mouseState)
		{
			QCursor cursor(m_mouseStateCursor[i].cursorShape);
			setCursor(cursor);

			return;
		}
	}

	// ЕСЛИ ВИСНЕТ ГДЕ ТО ЗДЕСЬ, ТО ТЫ ДОБАВИЛ КАКОЕТО ДЕЙСТВИЕ И НЕ ДОБАВИЛ ЗАПИСЬ В mouseStateToCursor!!!!!!!
	//
	int movingEdgePointIndex = -1;

	// Частные случаи установки курсора
	//
	if (mouseState() == MouseState::None)
	{
		// Определить над каким элементом мышка, и что с ним можно сделать
		//
		// Convert pixels to document points
		//
		QPointF docPos = widgetPointToDocument(mousePos, false);

		if (selectedItems().empty() == true)
		{
			auto itemUnderPoint = editSchemeView()->activeLayer()->getItemUnderPoint(docPos);

			// Если элемент не выделен, то его можно только перемещать
			//
			if (itemUnderPoint != nullptr &&
				editSchemeView()->getPossibleAction(itemUnderPoint.get(), docPos, &movingEdgePointIndex) == VideoItemAction::MoveItem)
			{
				setCursor(Qt::SizeAllCursor);
				return;
			}
		}

		for (auto si = editSchemeView()->selectedItems().begin(); si != editSchemeView()->selectedItems().end(); ++si)
		{
			VideoItemAction possibleAction = editSchemeView()->getPossibleAction(si->get(), docPos, &movingEdgePointIndex);

			if (possibleAction != VideoItemAction::NoAction)
			{
				// Changing size, is possible for only one selected object
				//
				if (editSchemeView()->selectedItems().size() == 1)
				{
					auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor), std::end(m_sizeActionToMouseCursor),
						[&possibleAction](const SizeActionToMouseCursor& c) -> bool
						{
							return c.action == possibleAction;
						}
						);

					if (findResult != std::end(m_sizeActionToMouseCursor))
					{
						//qDebug() << Q_FUNC_INFO << static_cast<int>(findResult->cursorShape);
						setCursor(findResult->cursorShape);
						return;
					}
				}

				// --
				//
				switch (possibleAction)
				{
				case VideoItemAction::MoveItem:
					setCursor(Qt::SizeAllCursor);
					return;
				case VideoItemAction::MoveStartLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				case VideoItemAction::MoveEndLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				case VideoItemAction::MoveHorizontalEdge:
					setCursor(Qt::SplitVCursor);
					return;
				case VideoItemAction::MoveVerticalEdge:
					setCursor(Qt::SplitHCursor);
					return;
				case VideoItemAction::MoveConnectionLinePoint:
					setCursor(Qt::SizeAllCursor);
					return;
				default:
					void();
				}
			}
		}

		QCursor cursor(Qt::ArrowCursor);
		setCursor(cursor);
		return;
	}

	// Назначение курсора для создания нового элемента по типам
	//
	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(editSchemeView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(editSchemeView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(editSchemeView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	return;
}

void EditSchemeWidget::resetAction()
{
	setMouseState(MouseState::None);
	editSchemeView()->m_newItem.reset();

	editSchemeView()->m_mouseSelectionStartPoint = QPoint();
	editSchemeView()->m_mouseSelectionEndPoint = QPoint();
	editSchemeView()->m_editStartDocPt = QPointF();
	editSchemeView()->m_editEndDocPt = QPointF();

	editSchemeView()->m_movingVertexPoints.clear();

	setMouseCursor(mapFromGlobal(QCursor::pos()));

	editSchemeView()->update();

	return;
}

void EditSchemeWidget::clearSelection()
{
	editSchemeView()->clearSelection();
}

void EditSchemeWidget::contextMenu(const QPoint& pos)
{
	if (mouseState() == MouseState::AddSchemePosConnectionNextPoint)
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

	// Disable some actions in ReadOnly mode
	//
	m_addAction->setDisabled(readOnly());
	m_editPasteAction->setDisabled(readOnly());
	m_deleteAction->setDisabled(readOnly());

	// Version Control enable/disable items
	//
	m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);
	m_fileCheckInAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileCheckOutAction->setEnabled(readOnly() == true && fileInfo().state() == VcsState::CheckedIn);
	m_fileUndoChangesAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);
	m_fileGetWorkcopyAction->setEnabled(true);
	m_fileSetWorkcopyAction->setEnabled(readOnly() == false && fileInfo().state() == VcsState::CheckedOut);

	m_propertiesAction->setDisabled(editSchemeView()->selectedItems().empty());

	// Compose menu
	//
	QMenu menu(this);

	QList<QAction*> actions;

	actions << m_fileAction;
	actions << m_addAction;
	actions << m_editAction;

	actions << m_separatorAction0;
	actions << m_layersAction;
	actions << m_propertiesAction;

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void EditSchemeWidget::escapeKey()
{
	if (mouseState() != MouseState::None)
	{
		resetAction();
	}
	else
	{
		editSchemeView()->clearSelection();
	}

	editSchemeView()->update();
	return;
}

void EditSchemeWidget::deleteKey()
{
	auto selection = editSchemeView()->selectedItems();

	if (mouseState() == MouseState::None &&
		selection.empty() == false)
	{
		m_editEngine->runDeleteItem(selection, activeLayer());
	}

	return;
}

void EditSchemeWidget::undo()
{
	m_editEngine->undo(1);

	if (m_schemePropertiesDialog != nullptr && m_schemePropertiesDialog->isVisible())
	{
		m_schemePropertiesDialog->setScheme(scheme());
	}
}

void EditSchemeWidget::redo()
{
	m_editEngine->redo(1);

	if (m_schemePropertiesDialog != nullptr && m_schemePropertiesDialog->isVisible())
	{
		m_schemePropertiesDialog->setScheme(scheme());
	}
}

void EditSchemeWidget::editEngineStateChanged(bool canUndo, bool canRedo)
{
	if (m_undoAction == nullptr ||
		m_redoAction == nullptr)
	{
		assert(m_undoAction);
		assert(m_redoAction);
		return;
	}

	m_undoAction->setEnabled(canUndo);
	m_redoAction->setEnabled(canRedo);

	return;
}

void EditSchemeWidget::modifiedChangedSlot(bool modified)
{
	m_fileSaveAction->setEnabled(modified);
	emit modifiedChanged(modified);
	return;
}

void EditSchemeWidget::selectAll()
{
	editSchemeView()->clearSelection();

	std::vector<std::shared_ptr<VFrame30::SchemeItem>> items;
	items.assign(editSchemeView()->activeLayer()->Items.begin(), editSchemeView()->activeLayer()->Items.end());

	editSchemeView()->setSelectedItems(items);

	editSchemeView()->update();
	return;
}

void EditSchemeWidget::schemeProperties()
{
	if (m_schemePropertiesDialog == nullptr)
	{
		m_schemePropertiesDialog = new SchemePropertiesDialog(m_editEngine, this);
	}

	m_schemePropertiesDialog->setScheme(scheme());
	m_schemePropertiesDialog->show();
	return;
}

void EditSchemeWidget::properties()
{
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemeItemPropertiesDialog(m_editEngine, this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemeView()->selectedItems());
	m_itemsPropertiesDialog->show();
	return;
}

void EditSchemeWidget::layers()
{
	SchemeLayersDialog schemeLayersDialog(editSchemeView(), this);
	if (schemeLayersDialog.exec() == QDialog::Accepted)
	{

	}
	update();
	return;
}

void EditSchemeWidget::selectionChanged()
{
	if (m_itemsPropertiesDialog == nullptr)
	{
		m_itemsPropertiesDialog = new SchemeItemPropertiesDialog(m_editEngine, this);
	}

	m_itemsPropertiesDialog->setObjects(editSchemeView()->selectedItems());
	return;
}

void EditSchemeWidget::addFblElement()
{

	// Get available Afb list
	//
	std::vector<DbFileInfo> fileList;

	bool result = db()->getFileList(&fileList, db()->afblFileId(), "afb", this);
	if (result == false || fileList.empty() == true)
	{
		return;
	}

	// Read all Afb's and refresh it in scheme
	//
	std::vector<std::shared_ptr<DbFile>> files;
	result = db()->getLatestVersion(fileList, &files, this);
	if (result == false)
	{
		return;
	}

	std::vector<std::shared_ptr<Afbl::AfbElement>> elements;
	elements.reserve(files.size());

	for (auto& f : files)
	{
		std::shared_ptr<Afbl::AfbElement> afb = std::make_shared<Afbl::AfbElement>();
		result = afb->loadFromXml(f->data());

		elements.push_back(afb);
	}

	scheme()->setAfbCollection(elements);

	ChooseAfbDialog* dialog = new ChooseAfbDialog(elements, this);

	if (dialog->exec() == QDialog::Accepted)
	{
		int index = dialog->index();

		if (index < 0 || static_cast<size_t>(index) >= elements.size())
		{
			assert(false);
			return;
		}

		std::shared_ptr<Afbl::AfbElement> afb = elements[index];

		addItem(std::make_shared<VFrame30::VideoItemFblElement>(scheme()->unit(), *(afb.get())));
	}

	return;
}

MouseState EditSchemeWidget::mouseState() const
{
	return editSchemeView()->mouseState();
}

void EditSchemeWidget::setMouseState(MouseState state)
{
	editSchemeView()->setMouseState(state);
	return;
}

const DbFileInfo& EditSchemeWidget::fileInfo() const
{
	return m_fileInfo;
}

void EditSchemeWidget::setFileInfo(const DbFileInfo& fi)
{
	m_fileInfo = fi;
}

bool EditSchemeWidget::snapToGrid() const
{
	return m_snapToGrid;
}

void EditSchemeWidget::setSnapToGrid(bool value)
{
	m_snapToGrid = value;
}

bool EditSchemeWidget::readOnly() const
{
	assert(m_editEngine);
	return m_editEngine->readOnly();
}

void EditSchemeWidget::setReadOnly(bool value)
{
	assert(m_editEngine);
	m_editEngine->setReadOnly(value);
}

bool EditSchemeWidget::modified() const
{
	assert(m_editEngine);
	return m_editEngine->modified();
}

void EditSchemeWidget::setModified()
{
	assert(m_editEngine);
	m_editEngine->setModified();
}


void EditSchemeWidget::resetModified()
{
	assert(m_editEngine);
	m_editEngine->resetModified();
}

void EditSchemeWidget::resetEditEngine()
{
	assert(m_editEngine);
	m_editEngine->reset();
}

