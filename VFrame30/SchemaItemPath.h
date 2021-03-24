#pragma once

#include "PosConnectionImpl.h"

namespace VFrame30
{
	/*! \class SchemaItemPath
		\ingroup staticSchemaItems
		\brief This item is used to display paths

		This item is used to display paths.

		<b>Event handlers</b>

		To customize item's apperance and behaviour, event handler code is placed to following properties of the schema item using RPCT:

		- <b>ClickScript</b> contains mouse click event handler code.
		Click event is generated each time when user clicks mouse button on the item and <b>AcceptClick</b> property is set to true;<br>
		-  <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn.

		<b>ClickScript</b> and <b>PreDrawScript</b> event handler function protypes:

		\code
		function(schemaItem)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemPath.<br>

		<b>PreDrawScript example 1:</b>
		\code
		(function(schemaItem)
		{
			// Change the color of the line. Color is set by name
			//
			schemaItem.LineColor = "red";
		})
		\endcode

		<b>PreDrawScript example 2:</b>
		\code
		(function(schemaItem)
		{
			// Change the color of the line depending on blink phase. Color is set by hexdecimal number
			//
			schemaItem.LineColor = schemaItem.BlinkPhase ? "#c00000" : "#ffffff";
		})
		\endcode
	*/
	class VFRAME30LIBSHARED_EXPORT SchemaItemPath : public PosConnectionImpl
	{
		Q_OBJECT

		/// \brief Line weight, in pixels
		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)

		/// \brief Line color name
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

	public:
		SchemaItemPath(void);
		explicit SchemaItemPath(SchemaUnit unit);
		virtual ~SchemaItemPath(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const final;

		// Properties and Data
		//
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		E::LineStyle lineStyle() const;
		void setLineStyle(E::LineStyle value);

		E::LineStyleCap lineStyleCap() const;
		void setLineStyleCap(E::LineStyleCap value);

		E::LineCap lineCapStart() const;
		void setLineCapStart(E::LineCap value);

		E::LineCap lineCapEnd() const;
		void setLineCapEnd(E::LineCap value);

		double lineCapFactor() const;
		void setLineCapFactor(double value);

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QColor m_lineColor;
		E::LineStyle m_lineStyle = E::SolidLine;

		E::LineStyleCap m_lineStyleCap = E::LineStyleCap::SquareCap;

		E::LineCap m_lineCapStart = E::LineCap::NoCap;
		E::LineCap m_lineCapEnd = E::LineCap::NoCap;
		double m_lineCapFactor = 2.0;
	};
}
