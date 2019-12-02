#pragma once

#include "PosLineImpl.h"

namespace VFrame30
{
	/*! \class SchemaItemLine
		\ingroup staticSchemaItems
		\brief This item is used to display lines

		This item is used to display lines.

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
	class VFRAME30LIBSHARED_EXPORT SchemaItemLine : public PosLineImpl
	{
		Q_OBJECT

		/// \brief Line weight, in pixels
		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)

		/// \brief Line color name
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

	public:
		SchemaItemLine(void);
		explicit SchemaItemLine(SchemaUnit unit);
		virtual ~SchemaItemLine(void);

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
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QColor m_lineColor;
	};
}
