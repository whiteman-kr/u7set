#include "EditConnectionLine.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PosConnectionImpl.h"
#include "../VFrame30/SchemaPoint.h"
#include "../VFrame30/SchemaItem.h"
#include "../VFrame30/SchemaLayer.h"


EditConnectionLine::EditConnectionLine(std::shared_ptr<VFrame30::PosConnectionImpl> item, EditConnectionLine::EditMode mode)
{
	if (item == nullptr)
	{
		assert(item);
		return;
	}

	m_mode = mode;

	const std::list<VFrame30::SchemaPoint>& itemPoints = item->GetPointList();
	m_basePoints.insert(m_basePoints.begin(), itemPoints.begin(), itemPoints.end());

	return;
}

void EditConnectionLine::clear()
{
	m_basePoints.clear();
	m_extensionPoints.clear();
	return;
}

void EditConnectionLine::clearExtensionPoints()
{
	m_extensionPoints.clear();
}

void EditConnectionLine::addBasePoint(const QPointF& pt)
{
	if (m_mode == AddToEnd)
	{
		m_basePoints.push_back(pt);
		return;
	}

	if (m_mode == AddToBegin)
	{
		m_basePoints.push_front(pt);
		return;
	}

	if (m_mode == MoveToPin)
	{
		if (m_moveToPin.moveLinkBack == true)
		{
			m_basePoints.push_back(pt);
			return;
		}
		else
		{
			m_basePoints.push_front(pt);
			return;
		}
	}

	assert(false);
	return;
}

void EditConnectionLine::addExtensionPoint(const QPointF& pt)
{
	if (m_mode == AddToEnd)
	{
		m_extensionPoints.push_back(pt);
		return;
	}

	if (m_mode == AddToBegin)
	{
		m_extensionPoints.push_front(pt);
		return;
	}

	if (m_mode == MoveToPin)
	{
		if (m_moveToPin.moveLinkBack == true)
		{
			m_extensionPoints.push_back(pt);
			return;
		}
		else
		{
			m_extensionPoints.push_front(pt);
			return;
		}
	}

	assert(false);
	return;
}

void EditConnectionLine::removeLastBasePoint()
{
	if (m_mode == MoveToPin)
	{
		if (m_moveToPin.moveLinkBack == true)
		{
			m_basePoints.pop_back();
			return;
		}
		else
		{
			m_basePoints.pop_front();
			return;
		}
	}

	assert(false);
	return;
}

void EditConnectionLine::moveEndPointPos(std::shared_ptr<VFrame30::SchemaLayer> layer,
										 QPointF toPoint,
										 PreferedMovePointWay preferedWay,
										 double gridSize)
{
	assert(layer);

	auto points = basePoints();
	auto extPoints = extensionPoints();

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

	QPointF ptBase = lastBasePoint();

	// Cases:
	//	PreferedMovePointWay::HorzWay --  If docPoint ouside of horzline, add to main part 3 pointx
	//	------------+(1)
	//              |
	//           (2)+--------+(3)
	//
	//	PreferedMovePointWay::VertCorner -- If docPoint on horz line, add to main part two points
	//	------------+(1)
	//              |
	//       =======+=======
	//             (2)
	//
	//	PreferedMovePointWay::HorzCorner -- move (1), (2) in horz way
	//	                +------------------
	//         (1)      |
	//   [ITEM]-+-------+(2
	//

	// Try to to detect situation for PreferedMovePointWay::VertCorner
	//
	bool docPointIsOnHorzLine = false;
	std::list<std::shared_ptr<VFrame30::SchemaItem>> linksUnderDocPoint = layer->getItemListUnderPoint(toPoint, "VFrame30::SchemaItemLink");

	if (preferedWay == PreferedMovePointWay::Auto &&
		linksUnderDocPoint.empty() == false)
	{
		for (std::shared_ptr<VFrame30::SchemaItem> connItem : linksUnderDocPoint)
		{
			VFrame30::IPosConnection* connectionUndertDocPoint = dynamic_cast<VFrame30::IPosConnection*>(connItem.get());

			// Get all horizontal lines for item connectionUndertDocPoint
			//
			std::list<VFrame30::SchemaPoint> underItemPointsList = connectionUndertDocPoint->GetPointList();
			std::vector<VFrame30::SchemaPoint> underItemPoints = {underItemPointsList.begin(), underItemPointsList.end()};		// In the next loop more conv

			for (size_t i = 0; i < underItemPoints.size(); i++)
			{
				if (i == 0)
				{
					continue;
				}

				VFrame30::SchemaPoint curPos = underItemPoints[i];
				VFrame30::SchemaPoint prevPos = underItemPoints[i - 1];

				if (std::abs(curPos.Y - prevPos.Y) < 0.000001 &&			// it's horiznotal
					std::abs(curPos.Y - toPoint.y()) < 0.000001 &&		// docPoint.y is on this link
					std::min(curPos.X, prevPos.X) <= toPoint.x() &&		// docPoint.x is on this link
					std::max(curPos.X, prevPos.X) >= toPoint.x())			// docPoint.x is on this link
				{
					// This is horz line, docPoint lies on it
					//
					docPointIsOnHorzLine = true;
					break;
				}
			}

			if (docPointIsOnHorzLine == true)
			{
				break;
			}
		}
	}

	if ((preferedWay == PreferedMovePointWay::Auto && docPointIsOnHorzLine == true) ||
		(preferedWay == PreferedMovePointWay::VertCorner))
	{
		//	PreferedMovePointWay::VertCorner -- If docPoint on horz line, add to main part two points
		//	------------+(1) - cornerPoint
		//              |
		//       =======+=======
		//             (2) docCpoint
		//
		QPointF cornerPoint(toPoint.x(), ptBase.y());		// (1)
		cornerPoint = CUtils::snapToGrid(cornerPoint, gridSize);

		clearExtensionPoints();
		addExtensionPoint(cornerPoint);
		addExtensionPoint(toPoint);

		return;
	}

	if (preferedWay == PreferedMovePointWay::HorzCorner)
	{
		//	PreferedMovePointWay::HorzCorner -- move (1), (2) in horz way
		//	                +------------------
		//         (1)      |
		//   [ITEM]-+-------+(2
		//
		QPointF cornerPoint(ptBase.x(), toPoint.y());		// (1)
		cornerPoint = CUtils::snapToGrid(cornerPoint, gridSize);

		clearExtensionPoints();
		addExtensionPoint(cornerPoint);
		addExtensionPoint(toPoint);

		return;
	}

	if (preferedWay == PreferedMovePointWay::Auto ||
		preferedWay == PreferedMovePointWay::HorzWay)
	{
		//	1. If docPoint ouside of horzline, add to main part 3 pointx
		//	------------+(1)
		//              |
		//           (2)+--------+(3)
		//

		// Add extra points
		//
		double horzDistance = std::abs(ptBase.x() - toPoint.x()) * (ptBase.x() - toPoint.x() > 0.0 ? -1.0 : 1.0);
		double midPoint = 0.0;

		if (std::abs(ptBase.x() - toPoint.x()) < gridSize * 1.0)
		{
			midPoint = ptBase.x();
		}
		else
		{
			midPoint = ptBase.x() + horzDistance / 2;
		}

		QPointF onePoint(midPoint, ptBase.y());
		onePoint = CUtils::snapToGrid(onePoint, gridSize);

		clearExtensionPoints();

		// if onePoint on previous line, then move it to base
		//
		if (points.size() > 1)
		{
			QPointF lastLinkPt1 = *std::prev(points.end(), 2);
			QPointF lastLinkPt2 = points.back();

			if (std::abs(lastLinkPt1.y() - lastLinkPt2.y()) < 0.0000001 &&							// prev line is horizontal
				std::abs(lastLinkPt1.y() - onePoint.y()) < 0.0000001 &&
				((lastLinkPt2.x() - lastLinkPt1.x() > 0 && ptBase.x() - onePoint.x() > 0) ||		// new line on the sime side
				 (lastLinkPt2.x() - lastLinkPt1.x() < 0 && ptBase.x() - onePoint.x() < 0)
				))
			{
				onePoint.setX(ptBase.x());
				onePoint.setY(ptBase.y());
			}
		}

		QPointF twoPoint(onePoint.x(), toPoint.y());

		if (onePoint != ptBase)
		{
			addExtensionPoint(onePoint);
		}
		addExtensionPoint(twoPoint);
		addExtensionPoint(toPoint);

		return;
	}

	assert(false);
	return;

}

int EditConnectionLine::moveExtensionPointsToBasePoints()
{
	int addedPoints = static_cast<int>(m_extensionPoints.size());

	if (m_mode == AddToEnd)
	{
		m_basePoints.insert(m_basePoints.end(), m_extensionPoints.begin(), m_extensionPoints.end());
		m_extensionPoints.clear();
		return addedPoints;
	}

	if (m_mode == AddToBegin)
	{
		m_basePoints.insert(m_basePoints.begin(), m_extensionPoints.begin(), m_extensionPoints.end());
		m_extensionPoints.clear();
		return addedPoints;
	}

	assert(false);
	return 0;
}

std::list<QPointF> EditConnectionLine::basePoints() const
{
	std::list<QPointF> result = m_basePoints;

	if (m_mode == AddToBegin ||
		(m_mode == MoveToPin && m_moveToPin.moveLinkBack == false))
	{
		result.reverse();
	}

	return result;
}

std::list<QPointF> EditConnectionLine::extensionPoints() const
{
	std::list<QPointF> result = m_extensionPoints;

	if (m_mode == AddToBegin ||
		(m_mode == MoveToPin && m_moveToPin.moveLinkBack == false))
	{
		result.reverse();
	}

	return result;
}

std::vector<QPointF> EditConnectionLine::points() const
{
	std::vector<QPointF> result;
	result.reserve(m_basePoints.size() + m_extensionPoints.size());

	if (m_mode == AddToEnd)
	{
		result.insert(result.begin(), m_basePoints.begin(), m_basePoints.end());
		result.insert(result.end(), m_extensionPoints.begin(), m_extensionPoints.end());

		return result;
	}

	if (m_mode == AddToBegin)
	{
		result.insert(result.end(), m_extensionPoints.begin(), m_extensionPoints.end());
		result.insert(result.begin(), m_basePoints.begin(), m_basePoints.end());

		return result;
	}

	if (m_mode == EditPoint ||
		m_mode == EditEdge)
	{
		result.insert(result.begin(), m_basePoints.begin(), m_basePoints.end());
	}

	assert(false);
	return result;
}

QPointF EditConnectionLine::lastBasePoint() const
{
	QPointF result;

	if (m_basePoints.empty() == true)
	{
		assert(m_basePoints.empty() == false);
		return result;
	}

	if (m_mode == AddToEnd)
	{
		result = m_basePoints.back();
		return result;
	}

	if (m_mode == AddToBegin)
	{
		result = m_basePoints.front();
		return result;
	}

	if (m_mode == MoveToPin)
	{
		if (m_moveToPin.moveLinkBack == true)
		{
			return m_basePoints.back();
		}
		else
		{
			return m_basePoints.front();
		}
	}

	assert(false);
	return result;
}

QPointF EditConnectionLine::lastExtensionPoint() const
{
	QPointF result;

	if (m_extensionPoints.empty() == true)
	{
		assert(m_extensionPoints.empty() == false);
		return result;
	}

	if (m_mode == AddToEnd)
	{
		result = m_extensionPoints.back();
		return result;
	}

	if (m_mode == AddToBegin)
	{
		result = m_extensionPoints.front();
		return result;
	}

	if (m_mode == MoveToPin)
	{
		if (m_moveToPin.moveLinkBack == true)
		{
			return m_extensionPoints.back();
		}
		else
		{
			return m_extensionPoints.front();
		}
	}

	assert(false);
	return result;
}

void EditConnectionLine::setPointToItem(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem) const
{
	if (schemaItem == nullptr)
	{
		assert(schemaItem);
		return;
	}

	std::list<VFrame30::SchemaPoint> itemPoints(m_basePoints.begin(), m_basePoints.end());
	itemPoints.unique();

	schemaItem->DeleteAllPoints();
	schemaItem->SetPointList(itemPoints);

	return;
}

void EditConnectionLine::drawOutline(VFrame30::CDrawParam* drawParam) const
{
	if (drawParam == nullptr)
	{
		assert(drawParam);
		return;
	}

	if (m_basePoints.empty() == true ||
		m_basePoints.size() + m_extensionPoints.size() < 2)
	{
		return;
	}

	QPainter* p = drawParam->painter();

	// Draw the main part
	//
	QPolygonF polyline;
	polyline.reserve(static_cast<int>(m_basePoints.size()));

	for (const QPointF& pt : m_basePoints)
	{
		polyline.push_back(pt);
	}

	QPen pen(Qt::darkRed);
	pen.setWidthF(0);
	p->setPen(pen);

	p->drawPolyline(polyline);

	// Draw extPoints
	//
	QPolygonF extPolyline;
	extPolyline.reserve(static_cast<int>(m_extensionPoints.size()) + 1);

	if (m_mode == AddToEnd ||
		(m_mode == MoveToPin && m_moveToPin.moveLinkBack == true))
	{
		extPolyline.push_back(m_basePoints.back());
	}

	for (const QPointF pt : m_extensionPoints)
	{
		extPolyline.push_back(pt);
	}

	if (m_mode == AddToBegin ||
		(m_mode == MoveToPin && m_moveToPin.moveLinkBack == false))
	{
		extPolyline.push_back(m_basePoints.front());
	}

	QPen extPen(Qt::red);
	extPen.setWidth(0);
	p->setPen(extPen);

	p->drawPolyline(extPolyline);

	return;
}

EditConnectionLine::EditMode EditConnectionLine::mode() const
{
	return m_mode;
}

void EditConnectionLine::setMode(EditConnectionLine::EditMode value)
{
	m_mode = value;
}

void EditConnectionLine::modifyPoint(const QPointF& point)
{
	assert(m_mode == EditMode::EditPoint);		// Use these functions only in EditPoint mode

	if (m_editPoint.pointIndex >= m_editPoint.initialState.size())
	{
		assert(m_editPoint.pointIndex < m_editPoint.initialState.size());
		return;
	}

	std::vector<QPointF> points = m_editPoint.initialState;

	// Shift the previouse point
	//
	if (m_editPoint.pointIndex - 1 >= 0)
	{
		int index = static_cast<int>(m_editPoint.pointIndex);
		bool sameDirrection = true;
		bool wasVert = true;
		bool wasHorz = true;
		QPointF curPoint = points[index];

		while (index > 0 && sameDirrection == true)
		{
			QPointF prevPoint = points[index - 1];

			if (std::abs(prevPoint.x() - curPoint.x()) < std::abs(prevPoint.y() - curPoint.y()))
			{
				if (wasVert == true)
				{
					// The line is vertical
					//
					prevPoint.rx() = point.x();
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
					// The line is horizontal
					//
					prevPoint.ry() = point.y();
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

	// Shift the next point
	//
	if (m_editPoint.pointIndex + 1 < static_cast<int>(points.size()))
	{
		int index = static_cast<int>(m_editPoint.pointIndex);
		bool sameDirrection = true;
		bool wasVert = true;
		bool wasHorz = true;
		QPointF curPoint = points[index];

		while (index + 1 < static_cast<int>(points.size()) && sameDirrection == true)
		{
			QPointF nextPoint = points[index + 1];

			if (std::abs(nextPoint.x() - curPoint.x()) < std::abs(nextPoint.y() - curPoint.y()))
			{
				if (wasVert == true)
				{
					// The line is vertical
					//
					nextPoint.rx() = point.x();
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
					// The line is horizontal
					//
					nextPoint.ry() = point.y();
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

	// Shift the moving point
	//
	points[m_editPoint.pointIndex] = point;
	m_editPoint.pointCurrState = point;

	// Set result
	//
	m_basePoints.assign(points.begin(), points.end());

	return;
}

bool EditConnectionLine::addPointAndSwitchMode(const QPointF& point)
{
	if (m_editPoint.pointIndex == 0)
	{
		modifyPoint(point);		// after modifyPoint - m_basePoints is a new m_editPointInitialState

		m_editPoint.initialState.assign(m_basePoints.begin(), m_basePoints.end());

		// Switch to mode AddToBegin
		//
		setMode(AddToBegin);
		addExtensionPoint(point);

		return true;
	}

	if (m_editPoint.pointIndex == m_editPoint.initialState.size() - 1)
	{
		modifyPoint(point);		// after modifyPoint - m_basePoints is a new m_editPointInitialState

		m_editPoint.initialState.assign(m_basePoints.begin(), m_basePoints.end());

		// Switch to mode AddToBegin
		//
		setMode(AddToEnd);
		addExtensionPoint(point);

		return true;
	}

	return false;
}

QPointF EditConnectionLine::editPointCurrState() const
{
	assert(m_mode == EditMode::EditPoint);		// Use these functions only in EditPoint mode
	return m_editPoint.pointCurrState;
}

int EditConnectionLine::editPointIndex() const
{
	assert(m_mode == EditMode::EditPoint);		// Use these functions only in EditPoint mode
	return m_editPoint.pointIndex;
}

void EditConnectionLine::setEditPointIndex(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem, int pointIndex)
{
	assert(m_mode == EditMode::EditPoint);		// Use these functions only in EditPoint mode

	if (schemaItem == nullptr)
	{
		assert(schemaItem);
		return;
	}

	const std::list<VFrame30::SchemaPoint>& itemPoints = schemaItem->GetPointList();

	if (pointIndex >= itemPoints.size())
	{
		assert(pointIndex < itemPoints.size());
		return;
	}

	m_editPoint.pointIndex = pointIndex;

	// Save item's initial state
	//
	m_editPoint.initialState.reserve(itemPoints.size());
	m_editPoint.initialState.assign(itemPoints.begin(), itemPoints.end());

	m_editPoint.pointCurrState = m_editPoint.initialState[m_editPoint.pointIndex];

	return;
}

void EditConnectionLine::modifyEdge(double value)
{
	assert(m_mode == EditMode::EditEdge);		// Use these functions only in EditEdge mode

	if (m_editEdge.edgeIndex >= m_editEdge.initialState.size() - 1)
	{
		assert(m_editEdge.edgeIndex < m_editEdge.initialState.size() - 1);
		return;
	}

	m_editEdge.edgeCurrState = value;

	std::vector<QPointF> points = m_editEdge.initialState;

	QPointF oldEdgeStart = points[m_editEdge.edgeIndex];
	QPointF oldEdgeEnd = points[m_editEdge.edgeIndex + 1];

	if (getDirrection(oldEdgeStart, oldEdgeEnd) == Dirrection::Horz)
	{
		points[m_editEdge.edgeIndex].ry() = value;
		points[m_editEdge.edgeIndex + 1].ry() = value;

		// ≈сли по сторонам есть еще √ќ–»«ќЌј“Ћ№Ќџ≈ линии то добавить точку,
		// что бы ребро не т€нуло по диагонали соседние отрезки
		//
		if (m_editEdge.edgeIndex + 2 < static_cast<int>(points.size()) &&
			getDirrection(points[m_editEdge.edgeIndex + 2], oldEdgeEnd) == Dirrection::Horz)
		{
			points.insert(points.begin() + m_editEdge.edgeIndex + 2, oldEdgeEnd);
		}

		if (m_editEdge.edgeIndex - 1 >= 0 &&
			getDirrection(points[m_editEdge.edgeIndex - 1], oldEdgeStart) == Dirrection::Horz)
		{
			points.insert(points.begin() + m_editEdge.edgeIndex, oldEdgeStart);
		}
	}
	else
	{
		points[m_editEdge.edgeIndex].rx() = value;
		points[m_editEdge.edgeIndex + 1].rx() = value;

		// ≈сли по сторонам есть еще ¬≈–“» јЋ№Ќџ≈ линии то добавить точку,
		// что бы ребро не т€нуло по диагонали соседние отрезки
		//
		if (m_editEdge.edgeIndex + 2 < static_cast<int>(points.size()) &&
			getDirrection(points[m_editEdge.edgeIndex + 2], oldEdgeEnd) == Dirrection::Vert)
		{
			points.insert(points.begin() + m_editEdge.edgeIndex + 2, oldEdgeEnd);
		}

		if (m_editEdge.edgeIndex - 1 >= 0 &&
			getDirrection(points[m_editEdge.edgeIndex - 1], oldEdgeStart) == Dirrection::Vert)
		{
			points.insert(points.begin() + m_editEdge.edgeIndex, oldEdgeStart);
		}
	}

	// Set result
	//
	m_basePoints.assign(points.begin(), points.end());
	m_basePoints.unique();

	return;
}

double EditConnectionLine::editEdgetCurrState() const
{
	assert(m_mode == EditMode::EditEdge);
	return m_editEdge.edgeCurrState;
}

int EditConnectionLine::editEdgeIndex() const
{
	assert(m_mode == EditMode::EditEdge);
	return m_editEdge.edgeIndex;
}

void EditConnectionLine::setEditEdgeIndex(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem, int edgeIndex)
{
	assert(m_mode == EditMode::EditEdge);

	if (schemaItem == nullptr)
	{
		assert(schemaItem);
		return;
	}

	const std::list<VFrame30::SchemaPoint>& itemPoints = schemaItem->GetPointList();

	if (edgeIndex > m_basePoints.size() - 1)
	{
		assert(edgeIndex < m_basePoints.size() - 1);
		return;
	}

	m_editEdge.edgeIndex = edgeIndex;

	// Save item's initial state
	//
	m_editEdge.initialState.reserve(itemPoints.size());
	m_editEdge.initialState.assign(itemPoints.begin(), itemPoints.end());

	if (getDirrection(m_editEdge.initialState[m_editEdge.edgeIndex], m_editEdge.initialState[m_editEdge.edgeIndex + 1]) == Dirrection::Horz)
	{
		m_editEdge.edgeCurrState = m_editEdge.initialState[m_editEdge.edgeIndex].y();
	}
	else
	{
		m_editEdge.edgeCurrState = m_editEdge.initialState[m_editEdge.edgeIndex].x();
	}

	return;
}

void EditConnectionLine::moveToPin_init(std::shared_ptr<VFrame30::PosConnectionImpl> link,
									   VFrame30::ConnectionDirrection pinDirrection,
									   QPointF pinInitialPos)
{
	if (link == nullptr)
	{
		assert(link);
		return;
	}

	clear();

	m_mode = MoveToPin;

	m_moveToPin.schemaItem = link;

	std::list<VFrame30::SchemaPoint> itemPoints = removeUnwantedPoints(link->GetPointList());

	m_moveToPin.initialState.reserve(itemPoints.size());
	m_moveToPin.initialState.assign(itemPoints.begin(), itemPoints.end());

	m_moveToPin.pinDirrection = pinDirrection;
	m_moveToPin.pinInitialPos = pinInitialPos;

	m_basePoints.assign(m_moveToPin.initialState.begin(), m_moveToPin.initialState.end());
	m_extensionPoints.clear();

	if (itemPoints.front() == VFrame30::SchemaPoint(pinInitialPos))
	{
		m_moveToPin.moveLinkBack = false;
	}
	else
	{
		m_moveToPin.moveLinkBack = true;
	}

	assert(m_basePoints.size() >= 2);
	assert(m_extensionPoints.empty() == true);

	// --
	//
	QPointF lastBasePt = lastBasePoint();

	removeLastBasePoint();
	addExtensionPoint(lastBasePt);

	return;
}

void EditConnectionLine::moveToPin_offset(std::shared_ptr<VFrame30::SchemaLayer> layer,
								   QPointF offset,
								   double gridSize)
{
	assert(m_mode == MoveToPin);

	QPointF toPoint = m_moveToPin.pinInitialPos;
	toPoint += offset;

	if (m_moveToPin.moveWholeLink == false)
	{
		// if all links are horz, the forceToCorner cannot be true
		//
		std::vector<QPointF> reseacrhPoints = m_moveToPin.initialState;
		if (m_moveToPin.moveLinkBack == true)
		{
			std::reverse(reseacrhPoints.begin(), reseacrhPoints.end());
		}

		bool allLinksAreHorz = true;

		for (size_t i = 0; i < reseacrhPoints.size(); i++)
		{
			if (i == 0)
			{
				continue;
			}

			if (isHorz(reseacrhPoints[i - 1], reseacrhPoints[i]) == false)
			{
				allLinksAreHorz = false;
				break;
			}
		}

		EditConnectionLine::PreferedMovePointWay preferedMove = EditConnectionLine::Auto;

		if (allLinksAreHorz == false)
		{
			preferedMove = EditConnectionLine::HorzCorner;
//			m_basePoints.assign(m_moveToPin.initialState.begin(), m_moveToPin.initialState.end());
//			removeLastBasePoint();
//			removeLastBasePoint();
		}

		// move end point
		//
		moveEndPointPos(layer, toPoint, preferedMove, gridSize);
	}
	else
	{
		clearExtensionPoints();
		m_basePoints.assign(m_moveToPin.initialState.begin(), m_moveToPin.initialState.end());

		for (QPointF& pt : m_basePoints)
		{
			pt += offset;
		}
	}

	return;
}

std::shared_ptr<VFrame30::SchemaItem> EditConnectionLine::moveToPin_schemaItem() const
{
	assert(m_mode == MoveToPin);
	return m_moveToPin.schemaItem;
}

void EditConnectionLine::moveToPin_setMoveWholeLink()
{
	assert(m_mode == MoveToPin);
	m_moveToPin.moveWholeLink = true;

	clearExtensionPoints();
	m_basePoints.assign(m_moveToPin.initialState.begin(), m_moveToPin.initialState.end());

	return;
}


EditConnectionLine::Dirrection EditConnectionLine::getDirrection(const QPointF& pt1, const QPointF& pt2)
{
	if (std::abs(pt1.y() - pt2.y()) < 0.000001)
	{
		return Dirrection::Horz;
	}

	if (std::abs(pt1.x() - pt2.x()) < 0.000001)
	{
		return Dirrection::Vert;
	}

	assert(false);
	return Dirrection::Horz;
}

bool EditConnectionLine::isHorz(const QPointF& pt1, const QPointF& pt2)
{
	return getDirrection(pt1, pt2) == Dirrection::Horz;
}

bool EditConnectionLine::isVert(const QPointF& pt1, const QPointF& pt2)
{
	return getDirrection(pt1, pt2) == Dirrection::Vert;
}

std::vector<VFrame30::SchemaPoint> EditConnectionLine::removeUnwantedPoints(const std::vector<VFrame30::SchemaPoint>& source)
{
	std::vector<VFrame30::SchemaPoint> result = source;

	int sameXPosCount = 0;			// Pairs of points amount by X coordinate
	int sameYPosCount = 0;			// Pairs of points amount by Y coordinate

	size_t currentPointIndex = 0;	// Index of current point to process

	// In cycle we are processing current point with previous point
	//

	for (currentPointIndex = 1; currentPointIndex < result.size(); currentPointIndex++)
	{
		const VFrame30::SchemaPoint& curPoint = result.at(currentPointIndex);
		const VFrame30::SchemaPoint& prevPoint = result.at(currentPointIndex - 1);

		if (std::abs(curPoint.X - prevPoint.X) < 0.0000001)
		{
			sameXPosCount ++;
		}
		else
		{
			// Remove points only if we have more than one pair with same
			// X coordinates
			//
			if (sameXPosCount > 1)
			{
				assert(currentPointIndex > 0);
				assert(currentPointIndex <= result.size());

				size_t startIndex = currentPointIndex - sameXPosCount;
				size_t lastIndex = currentPointIndex - 1;

				result.erase(result.begin() + startIndex, result.begin() + lastIndex);

				currentPointIndex = currentPointIndex - sameXPosCount-1;
				sameYPosCount = 0;
			}

			sameXPosCount = 0;
		}

		if (std::abs(curPoint.Y - prevPoint.Y) < 0.0000001)
		{
			sameYPosCount++;
		}
		else
		{
			// Remove points only if we have more than one pair with same
			// X coordinates
			//
			if (sameYPosCount > 1)
			{
				assert(currentPointIndex > 0);
				assert(currentPointIndex <= result.size());

				size_t startIndex = currentPointIndex - sameYPosCount;
				size_t lastIndex = currentPointIndex - 1;

				result.erase(result.begin() + startIndex, result.begin() + lastIndex);

				currentPointIndex = currentPointIndex - sameYPosCount-1;
				sameXPosCount = 0;
			}

			sameYPosCount = 0;
		}
	}

	// If some pairs with same coordinate values are placed at the end of
	// the line, we must remove them!
	//
	if (sameYPosCount > 1)
	{
		assert(currentPointIndex == result.size());

		size_t beginIndex = currentPointIndex - sameYPosCount;
		size_t lastIndex = currentPointIndex - 1;

		result.erase(result.begin() + beginIndex, result.begin() + lastIndex);
	}

	if (sameXPosCount > 1)
	{
		assert(currentPointIndex == result.size());

		size_t beginIndex = currentPointIndex - sameXPosCount;
		size_t lastIndex = currentPointIndex - 1;

		result.erase(result.begin() + beginIndex, result.begin() + lastIndex);
	}

	// Check points before return
	//
#ifdef _DEBUG
	for (currentPointIndex = 1; currentPointIndex < result.size(); currentPointIndex++)
	{
		const VFrame30::SchemaPoint& curPoint = result.at(currentPointIndex);
		const VFrame30::SchemaPoint& prevPoint = result.at(currentPointIndex - 1);

		// Points must be connected by X or Y axis. In other way - exception must be rised
		//

		assert((std::abs(curPoint.X - prevPoint.X) < 0.0000001) ||
				(std::abs(curPoint.Y - prevPoint.Y) < 0.0000001));
	}
#endif

	return result;
}

std::list<VFrame30::SchemaPoint> EditConnectionLine::removeUnwantedPoints(const std::list<VFrame30::SchemaPoint>& source)
{
	std::vector<VFrame30::SchemaPoint> sourceVector(source.begin(), source.end());
	sourceVector = removeUnwantedPoints(sourceVector);

	std::list<VFrame30::SchemaPoint> result(sourceVector.begin(), sourceVector.end());
	return result;
}



