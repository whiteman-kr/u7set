#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

namespace VFrame30
{
	/*! \class SchemaItemRect
		\ingroup staticSchemaItems
		\brief This item is used to display rectangles and text messages

		This item is used to display rectangles and text messages.

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
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemRect.<br>

		<b>ClickScript example:</b>
		\code
		(function(schemaItem)
		{
			// Switch to schema with "CONTENTS" identifier
			//
			view.setSchema("CONTENTS");
		})
		\endcode

		<b>PreDrawScript example 1:</b>
		\code
		(function(schemaItem)
		{
			// Set item's text, background color and fill color
			//
			schemaItem.Text = "Hello World";
			schemaItem.FillColor = "red";
			schemaItem.BackColor = "#000000";
		})
		\endcode

		<b>PreDrawScript example 2:</b>
		\code
		(function(schemaItem)
		{
			// Get the state of the signal "#APPSIGNALID01"
			//
			var state = signals.signalState("#APPSIGNALID01");

			if (state == undefined || state.Valid == false)
			{
				// If signal does not exist or not valid - display blinking black/white "Alert" text on blinking red/black foreground
				//
				schemaItem.Text = "Alert";
				schemaItem.FillColor = schemaItem.BlinkPhase ? "red" : "black";
				schemaItem.TextColor = schemaItem.BlinkPhase ? "black" : "white";
			}
			else
			{
				// Signal is OK - display "OK" black text on white background
				//
				schemaItem.Text = "OK";
				schemaItem.FillColor = "white";
				schemaItem.TextColor = "black";
			}
		})
		\endcode
	*/
	class VFRAME30LIBSHARED_EXPORT SchemaItemRect : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Border line weight, in pixels
		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)

		/// \brief Border line color name
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		/// \brief Rectangle fill color name
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

		/// \brief Switches rectangle filling
		Q_PROPERTY(double Fill READ fill WRITE setFill)

		/// \brief Switches rectangle border drawing
		Q_PROPERTY(double DrawRect READ drawRect WRITE setDrawRect)

		// Text Category Properties

		/// \brief Text color name
		Q_PROPERTY(QColor TextColor READ textColor WRITE setTextColor)

		/// \brief Rectangle text
		Q_PROPERTY(QString Text READ text WRITE setText)

		/// \brief Horizontal text alignment
		Q_PROPERTY(E::HorzAlign AlignHorz READ horzAlign WRITE setHorzAlign)

		/// \brief Vertical text alignment
		Q_PROPERTY(E::VertAlign AlignVert READ vertAlign WRITE setVertAlign)

		/// \brief Font name
		Q_PROPERTY(QString FontName READ getFontName WRITE setFontName)

		/// \brief Font size
		Q_PROPERTY(double FontSize READ getFontSize WRITE setFontSize)

		/// \brief Font bold
		Q_PROPERTY(bool FontBold READ getFontBold WRITE setFontBold)

		/// \brief Font italic
		Q_PROPERTY(bool FontItalic READ getFontItalic WRITE setFontItalic)


	public:
		SchemaItemRect(void);
		explicit SchemaItemRect(SchemaUnit unit);
		virtual ~SchemaItemRect(void);

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Properties and Data
		//
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		const QString& text() const;
		void setText(QString value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		DECLARE_FONT_PROPERTIES(Font)

		bool fill() const;
		void setFill(bool fill);

		bool drawRect() const;
		void setDrawRect(bool value);

	private:
		double m_weight = 0.0;				// Line weight, in pixels or inches depends on UnitDocPt
		QColor m_lineColor;
		QColor m_fillColor;
		QColor m_textColor;
		QString m_text;
		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_fill = true;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
