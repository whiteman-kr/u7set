#pragma once

#include "PosRectImpl.h"
#include "ImageItem.h"

class AppSignalState;
class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{

	/*! \class SchemaItemImageValue
		\ingroup dynamicSchemaItems
		\brief This item is used to display different images depending on signal values

		This item is used to display different images depending on signal values.

		Item contains an array of images specified by <b>Images</b> property (not accessible by scripts). Each image can be specified by bitmap or by <b>Svg</b> code.
		Each array item has a string identifier.

		An example of Svg code can be found in \ref VFrame30::SchemaItemImage "SchemaItemImage" description.

		Signal identidiers set to the schema item are stored in <b>SignalIDs</b> array property.

		To set an image to display, set <b>CurrentImageID</b> property to required item identifier depending on signal states.
		Usually it is done by <b>PreDrawScript</b> event handler code.

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
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemImageValue.<br>

		<b>Example:</b>

		Assume schema item displays a limit switch of a control rod. Its state is described by two application signals:
		"#ROD_SWITCH_ACTIVE" and "#ROD_SWITCH_UP". Image should be displayed by following logic:

		<table>
		<caption id="multi_row">Rod Limit Switch Item Logic</caption>
		<tr><th>ROD_SWITCH_ACTIVE  <th>ROD_SWITCH_UP  <th>Image ID		      <th>Image
		<tr><td>TRUE				<td>TRUE			<td>UP_ACTIVE		<td><img src="suz_up_active.bmp" align="left"/>
		<tr><td>TRUE				<td>FALSE			<td>DOWN_ACTIVE		<td><img src="suz_down_active.bmp" align="left"/>
		<tr><td>FALSE				<td>TRUE			<td>UP_NOACTIVE		<td><img src="suz_up_noactive.bmp" align="left"/>
		<tr><td>FALSE				<td>FALSE			<td>DOWN_NOACTIVE	<td><img src="suz_down_noactive.bmp" align="left"/>
		<tr><td>NON-VALID			<td>ANY				<td>NONVALID		<td><img src="suz_nonvalid.bmp" align="left"/>
		<tr><td>ANY					<td>NON-VALID		<td>NONVALID		<td><img src="suz_nonvalid.bmp" align="left"/>
		</table>

		Assume bitmaps have been added to <b>Images</b> property with identifiers shown in the table above and
		two application signal identifiers "#ROD_SWITCH_ACTIVE" and "#ROD_SWITCH_UP" are added to <b>AppSignalIDs</b> property of SchemaItemImageValue.

		First, define a function in <b>GlobalScript</b> property of Monitor that implements the logic:

		\code
		function RodSwitchLogic(schemaItem, activeSignalId, upSignalId)
		{
			// Get signals states
			//
			let activeSignalState = signals.signalState(activeSignalId);
			let upSignalState = signals.signalState(upSignalId);

			// Check for validity
			//
			if (activeSignalState == undefined || upSignalState == undefined ||
				activeSignalState.Valid == false || upSignalState.Valid == false)
				{
					schemaItem.CurrentImageID = "NONVALID";
					return;
				}

			// Choose the required image
			//

			if (activeSignalState.Value == 0)
			{
				if (upSignalState.Value == 0)
				{
					schemaItem.CurrentImageID = "DOWN_NOACTIVE";
				}
				else
				{
					schemaItem.CurrentImageID = "UP_NOACTIVE";
				}
			}
			else
			{
				if (upSignalState.Value == 0)
				{
					schemaItem.CurrentImageID = "DOWN_ACTIVE";
				}
				else
				{
					schemaItem.CurrentImageID = "UP_ACTIVE";
				}
			}

			return;
		}
		\endcode

		And the second, add <b>PreDrawScript</b> event handler:

		\code
		(function(schemaItem)
		{
			// Check if two signal identifiers are added to property AppSignalIDs
			//
			if (schemaItem.AppSignalIDs.length != 2)
			{
				return;
			}

			// Get identifiers and call the function from global script
			//

			let activeSignalId = schemaItem.AppSignalIDs[0];
			let upSignalId = schemaItem.AppSignalIDs[1];

			RodSwitchLogic(schemaItem, activeSignalId, upSignalId);

			return;
		})
		\endcode
	*/
	class SchemaItemImageValue : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Application signal identifiers array. <b>Use AppSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)

		/// \brief Application signal identifiers array. Use <b>AppSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList AppSignalIDs READ signalIds WRITE setSignalIds)

		/// \brief An identifier of current image
		Q_PROPERTY(QString CurrentImageID READ currentImageId WRITE setCurrentImageId)

		/// \brief Border line weight, in pixels
		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		/// \brief Bounding rectangle drawing
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		/// \brief Rectangle filling
		Q_PROPERTY(bool Fill READ fillRect WRITE setFillRect)

		/// \brief Border line color name
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		/// \brief Rectangle fill color name
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

	public:
		SchemaItemImageValue(void);
		explicit SchemaItemImageValue(SchemaUnit unit);
		virtual ~SchemaItemImageValue(void);

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
		//bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		void drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect);

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Java Script invocables specific for SchemaItemImageValue
		//
	public slots:

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		const PropertyVector<ImageItem>& images() const;
		void setImages(const PropertyVector<ImageItem>& value);

		QString currentImageId() const;
		void setCurrentImageId(QString value);

		double lineWeight() const;
		void setLineWeight(double lineWeight);

		const QColor& lineColor() const;
		void setLineColor(const QColor& color);

		const QColor& fillColor() const;
		void setFillColor(const QColor& color);

		bool drawRect() const;
		void setDrawRect(bool value);

		bool fillRect() const;
		void setFillRect(bool value);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		PropertyVector<ImageItem> m_images;	// Each image is a std::shared_ptr

		// MonitorMode variables
		//
		QString m_currentImageId;

		// --
		//
		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgba(0x00, 0x00, 0x00, 0xFF)};
		QColor m_fillColor = {qRgba(0x00, 0x00, 0xC0, 0xFF)};

		bool m_drawRect = false;							// Rect is visible, thikness 0 is possible
		bool m_fillRect = false;

		// Drawing resources
		//
		mutable std::unique_ptr<QPen> m_rectPen;
		mutable std::unique_ptr<QBrush> m_fillBrush;
	};
}


