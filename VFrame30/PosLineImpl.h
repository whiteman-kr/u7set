#pragma once

#include "SchemeItem.h"
#include "../include/CUtils.h"
#include "Settings.h"

namespace VFrame30
{
	// Интерфейс для VideoItem который хранит координаты в виде направленной линии,
	// Хранятся либо в дюймах либо в точках в зависимости от Unit
	//
	class IVideoItemPosLine
	{
	public:
		virtual double startXDocPt() const = 0;
		virtual void setStartXDocPt(double value) = 0;

		virtual double startYDocPt() const = 0;
		virtual void setStartYDocPt(double value) = 0;

		virtual double endXDocPt() const = 0;
		virtual void setEndXDocPt(double value) = 0;

		virtual double endYDocPt() const = 0;
		virtual void setEndYDocPt(double value) = 0;
	};


	class VFRAME30LIBSHARED_EXPORT PosLineImpl : public SchemeItem, public IVideoItemPosLine
	{
		Q_OBJECT

	protected:
		PosLineImpl(void);
		virtual ~PosLineImpl(void);

	private:
		void Init(void);

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

		virtual void SetWidthInDocPt(double widthInDocPt) override;
		virtual void SetHeightInDocPt(double heightInDocPt) override;

		// Draw Functions
		//
	public:
		// Рисование элемента при его создании изменении
		//
		virtual void DrawOutline(CDrawParam* drawParam) const override;

		// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
		//
		virtual void DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const override;

		// Determine and Calculation Functions
		//
	public:
		// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
		// координаты и размер прямоугольника заданы в дюймах или пикселях
		// 
		virtual bool IsIntersectRect(double x, double y, double width, double height) const override;

		// Get VideoItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const override;

		// IVideoItemPosLine implementation
		//
	private:
		double m_startXDocPt;
		double m_startYDocPt;
		double m_endXDocPt;
		double m_endYDocPt;

		// IVideoItemPosLine implementation
		//
	public:
		virtual double startXDocPt() const override;
		virtual void setStartXDocPt(double value)  override;

		virtual double startYDocPt() const  override;
		virtual void setStartYDocPt(double value) override;

		virtual double endXDocPt() const  override;
		virtual void setEndXDocPt(double value)  override;

		virtual double endYDocPt() const  override;
		virtual void setEndYDocPt(double value)  override;

		// IVideoItemPropertiesPos implementation
		//
	public:
		virtual double left() const  override;
		virtual void setLeft(double value)  override;

		virtual double top() const override;
		virtual void setTop(double value)  override;

		virtual double width() const override;
		virtual void setWidth(double value)  override;

		virtual double height() const override;
		virtual void setHeight(double value) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<SchemePoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemePoint>& points) override;
	};
}


