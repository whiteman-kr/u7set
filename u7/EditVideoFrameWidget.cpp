#include "Stable.h"
#include "EditVideoFrameWidget.h"
#include "../VFrame30/VideoItemLine.h"
#include "../VFrame30/VideoItemRect.h"
#include "../VFrame30/VideoItemConnectionLine.h"
#include "../VFrame30/VideoItemSignal.h"
#include "../VFrame30/VideoItemFblElement.h"
#include "../VFrame30/VideoItemLink.h"


const EditVideoFrameWidget::MouseStateCursor EditVideoFrameWidget::m_mouseStateCursor[] =
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

const EditVideoFrameWidget::SizeActionToMouseCursor EditVideoFrameWidget::m_sizeActionToMouseCursor[] =
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
EditVideoFrameView::EditVideoFrameView(QWidget* parent) :
	VFrame30::VideoFrameView(parent),
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

EditVideoFrameView::EditVideoFrameView(std::shared_ptr<VFrame30::CVideoFrame>& videoFrame, QWidget* parent)
	: VFrame30::VideoFrameView(videoFrame, parent),
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

void EditVideoFrameView::paintEvent(QPaintEvent* pe)
{
	// Draw videoframe
	//
	VFrame30::VideoFrameView::paintEvent(pe);

	// Draw other -- selection, grid, outlines, rullers, etc
	//

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p);

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0);

	// Draw VideoFrame
	//
	QRectF clipRect(0, 0, videoFrame()->docWidth(), videoFrame()->docHeight());

	drawParam.setControlBarSize(
		videoFrame()->unit() == VFrame30::SchemeUnit::Display ?	10 * (100.0 / zoom()) : mm2in(2.4) * (100.0 / zoom()));

	// Draw selection
	//
	if (m_selectedItems.empty() == false)
	{
		VFrame30::CVideoItem::DrawSelection(&drawParam, m_selectedItems, m_selectedItems.size() == 1);
	}

	// Draw newItem outline
	//
	if (m_newItem)
	{
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> outlines;
		outlines.push_back(m_newItem);

		VFrame30::CVideoItem::DrawOutline(&drawParam, outlines);
	}

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

void EditVideoFrameView::drawSelectionArea(QPainter* p)
{
	QRectF r(m_mouseSelectionStartPoint, m_mouseSelectionEndPoint);

	QPen pen(QColor(0x33, 0x99, 0xFF, 0xE6));
	pen.setWidth(0);

	p->setPen(pen);
	p->setBrush(QColor(0x33, 0x99, 0xFF, 0x33));

	p->drawRect(r);

	return;
}

void EditVideoFrameView::drawMovingItems(VFrame30::CDrawParam* drawParam)
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
		[xdif, ydif](std::shared_ptr<VFrame30::CVideoItem> si)
		{
			si->MoveItem(xdif, ydif);
		}
		);

	// Draw outline
	//
	VFrame30::CVideoItem::DrawOutline(drawParam, m_selectedItems);

	// Shift position back
	//
	std::for_each(m_selectedItems.begin(), m_selectedItems.end(),
		[xdif, ydif](std::shared_ptr<VFrame30::CVideoItem> si)
		{
			si->MoveItem(-xdif, -ydif);
		}
		);

	return;
}

void EditVideoFrameView::drawRectSizing(VFrame30::CDrawParam* drawParam)
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
	std::vector<VFrame30::VideoItemPoint> oldPos = si->getPointList();

	// set new pos
	//
	double x1 = itemPos->leftDocPt();
	double y1 = itemPos->topDocPt();
	double x2 = x1 + itemPos->widthDocPt();
	double y2 = y1 + itemPos->heightDocPt();

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		x1 += xdif;
		y1 += ydif;
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		break;
	default:
		assert(false);
		break;
	}

	itemPos->setLeftDocPt(std::min(x1, x2));
	itemPos->setTopDocPt(std::min(y1, y2));
	itemPos->setWidthDocPt(std::abs(x2 - x1));
	itemPos->setHeightDocPt(std::abs(y2 - y1));

	// Save result for drawing rullers
	//
	m_addRectStartPoint = VFrame30::VideoItemPoint(x1, y1);
	m_addRectEndPoint = VFrame30::VideoItemPoint(x2, y2);

	// Draw item outline
	//
	VFrame30::CVideoItem::DrawOutline(drawParam, m_selectedItems);

	// restore position
	//
	si->setPointList(oldPos);
	return;
}

void EditVideoFrameView::drawMovingLinePoint(VFrame30::CDrawParam* drawParam)
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

	// Draw outline
	//
	VFrame30::CVideoItem::DrawOutline(drawParam, m_selectedItems);

	// Resotore points
	//
	si->setPointList(oldPos);

	return;
}

void EditVideoFrameView::drawMovingEdgesOrVertexConnectionLine(VFrame30::CDrawParam* drawParam)
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

	std::list<VFrame30::VideoItemPoint> pointsList = itemPos->GetPointList();
	std::vector<VFrame30::VideoItemPoint> points(pointsList.begin(), pointsList.end());

	// Save position
	//
	auto oldPos = si->getPointList();

	if (m_movingEdgePointIndex < 0 || m_movingEdgePointIndex >= static_cast<int>(points.size()))
	{
		assert(m_movingEdgePointIndex >= 0);
		assert(m_movingEdgePointIndex < static_cast<int>(points.size()));
		return;
	}

	// Calculate new position
	//
	switch (mouseState())
	{
	case MouseState::MovingHorizontalEdge:
		{
			double diff = m_editEndMovingEdge - m_editStartMovingEdge;

			VFrame30::VideoItemPoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::VideoItemPoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::VideoItemPoint op = points[m_movingEdgePointIndex];
			op.Y += diff;

			points[m_movingEdgePointIndex] = op;

			//
			op = points[m_movingEdgePointIndex + 1];
			op.Y += diff;

			points[m_movingEdgePointIndex + 1] = op;

			// ���� �� �������� ���� ��� �������������� ����� �� �������� �����,
			// ��� �� ����� �� ������ �� ��������� �������� �������
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

			VFrame30::VideoItemPoint oldEdgeStart = points[m_movingEdgePointIndex];
			VFrame30::VideoItemPoint oldEdgeEnd = points[m_movingEdgePointIndex + 1];

			VFrame30::VideoItemPoint op = points[m_movingEdgePointIndex];
			op.X += diff;

			points[m_movingEdgePointIndex] = op;
			//
			op = points[m_movingEdgePointIndex + 1];
			op.X += diff;

			points[m_movingEdgePointIndex + 1] = op;

			// ���� �� �������� ���� ��� ������������ ����� �� �������� �����,
			// ��� �� ����� �� ������ �� ��������� �������� �������
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

			// �������� ���������� �����
			//
			if (m_movingEdgePointIndex - 1 >= 0)
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::VideoItemPoint curPoint = points[index];

				while (index > 0 && sameDirrection == true)
				{
					VFrame30::VideoItemPoint prevPoint = points[index - 1];

					if (std::abs(prevPoint.X - curPoint.X) < std::abs(prevPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// ����� ������������
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
							// ����� ��������������
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

			// �������� ��������� �����
			//
			if (m_movingEdgePointIndex + 1 < static_cast<int>(points.size()))
			{
				int index = m_movingEdgePointIndex;
				bool sameDirrection = true;
				bool wasVert = true;
				bool wasHorz = true;
				VFrame30::VideoItemPoint curPoint = points[index];

				while (index + 1 < static_cast<int>(points.size()) && sameDirrection == true)
				{
					VFrame30::VideoItemPoint nextPoint = points[index + 1];

					if (std::abs(nextPoint.X - curPoint.X) < std::abs(nextPoint.Y - curPoint.Y))
					{
						if (wasVert == true)
						{
							// ����� ������������
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
							// ����� ��������������
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

			// �������� ������������ �������
			//
			VFrame30::VideoItemPoint pt = points[m_movingEdgePointIndex];

			pt.X += diffX;
			pt.Y += diffY;

			points[m_movingEdgePointIndex] = pt;
		}
		break;
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
	VFrame30::CVideoItem::DrawOutline(drawParam, m_selectedItems);

	// Restore ald position
	//
	si->setPointList(oldPos);

	return;
}

void EditVideoFrameView::drawGrid(QPainter* p)
{
	assert(p);

	auto unit = videoFrame()->unit();

	double frameWidth = videoFrame()->docWidth();
	double frameHeight = videoFrame()->docHeight();

	double gridSize = unit == VFrame30::SchemeUnit::Display ? GridSizeDisplay : GridSizeMm;

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
		// ��������� ����, ���� ����� ��������� ����� ��� 2 �� ���� � �����
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

VideoItemAction EditVideoFrameView::getPossibleAction(VFrame30::CVideoItem* videoItem, QPointF point, int* outMovingEdgePointIndex)
{
	if (videoItem == nullptr)
	{
		assert(videoItem != nullptr);
		return VideoItemAction::NoAction;
	}

	if (videoItem->itemUnit() != videoFrame()->unit())
	{
		assert(videoItem->itemUnit() == videoFrame()->unit());
		return VideoItemAction::NoAction;
	}

	if (outMovingEdgePointIndex != nullptr)
	{
		*outMovingEdgePointIndex = -1;
	}

	float controlBarSize = ControlBar(videoItem->itemUnit(), zoom());

	// ���������� schemeItem � point ����������� ����
	//
	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(videoItem) != nullptr)
	{
		VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(videoItem) ;

		// ���� ������ ������������� �� SchemeItemAction.MoveItem
		//
		if (videoItem->IsIntersectPoint(point.x(), point.y()) == true)
		{
			return VideoItemAction::MoveItem;
		}

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		//
		QRectF itemRectangle;		// ����������������� �������������

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

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		//
		double x1 = itemPos->startXDocPt();
		double y1 = itemPos->startYDocPt();
		double x2 = itemPos->endXDocPt();
		double y2 = itemPos->endYDocPt();

		// ��������������, �� ������� ����� ��������� � �������� �������
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

		// ���� ������ �� �����, �� SchemeItemAction.MoveItem
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

		// �������� �� ������ ����������� ��������������� ControlBarSizeIn
		//
		std::list<VFrame30::VideoItemPoint> points = itemPos->GetPointList();

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

		// �������� ���� ��������
		//
		VFrame30::VideoItemPoint lastPoint;

		pointIndex = 0;
		for (auto pt = points.begin(); pt != points.end(); pt++, pointIndex++)
		{
			if (pt == points.begin())
			{
				lastPoint = *pt;
				continue;
			}

			// ������������ �����
			//
			double x1 = std::min(lastPoint.X, pt->X);
			double y1 = std::min(lastPoint.Y, pt->Y);
			double x2 = std::max(lastPoint.X, pt->X);
			double y2 = std::max(lastPoint.Y, pt->Y);

			// ���������� �����
			//
			if (std::abs(x1 - x2) < std::abs(y1 - y2))
			{
				// ����� ������������
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
				// ����� ��������������
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


QUuid EditVideoFrameView::activeLayerGuid() const
{
	if (m_activeLayer >= static_cast<int>(videoFrame()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(videoFrame()->Layers.size()));
		return QUuid();
	}

	return videoFrame()->Layers[m_activeLayer]->guid();
}

std::shared_ptr<VFrame30::CVideoLayer> EditVideoFrameView::activeLayer()
{
	if (m_activeLayer >= static_cast<int>(videoFrame()->Layers.size()))
	{
		assert(m_activeLayer < static_cast<int>(videoFrame()->Layers.size()));
		return std::make_shared<VFrame30::CVideoLayer>("Error", false);
	}

	return videoFrame()->Layers[m_activeLayer];
}

void EditVideoFrameView::setActiveLayer(std::shared_ptr<VFrame30::CVideoLayer> layer)
{
	for (int i = 0; i < static_cast<int>(videoFrame()->Layers.size()); i++)
	{
		if (videoFrame()->Layers[i] == layer)
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

MouseState EditVideoFrameView::mouseState() const
{
	return m_mouseState;
}

void EditVideoFrameView::setMouseState(MouseState state)
{
	m_mouseState = state;
}

const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& EditVideoFrameView::selectedItems() const
{
	return m_selectedItems;
}

void EditVideoFrameView::setSelectedItems(const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items)
{
	m_selectedItems = items;
}

void EditVideoFrameView::setSelectedItems(const std::list<std::shared_ptr<VFrame30::CVideoItem>>& items)
{
	m_selectedItems.clear();
	m_selectedItems.insert(m_selectedItems.begin(), items.begin(), items.end());
}

void EditVideoFrameView::clearSelection()
{
	m_selectedItems.clear();
}

//
//
// EditVideoFrameWidget
//
//
EditVideoFrameWidget::EditVideoFrameWidget(std::shared_ptr<VFrame30::CVideoFrame> videoFrame, const DbFileInfo& fileInfo) :
	m_fileInfo(fileInfo),
	m_snapToGrid(true),
	m_editEngine(nullptr)
{
	createActions();

	// Left Button Down
	//
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditVideoFrameWidget::mouseLeftDown_None, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineStartPoint, std::bind(&EditVideoFrameWidget::mouseLeftDown_AddSchemePosLineStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectStartPoint, std::bind(&EditVideoFrameWidget::mouseLeftDown_AddSchemePosRectStartPoint, this, std::placeholders::_1)));
	m_mouseLeftDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionStartPoint, std::bind(&EditVideoFrameWidget::mouseLeftDown_AddSchemePosConnectionStartPoint, this, std::placeholders::_1)));

	// Left Button Up
	//
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditVideoFrameWidget::mouseLeftUp_Selection, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditVideoFrameWidget::mouseLeftUp_Moving, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditVideoFrameWidget::mouseLeftUp_SizingRect, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineEndPoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_AddSchemePosLineEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectEndPoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_AddSchemePosRectEndPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditVideoFrameWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditVideoFrameWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));
	m_mouseLeftUpStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditVideoFrameWidget::mouseLeftUp_MovingEdgeOrVertex, this, std::placeholders::_1)));

	// Moouse Mov
	//
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Scrolling, std::bind(&EditVideoFrameWidget::mouseMove_Scrolling, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Selection, std::bind(&EditVideoFrameWidget::mouseMove_Selection, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::Moving, std::bind(&EditVideoFrameWidget::mouseMove_Moving, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopLeft, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTop, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingTopRight, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingRight, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomRight, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottom, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingBottomLeft, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::SizingLeft, std::bind(&EditVideoFrameWidget::mouseMove_SizingRect, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingStartLinePoint, std::bind(&EditVideoFrameWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingEndLinePoint, std::bind(&EditVideoFrameWidget::mouseMove_MovingLinePoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosLineEndPoint, std::bind(&EditVideoFrameWidget::mouseMove_AddSchemePosLineEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosRectEndPoint, std::bind(&EditVideoFrameWidget::mouseMove_AddSchemePosRectEndPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditVideoFrameWidget::mouseMove_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingHorizontalEdge, std::bind(&EditVideoFrameWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingVerticalEdge, std::bind(&EditVideoFrameWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));
	m_mouseMoveStateAction.push_back(MouseStateAction(MouseState::MovingConnectionLinePoint, std::bind(&EditVideoFrameWidget::mouseMove_MovingEdgesOrVertex, this, std::placeholders::_1)));

	// Mouse Right Button Down
	//
	//m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::None, std::bind(&EditVideoFrameWidget::mouseRightDown_None, this, std::placeholders::_1)));
	m_mouseRightDownStateAction.push_back(MouseStateAction(MouseState::AddSchemePosConnectionNextPoint, std::bind(&EditVideoFrameWidget::mouseRightDown_AddSchemePosConnectionNextPoint, this, std::placeholders::_1)));


	// --
	//
	horzScrollBarValue = 0;
	vertScrollBarValue = 0;

	setBackgroundRole(QPalette::Dark);
	setAlignment(Qt::AlignCenter);
	setMouseTracking(true);

	// --
	//
	m_videoFrameView = new EditVideoFrameView(videoFrame, this);

	m_videoFrameView->setZoom(100);
	setWidget(m_videoFrameView);

	// --
	//
	connect(this, &QWidget::customContextMenuRequested, this, &EditVideoFrameWidget::contextMenu);
	setCorrespondingContextMenu();

	// Edit Engine
	//
	m_editEngine = new EditEngine::EditEngine(videoFrameView(), horizontalScrollBar(), verticalScrollBar(), this);

	connect(m_editEngine, &EditEngine::EditEngine::stateChanged, this, &EditVideoFrameWidget::editEngineStateChanged);
	connect(m_editEngine, &EditEngine::EditEngine::modifiedChanged, this, &EditVideoFrameWidget::modifiedChanged);

	return;
}

EditVideoFrameWidget::~EditVideoFrameWidget()
{
}

void EditVideoFrameWidget::createActions()
{
	// Escape Button Pressed
	//
	m_escapeAction = new QAction(tr("Escape"), this);
	m_escapeAction->setEnabled(true);
	m_escapeAction->setMenuRole(QAction::NoRole);
	m_escapeAction->setShortcut(QKeySequence(Qt::Key_Escape));
	connect(m_escapeAction, &QAction::triggered, this, &EditVideoFrameWidget::escapeKey);
	addAction(m_escapeAction);

	//
	// File
	//
	m_fileAction = new QAction(tr("File"), this);
	m_fileAction->setEnabled(true);

	m_fileCheckInAction = new QAction(tr("Check In"), this);
	m_fileCheckInAction->setStatusTip(tr("Check In changes..."));
	m_fileCheckInAction->setEnabled(false);
	connect(m_fileCheckInAction, &QAction::triggered, this, &EditVideoFrameWidget::checkInFile);

	m_fileCheckOutAction = new QAction(tr("Check Out"), this);
	m_fileCheckOutAction->setStatusTip(tr("Check Out for edit..."));
	m_fileCheckOutAction->setEnabled(false);
	connect(m_fileCheckOutAction, &QAction::triggered, this, &EditVideoFrameWidget::checkOutFile);

	m_fileUndoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_fileUndoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_fileUndoChangesAction->setEnabled(false);
	connect(m_fileUndoChangesAction, &QAction::triggered, this, &EditVideoFrameWidget::undoChangesFile);

	m_fileSeparatorAction0 = new QAction(this);
	m_fileSeparatorAction0->setSeparator(true);

	m_fileSaveAction = new QAction(tr("Save"), this);
	m_fileSaveAction->setStatusTip(tr("Save current changes..."));
	m_fileSaveAction->setEnabled(false);
	m_fileSaveAction->setShortcut(QKeySequence::Save);
	connect(m_fileSaveAction, &QAction::triggered, this, &EditVideoFrameWidget::saveWorkcopy);
	addAction(m_fileSaveAction);

	m_fileSeparatorAction1 = new QAction(this);
	m_fileSeparatorAction1->setSeparator(true);

	m_fileGetWorkcopyAction = new QAction(tr("Get Workcopy..."), this);
	m_fileGetWorkcopyAction->setStatusTip(tr("Get file workcopy"));
	m_fileGetWorkcopyAction->setEnabled(true);
	connect(m_fileGetWorkcopyAction, &QAction::triggered, this, &EditVideoFrameWidget::getCurrentWorkcopy);

	m_fileSetWorkcopyAction = new QAction(tr("Set Workcopy..."), this);
	m_fileSetWorkcopyAction->setStatusTip(tr("Set file workcopy"));
	m_fileSetWorkcopyAction->setEnabled(false);
	connect(m_fileSetWorkcopyAction, &QAction::triggered, this, &EditVideoFrameWidget::setCurrentWorkcopy);

	m_fileSeparatorAction2 = new QAction(this);
	m_fileSeparatorAction2->setSeparator(true);

	m_filePropertiesAction = new QAction(tr("Properties..."), this);
	m_filePropertiesAction->setStatusTip(tr("Edit file properties"));
	m_filePropertiesAction->setEnabled(false);
	//connect(m_filePropertiesAction, &QAction::triggered, this, &EditVideoFrameWidget::slot_saveWarkcopy);

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
				addItem(std::make_shared<VFrame30::CVideoItemLine>(videoFrame()->unit()));
			});

	m_addConnectionLineAction = new QAction(tr("Connection Line"), this);
	m_addConnectionLineAction->setEnabled(true);
	connect(m_addConnectionLineAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemConnectionLine>(videoFrame()->unit()));
			});

	m_addRectAction = new QAction(tr("Rect"), this);
	m_addRectAction->setEnabled(true);
	connect(m_addRectAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemRect>(videoFrame()->unit()));
			});

	m_addSeparatorAction0 = new QAction(this);
	m_addSeparatorAction0->setSeparator(true);

	m_addInputSignalAction = new QAction(tr("Input"), this);
	m_addInputSignalAction->setEnabled(true);
	connect(m_addInputSignalAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemInputSignal>(videoFrame()->unit()));
			});

	m_addOutputSignalAction = new QAction(tr("Output"), this);
	m_addOutputSignalAction->setEnabled(true);
	connect(m_addOutputSignalAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemOutputSignal>(videoFrame()->unit()));
			});

	m_addFblElementAction = new QAction(tr("FBL Element"), this);
	m_addFblElementAction->setEnabled(true);
	connect(m_addFblElementAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemFblElement>(videoFrame()->unit()));
			});

	m_addLinkAction = new QAction(tr("Link"), this);
	m_addLinkAction->setEnabled(true);
	connect(m_addLinkAction, &QAction::triggered,
			[this](bool)
			{
				addItem(std::make_shared<VFrame30::CVideoItemLink>(videoFrame()->unit()));
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
	connect(m_undoAction, &QAction::triggered, this, &EditVideoFrameWidget::undo);
	addAction(m_undoAction);

	// Edit->Redo
	//
	m_redoAction = new QAction(tr("Redo"), this);
	m_redoAction->setEnabled(false);
	m_redoAction->setShortcut(QKeySequence::Redo);
	connect(m_redoAction, &QAction::triggered, this, &EditVideoFrameWidget::redo);
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
	connect(m_selectAllAction, &QAction::triggered, this, &EditVideoFrameWidget::selectAll);
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
	connect(m_deleteAction, &QAction::triggered, this, &EditVideoFrameWidget::deleteKey);
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
	m_propertiesAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter));
	//connect(m_propertiesAction, &QAction::triggered, this, &EditVideoFrameWidget::____);
	addAction(m_propertiesAction);

	//
	// View
	//
	m_viewAction = new QAction(tr("View"), this);
	m_viewAction->setEnabled(true);

	// View->ZoomIn
	//
	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, this, &EditVideoFrameWidget::zoomIn);
	addAction(m_zoomInAction);

	// View->ZoomOut
	//
	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, this, &EditVideoFrameWidget::zoomOut);
	addAction(m_zoomOutAction);

	// View->Zoom100
	//
	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, this, &EditVideoFrameWidget::zoom100);
	addAction(m_zoom100Action);

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


	//
	// Create Sub Menus
	//
	m_fileMenu = new QMenu();
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

	m_addMenu = new QMenu();
	m_addAction->setMenu(m_addMenu);
		m_addMenu->addAction(m_addLineAction);
		m_addMenu->addAction(m_addConnectionLineAction);
		m_addMenu->addAction(m_addRectAction);
		m_addMenu->addAction(m_addSeparatorAction0);
		m_addMenu->addAction(m_addInputSignalAction);
		m_addMenu->addAction(m_addOutputSignalAction);
		m_addMenu->addAction(m_addFblElementAction);
		m_addMenu->addAction(m_addLinkAction);

	m_editMenu = new QMenu();
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


	m_viewMenu = new QMenu();
	m_viewAction->setMenu(m_viewMenu);
		m_viewMenu->addAction(m_zoomInAction);
		m_viewMenu->addAction(m_zoomOutAction);
		m_viewMenu->addAction(m_zoom100Action);
		m_viewMenu->addAction(m_viewSeparatorAction0);
		m_viewMenu->addAction(m_snapToGridAction);

	return;
}

// Set corresponding to the current situation and user actions context menu
//
void EditVideoFrameWidget::setCorrespondingContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	return;
}

void EditVideoFrameWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MidButton)
	{
		// Enter to scrolling mode
		//
		mousePos = event->pos();

		horzScrollBarValue = horizontalScrollBar()->value();
		vertScrollBarValue = verticalScrollBar()->value();

		setCursor(Qt::OpenHandCursor);

		event->accept();
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

void EditVideoFrameWidget::mouseReleaseEvent(QMouseEvent* event)
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
	}


	setMouseCursor(event->pos());

	//unsetCursor();
	event->ignore();
}

void EditVideoFrameWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons().testFlag(Qt::MidButton) == true)
	{
		int dx = event->x() - mousePos.x();
		int dy = event->y() - mousePos.y();

		horizontalScrollBar()->setValue(horzScrollBarValue - dx);
		verticalScrollBar()->setValue(vertScrollBarValue - dy);

		event->accept();
		return;
	}

	for (auto msa = m_mouseMoveStateAction.begin(); msa != m_mouseMoveStateAction.end(); ++msa)
	{
		if (msa->mouseState == mouseState())
		{
			msa->action(event);

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

void EditVideoFrameWidget::wheelEvent(QWheelEvent* event)
{
	if (widget() == nullptr)
	{
		return;
	}

	// While midButton is pressed, this is move mode, don't change zoom
	//
	if (event->buttons().testFlag(Qt::MidButton))
	{
		return;
	}

	int numDegrees = event->delta() / 8;
	int numSteps = numDegrees / 15;

	if (numSteps != 0)
	{
		double zoom = videoFrameView()->zoom() + numSteps * 10;

		QPointF oldDocPos;
		MousePosToDocPoint(event->pos(), &oldDocPos);

		videoFrameView()->setZoom(zoom, false);

		QPointF newDocPos;
		MousePosToDocPoint(event->pos(), &newDocPos);

		//
		//
		QPointF dPos = (newDocPos - oldDocPos);

		//   (),          ( )
		//
		int newHorzValue = 0;
		int newVertValue = 0;

		switch (videoFrame()->unit())
		{
		case VFrame30::SchemeUnit::Display:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
			break;
		case VFrame30::SchemeUnit::Inch:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * (zoom / 100.0) * logicalDpiX());
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * (zoom / 100.0) * logicalDpiY());
			break;
		default:
			assert(false);
		}

		horizontalScrollBar()->setValue(newHorzValue);
		verticalScrollBar()->setValue(newVertValue);
	}

	event->accept();
	return;
}

void EditVideoFrameWidget::mouseLeftDown_None(QMouseEvent* me)
{
	if (me->modifiers().testFlag(Qt::ShiftModifier) == false)
	{
		QPointF docPoint = widgetPointToDocument(me->pos(), false);

		// ���� ������� ���� ������, � ���� �� ��������� ��������� ����� ������� ��� ��������� �����, ������ � �.�.
		//
		if (selectedItems().size() == 1)
		{
			// ��������� ����������� �� ����� �� ����� �������� ����� ��������� �������� ������ �������
			//
			int movingEdgePointIndex = 0;
			auto selectedItem = selectedItems()[0];

			VideoItemAction possibleAction = videoFrameView()->getPossibleAction(selectedItem.get(), docPoint, &movingEdgePointIndex);

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
					// ������ �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartDocPt = docPoint;
					videoFrameView()->m_editEndDocPt = docPoint;

					setMouseState(findResult->mouseState);

					setMouseCursor(me->pos());
					videoFrameView()->update();
					return;
				}
			}

			// ��������� �� ��������� ������ �����
			//
			if (dynamic_cast<VFrame30::IVideoItemPosLine*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == VideoItemAction::MoveStartLinePoint)
				{
					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartDocPt = docPoint;
					videoFrameView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingStartLinePoint);

					setMouseCursor(me->pos());
					videoFrameView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveEndLinePoint)
				{
					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartDocPt = docPoint;
					videoFrameView()->m_editEndDocPt = docPoint;

					setMouseState(MouseState::MovingEndLinePoint);

					setMouseCursor(me->pos());
					videoFrameView()->update();

					return;
				}
			}

			// ��������� �� ��������� ������ � ����� ����������
			//
			if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(selectedItem.get()) != nullptr)
			{
				if (possibleAction == VideoItemAction::MoveHorizontalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartMovingEdge = docPoint.y();
					videoFrameView()->m_editEndMovingEdge = docPoint.y();

					videoFrameView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingHorizontalEdge);

					setMouseCursor(me->pos());
					videoFrameView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveVerticalEdge)
				{
					assert(movingEdgePointIndex != -1);

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartMovingEdge = docPoint.x();
					videoFrameView()->m_editEndMovingEdge = docPoint.x();

					videoFrameView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingVerticalEdge);

					setMouseCursor(me->pos());
					videoFrameView()->update();

					return;
				}

				if (possibleAction == VideoItemAction::MoveConnectionLinePoint)
				{
					assert(movingEdgePointIndex != -1);

					// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
					//
					docPoint = widgetPointToDocument(me->pos(), snapToGrid());

					videoFrameView()->m_editStartMovingEdgeX = docPoint.x();
					videoFrameView()->m_editStartMovingEdgeY = docPoint.y();

					videoFrameView()->m_editEndMovingEdgeX = docPoint.x();
					videoFrameView()->m_editEndMovingEdgeY = docPoint.y();

					videoFrameView()->m_movingEdgePointIndex = movingEdgePointIndex;

					setMouseState(MouseState::MovingConnectionLinePoint);

					setMouseCursor(me->pos());
					videoFrameView()->update();

					return;
				}
			}
		}

		// ��������� ���������� ��������, �� ����������� ���������� ������� �� �����������
		//
		for (auto si = selectedItems().begin(); si != selectedItems().end(); ++si)
		{
			int movingEdgePointIndex = 0;
			VideoItemAction possibleAction = videoFrameView()->getPossibleAction(si->get(), docPoint, &movingEdgePointIndex);

			if (possibleAction == VideoItemAction::MoveItem)
			{
				// ������ �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
				//
				docPoint = widgetPointToDocument(me->pos(), snapToGrid());

				videoFrameView()->m_editStartDocPt = docPoint;
				videoFrameView()->m_editEndDocPt = docPoint;

				setMouseState(MouseState::Moving);

				setMouseCursor(me->pos());
				videoFrameView()->update();
				return;
			}
		}

		// ���� ���� �� ����������� ������� ����� ���������� (���������� ��� ������ ������)
		// �� ������� � ����� ����������� �������
		//
		auto itemUnderPoint = videoFrameView()->activeLayer()->getItemUnderPoint(docPoint);

		if (itemUnderPoint != nullptr)
		{
			if (std::find(selectedItems().begin(), selectedItems().end(), itemUnderPoint) != selectedItems().end())
			{
				// ������� � ����� ����������� ���� ���������� ���������
				// SelectedItems ��� ��������
				//
			}
			else
			{
				// ������� � ����� ����������� ������ ��������
				//
				videoFrameView()->clearSelection();
				selectedItems().push_back(itemUnderPoint);
			}

			// �������� ����� Xin � Yin ����������� � �����, ��������� ������ ���� ��� ����������� ������� �������� ��� �����
			//
			docPoint = widgetPointToDocument(me->pos(), snapToGrid());

			videoFrameView()->m_editStartDocPt = docPoint;
			videoFrameView()->m_editEndDocPt = docPoint;

			setMouseState(MouseState::Moving);

			setMouseCursor(me->pos());
			videoFrameView()->update();
			return;
		}
	}

	// ���� ���� ������ ������ ��������� �� �� ���������� � ��������� � ����� ����������� ��������
	//
	if (me->modifiers().testFlag(Qt::ShiftModifier) == false)
	{
		videoFrameView()->clearSelection();
	}

	// ��������� �������� ��� �������
	//
	videoFrameView()->m_mouseSelectionStartPoint = widgetPointToDocument(me->pos(), false);
	videoFrameView()->m_mouseSelectionEndPoint = videoFrameView()->m_mouseSelectionStartPoint;

	setMouseState(MouseState::Selection);

	videoFrameView()->update();

	setMouseCursor(me->pos());

	return;
}

void EditVideoFrameWidget::mouseLeftDown_AddSchemePosLineStartPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);
		resetAction();
		return;
	}

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(videoFrameView()->m_newItem.get());

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

void EditVideoFrameWidget::mouseLeftDown_AddSchemePosRectStartPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		resetAction();
		return;
	}

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		resetAction();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	videoFrameView()->m_addRectStartPoint = docPoint;
	videoFrameView()->m_addRectEndPoint = docPoint;

	itemPos->setLeftDocPt(docPoint.x());
	itemPos->setTopDocPt(docPoint.y());

	setMouseState(MouseState::AddSchemePosRectEndPoint);

	return;
}

void EditVideoFrameWidget::mouseLeftDown_AddSchemePosConnectionStartPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());


	itemPos->DeleteAllPoints();

	itemPos->AddPoint(docPoint.x(), docPoint.y());		        // ����� ����������� ��� �����
	itemPos->AddExtensionPoint(docPoint.x(), docPoint.y());

	// ��������� ��� ���������� ���������� ����
	//
	if (dynamic_cast<VFrame30::CFblItem*>(videoFrameView()->m_newItem.get()) != nullptr)
	{
		// ??
		//VFrame30Ext.IFblItem fblItem = schemeView.newItem as VFrame30Ext.IFblItem;
	}

	setMouseState(MouseState::AddSchemePosConnectionNextPoint);

	return;
}

void EditVideoFrameWidget::mouseLeftUp_Selection(QMouseEvent* me)
{
	// ���������� ����� �������� ������ � ���������� �������,
	// �������� �� � selectedItem
	//
	bool shiftIsPressed = me->modifiers().testFlag(Qt::ShiftModifier);

	// ����� ���� ����� ��� ���� ��� �����, �� � �������� ��������� ��� ��������,
	// ������� � ��������� ��� �������� �� ���������
	//
	if (shiftIsPressed == false)
	{
		videoFrameView()->clearSelection();
	}

	videoFrameView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);

	// ��������� ���������� ��������� ��� �����
	//
	QRectF pageSelectionArea = QRectF(videoFrameView()->m_mouseSelectionStartPoint, videoFrameView()->m_mouseSelectionEndPoint);

	// ����� ��������� ������ pageSelectionArea
	//
	auto activeLayer = videoFrameView()->activeLayer();

	// ���� ���� ��� �������� �����, �� ��������� �������� ������ �������� ��������
	//
	if (videoFrameView()->m_mouseSelectionStartPoint == videoFrameView()->m_mouseSelectionEndPoint)
	{
		auto item = activeLayer->getItemUnderPoint(pageSelectionArea.topLeft());

		if (item != nullptr)
		{
			// ���� ����� ������� ��� ���� � ������, �� ������� ��� �� ������ ����������
			//
			auto findResult = std::find(selectedItems().begin(), selectedItems().end(), item);

			if (findResult != selectedItems().end())
			{
				selectedItems().erase(findResult);
			}
			else
			{
				selectedItems().push_back(item);
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
				selectedItems().erase(findResult);
			}
			else
			{
				selectedItems().push_back(*item);
			}
		}
	}

	// --
	//
	videoFrameView()->m_mouseSelectionStartPoint = QPoint();
	videoFrameView()->m_mouseSelectionEndPoint = QPoint();

	resetAction();

	return;
}

void EditVideoFrameWidget::mouseLeftUp_Moving(QMouseEvent* event)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() != true);
		return;
	}

	QPointF mouseMovingStartPointIn = videoFrameView()->m_editStartDocPt;
	QPointF mouseMovingEndPointIn = widgetPointToDocument(event->pos(), snapToGrid());

	videoFrameView()->m_editEndDocPt = mouseMovingEndPointIn;

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
		m_editEngine->runMoveItem(xdif, ydif, selectedItems());
	}
	else
	{
		// Copy VideoItems and move copied items
		//
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> newItems;

		std::for_each(selectedItems().begin(), selectedItems().end(),
			[xdif, ydif, &newItems](const std::shared_ptr<VFrame30::CVideoItem>& si)
			{
				QByteArray data;

				bool result = si->Save(data);

				if (result == false || data.isEmpty() == true)
				{
					assert(result == true);
					assert(data.isEmpty() == false);
					return;
				}

				VFrame30::CVideoItem* newItemRawPtr = VFrame30::CVideoItem::Create(data);

				if (newItemRawPtr == nullptr)
				{
					assert(newItemRawPtr != nullptr);
					return;
				}

				std::shared_ptr<VFrame30::CVideoItem> newItem(newItemRawPtr);

				newItem->MoveItem(xdif, ydif);

				newItems.push_back(newItem);
				return;
			}
			);

		m_editEngine->runAddItem(newItems, videoFrameView()->activeLayer());
	}

	resetAction();
	return;
}

void EditVideoFrameWidget::mouseLeftUp_SizingRect(QMouseEvent* event)
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

	if (videoFrameView()->m_editStartDocPt.isNull() == true ||
		videoFrameView()->m_editEndDocPt.isNull() == true)
	{
		assert(videoFrameView()->m_editStartDocPt.isNull() == false);
		assert(videoFrameView()->m_editEndDocPt.isNull() == false);
		return;
	}

	if (selectedItems().size() != 1)
	{
		assert(selectedItems().size() == 1);
		return;
	}

	QPointF mouseSizingStartPointDocPt = videoFrameView()->m_editStartDocPt;
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

	switch (mouseState())
	{
	case MouseState::SizingTopLeft:
		x1 += xdif;
		y1 += ydif;
		break;
	case MouseState::SizingTop:
		y1 += ydif;
		break;
	case MouseState::SizingTopRight:
		x2 += xdif;
		y1 += ydif;
		break;
	case MouseState::SizingRight:
		x2 += xdif;
		break;
	case MouseState::SizingBottomRight:
		x2 += xdif;
		y2 += ydif;
		break;
	case MouseState::SizingBottom:
		y2 += ydif;
		break;
	case MouseState::SizingBottomLeft:
		x1 += xdif;
		y2 += ydif;
		break;
	case MouseState::SizingLeft:
		x1 += xdif;
		break;
	default:
		assert(false);
		break;
	}

	// --
	//
	std::vector<VFrame30::VideoItemPoint> itemPoints;

	itemPoints.push_back(VFrame30::VideoItemPoint(std::min(x1, x2), std::min(y1, y2)));
	itemPoints.push_back(VFrame30::VideoItemPoint(std::min(x1, x2) + std::abs(x2 - x1), std::min(y1, y2) + std::abs(y2 - y1)));

	m_editEngine->runSetPoints(itemPoints, si);

	resetAction();
	return;
}

void EditVideoFrameWidget::mouseLeftUp_MovingLinePoint(QMouseEvent* event)
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

	std::vector<VFrame30::VideoItemPoint> points(2);

	QPointF spt = videoFrameView()->m_editStartDocPt;
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
		points[0] = static_cast<VFrame30::VideoItemPoint>(QPointF(itemPos->startXDocPt() + xdif,itemPos->startYDocPt() + ydif));
		points[1] = static_cast<VFrame30::VideoItemPoint>(QPointF(itemPos->endXDocPt(), itemPos->endYDocPt()));
	}

	if (mouseState() == MouseState::MovingEndLinePoint)
	{
		points[0] = static_cast<VFrame30::VideoItemPoint>(QPointF(itemPos->startXDocPt(),itemPos->startYDocPt()));
		points[1] = static_cast<VFrame30::VideoItemPoint>(QPointF(itemPos->endXDocPt() + xdif, itemPos->endYDocPt() + ydif));
	}

	m_editEngine->runSetPoints(points, si);

	//--
	//
	resetAction();
	return;
}

void EditVideoFrameWidget::mouseLeftUp_AddSchemePosLineEndPoint(QMouseEvent* event)
{
	assert(videoFrameView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		videoFrameView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	itemPos->setEndXDocPt(docPoint.x());
	itemPos->setEndYDocPt(docPoint.y());

	if (abs(itemPos->startXDocPt() - itemPos->endXDocPt()) < 0.000001 &&
		abs(itemPos->startYDocPt() - itemPos->endYDocPt()) < 0.000001)
	{
		// The line is empty
		//
		update();
	}
	else
	{
		// Add item to the active layer
		//
		m_editEngine->runAddItem(videoFrameView()->m_newItem, videoFrameView()->activeLayer());
	}

	resetAction();

	return;
}

void EditVideoFrameWidget::mouseLeftUp_AddSchemePosRectEndPoint(QMouseEvent* event)
{
	assert(videoFrameView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		videoFrameView()->m_newItem.reset();
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	videoFrameView()->m_addRectEndPoint = docPoint;

	QPointF sp = videoFrameView()->m_addRectStartPoint;
	QPointF ep = videoFrameView()->m_addRectEndPoint;

	itemPos->setWidthDocPt(abs(sp.x() - ep.x()));
	itemPos->setHeightDocPt(abs(sp.y() - ep.y()));
	itemPos->setLeftDocPt(std::min(sp.x(), ep.x()));
	itemPos->setTopDocPt(std::min(sp.y(), ep.y()));

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
		m_editEngine->runAddItem(videoFrameView()->m_newItem, videoFrameView()->activeLayer());
	}

	resetAction();

	return;
}

void EditVideoFrameWidget::mouseLeftUp_AddSchemePosConnectionNextPoint(QMouseEvent*)
{
	assert(videoFrameView()->m_newItem != nullptr);

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);
		videoFrameView()->m_newItem.reset();
		return;
	}

	auto points = itemPos->GetPointList();

	if (points.size() >= 2)
	{
		itemPos->RemoveSamePoints();
		itemPos->DeleteAllExtensionPoints();

		// ���� ����� �������� ��� ���������� �� ����� �� ��������, �� ��������� ��� ��� (��� ���) �����
		//
		VFrame30::VideoItemPoint startPoint = points.front();
		VFrame30::VideoItemPoint endPoint = points.back();

		// ����� ��������� ������� ����� �� ������ startPoint, endPoint
		//
		QUuid startItemGuid = QUuid();			// ����� ���������� �� �������� ����� ������� ����� ����������� � ������ ����� ����� �����
		bool startPointAddedToOther = false;	// ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)
		bool endPointAddedToOther = false;		// ����� ������� ��� ����������� � ������������� (�������� ����� ����� �� ����� ���������)

		auto schemeItemStartPoint = activeLayer()->getItemUnderPoint(QPointF(startPoint.X, startPoint.Y));
		auto schemeItemEndPoint = activeLayer()->getItemUnderPoint(QPointF(endPoint.X, endPoint.Y));

		if (schemeItemStartPoint != nullptr &&
			schemeItemStartPoint->metaObject()->className() == videoFrameView()->m_newItem->metaObject()->className())
		{
			// ��� ����� �� ��������, ���� ����� schemeItemStartPoint ����� �� ������ ��� ��������� ����� schemeItemStartPoint,
			// �� ���������� schemeItemStartPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemStartPoint.get()) != nullptr);

			VFrame30::IVideoItemPosConnection* existingItemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemStartPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - startPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - startPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ��������� ����� ������ �����
				//
				startItemGuid = schemeItemStartPoint->guid();		// ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����, ��� �� �� ���������� ������
				startPointAddedToOther = true;

				// --
				points.reverse();
				existingItemPoints.pop_front();
				points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());
				points.reverse();								// ���� ����� ����������� �� ��������� �����, �� ���� Recerse ����� �����

				std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
				m_editEngine->runSetPoints(newPoints, schemeItemStartPoint);
			}
			else
			{
				if (std::abs(existingItemPoints.back().X - startPoint.X) < 0.000001 &&
					std::abs(existingItemPoints.back().Y - startPoint.Y) < 0.000001)
				{
					// ������ ����� ����� ����� ����� �� ��������� ����� ������������ �����
					//
					startItemGuid = schemeItemStartPoint->guid();	// ���������, ��� �� ����� �� ���������� � ���� �� ����� � ��������� �����, ��� �� �� ���������� ������
					startPointAddedToOther = true;

					// --
					points.pop_front();

					existingItemPoints.insert(existingItemPoints.end(), points.begin(), points.end());
					points.clear();
					points.assign(existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemStartPoint);
				}
			}
		}

		// --
		//
		if (schemeItemEndPoint != nullptr &&
			schemeItemEndPoint->metaObject()->className() == videoFrameView()->m_newItem->metaObject()->className() &&	// ������ ���� ����� �� ��� ��������
			schemeItemEndPoint->guid() != startItemGuid)								// ������� �� ������ ���� � ���������� ������� ����������� � ���� �� �����
		{
			// ��� ����� �� ��������, ���� ����� schemeItemEndPoint ����� �� ������ ��� ��������� ����� schemeItemEndPoint,
			// �� ���������� schemeItemEndPoint � ����� �������
			//
			assert(dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemEndPoint.get()) != nullptr);
			VFrame30::IVideoItemPosConnection* existingItemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(schemeItemEndPoint.get());

			auto existingItemPoints = existingItemPos->GetPointList();

			if (std::abs(existingItemPoints.front().X - endPoint.X) < 0.000001 &&
				std::abs(existingItemPoints.front().Y - endPoint.Y) < 0.000001)
			{
				// ��������� ����� ����� ����� ����� �� ������ ����� ������ �����
				//
				endPointAddedToOther = true;

				if (startPointAddedToOther == true)	// ����� ����� ��� ���� ��������� ��������� schemeItemStartPoint
				{
					// ����������� ���� �������� - schemeItemStartPoint � schemeItemEndPoint
					//
					existingItemPoints.pop_front();
					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					m_editEngine->runDeleteItem(schemeItemStartPoint, activeLayer());

					std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
				}
				else
				{
					// � �������� schemeItemEndPoint �������� points
					//
					existingItemPoints.pop_front();

					points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

					std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
					m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
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

					if (startPointAddedToOther == true)	// ����� ����� ��� ���� ��������� ��������� schemeItemStartPoint
					{
						// ����������� ���� �������� - schemeItemStartPoint � schemeItemEndPoint
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						m_editEngine->runDeleteItem(schemeItemStartPoint, activeLayer());

						std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
						m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
					}
					else
					{
						// � �������� schemeItemEndPoint �������� points
						//
						existingItemPoints.reverse();
						existingItemPoints.pop_front();
						points.insert(points.end(), existingItemPoints.begin(), existingItemPoints.end());

						std::vector<VFrame30::VideoItemPoint> newPoints(points.begin(), points.end());
						m_editEngine->runSetPoints(newPoints, schemeItemEndPoint);
					}
				}
			}
		}

		if (startPointAddedToOther == false && endPointAddedToOther == false)
		{
			m_editEngine->runAddItem(videoFrameView()->m_newItem, activeLayer());
		}
	}

	resetAction();

	return;
}

void EditVideoFrameWidget::mouseLeftUp_MovingEdgeOrVertex(QMouseEvent*)
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
		std::abs(videoFrameView()->m_editEndMovingEdge - videoFrameView()->m_editStartMovingEdge) < 0.000001)
	{
		// ��������� ��������� ������, ������ � �� ���� ��������� �������
		//
		resetAction();
		return;
	}

	if (mouseState() == MouseState::MovingConnectionLinePoint &&
		std::abs(videoFrameView()->m_editEndMovingEdgeX - videoFrameView()->m_editStartMovingEdgeX) < 0.000001 &&
		std::abs(videoFrameView()->m_editEndMovingEdgeY - videoFrameView()->m_editStartMovingEdgeY) < 0.000001)
	{
		// ��������� ��������� ������, ������ � �� ���� ��������� �������
		//
		resetAction();
		return;
	}

	std::vector<VFrame30::VideoItemPoint> setPoints(videoFrameView()->m_movingVertexPoints.begin(), videoFrameView()->m_movingVertexPoints.end());
	m_editEngine->runSetPoints(setPoints, si);

	resetAction();
	return;
}

void EditVideoFrameWidget::mouseMove_Scrolling(QMouseEvent*)
{
	// To Do
	assert(false);
	return;
}

void EditVideoFrameWidget::mouseMove_Selection(QMouseEvent* me)
{
	// ��������� ����������� ���������.
	//
	videoFrameView()->m_mouseSelectionEndPoint = widgetPointToDocument(me->pos(), false);
	videoFrameView()->update();

	return;
}

void EditVideoFrameWidget::mouseMove_Moving(QMouseEvent* me)
{
	if (selectedItems().empty() == true)
	{
		assert(selectedItems().empty() == false);
		setMouseState(MouseState::None);
		return;
	}

	videoFrameView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	videoFrameView()->update();
	return;
}

void EditVideoFrameWidget::mouseMove_SizingRect(QMouseEvent* me)
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

	videoFrameView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());

	videoFrameView()->update();
	return;
}

void EditVideoFrameWidget::mouseMove_MovingLinePoint(QMouseEvent* me)
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

	videoFrameView()->m_editEndDocPt = widgetPointToDocument(me->pos(), snapToGrid());
	videoFrameView()->update();
	return;
}

void EditVideoFrameWidget::mouseMove_AddSchemePosLineEndPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosLine* itemPos = dynamic_cast<VFrame30::IVideoItemPosLine*>(videoFrameView()->m_newItem.get());

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

	videoFrameView()->update();
	return;
}

void EditVideoFrameWidget::mouseMove_AddSchemePosRectEndPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosRect* itemPos = dynamic_cast<VFrame30::IVideoItemPosRect*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

	videoFrameView()->m_addRectEndPoint = docPoint;

	QPointF sp = videoFrameView()->m_addRectStartPoint;
	QPointF ep = videoFrameView()->m_addRectEndPoint;

	itemPos->setWidthDocPt(std::abs(sp.x() - ep.x()));
	itemPos->setHeightDocPt(std::abs(sp.y() - ep.y()));
	itemPos->setLeftDocPt(std::min(sp.x(), ep.x()));
	itemPos->setTopDocPt(std::min(sp.y(), ep.y()));

	videoFrameView()->update();

	return;
}

void EditVideoFrameWidget::mouseMove_AddSchemePosConnectionNextPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoFrameView()->m_newItem.get());

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
	VFrame30::VideoItemPoint ptBase;

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

	// ��������� ���� �� �������� 90 �������� ��������, ����� � �����������
	//
	double angle = std::floor(v.angle() / 90.0 + 0.5) * 90.0;


	// ������� �����, �������������� ������, � ��������� ��� �� ����
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

	videoFrameView()->update();

	// ����� ����� � ����� ����� "����������" ����.... ������ ��������� ���������� ����� � ������ �������
	//

	return;
}

void EditVideoFrameWidget::mouseMove_MovingEdgesOrVertex(QMouseEvent* event)
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
		videoFrameView()->m_editEndMovingEdge = docPoint.y();
		break;
	case MouseState::MovingVerticalEdge:
		videoFrameView()->m_editEndMovingEdge = docPoint.x();
		break;
	case MouseState::MovingConnectionLinePoint:
		videoFrameView()->m_editEndMovingEdgeX = docPoint.x();
		videoFrameView()->m_editEndMovingEdgeY = docPoint.y();
		break;
	default:
		assert(false);
	}

	videoFrameView()->update();

	return;
}

void EditVideoFrameWidget::mouseRightDown_None(QMouseEvent*)
{
	// CURRENTLY THIS ACTION IS DISABLED IN CONSTRUCTOR, ADD IT TO THE RightClickPress array
	//
	// To Do from old project
	//
	return;
}

void EditVideoFrameWidget::mouseRightDown_AddSchemePosConnectionNextPoint(QMouseEvent* event)
{
	if (videoFrameView()->m_newItem == nullptr)
	{
		assert(videoFrameView()->m_newItem != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	VFrame30::IVideoItemPosConnection* itemPos = dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoFrameView()->m_newItem.get());

	if (itemPos == nullptr)
	{
		assert(itemPos != nullptr);

		setMouseState(MouseState::None);
		setMouseCursor(event->pos());
		return;
	}

	QPointF docPoint = widgetPointToDocument(event->pos(), snapToGrid());

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
	videoFrameView()->update();

	return;
}

std::shared_ptr<VFrame30::CVideoFrame>& EditVideoFrameWidget::videoFrame()
{
	return m_videoFrameView->videoFrame();
}

std::shared_ptr<VFrame30::CVideoFrame>& EditVideoFrameWidget::videoFrame() const
{
	return m_videoFrameView->videoFrame();
}

EditVideoFrameView* EditVideoFrameWidget::videoFrameView()
{
	return m_videoFrameView;
}

const EditVideoFrameView* EditVideoFrameWidget::videoFrameView() const
{
	return m_videoFrameView;
}

const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& EditVideoFrameWidget::selectedItems() const
{
	return m_videoFrameView->m_selectedItems;
}

std::shared_ptr<VFrame30::CVideoLayer> EditVideoFrameWidget::activeLayer()
{
	return m_videoFrameView->activeLayer();
}

std::vector<std::shared_ptr<VFrame30::CVideoItem>>& EditVideoFrameWidget::selectedItems()
{
	return m_videoFrameView->m_selectedItems;
}

QPointF EditVideoFrameWidget::widgetPointToDocument(const QPoint& widgetPoint, bool snapToGrid) const
{
	double docX = 0;	// Result
	double docY = 0;

	double dpiX = logicalDpiX();
	double dpiY = logicalDpiY();

	int widthInPixels = videoFrame()->GetDocumentWidth(dpiX, zoom());
	int heightInPixels = videoFrame()->GetDocumentHeight(dpiY, zoom());

	QRect clientRect = geometry();

	int startX = 0;
	int startY = 0;

	if (clientRect.width() >= widthInPixels)
	{
		startX = (clientRect.width() - widthInPixels) / 2;
	}
	else
	{
		startX = -horizontalScrollBar()->value();
	}

	if (clientRect.height() >= heightInPixels)
	{
		startY = (clientRect.height() - heightInPixels) / 2;
	}
	else
	{
		startY = -verticalScrollBar()->value();
	}

	double x = widgetPoint.x() - startX;		// ���������� � ������
	double y = widgetPoint.y() - startY;

	// ���������� � ���������
	//
	if (videoFrame()->unit() == VFrame30::SchemeUnit::Display)
	{
		docX = x / (zoom() / 100.0);
		docY = y / (zoom() / 100.0);
	}
	else
	{
		docX = x / (dpiX * (zoom() / 100.0));
		docY = y / (dpiY * (zoom() / 100.0));
	}

	if (snapToGrid == true)
	{
		QPointF snapped = this->snapToGrid(QPointF(docX, docY));
		return snapped;
	}

	return QPointF(docX, docY);
}

QPointF EditVideoFrameWidget::snapToGrid(QPointF pt) const
{
	QPointF result;
	double gridSize = videoFrame()->unit() == VFrame30::SchemeUnit::Display ? GridSizeDisplay : GridSizeMm;

	// SnapToGrid ��� Xin
	//
	double restX = pt.x() - (double)((int)(pt.x() / gridSize)) * gridSize;

	if (restX <= gridSize / 2)
	{
		result.setX((double)((int)(pt.x() / gridSize)) * gridSize);
	}
	else
	{
		result.setX((double)((int)(pt.x() / gridSize)) * gridSize + gridSize);
	}

	// SnapToGrid ��� YXin
	//
	double restY = pt.y() - (double)((int)(pt.y() / gridSize)) * gridSize;

	if (restY <= gridSize / 2)
	{
		result.setY((double)((int)(pt.y() / gridSize)) * gridSize);
	}
	else
	{
		result.setY((double)((int)(pt.y() / gridSize)) * gridSize + gridSize);
	}

	return result;
}

bool EditVideoFrameWidget::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
{
	if (pDestDocPos == nullptr)
	{
		assert(pDestDocPos != nullptr);
		return false;
	}

	dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
	dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

	double zoom = videoFrameView()->zoom();

	int widthInPixels = videoFrame()->GetDocumentWidth(dpiX, zoom);
	int heightInPixels = videoFrame()->GetDocumentHeight(dpiY, zoom);

	int startX = 0;
	int startY = 0;

	if (rect().width() >= widthInPixels)
	{
		startX = (rect().width() - widthInPixels) / 2;
	}
	else
	{
		startX = -horizontalScrollBar()->value();
	}

	if (rect().height() >= heightInPixels)
	{
		startY = (rect().height() - heightInPixels) / 2;
	}
	else
	{
		startY = -verticalScrollBar()->value();
	}

	int x = mousePos.x() - startX;
	int y = mousePos.y() - startY;

	if (videoFrame()->unit() == VFrame30::SchemeUnit::Display)
	{
		pDestDocPos->setX(x / (zoom / 100.0));
		pDestDocPos->setY(y / (zoom / 100.0));
	}
	else
	{
		pDestDocPos->setX(x / (dpiX * (zoom / 100.0)));
		pDestDocPos->setY(y / (dpiY * (zoom / 100.0)));
	}

	return true;
}

void EditVideoFrameWidget::addItem(std::shared_ptr<VFrame30::CVideoItem> newItem)
{
	if (newItem == nullptr)
	{
		assert(newItem != nullptr);
		return;
	}

	videoFrameView()->m_newItem = newItem;

	bool posInterfaceFound = false;

	// ����������� ������� � ������������� ISchemePosLine
	//
	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosLineStartPoint);
	}

	// ����������� ������� � ������������� ISchemePosRect
	//
	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosRectStartPoint);
	}

	// ����������� ������� � ������������� ISchemePosConnection
	//
	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(newItem.get()) != nullptr)
	{
		posInterfaceFound = true;
		setMouseState(MouseState::AddSchemePosConnectionStartPoint);
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

void EditVideoFrameWidget::setMouseCursor(QPoint mousePos)
{
	setCursor(QCursor(Qt::CursorShape::ArrowCursor));

	for (int i = 0; i < sizeof(m_mouseStateCursor) / sizeof(m_mouseStateCursor[0]); i++)
	{
		if (mouseState() == m_mouseStateCursor[i].mouseState)
		{
			QCursor cursor(m_mouseStateCursor[i].cursorShape);
			setCursor(cursor);
			return;
		}
	}

	// ���� ������ ��� �� �����, �� �� ������� ������� �������� � �� ������� ������ � mouseStateToCursor!!!!!!!
	//

	int movingEdgePointIndex = -1;

	// ������� ������ ��������� �������
	//
	if (mouseState() == MouseState::None)
	{
		// ���������� ��� ����� ��������� �����, � ��� � ��� ����� �������
		//
		// Convert pixels to document points
		//
		QPointF docPos = widgetPointToDocument(mousePos, false);

		if (selectedItems().empty() == true)
		{
			auto itemUnderPoint = videoFrameView()->activeLayer()->getItemUnderPoint(docPos);

			// ���� ������� �� �������, �� ��� ����� ������ ����������
			//
			if (itemUnderPoint != nullptr &&
				videoFrameView()->getPossibleAction(itemUnderPoint.get(), docPos, &movingEdgePointIndex) == VideoItemAction::MoveItem)
			{
				setCursor(Qt::SizeAllCursor);
				return;
			}
		}

		for (auto si = videoFrameView()->selectedItems().begin(); si != videoFrameView()->selectedItems().end(); ++si)
		{
			VideoItemAction possibleAction = videoFrameView()->getPossibleAction(si->get(), docPos, &movingEdgePointIndex);

			if (possibleAction != VideoItemAction::NoAction)
			{
				// ���������� ������� ��������� ������ ��� ������ ����������� �������
				//
				if (videoFrameView()->selectedItems().size() == 1)
				{
					auto findResult = std::find_if(std::begin(m_sizeActionToMouseCursor), std::end(m_sizeActionToMouseCursor),
						[&possibleAction](const SizeActionToMouseCursor& c) -> bool
						{
							return c.action == possibleAction;
						}
						);

					if (findResult != std::end(m_sizeActionToMouseCursor))
					{
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
				}
			}
		}

		QCursor cursor(Qt::ArrowCursor);
		setCursor(cursor);
		return;
	}

	// ���������� ������� ��� �������� ������ �������� �� �����
	//
	if (dynamic_cast<VFrame30::IVideoItemPosLine*>(videoFrameView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IVideoItemPosRect*>(videoFrameView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	if (dynamic_cast<VFrame30::IVideoItemPosConnection*>(videoFrameView()->m_newItem.get()) != nullptr)
	{
		QCursor cursor(Qt::CursorShape::CrossCursor);
		setCursor(cursor);
		return;
	}

	return;
}

void EditVideoFrameWidget::resetAction()
{
	setMouseState(MouseState::None);
	videoFrameView()->m_newItem.reset();

	videoFrameView()->m_mouseSelectionStartPoint = QPoint();
	videoFrameView()->m_mouseSelectionEndPoint = QPoint();
	videoFrameView()->m_editStartDocPt = QPointF();
	videoFrameView()->m_editEndDocPt = QPointF();

	videoFrameView()->m_movingVertexPoints.clear();

	setMouseCursor(mapFromGlobal(QCursor::pos()));

	videoFrameView()->update();

	return;
}

void EditVideoFrameWidget::clearSelection()
{
	videoFrameView()->clearSelection();
}

void EditVideoFrameWidget::contextMenu(const QPoint& pos)
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

	// Compose menu
	//
	QMenu menu(this);

	QList<QAction*> actions;

	actions << m_fileAction;
	actions << m_addAction;
	actions << m_editAction;

	actions << m_separatorAction0;
	actions << m_propertiesAction;

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void EditVideoFrameWidget::escapeKey()
{
	if (mouseState() != MouseState::None)
	{
		resetAction();
	}
	else
	{
		videoFrameView()->clearSelection();
	}

	videoFrameView()->update();
	return;
}

void EditVideoFrameWidget::deleteKey()
{
	auto selection = videoFrameView()->selectedItems();

	if (mouseState() == MouseState::None &&
		selection.empty() == false)
	{
		m_editEngine->runDeleteItem(selection, activeLayer());
	}

	return;
}

void EditVideoFrameWidget::undo()
{
	m_editEngine->undo(1);
}

void EditVideoFrameWidget::redo()
{
	m_editEngine->redo(1);
}

void EditVideoFrameWidget::editEngineStateChanged(bool canUndo, bool canRedo)
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

void EditVideoFrameWidget::zoomIn()
{
	setZoom(zoom() + 10);
	return;
}

void EditVideoFrameWidget::modifiedChanged(bool modified)
{
	m_fileSaveAction->setEnabled(modified);
	return;
}

void EditVideoFrameWidget::zoomOut()
{
	setZoom(zoom() - 10);
	return;
}

void EditVideoFrameWidget::zoom100()
{
	setZoom(100);
	return;
}

void EditVideoFrameWidget::selectAll()
{
	videoFrameView()->clearSelection();

	std::vector<std::shared_ptr<VFrame30::CVideoItem>> items;
	items.assign(videoFrameView()->activeLayer()->Items.begin(), videoFrameView()->activeLayer()->Items.end());

	videoFrameView()->setSelectedItems(items);

	videoFrameView()->update();
	return;
}

MouseState EditVideoFrameWidget::mouseState() const
{
	return m_videoFrameView->mouseState();
}

void EditVideoFrameWidget::setMouseState(MouseState state)
{
	m_videoFrameView->setMouseState(state);
	return;
}

double EditVideoFrameWidget::zoom() const
{
	if (videoFrameView() == nullptr)
	{
		assert(videoFrameView() != nullptr);
		return 0;
	}

	return videoFrameView()->zoom();
}

void EditVideoFrameWidget::setZoom(double zoom, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
{
	QPoint widgetCenterPoint(size().width() / 2, size().height() / 2);

	QPointF oldDocPos;
	MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

	videoFrameView()->setZoom(zoom, false);

	QPointF newDocPos;
	MousePosToDocPoint(widgetCenterPoint, &newDocPos);

	// --
	//
	QPointF dPos = (newDocPos - oldDocPos);

	// --
	//
	int newHorzValue = 0;
	int newVertValue = 0;

	switch (videoFrame()->unit())
	{
	case VFrame30::SchemeUnit::Display:
		newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
		newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
		break;
	case VFrame30::SchemeUnit::Inch:
		newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * (zoom / 100.0) * logicalDpiX());
		newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * (zoom / 100.0) * logicalDpiY());
		break;
	default:
		assert(false);
	}

	horizontalScrollBar()->setValue(horzScrollValue == -1 ? newHorzValue : horzScrollValue);
	verticalScrollBar()->setValue(vertScrollValue == -1 ? newVertValue : vertScrollValue);

	return;
}

const DbFileInfo& EditVideoFrameWidget::fileInfo() const
{
	return m_fileInfo;
}

void EditVideoFrameWidget::setFileInfo(const DbFileInfo& fi)
{
	m_fileInfo = fi;
}

bool EditVideoFrameWidget::snapToGrid() const
{
	return m_snapToGrid;
}

void EditVideoFrameWidget::setSnapToGrid(bool value)
{
	m_snapToGrid = value;
}

bool EditVideoFrameWidget::readOnly() const
{
	assert(m_editEngine);
	return m_editEngine->readOnly();
}

void EditVideoFrameWidget::setReadOnly(bool value)
{
	assert(m_editEngine);
	m_editEngine->setReadOnly(value);
}

bool EditVideoFrameWidget::modified() const
{
	assert(m_editEngine);
	return m_editEngine->modified();
}

void EditVideoFrameWidget::setModified()
{
	assert(m_editEngine);
	m_editEngine->setModified();
}


void EditVideoFrameWidget::resetModified()
{
	assert(m_editEngine);
	m_editEngine->resetModified();
}

void EditVideoFrameWidget::resetEditEngine()
{
	assert(m_editEngine);
	m_editEngine->reset();
}

