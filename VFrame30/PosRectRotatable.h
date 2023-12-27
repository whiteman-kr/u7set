#pragma once
#include "DrawParam.h"
#include "PosRectImpl.h"

namespace VFrame30
{
	Q_NAMESPACE // Need for Q_ENUM_NS

	enum class RotationPoint
	{
		TopLeft,
		TopRight,
		BottomRight,
		BottomLeft,
		Center
	};

	Q_ENUM_NS(RotationPoint)


	class PosRectRotatable : public PosRectImpl
	{
		Q_OBJECT

	public:
		PosRectRotatable(void);

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void drawHighlightRect(CDrawParam* drawParam, const QRectF& rect) const override;

		virtual void drawOutline(CDrawParam* drawParam) const override;
		virtual void drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const override;
		virtual void drawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;
		void drawSelectionPrivate(CDrawParam* drawParam, bool drawSizeBar) const;
		virtual void drawCompareAction(CDrawParam* drawParam, QColor color) const override;
		virtual void drawCommentDim(CDrawParam* drawParam) const override;

		// Returns 8 rectangles for control bars, which are used to resize the item.
		// The rectangles in the next order: LeftTop, Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left.
		// Some control bars may not be returned if the item does not support resizing in that direction,
		// it depends on the rotationPoint and angle. in the case null rect is returned.
		//
		std::array<QRectF, 8> controlBarRects(double controlBarSize) const;

	protected:
		template<std::invocable Func, typename... Args>
		auto drawRotated(CDrawParam* drawParam, Func&& func, Args&&... args) const
		{
			Q_ASSERT(drawParam != nullptr);

			QPainter* painter = drawParam->painter();
			bool rotated = m_angle != 0.0;
			auto rotationPoint = rotationPointInDocPt();

			if (rotated == true)
			{
				painter->save();
				painter->translate(rotationPoint);
				painter->rotate(angle());
			}

			std::shared_ptr<int*> restore(nullptr, [rotated, painter](void*)
										  {
											  if (rotated == true)
											  {
												  painter->restore();
											  }
										  });

			return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		}

		// Determine and Calculation Functions
		//
	public:
		virtual bool isIntersectRect(double x, double y, double width, double height) const override;

		// Get SchemaItem bounding rectangle in itemUnit()
		// If angle is not 0, then bounding rect is translated to the rotation point.
		//
		virtual QRectF boundingRectInDocPt(const CDrawParam* drawParam) const override;

		QPointF rotationPointInDocPt() const;

	public:
		RotationPoint rotationPoint() const;
		void setRotationPoint(RotationPoint value);

		double angle() const;
		void setAngle(double value);

	private:
		RotationPoint m_rotationPoint{RotationPoint::TopLeft};
		double m_angle{};
	};

} // namespace VFrame30