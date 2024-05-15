#pragma once

#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/ImageItem.h>
#include <VFrame30/PosRectRotatable.h>

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

		Signal identifiers set to the schema item are stored in <b>SignalIDs</b> array property.

		To set an image to display, set <b>CurrentImageID</b> property to required item identifier depending on signal states.
		Usually it is done by <b>PreDrawScript</b> event handler code.

		<b>Event handlers</b>

		To customize item's appearance and behaviour, event handler code is placed to following properties of the schema item using RPCT:

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
	class SchemaItemImageValue : public PosRectRotatable, public IMatsSchemaItemAssociations
	{
		Q_OBJECT

		/// \brief Application signal identifiers array. <b>Use appSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList signalIDs READ signalIds WRITE setSignalIds)
		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)

		/// \brief Application signal identifiers array. Use <b>appSignalIDs.length</b> to get number of identifiers
		Q_PROPERTY(QStringList appSignalIDs READ signalIds WRITE setSignalIds)
		Q_PROPERTY(QStringList AppSignalIDs READ signalIds WRITE setSignalIds)

		/// \brief An identifier of current image
		Q_PROPERTY(QString currentImageID READ currentImageId WRITE setCurrentImageId)
		Q_PROPERTY(QString CurrentImageID READ currentImageId WRITE setCurrentImageId)

		/// \brief Border line weight, in pixels
		Q_PROPERTY(double lineWeight READ lineWeight WRITE setLineWeight)
		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		/// \brief Bounding rectangle drawing
		Q_PROPERTY(bool drawRect READ drawRect WRITE setDrawRect)
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		/// \brief Rectangle filling
		Q_PROPERTY(bool fill READ fillRect WRITE setFillRect)
		Q_PROPERTY(bool Fill READ fillRect WRITE setFillRect)

		/// \brief Border line color name
		Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		/// \brief Rectangle fill color name
		Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

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
		SchemaItemImageValue(void);
		explicit SchemaItemImageValue(SchemaUnit unit);
		virtual ~SchemaItemImageValue(void);

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
		void initDrawingResources() const;
		// bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		void drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect);

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Java Script invocable specific for SchemaItemImageValue
		//
	public slots:
		/// \brief Returns \ref VFrame30::ScriptImageItem "ScriptImageItem" specified by <b>imageId</b>.
		///
		QObject* imageItem(QString imageId);

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override { return {}; };
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		QString signalIdsString(const Context* context) const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		QStringList signalIds(const Context* context) const;
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

		PropertyVector<ImageItem> m_images; // Each image is a std::shared_ptr

		// MonitorMode variables
		//
		QString m_currentImageId;

		// --
		//
		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgba(0x00, 0x00, 0x00, 0xFF)};
		QColor m_fillColor = {qRgba(0x00, 0x00, 0xC0, 0xFF)};

		bool m_drawRect = false; // Rect is visible, thickness 0 is possible
		bool m_fillRect = false;

		// Drawing resources
		//
		mutable std::unique_ptr<QPen> m_rectPen;
		mutable std::unique_ptr<QBrush> m_fillBrush;
	};
} // namespace VFrame30
