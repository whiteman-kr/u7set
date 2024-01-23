#include "PosRectRotatable.h"
#include "PropertyNames.h"

namespace VFrame30
{
	PosRectRotatable::PosRectRotatable(void)
	{
	}

	void PosRectRotatable::propertyDemand(const QString& prop)
	{
		PosRectImpl::propertyDemand(prop);

		ADD_PROPERTY_GET_SET_CAT(VFrame30::RotationPoint, PropertyNames::rotationPoint, PropertyNames::positionAndSizeCategory, true, PosRectRotatable::rotationPoint, PosRectRotatable::setRotationPoint)
			->setDescription(PropertyNames::rotationPointDescription);

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::angle, PropertyNames::positionAndSizeCategory, true, PosRectRotatable::angle, PosRectRotatable::setAngle)
			->setDescription(PropertyNames::angleDescription);
		
		return;
	}

	bool PosRectRotatable::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::PosRectRotatable* mm = message->mutable_schemaitem()->mutable_posrectrotatable();

		mm->set_rotationpoint(static_cast<int32_t>(m_rotationPoint));
		mm->set_angle(m_angle);

		return true;
	}

	bool PosRectRotatable::LoadData(const Proto::Envelope& message)
	{
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		if (message.schemaitem().has_posrectrotatable() == false)
		{
			// Old versions of the schemaitem did not have this field.
			//
			return true;
		}

		const Proto::PosRectRotatable& mm = message.schemaitem().posrectrotatable();

		m_rotationPoint = static_cast<RotationPoint>(mm.rotationpoint());
		m_angle = mm.angle();

		return true;
	}

	void PosRectRotatable::drawHighlightRect(CDrawParam* drawParam, const QRectF& rect) const
	{
		return drawRotated(drawParam, [drawParam, &rect, this]()
						   {
							   return PosRectImpl::drawHighlightRect(drawParam, rect);
						   });
	}

	void PosRectRotatable::drawOutline(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return PosRectImpl::drawOutline(drawParam);
						   });
	}

	void PosRectRotatable::drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const
	{
		return drawRotated(drawParam, [drawParam, issue, this]()
						   {
							   return PosRectImpl::drawIssue(drawParam, issue);
						   });
	}

	void PosRectRotatable::drawSelection(CDrawParam* drawParam, bool drawSelection) const
	{
		return drawRotated(drawParam, [drawParam, drawSelection, this]()
						   {
							   return drawSelectionPrivate(drawParam, drawSelection);
						   });
	}

	void PosRectRotatable::drawSelectionPrivate(CDrawParam* drawParam, bool drawSizeBar) const
	{
		QPainter* p = drawParam->painter();

		// Drawing resources initialization
		//
		if (selectionPen.get() == nullptr)
		{
			selectionPen = std::make_shared<QPen>(QColor(0x33, 0x99, 0xFF, 0x80));
		}

		selectionPen->setColor(isLocked() == true ? SchemaItem::lockedSelectionColor : SchemaItem::selectionColor);

		// --
		//
		QPainter::RenderHints oldRenderingHints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		// --
		//
		QRectF r = boundingRectInDocPt(drawParam);

		double lineWeight = drawParam->controlBarSize() / 2.0f;
		selectionPen->setWidthF(lineWeight);

		p->setPen(*selectionPen);
		p->drawRect(r);

		if (drawSizeBar == true && isLocked() == false)
		{
			//double fx = r.left();
			//double fy = r.top();
			//double width = r.width();
			//double height = r.height();

			// For tilted item draw only three control bars, right, bottom and bottom right.
			// The first in pair is a flag to draw the control bar if angle != 0.
			//
			std::array<QRectF, 8> controlRectangles = controlBarRects(drawParam->controlBarSize());
			
			if (angle() != 0)
			{
				// controlBarRects returns rectangles in the schema coordinate system,
				// but if angle is no 0, then we need to translate them to the rotation point.
				// 
				auto rotationPoint = rotationPointInDocPt();
				for (auto& rect : controlRectangles)
				{
					if (rect.isNull() == false)
					{
						rect.translate(-rotationPoint);
					}
				}
			}

			for (const auto& rect : controlRectangles)
			{
				if (rect.isNull() == true)
				{
					continue;
				}

				p->fillRect(rect, selectionPen->color());
			}
		}

		// --
		//
		p->setRenderHints(oldRenderingHints);
		return;
	}

	void PosRectRotatable::drawCompareAction(CDrawParam* drawParam, QColor color) const
	{
		return drawRotated(drawParam, [drawParam, color, this]()
						   {
							   return PosRectImpl::drawCompareAction(drawParam, color);
						   });
	}

	void PosRectRotatable::drawCommentDim(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return PosRectImpl::drawCommentDim(drawParam);
						   });
	}

	std::array<QRectF, 8> PosRectRotatable::controlBarRects(double controlBarSize) const
	{
		std::array<QRectF, 8> controlRectangles;

		auto rect = QRectF{leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()};
		double cbs = controlBarSize;
		double fx = rect.left();
		double fy = rect.top();
		double width = rect.width();
		double height = rect.height();

		controlRectangles[0] = QRectF{fx - cbs, fy - cbs, cbs, cbs};
		controlRectangles[1] = QRectF{fx + width / 2 - cbs / 2, fy - cbs, cbs, cbs};
		controlRectangles[2] = QRectF{fx + width, fy - cbs, cbs, cbs};
		controlRectangles[3] = QRectF{fx + width, fy + height / 2 - cbs / 2, cbs, cbs};
		controlRectangles[4] = QRectF{fx + width, fy + height, cbs, cbs};
		controlRectangles[5] = QRectF{fx + width / 2 - cbs / 2, fy + height, cbs, cbs};
		controlRectangles[6] = QRectF{fx - cbs, fy + height, cbs, cbs};
		controlRectangles[7] = QRectF{fx - cbs, fy + height / 2 - cbs / 2, cbs, cbs};

		if (angle() != 0)
		{
			// For different rotation points allow different control bars.
			//
			switch (rotationPoint())
			{
			case RotationPoint::TopLeft:
				controlRectangles[0] = {}; // Top left
				controlRectangles[1] = {}; // Top
				controlRectangles[2] = {}; // Top right
				//controlRectangles[3] = {}; // Right
				//controlRectangles[4] = {}; // Bottom right
				//controlRectangles[5] = {}; // Bottom
				controlRectangles[6] = {}; // Bottom left
				controlRectangles[7] = {}; // Left
				break;
			case RotationPoint::TopRight:
				controlRectangles[0] = {}; // Top left
				controlRectangles[1] = {}; // Top
				controlRectangles[2] = {}; // Top right
				controlRectangles[3] = {}; // Right
				controlRectangles[4] = {}; // Bottom right
				//controlRectangles[5] = {}; // Bottom
				//controlRectangles[6] = {}; // Bottom left
				//controlRectangles[7] = {}; // Left
				break;
			case RotationPoint::BottomRight:
				//controlRectangles[0] = {}; // Top left
				//controlRectangles[1] = {}; // Top
				controlRectangles[2] = {}; // Top right
				controlRectangles[3] = {}; // Right
				controlRectangles[4] = {}; // Bottom right
				controlRectangles[5] = {}; // Bottom
				controlRectangles[6] = {}; // Bottom left
				//controlRectangles[7] = {}; // Left
				break;
			case RotationPoint::BottomLeft:
				controlRectangles[0] = {}; // Top left
				//controlRectangles[1] = {}; // Top
				//controlRectangles[2] = {}; // Top right
				//controlRectangles[3] = {}; // Right
				controlRectangles[4] = {}; // Bottom right
				controlRectangles[5] = {}; // Bottom
				controlRectangles[6] = {}; // Bottom left
				controlRectangles[7] = {}; // Left
				break;
			case RotationPoint::Center:
				controlRectangles[0] = {}; // Top left
				controlRectangles[1] = {}; // Top
				controlRectangles[2] = {}; // Top right
				controlRectangles[3] = {}; // Right
				controlRectangles[4] = {}; // Bottom right
				controlRectangles[5] = {}; // Bottom
				controlRectangles[6] = {}; // Bottom left
				controlRectangles[7] = {}; // Left
				break;
			default:
				Q_ASSERT(false);
			}
		}

		return controlRectangles;
	}

	bool PosRectRotatable::isIntersectRect(double x, double y, double width, double height) const
	{
		QRectF itemRect{leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()};

		if (angle() == 0)
		{
			// Use the fast way to check if the rectangles intersect.
			//
			return PosRectImpl::isIntersectRect(x, y, width, height);
		}

		// Find if the variable rect rotated to angle() around it's top left point is intersected with otherRect (which is not rotated).
		//
		auto rotatePoint = rotationPointInDocPt();

		QTransform rotationTransform;
		rotationTransform.translate(rotatePoint.x(), rotatePoint.y());
		rotationTransform.rotate(angle());
		rotationTransform.translate(-rotatePoint.x(), -rotatePoint.y());

		// Apply the rotation transformation to this rectangle
		//
		QPolygonF rotatedRect = rotationTransform.map(itemRect);

		// Create a path from the rotated rectangle to get intersections with the other rectangle.
		//
		QPainterPath path;
		path.addPolygon(rotatedRect);
		path.closeSubpath();

		return path.intersects(QRectF{x, y, width, height});
	}

	QRectF PosRectRotatable::boundingRectInDocPt(const CDrawParam* drawParam) const
	{
		auto result = PosRectImpl::boundingRectInDocPt(drawParam);

		if (angle() != 0)
		{
			result.translate(-rotationPointInDocPt());
		}

		return result;
	}

	QPointF PosRectRotatable::rotationPointInDocPt() const
	{
		QRectF rect{leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()};

		switch (rotationPoint())
		{
		case RotationPoint::TopLeft:
			return rect.topLeft();
		case RotationPoint::TopRight:
			return rect.topRight();
		case RotationPoint::BottomRight:
			return rect.bottomRight();
		case RotationPoint::BottomLeft:
			return rect.bottomLeft();
		case RotationPoint::Center:
			return rect.center();
		}

		Q_ASSERT(false);
		return rect.topLeft();
	}

	RotationPoint PosRectRotatable::rotationPoint() const
	{
		return m_rotationPoint;
	}

	void PosRectRotatable::setRotationPoint(RotationPoint value)
	{
		m_rotationPoint = value;
	}

	double PosRectRotatable::angle() const
	{
		return m_angle;
	}

	void PosRectRotatable::setAngle(double value)
	{
		m_angle = value;
	}
} // namespace VFrame30