#pragma once

#include <VFrame30/FontParam.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/Session.h>
#include <VFrame30/PosRectRotatable.h>

class QPen;
class QBrush;

class AppSignalState;
class AppSignalParam;
class TuningSignalState;


namespace HardwareLib
{
	class DiagSignal;
}

namespace VFrame30
{
	/*! \class SchemaItemDiagValue
		\ingroup dynamicSchemaItems
		\brief This item is used to display signal values

		This item is used to display signal values.

		Information displayed by this item is fully customizable by scripts. Script code can receive signal parameters and states from data services,
		set text, colors, font size to any values depending on customers requirements.

		Signal identifiers set to the schema item are stored in <b>SignalIDs</b> array property.

		To modify contents of the item, set <b>Text</b>, <b>TextColor</b>, <b>FillColor</b>, <b>LineColor</b> properties etc.

		<b>Event handlers</b>

		To customize item's appearance and behavior, event handler code is placed to following properties of the schema item using RPCT:

		- <b>ClickScript</b> contains mouse click event handler code.
		Click event is generated each time when user clicks mouse button on the item and <b>AcceptClick</b> property is set to true;<br>
		-  <b>PreDrawScript</b> contains pre-draw event handler code. Pre-draw event is generated each time before item is redrawn.

		<b>ClickScript</b> and <b>PreDrawScript</b> event handler function protypes:

		\code
		function(schemaItem)
		\endcode

		Parameters:<br>
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemDiagValue.<br>

		<b>PreDrawScript example:</b>

		\code
		(function(schemaItemDiagValue)
		{
			// Check for signals number
			//
			if (schemaItemDiagValue.SignalIDs.length != 1)
			{
				schemaItemDiagValue.Text = "No Signals!";
				return;
			}

			// Take first signal identifier
			//
			let appSignalId = schemaItemDiagValue.SignalIDs[0];

			// Get data from TuningService
			//
			let signalParam = tuning.signalParam(appSignalId);
			let signalState = tuning.signalState(appSignalId);

			if (signalState == undefined)
			{
				// Signal was not found
				//
				schemaItemDiagValue.Text = appSignalId;
			}
			else
			{
				// Get signal state
				//
				if (signalState.Valid == true)
				{
					// Signal state is valid
					//
					schemaItemDiagValue.Text = signalState.Value;
					schemaItemDiagValue.TextColor = "black";
					schemaItemDiagValue.FillColor = "white";
					schemaItemDiagValue.LineColor = "#000000";
				}
				else
				{
					// Signal state is not valid
					//
					schemaItemDiagValue.Text = "?";
					schemaItemDiagValue.TextColor = schemaItemDiagValue.BlinkPhase ? "white" : "black";
					schemaItemDiagValue.FillColor = schemaItemDiagValue.BlinkPhase ? "black" : "#A00000";
					schemaItemDiagValue.LineColor = "#A00000";
				}
			}
		})
		\endcode
	*/
	class SchemaItemDiagValue final : public PosRectRotatable,
									  public IMatsSchemaItemAssociations
	{
		Q_OBJECT

		/// \brief Diagnostics signal identifiers array. Use <b>diagSignalIDs.length</b> to get number of identifiers.
		Q_PROPERTY(QStringList diagSignalIDs READ diagSignalIds WRITE setDiagSignalIds)
		Q_PROPERTY(QStringList DiagSignalIDs READ diagSignalIds WRITE setDiagSignalIds)

		// Appearance
		//

		/// \brief Border line weight, in pixels
		Q_PROPERTY(double lineWeight READ lineWeight WRITE setLineWeight)
		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		/// \brief Border line color name
		Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		/// \brief Rectangle fill color name
		Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

		/// \brief Text color name
		Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
		Q_PROPERTY(QColor TextColor READ textColor WRITE setTextColor)

		/// \brief Bounding rectangle drawing
		Q_PROPERTY(bool drawRect READ drawRect WRITE setDrawRect)
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		// Text Category Properties
		//

		/// \brief Horizontal text alignment
		Q_PROPERTY(E::HorzAlign alignHorz READ horzAlign WRITE setHorzAlign)
		Q_PROPERTY(E::HorzAlign AlignHorz READ horzAlign WRITE setHorzAlign)

		/// \brief Vertical text alignment
		Q_PROPERTY(E::VertAlign alignVert READ vertAlign WRITE setVertAlign)
		Q_PROPERTY(E::VertAlign AlignVert READ vertAlign WRITE setVertAlign)

		/// \brief Font name
		Q_PROPERTY(QString fontName READ getFontName WRITE setFontName)
		Q_PROPERTY(QString FontName READ getFontName WRITE setFontName)

		/// \brief Font size
		Q_PROPERTY(double fontSize READ getFontSize WRITE setFontSize)
		Q_PROPERTY(double FontSize READ getFontSize WRITE setFontSize)

		/// \brief Font bold
		Q_PROPERTY(bool fontBold READ getFontBold WRITE setFontBold)
		Q_PROPERTY(bool FontBold READ getFontBold WRITE setFontBold)

		/// \brief Font italic
		Q_PROPERTY(bool fontItalic READ getFontItalic WRITE setFontItalic)
		Q_PROPERTY(bool FontItalic READ getFontItalic WRITE setFontItalic)

		/// \brief Text
		Q_PROPERTY(QString text READ text WRITE setText)
		Q_PROPERTY(QString Text READ text WRITE setText)

		/// \brief Precision
		Q_PROPERTY(int precision READ precision WRITE setPrecision)
		Q_PROPERTY(int Precision READ precision WRITE setPrecision)

		/// \brief Angle of rotation
		Q_PROPERTY(double angle READ angle WRITE setAngle)
		Q_PROPERTY(double Angle READ angle WRITE setAngle)

		/**
		* @brief Rotation point of the item.
		*
		* This property represents the rotation point of the item. The rotation point is the
		* point around which the item is rotated when the rotation transformation (property Angle) is applied.
		* Setting this property allows to specify a custom rotation point for the item.
		*
		* Possible values for this property are:
		* - RotationPoint::TopLeft (0)
		* - RotationPoint::TopRight (1)
		* - RotationPoint::BottomRight (2)
		* - RotationPoint::BottomLeft (3)
		* - RotationPoint::Center (4)
		*/
		Q_PROPERTY(RotationPoint rotationPoint READ rotationPoint WRITE setRotationPoint)
		Q_PROPERTY(RotationPoint RotationPoint READ rotationPoint WRITE setRotationPoint)

	public:
		SchemaItemDiagValue(void);
		explicit SchemaItemDiagValue(SchemaUnit unit);
		virtual ~SchemaItemDiagValue(void) = default;

	protected:
		virtual void propertyDemand(const QString& prop) override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;
		void drawPrivate(CDrawParam* drawParam) const;

		virtual void drawHighlight(CDrawParam* drawParam) const override;
		void drawHighlightPrivate(CDrawParam* drawParam) const;

	protected:
		void drawText(CDrawParam* drawParam, const Context* context, const QRectF& rect) const;

		QString parseText(QString text, const Context* context, const Session& session, const HardwareLib::DiagSignal& signal/*, const AppSignalState& signalState*/) const;
		QString formatNumber(double value, const HardwareLib::DiagSignal& signal) const;

		bool getSignalState(QString diagSignalObjectId, const Context* context, HardwareLib::DiagSignal* signalParam/*, AppSignalState* appSignalState*/) const;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Java Script invocable specific for SchemaItemDiagValue
		//
	public:

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override;
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		QString diagSignalIdsString() const;
		QString diagSignalIdsString(const Context* context) const;
		void setDiagSignalIdsString(const QString& value);

		QStringList diagSignalIds() const;
		QStringList diagSignalIds(const Context* context) const;
		void setDiagSignalIds(const QStringList& value);

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
		QStringList m_diagSignalIds = {"USB3_SDSA_CHASSIS01_MD00_SIGNALID"};

		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgb(0x00, 0x00, 0x00)};
		QColor m_fillColor = {qRgb(0x00, 0x64, 0x00)};
		QColor m_textColor = {qRgb(0xF0, 0xF0, 0xF0)};

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_drawRect = false;       // Rect is visible, thickness 0 is possible

		QString m_text = {"$(value)"}; // $(value)			: signal value
									   // $(caption)		: caption
									   // $(signalid)		: SignalID (CustomSignalID)
									   // $(appsignalid)	: AppSignalID (#APPSIGANLID)
									   // $(equipmentid)	: Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)
									   // $(highlimit)		: High limit
									   // $(lowlimit)		: Low limit
									   // $(units)			: Signal units

		int m_precision = -1;          // decimal places, -1 means take value from Signal
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
	};
} // namespace VFrame30
