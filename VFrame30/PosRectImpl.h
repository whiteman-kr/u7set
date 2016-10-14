#pragma once

#include "SchemaItem.h"
#include "Settings.h"

class QPen;

namespace VFrame30
{
	class CDrawParam;

	// Интерфейс для SchemItem который хранит координаты в виде прямоугольника, координаты должны быть нормализированы.
	// Хранятся либо в дюймах либо в точках в зависимости от Unit
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


	// Реализация базовых интерефейсов для элементов прямоугольного типа
	//
	class VFRAME30LIBSHARED_EXPORT PosRectImpl : public SchemaItem, public IPosRect
	{
		Q_OBJECT

	protected:
		PosRectImpl(void);
		virtual ~PosRectImpl(void);

	private:
		void Init(void);

		// Public methods
		//
	public:
		virtual void debug() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Action Functions
		//
	public:
		virtual void MoveItem(double horzOffsetDocPt, double vertOffsetDocPt) override;

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
		// Рисование элемента при его создании изменении
		//
		virtual void DrawOutline(CDrawParam* drawParam) const override;

		// Draw item issue
		//
		virtual void DrawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const override;

		// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
		//
		virtual void DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;

		// Draw comment "dim"
		//
		virtual void drawCommentDim(CDrawParam* drawParam) const override;

		// Determine and Calculation Functions
		//
	public:
		// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
		// координаты и размер прямоугольника заданы в дюймах или пикселях
		// 
		virtual bool IsIntersectRect(double x, double y, double width, double height) const override;

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const override;

		// IPosRect
		//
	private:
		double m_leftDocPt;
		double m_topDocPt;
		double m_widthDocPt;
		double m_heightDocPt;

		// Drawing resources
		//
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

		// Реализация интерефейса ISchemaItemPropertiesPos
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


