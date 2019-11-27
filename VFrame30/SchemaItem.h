#pragma once

#include "VFrame30Lib_global.h"
#include "SchemaPoint.h"
#include "../lib/TypesAndEnums.h"
#include "../lib/PropertyObject.h"
#include "../lib/ProtoSerialization.h"
#include "../lib/DebugInstCounter.h"
#include "../lib/OutputLog.h"
#include <QJSValue>


class QJSEngine;
class QPaintDevice;

namespace VFrame30
{
	class Schema;
	class SchemaLayer;
	class SchemaItem;
	class SchemaItemAfb;
	class SchemaItemSignal;
	class SchemaView;

	class FblItemRect;
	class FblItem;

	class CDrawParam;
}

using SchemaItemPtr = std::shared_ptr<VFrame30::SchemaItem>;

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
		virtual void setLeft(const double& value) = 0;

		virtual double top() const = 0;
		virtual void setTop(const double& value) = 0;

		virtual double width() const = 0;
		virtual void setWidth(const double& value) = 0;

		virtual double height() const = 0;
		virtual void setHeight(const double& value) = 0;
	};

	// Interface IPointList, for storing and restoring point list
	//
	class IPointList
	{
	public:
		virtual std::vector<SchemaPoint> getPointList() const = 0;
		virtual void setPointList(const std::vector<SchemaPoint>& points) = 0;
	};


	/*! \class SchemaItem
		\brief Base class for all items displayed on schemas.

		Base class for all items displayed on schemas.
	*/
	class VFRAME30LIBSHARED_EXPORT SchemaItem :
		public PropertyObject,
		public ISchemaItemPropertiesPos,
		public IPointList,
		public Proto::ObjectSerialization<SchemaItem>,
		public DebugInstCounter<SchemaItem>
	{
		Q_OBJECT

		/// \brief Object name
		Q_PROPERTY(QString ObjectName READ objectName)

		/// \brief Blining phase. This value is inverted each time schema is being redrawn
		Q_PROPERTY(bool BlinkPhase READ blinkPhase)

		/// \brief Turns <b>ClickScript</b> script call when user clicks on schema item
		Q_PROPERTY(bool AcceptClick READ acceptClick WRITE setAcceptClick)

		/*! \brief Contains mouse click event handler code.
		Click event is generated each time when user clicks mouse button on the item and <b>AcceptClick</b> property is set to true.
		*/
		Q_PROPERTY(QString ClickScript READ clickScript)

		/*! \brief Contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn.
		In most cases, this code is used to change visual apperance of an item before drawing.
		*/
		Q_PROPERTY(QString PreDrawScript READ preDrawScript)

	protected:
		SchemaItem();

	public:
		virtual ~SchemaItem();

	protected:
		virtual void propertyDemand(const QString& prop) override;

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
		virtual void moveItem(double horzOffsetDocPt, double vertOffsetDocPt);

		virtual void snapToGrid(double gridSize);

		virtual double GetWidthInDocPt() const;
		virtual double GetHeightInDocPt() const;

		virtual void SetWidthInDocPt(double widthInDocPt);
		virtual void SetHeightInDocPt(double heightInDocPt);

		static void dump(std::shared_ptr<SchemaItem> item);
		virtual void dump() const;

		virtual void clickEvent(QString globalScript, QJSEngine* engine,  QWidget* parentWidget);
		virtual bool preDrawEvent(QString globalScript, QJSEngine* engine);

	protected:
		bool runScript(QJSValue& evaluatedJs, QJSEngine* engine);
		QJSValue evaluateScript(QString script, QString globalScript, QJSEngine* engine, QWidget* parentWidget) const;
		QString formatSqriptError(const QJSValue& scriptValue) const;
		void reportSqriptError(const QJSValue& scriptValue, QWidget* parent) const;

		// Text search/replace
		//
	public:
		//	first - property where text found
		//	second - property value
		std::list<std::pair<QString, QString>> searchTextByProps(const QString& text, Qt::CaseSensitivity cs) const;
		virtual int replace(QString findText, QString replaceWith, Qt::CaseSensitivity cs);

		// Draw Functions
		//

		// Drawing item is in 100% scale
		// Graphcis must have sceen coordinate system (0, 0 - left up corner, down and right - positive coordinate values)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const;


	public:
		// Draw item's label
		//
		virtual void drawLabel(CDrawParam* drawParam) const;

		// Draw item outline, while creation or changing
		//
		virtual void drawOutline(CDrawParam* pDrawParam) const;
		static void drawOutline(CDrawParam* pDrawParam, const std::vector<std::shared_ptr<SchemaItem>>& items);

		// Draw item issue
		//
		virtual void drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const;

		// Draw debug info
		//
		virtual void drawDebugInfo(CDrawParam* drawParam, const QString& runOrderIndex) const;
		virtual void drawScriptError(CDrawParam* drawParam) const;

		// Draw item selection depending on position interface
		//
		virtual void drawSelection(CDrawParam* pDrawParam, bool drawSizeBar) const;
		static void drawSelection(CDrawParam* drawParam, const std::vector<std::shared_ptr<SchemaItem>>& items, bool drawSizeBar);

		virtual void drawCompareAction(CDrawParam* drawParam, QColor color) const;

		// Draw comment "dim"
		//
		virtual void drawCommentDim(CDrawParam* drawParam) const;

		// Determine and Calculation Functions
		//
	public:
		// Определение, входит ли точка в элемент, x и y в дюймах или в пикселях
		//
		virtual bool isIntersectPoint(double x, double y) const;

		// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
		// координаты и размер прямоугольника заданы в дюймах или пикселях
		//
		virtual bool isIntersectRect(double x, double y, double width, double height) const;

		static double penDeviceWidth(const QPaintDevice* device, double penWidth);

		// ISchemaItemPropertiesPos interface implementation
		//
	public:
		virtual double left() const override;
		virtual void setLeft(const double& value) override;

		virtual double top() const override;
		virtual void setTop(const double& value) override;

		virtual double width() const override;
		virtual void setWidth(const double& value) override;

		virtual double height() const override;
		virtual void setHeight(const double& value) override;

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

		bool isFblItemRect() const;
		FblItemRect* toFblItemRect();
		const FblItemRect* toFblItemRect() const;

		bool isFblItem() const;
		FblItem* toFblItem();
		const FblItem* toFblItem() const;

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

		bool isControl() const;

		bool isLocked() const;
		void setLocked(const bool& locked);

		bool isCommented() const;
		bool commented() const;
		void setCommented(const bool& value);

		QUuid guid() const;
		void setGuid(const QUuid& guid);
		virtual void setNewGuid();			// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		// Item position unit, can be inches or pixels
		//
		SchemaUnit itemUnit() const;
		void setItemUnit(SchemaUnit value);

		QString label() const;
		void setLabel(const QString& value);

		E::TextPos labelPos() const;
		void setLabelPos(const E::TextPos& value);

		bool acceptClick() const;
		void setAcceptClick(const bool& value);

		QString clickScript() const;
		void setClickScript(const QString& value);

		QString preDrawScript() const;
		void setPreDrawScript(const QString& value);

		bool blinkPhase() const;

		const CDrawParam* drawParam() const;
		void setDrawParam(CDrawParam* value);

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt(CDrawParam* drawParam) const;

		virtual QString toolTipText(int dpiX, int dpiY) const;

		QString lastScriptError() const;

		// Data
		//
	protected:
		bool m_static = true;
		bool m_locked = false;
		bool m_commented = false;
		QUuid m_guid;
		SchemaUnit m_itemUnit;		// Item position unit, can be inches or pixels

		QString m_label;
		E::TextPos m_labelPos = E::TextPos::RightTop;

		bool m_acceptClick = false;	// The SchemaItem accept mouse Left button click and runs script
		QString m_clickScript;		// Qt script on mouse left button click
		QString m_preDrawScript;

		bool m_blinkPhase = false;			// Taken from m_drawParam
		CDrawParam* m_drawParam = nullptr;	// Is filled before PreDrawScript (in Schema::draw) to have ability to call MacroExpander::expand in AppSiganlIDs getter from script

		QJSValue m_jsClickScript;		// Evaluated m_clickScript
		QJSValue m_jsPreDrawScript;		// Evaluated m_preDrawScript

		mutable QString m_lastScriptError;

	public:
		static const QColor errorColor;
		static const QColor warningColor;
		static const QColor selectionColor;
		static const QColor lockedSelectionColor;
		static const QColor commentedColor;
		static const QColor highlightColor1;
		static const QColor highlightColor2;
	};

#ifdef VFRAME30LIB_LIBRARY
	extern Factory<VFrame30::SchemaItem> SchemaItemFactory;
#endif

}
