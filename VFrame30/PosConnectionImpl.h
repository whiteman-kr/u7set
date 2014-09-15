#pragma once

#include "VideoItem.h"
#include "Settings.h"

namespace VFrame30
{
	// Интерфейс для VideoItem который хранит координаты в виде направленной линии,
	// Хранятся либо в дюймах либо в точках в зависимости от Unit
	//
	class IVideoItemPosConnection
	{
	public:
		virtual const std::list<VideoItemPoint>& GetPointList() const = 0;
		virtual void SetPointList(const std::list<VideoItemPoint>& points) = 0;

		virtual void AddPoint(double x, double y) = 0;
		virtual void RemoveSamePoints() = 0;
		virtual void DeleteAllPoints() = 0;
		virtual void DeleteLastPoint() = 0;

		// Работа с Extension точками, которые участвуют в отрисовке при создании элемента, DrawOutline
		//
		virtual const std::list<VideoItemPoint>& GetExtensionPoints() const = 0;
		virtual void SetExtensionPoints(const std::list<VideoItemPoint>& extPoints) = 0;
		virtual void AddExtensionPoint(double x, double y) = 0;
		virtual void DeleteAllExtensionPoints() = 0;
		virtual void DeleteLastExtensionPoint() = 0;
	};

	class VFRAME30LIBSHARED_EXPORT CPosConnectionImpl : public CVideoItem, public IVideoItemPosConnection
	{
		Q_OBJECT

	protected:
		CPosConnectionImpl(void);
		virtual ~CPosConnectionImpl(void);

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

		virtual double GetWidthInDocPt() const override;
		virtual double GetHeightInDocPt() const override;

		virtual void SetWidthInDocPt(double val) override;
		virtual void SetHeightInDocPt(double val) override;

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

		// Реализация интерефейса IVideoItemPosLine
		//
	private:
		std::list<VideoItemPoint> points;
		std::list<VideoItemPoint> extPoints;	// точки, которые используются при DrawOutline, не сериализируются

	public:
		virtual const std::list<VideoItemPoint>& GetPointList() const override;
		virtual void SetPointList(const std::list<VideoItemPoint>& points) override;
		virtual void AddPoint(double x, double y) override;
		virtual void RemoveSamePoints() override;
		virtual void DeleteAllPoints() override;
		virtual void DeleteLastPoint() override;

		virtual const std::list<VideoItemPoint>& GetExtensionPoints() const override;
		virtual void SetExtensionPoints(const std::list<VideoItemPoint>& extPoints) override;
		virtual void AddExtensionPoint(double x, double y) override;
		virtual void DeleteAllExtensionPoints() override;
		virtual void DeleteLastExtensionPoint() override;

		// Реализация интерефейса IVideoItemPropertiesPos
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
		virtual std::vector<VideoItemPoint> getPointList() const override;
		virtual void setPointList(const std::vector<VideoItemPoint>& points) override;
	};

}


