#pragma once

#include "SchemaPoint.h"
#include "../CommonLib/PropertyObject.h"
#include "../CommonLib/Factory.h"
#include "../CommonLib/DebugInstCounter.h"
#include "../lib/OutputLog.h"
#include "../Proto/ProtoSerialization.h"
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
	extern ::Factory<VFrame30::SchemaItem> SchemaItemFactory;

	// Интерфейс для SchemaItem который перводит любой тип хранения координат (ISchemaPosRect, ISchemaPosLine, ...) в
	// прямоугольник, для отображения в СВОЙСТВАХ ОБЪЕКТА. ВНИМАНИЕ! возврат элементов происходит в единицах мм, дюймы, точки.
	// ВНИМАНИЕ! Эти свойства нельзя использовать для рисования и вычисления новых координат!
	//
	class ISchemaItemPropertiesPos
	{
	public:
		[[nodiscard]] virtual double left() const = 0;
		virtual void setLeft(double value) = 0;

		[[nodiscard]] virtual double top() const = 0;
		virtual void setTop(double value) = 0;

		[[nodiscard]] virtual double width() const = 0;
		virtual void setWidth(double value) = 0;

		[[nodiscard]] virtual double height() const = 0;
		virtual void setHeight(double value) = 0;
	};

	// Interface IPointList, for storing and restoring point list
	//
	class IPointList
	{
	public:
		[[nodiscard]] virtual std::vector<SchemaPoint> getPointList() const = 0;
		virtual void setPointList(const std::vector<SchemaPoint>& points) = 0;
	};


	/*! \class SchemaItem
		\brief Base class for all items displayed on schemas.

		Base class for all items displayed on schemas.
	*/
	class SchemaItem :
		public PropertyObject,
		public ISchemaItemPropertiesPos,
		public IPointList,
		public Proto::ObjectSerialization<SchemaItem>,
		public DebugInstCounter<SchemaItem>
	{
		Q_OBJECT

		/// \brief Object name
		Q_PROPERTY(QString objectName READ objectName)
		Q_PROPERTY(QString ObjectName READ objectName)

		/*! \brief Turns <b>ClickScript</b> script call when user clicks mouse button on schema item

		  When this property is set to true, <b>ClickScript</b> event handler is called when user clicks left mouse button on a schema item.

		  \warning This property has no effect on SchemaItemPushButton and SchemaItemLineEdit.
		*/
		Q_PROPERTY(bool acceptClick READ acceptClick WRITE setAcceptClick)
		Q_PROPERTY(bool AcceptClick READ acceptClick WRITE setAcceptClick)

		/// \brief A script to run before each schema redraw event.
		Q_PROPERTY(QString preDrawScript READ preDrawScript WRITE setPreDrawScript)

		/*! \brief Blinking phase. Value of this property is inverted approximately each 250 milliseconds

		Blinking phase.	Value of this property is inverted (switched from true to false or vice versa) approximately each 250 milliseconds.
		Used to implement schema items blinking. For example, script code can paint an item with one color when value is set to true and
		with another color when it is set to false.

		Value inverting is based on system clock. This is required to synchronize blinking when multiple Monitor or TuningClient
		applications run on the same computer.

		\warning <b>preDrawScript</b> code calling is not synchronized with <b>blinkPhase</b> inverting. According to this, it is possible that <b>blinkPhase</b> value will not changed
		in two consecutive <b>preDrawScript</b> calls.
		*/
		Q_PROPERTY(bool blinkPhase READ blinkPhase)
		Q_PROPERTY(bool BlinkPhase READ blinkPhase)

		/// \brief Show or hide schema item in client Software (Monitor, TuningClient, etc)
		Q_PROPERTY(bool visible READ visible WRITE setVisible)
		Q_PROPERTY(bool Visible READ visible WRITE setVisible)

		/// \brief SchemaItem's tags.
		Q_PROPERTY(QStringList tags READ tagsAsList)
		Q_PROPERTY(QStringList Tags READ tagsAsList)

		/// \brief Item's type as a string, e.g. SchemaItemInput, SchemaItemUfb, SchemaItemAfb, SchemaItemLine, etc..
		Q_PROPERTY(QString type  READ type)

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

		virtual void clickEvent(QJSEngine* engine,  QWidget* parentWidget);
		virtual bool preDrawEvent(QJSEngine* engine);

	protected:
		bool runScript(QJSValue& evaluatedJs, QJSEngine* engine);
		QJSValue evaluateScript(QString script, QJSEngine* engine, QWidget* parentWidget) const;
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

		// Properties and Data
		//
	public:
		bool IsStatic() const noexcept;
		bool IsDynamic() const noexcept;

		QString type() const;

		bool isFblItemRect() const noexcept;
		FblItemRect* toFblItemRect();
		const FblItemRect* toFblItemRect() const;

		bool isFblItem() const noexcept;
		FblItem* toFblItem();
		const FblItem* toFblItem() const;

		bool isSchemaItemAfb() const noexcept;
		SchemaItemAfb* toSchemaItemAfb();
		const SchemaItemAfb* toSchemaItemAfb() const;

		template<typename SCHEMAITEMTYPE>
		bool isType() const noexcept
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

		bool isControl() const ;

		bool isLocked() const ;
		void setLocked(bool locked);

		bool isCommented() const;
		bool commented() const;
		void setCommented(bool value);

		bool visible() const;
		void setVisible(bool value);

		QUuid guid() const;
		void setGuid(QUuid guid);
		virtual void setNewGuid();			// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		// Item position unit, can be inches or pixels
		//
		SchemaUnit itemUnit() const noexcept;
		void setItemUnit(SchemaUnit value);

		[[nodiscard]] QString tagsAsString() const;
		[[nodiscard]] QStringList tagsAsList() const;

		void addTag(QString tag);
		void setTags(QString tags);
		void setTagsList(const QStringList& tags);

		/// \brief Check if SchemaItem has specified tag. There is an implicit tag that is the type of the item, e.g. SchemaItemInput, SchemaItemUfb, SchemaItemAfb, SchemaItemLine, etc..
		Q_INVOKABLE	bool hasTag(QString tag) const;

		QString label() const ;
		void setLabel(const QString& value);

		E::TextPos labelPos() const ;
		void setLabelPos(E::TextPos value);

		bool acceptClick() const ;
		void setAcceptClick(bool value);

		QString clickScript() const ;
		void setClickScript(QString value);

		QString preDrawScript() const ;
		void setPreDrawScript(QString value);

		bool blinkPhase() const;

		const CDrawParam* drawParam() const;
		void setDrawParam(CDrawParam* value);

		// Get SchemaItem bounding rectangle in itemUnit()
		//
		virtual QRectF boundingRectInDocPt(const CDrawParam* drawParam) const;

		virtual QString toolTipText(int dpiX, int dpiY) const;

		QString lastScriptError() const noexcept;

		// Data
		//
	protected:
		bool m_static = true;
		bool m_locked = false;
		bool m_commented = false;
		bool m_visible = true;		// Visible for client
		QUuid m_guid;
		SchemaUnit m_itemUnit;		// Item position unit, can be inches or pixels

		QStringList m_tags;

		QString m_label;
		E::TextPos m_labelPos = E::TextPos::RightTop;

		bool m_acceptClick = false;	// The SchemaItem accept mouse Left button click and runs script
		QString m_clickScript;		// Qt script on mouse left button click
		QString m_preDrawScript;

		// Runtime stuff
		//
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
}
