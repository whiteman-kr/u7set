#pragma once

#include "DrawParam.h"
#include "../include/PropertyObject.h"
#include "../include/ProtoSerialization.h"
#include "../include/DebugInstCounter.h"
#include "../include/TypesAndEnums.h"

namespace VFrame30
{
	class Scheme;
	class SchemeLayer;
}


namespace VFrame30
{
	struct VFRAME30LIBSHARED_EXPORT SchemePoint
	{
		double X;
		double Y;

		// Methods
		//
		SchemePoint() :
			X(0),
			Y(0)
		{
		}

		explicit SchemePoint(const Proto::SchemePoint& vip)
		{
			LoadData(vip);
		}

		explicit SchemePoint(QPointF point) :
			X(point.x()),
			Y(point.y())
		{
		}

		SchemePoint(double x, double y) :
			X(x),
			Y(y)
		{
		}

		bool operator == (const SchemePoint& pt) const
		{
			return std::abs(pt.X - X) < 0.000001 && std::abs(pt.Y - Y) < 0.000001;
		}

		bool operator < (const SchemePoint& pt) const
		{
			if (operator==(pt) == true)
			{
				return false;
			}

			if (std::abs(pt.Y - Y) < 0.000001)
			{
				return X < pt.X;
			}

			if (Y < pt.Y)
				return true;

			if (Y > pt.Y)
				return false;

			return false;
		}

		operator QPointF() const
		{
			return QPointF(X, Y);
		}

		bool SaveData(Proto::SchemePoint* vip) const
		{
			vip->set_x(X);
			vip->set_y(Y);
			return true;
		}
		bool LoadData(const Proto::SchemePoint& vip)
		{
			this->X = vip.x();
			this->Y = vip.y();
			return true;
		}
	};

	// Интерфейс для SchemeItem который перводит любой тип хранения координат (ISchemePosRect, ISchemePosLine, ...) в
	// прямоугольник, для отображения в СВОЙСТВАХ ОБЪЕКТА. ВНИМАНИЕ! возврат элементов происходит в единицах мм, дюймы, точки.
	// ВНИМАНИЕ! Эти свойства нельзя использовать для рисования и вычисления новых координат!
	//
	class ISchemeItemPropertiesPos
	{
	public:
		virtual double left() const = 0;
		virtual void setLeft(double value) = 0;

		virtual double top() const = 0;
		virtual void setTop(double value) = 0;

		virtual double width() const = 0;
		virtual void setWidth(double value) = 0;

		virtual double height() const = 0;
		virtual void setHeight(double value) = 0;
	};

	// Interface IPointList, for storing and restoring point list
	//
	class IPointList
	{
	public:
		virtual std::vector<SchemePoint> getPointList() const = 0;
		virtual void setPointList(const std::vector<SchemePoint>& points) = 0;
	};


	class VFRAME30LIBSHARED_EXPORT SchemeItem :
		public PropertyObject,
		public ISchemeItemPropertiesPos,
		public IPointList,
		public Proto::ObjectSerialization<SchemeItem>,
		public DebugInstCounter<SchemeItem>
	{
		Q_OBJECT

	protected:
		SchemeItem();

	public:
		virtual ~SchemeItem();

		// Serialization
		//
		friend Proto::ObjectSerialization<SchemeItem>;	// For call CreateObject from Proto::ObjectSerialization

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this func only for serialization, while creting new object it is not fully initialized
		// and must be read from somewhere
		//
		static SchemeItem* CreateObject(const Proto::Envelope& message);

		// Action Functions
		//
	public:
		virtual void MoveItem(double horzOffsetDocPt, double vertOffsetDocPt);

		virtual void snapToGrid(double gridSize);

		virtual double GetWidthInDocPt() const;
		virtual double GetHeightInDocPt() const;

		virtual void SetWidthInDocPt(double widthInDocPt);
		virtual void SetHeightInDocPt(double heightInDocPt);
		
		// Draw Functions
		//
	public:
		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* pDrawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const;

		// Draw item outlien, while creation or changing
		//
		virtual void DrawOutline(CDrawParam* pDrawParam) const;
		static void DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemeItem>>& items);

		// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
		//
		virtual void DrawSelection(CDrawParam* pDrawParam, bool drawSizeBar) const;
		static void DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemeItem>>& items, bool drawSizeBar);

		// Determine and Calculation Functions
		//
	public:	
		// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
		// 
		virtual bool IsIntersectPoint(double x, double y) const;

		// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
		// координаты и размер прямоугольника заданы в дюймах или пикселях
		// 
		virtual bool IsIntersectRect(double x, double y, double width, double height) const;

		// ISchemeItemPropertiesPos interface implementation
		//
	public:
		virtual double left() const override;
		virtual void setLeft(double) override;

		virtual double top() const override;
		virtual void setTop(double) override;

		virtual double width() const override;
		virtual void setWidth(double) override;

		virtual double height() const override;
		virtual void setHeight(double) override;

		// IPointList implementation
		//
	public:
		virtual std::vector<SchemePoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemePoint>& points) override;

		// Properties and Data
		//
	public:
		bool IsStatic() const;
		bool IsDynamic() const;

		virtual bool IsFblItem() const;

		bool IsLocked() const;
		void setLocked(bool locked);

		const QUuid& guid() const;
		void setGuid(const QUuid& guid);
		virtual void setNewGuid();			// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		// Item position unit, can be inches or pixels
		//
		SchemeUnit itemUnit() const;
		void setItemUnit(SchemeUnit value);

		bool acceptClick() const;
		void setAcceptClick(bool value);

		const QString& clickScript() const;
		void setClickScript(const QString& value);

		// Get SchemeItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const;		

		// Data
		//
	protected:
		bool m_static;
		bool m_locked;
		QUuid m_guid;
		SchemeUnit m_itemUnit;		// Item position unit, can be inches or pixels

		bool m_acceptClick;			// The SchemeItem accept mouse Left button click and runs script
		QString m_clickScript;		// Qt script on mouse left button click
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemeItem> SchemeItemFactory;
#endif
}


