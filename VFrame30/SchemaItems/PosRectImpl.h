#pragma once

#include "SchemaItem.h"

class QPen;

namespace VFrame30
{
	class CDrawParam;

	// Interface for SchemaItem for getting position in the way of rectangle.
	// Must be normalized.
	// Returns in inches or display points depending on SchemaItem::unit
	//
	class IPosRect
	{
	public:
		virtual double leftDocPt() const = 0;
		virtual void setLeftDocPt(double value) = 0;

		virtual double topDocPt() const = 0;
		virtual void setTopDocPt(double value) = 0;

		virtual double widthDocPt() const = 0;
		virtual void setWidthDocPt(double value) = 0;

		virtual double heightDocPt() const = 0;
		virtual void setHeightDocPt(double value) = 0;

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const = 0;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const = 0;
	};


	/*! \class PosRectImpl
		\brief PosRectImpl
	*/
	class PosRectImpl : public SchemaItem, public IPosRect
	{
		Q_OBJECT

		/// \brief Top position
		Q_PROPERTY(double top READ top WRITE setTop)
		Q_PROPERTY(double Top READ top WRITE setTop)

		/// \brief Left position
		Q_PROPERTY(double left READ left WRITE setLeft)
		Q_PROPERTY(double Left READ left WRITE setLeft)

		/// \brief Width
		Q_PROPERTY(double width READ width WRITE setWidth)
		Q_PROPERTY(double Width READ width WRITE setWidth)

		/// \brief Height
		Q_PROPERTY(double height READ height WRITE setHeight)
		Q_PROPERTY(double Height READ height WRITE setHeight)

	protected:
		PosRectImpl(void);
		virtual ~PosRectImpl(void) = default;

	private:
		void Init(void);

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Public methods
		//
	public:
		virtual void dump() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Action Functions
		//
	public:
		virtual void moveItem(double horzOffsetDocPt, double vertOffsetDocPt) override;

		virtual void snapToGrid(double gridSize) override;

		virtual double GetWidthInDocPt() const override;
		virtual double GetHeightInDocPt() const override;

		virtual void SetWidthInDocPt(double val) override;
		virtual void SetHeightInDocPt(double val) override;

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Draw Functions
		//
	public:
		virtual void drawHighlight(CDrawParam* drawParam) const override;
		virtual void drawHighlightRect(CDrawParam* drawParam, const QRectF& rect) const;

		virtual void drawOutline(CDrawParam* drawParam) const override;
		virtual void drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const override;
		virtual void drawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;
		virtual void drawCompareAction(CDrawParam* drawParam, QColor color) const override;
		virtual void drawCommentDim(CDrawParam* drawParam) const override;

		// Determine and Calculation Functions
		//
	public:
		virtual bool isIntersectRect(double x, double y, double width, double height) const override;

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt(const CDrawParam* drawParam) const override;

		// IPosRect
		//
	private:
		double m_leftDocPt;
		double m_topDocPt;
		double m_widthDocPt;
		double m_heightDocPt;

		// Drawing resources
		//
	protected:
		mutable std::shared_ptr<QPen> selectionPen;
		mutable std::shared_ptr<QPen> outlinePen;

	public:
		virtual double leftDocPt() const override;
		virtual void setLeftDocPt(double value) override;

		virtual double topDocPt() const override;
		virtual void setTopDocPt(double value) override;

		virtual double widthDocPt() const override;
		virtual void setWidthDocPt(double value) override;

		virtual double heightDocPt() const override;
		virtual void setHeightDocPt(double value) override;

		// ISchemaItemPropertiesPos implementation
		//
	public:
		virtual double left() const override;
		virtual void setLeft(double value) override;

		virtual double top() const override;
		virtual void setTop(double value) override;

		virtual double width() const override;
		virtual void setWidth(double value) override;

		virtual double height() const override;
		virtual void setHeight(double value) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<SchemaPoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemaPoint>& points) override;
	};
}


