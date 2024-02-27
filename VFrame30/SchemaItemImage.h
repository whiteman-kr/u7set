#pragma once

#include "ImageItem.h"
#include "PosRectRotatable.h"

namespace VFrame30
{
	/*! \class SchemaItemImage
		\ingroup staticSchemaItems
		\brief This item is used to display static images

		This item is used to display static images.

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
		<i>schemaItem</i> - a handle to schema item, type: SchemaItemImage.<br>

		<b>PreDrawScript example:</b>
		\code
		(function(schemaItem)
		{
			schemaItem.Svg = "<line x1="0" y1="0" x2="200" y2="200" stroke-width="1" stroke="rgb(0,0,0)"/>";
		})
		\endcode
	*/

	class SchemaItemImage final : public PosRectRotatable
	{
		Q_OBJECT

		/// \brief Allow image scaling
		Q_PROPERTY(bool allowScale READ allowScale WRITE setAllowScale)
		Q_PROPERTY(bool AllowScale READ allowScale WRITE setAllowScale)

		/// \brief Keep aspect ratio
		Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
		Q_PROPERTY(bool KeepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)

		/*! \brief Image displayed by the item

		This property specifies bitmap image displayed by the schema item. Image is loaded from external file and is stored in schema.
		The image is displayed only if <b>svg</b> property is empty.
		*/
		Q_PROPERTY(QImage image READ image WRITE setImage)
		Q_PROPERTY(QImage Image READ image WRITE setImage)

		/*! \brief SVG data for image

		Image can be described by SVG (Scalable Vector Graphic) code. If this property is empty, SchemaItemImage displays
		image specified by <b>image</b> property, otherwise displays image specified by <b>Svg</b> property.

		// Example:
		\code
		<svg>
		<line x1="0" y1="0" x2="200" y2="200" stroke-width="1" stroke="rgb(0,0,0)"/>
		</svg>
		\endcode
		*/
		Q_PROPERTY(QString svg READ svgData WRITE setSvgData)
		Q_PROPERTY(QString Svg READ svgData WRITE setSvgData)

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
		SchemaItemImage(void);
		explicit SchemaItemImage(SchemaUnit unit);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		// Drawings are performed in the coordinate system of the document. In 100% zoom.
		// Graphics must have a screen coordinate system (0, 0 - top left corner, down and right - positive coordinates).
		//
		virtual void draw(CDrawParam* drawParam) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Properties and Data
		//
	public:
		bool allowScale() const; // Applied only to raster images
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		const QImage& image() const;
		void setImage(const QImage& image);

		const QString& svgData() const;
		void setSvgData(const QString& data);

	private:
		ImageItem m_image;
	};
} // namespace VFrame30
