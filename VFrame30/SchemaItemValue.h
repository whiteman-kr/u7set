#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

class AppSignalState;
class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{
	/*! \class SchemaItemValue
		\ingroup dynamicSchemaItems
		\brief This item is used to display signal values

		This item is used to display signal values.

		Information displayed by this item is fully customizable by scripts. Script code can receive signal parameters and states from data services,
		set text, colors, font size to any values depending on customers requirements.

		Signal identidiers set to the schema item are stored in <b>SignalIDs</b> array property.

		To modify contents of the item, set <b>Text</b>, <b>TextColor</b>, <b>FillColor</b>, <b>LineColor</b> properties etc.

		<b>Event handlers</b>

		To customize item's apperance and behavior, event handler code is placed to following properties of the schema item using RPCT:

		- <b>ClickScript</b> contains mouse click event handler code.
		Click event is generated each time when user clicks mouse button on the item and <b>AcceptClick</b> property is set to true;<br>
		-  <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn.

		<b>ClickScript</b> and <b>PreDrawScript</b> event handler function protypes:

		\code
		function(schemaItem)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemValue.<br>

		<b>PreDrawScript example:</b>

		\code
		(function(schemaItemValue)
		{
			// Check for signals number
			//
			if (schemaItemValue.SignalIDs.length != 1)
			{
				schemaItemValue.Text = "No Signals!";
				return;
			}

			// Take first signal identifier
			//
			let appSignalId = schemaItemValue.SignalIDs[0];

			// Get data from TuningService
			//
			let signalParam = tuning.signalParam(appSignalId);
			let signalState = tuning.signalState(appSignalId);

			if (signalState == undefined)
			{
				// Signal was not found
				//
				schemaItemValue.Text = appSignalId;
			}
			else
			{
				// Get signal state
				//
				if (signalState.Valid == true)
				{
					// Signal state is valid
					//
					schemaItemValue.Text = signalState.Value;
					schemaItemValue.TextColor = "black";
					schemaItemValue.FillColor = "white";
					schemaItemValue.LineColor = "#000000";
				}
				else
				{
					// Signal state is not valid
					//
					schemaItemValue.Text = "?";
					schemaItemValue.TextColor = schemaItemValue.BlinkPhase ? "white" : "black";
					schemaItemValue.FillColor = schemaItemValue.BlinkPhase ? "black" : "#A00000";
					schemaItemValue.LineColor = "#A00000";
				}
			}
		})
		\endcode
	*/
	class VFRAME30LIBSHARED_EXPORT SchemaItemValue : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Application signal identifiers array. Use <b>AppSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)

		/// \brief Application signal identifiers array. Use <b>AppSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList AppSignalIDs READ signalIds WRITE setSignalIds)

		// Appearance
		//

		/// \brief Border line weight, in pixels
		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		/// \brief Border line color name
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		/// \brief Rectangle fill color name
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

		/// \brief Text color name
		Q_PROPERTY(QColor TextColor READ textColor WRITE setTextColor)

		/// \brief Bounding rectangle drawing
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		// Text Category Properties
		//

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

		/// \brief Text
		Q_PROPERTY(QString Text READ text WRITE setText)

		/// \brief Precision
		Q_PROPERTY(int Precision READ precision WRITE setPrecision)

	public:
		SchemaItemValue(void);
		explicit SchemaItemValue(SchemaUnit unit);
		virtual ~SchemaItemValue(void) = default;

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		void initDrawingResources() const;
		void drawText(CDrawParam* drawParam, const QRectF& rect) const;

		bool getSignalState(QString appSignalId, CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		QString parseText(QString text, CDrawParam* drawParam, const AppSignalParam& signal, const AppSignalState& signalState) const;
		QString formatNumber(double value, const AppSignalParam& signal) const;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Java Script invocables specific for SchemaItemValue
		//
	public:

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		double lineWeight() const;
		void setLineWeight(double lineWeight);

		const QColor& lineColor() const;
		void setLineColor(const QColor& color);

		const QColor& fillColor() const;
		void setFillColor(const QColor& color);

		const QColor& textColor() const;
		void setTextColor(const QColor& color);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		DECLARE_FONT_PROPERTIES(Font)

		bool drawRect() const;
		void setDrawRect(bool value);

		const QString& text() const;
		void setText(QString value);

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgb(0x00, 0x00, 0x00)};
		QColor m_fillColor = {qRgb(0x00, 0x00, 0xC0)};
		QColor m_textColor = {qRgb(0xF0, 0xF0, 0xF0)};

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_drawRect = false;		// Rect is visible, thikness 0 is possible

		QString m_text = {"$(value)"};	// $(value)			: signal value
										// $(caption)		: caption
										// $(signalid)		: SignalID (CustomSignalID)
										// $(appsignalid)	: AppSignalID (#APPSIGANLID)
										// $(equipmentid)	: Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)
										// $(highlimit)		: High limit
										// $(lowlimit)		: Low limit
										// $(units)			: Signal units

		int m_precision = -1;			// decimal places, -1 means take value from Signal
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;

		// Drawing resources
		//
		mutable std::unique_ptr<QPen> m_rectPen;
		mutable std::unique_ptr<QBrush> m_fillBrush;
	};
}
