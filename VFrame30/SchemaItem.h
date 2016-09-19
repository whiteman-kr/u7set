#pragma once

#include "SchemaPoint.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "../lib/PropertyObject.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/DebugInstCounter.h"
#include "../lib/Types.h"
#include "../lib/TypesAndEnums.h"

namespace VFrame30
{
	class Schema;
	class SchemaLayer;
	class SchemaItemAfb;
	class SchemaItemSignal;
}


namespace VFrame30
{
	// Интерфейс для SchemaItem который перводит любой тип хранения координат (ISchemaPosRect, ISchemaPosLine, ...) в
	// прямоугольник, для отображения в СВОЙСТВАХ ОБЪЕКТА. ВНИМАНИЕ! возврат элементов происходит в единицах мм, дюймы, точки.
	// ВНИМАНИЕ! Эти свойства нельзя использовать для рисования и вычисления новых координат!
	//
	class ISchemaItemPropertiesPos
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
		virtual std::vector<SchemaPoint> getPointList() const = 0;
		virtual void setPointList(const std::vector<SchemaPoint>& points) = 0;
	};


	class VFRAME30LIBSHARED_EXPORT SchemaItem :
		public PropertyObject,
		public ISchemaItemPropertiesPos,
		public IPointList,
		public Proto::ObjectSerialization<SchemaItem>,
		public DebugInstCounter<SchemaItem>
	{
		Q_OBJECT

	protected:
		SchemaItem();

	public:
		virtual ~SchemaItem();

		// Serialization
		//
		friend Proto::ObjectSerialization<SchemaItem>;	// For call CreateObject from Proto::ObjectSerialization

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this func only for serialization, while creting new object it is not fully initialized
		// and must be read from somewhere
		//
		static std::shared_ptr<SchemaItem> CreateObject(const Proto::Envelope& message);

		// Action Functions
		//
	public:
		virtual void MoveItem(double horzOffsetDocPt, double vertOffsetDocPt);

		virtual void snapToGrid(double gridSize);

		virtual double GetWidthInDocPt() const;
		virtual double GetHeightInDocPt() const;

		virtual void SetWidthInDocPt(double widthInDocPt);
		virtual void SetHeightInDocPt(double heightInDocPt);

		static void debug(std::shared_ptr<SchemaItem> item);
		virtual void debug() const;
		
		// Draw Functions
		//
	public:
		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* pDrawParam, const Schema* pFrame, const SchemaLayer* pLayer) const;

		// Draw item outline, while creation or changing
		//
		virtual void DrawOutline(CDrawParam* pDrawParam) const;
		static void DrawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items);

		// Draw item issue
		//
		virtual void DrawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const;

		// Draw debug info
		//
		virtual void DrawDebugInfo(CDrawParam* drawParam, const QString& runOrderIndex) const;

		// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
		//
		virtual void DrawSelection(CDrawParam* pDrawParam, bool drawSizeBar) const;
		static void DrawSelection(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items, bool drawSizeBar);

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

		// ISchemaItemPropertiesPos interface implementation
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
		virtual std::vector<SchemaPoint> getPointList() const override;
		virtual void setPointList(const std::vector<SchemaPoint>& points) override;

		// Properties and Data
		//
	public:
		bool IsStatic() const;
		bool IsDynamic() const;

		virtual bool IsFblItem() const;

		bool isSchemaItemAfb() const;
		SchemaItemAfb* toSchemaItemAfb();
		const SchemaItemAfb* toSchemaItemAfb() const;

		template<typename SCHEMAITEMTYPE>
		bool isType() const
		{
			return dynamic_cast<const SCHEMAITEMTYPE*>(this) != nullptr;
		}

		template<typename SCHEMAITEMTYPE>
		SCHEMAITEMTYPE* toType()
		{
			return dynamic_cast<SCHEMAITEMTYPE*>(this);
		}

		template<typename SCHEMAITEMTYPE>
		const SCHEMAITEMTYPE* toType() const
		{
			return dynamic_cast<const SCHEMAITEMTYPE*>(this);
		}


		bool IsLocked() const;
		void setLocked(bool locked);

		const QUuid& guid() const;
		void setGuid(const QUuid& guid);
		virtual void setNewGuid();			// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		// Item position unit, can be inches or pixels
		//
		SchemaUnit itemUnit() const;
		void setItemUnit(SchemaUnit value);

		bool acceptClick() const;
		void setAcceptClick(bool value);

		const QString& clickScript() const;
		void setClickScript(const QString& value);

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt() const;		

		// Data
		//
	protected:
		bool m_static;
		bool m_locked;
		QUuid m_guid;
		SchemaUnit m_itemUnit;		// Item position unit, can be inches or pixels

		bool m_acceptClick;			// The SchemaItem accept mouse Left button click and runs script
		QString m_clickScript;		// Qt script on mouse left button click

	public:
		static const QColor errorColor;
		static const QColor warningColor;
		static const QColor selectionColor;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemaItem> SchemaItemFactory;
#endif
}


