#pragma once

#include "PosRectImpl.h"
#include "ImageItem.h"

namespace VFrame30
{
	/*! \class SchemaItemImage
		\ingroup staticSchemaItems
		\brief This item is used to display static images

		This item is used to display static images.
	*/

	class VFRAME30LIBSHARED_EXPORT SchemaItemImage : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Allow image scaling
		Q_PROPERTY(bool AllowScale READ allowScale WRITE setAllowScale)

		/// \brief Keep aspect ratio
		Q_PROPERTY(bool KeepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)

		/*! \brief Image displayed by the item

		This property specifies bitmap image displayed by the schema item. Image is loaded from external file and is stored in schema.
		The image is displayed only if <b>Svg</b> property is empty.
		*/
		Q_PROPERTY(QImage Image READ image WRITE setImage)

		/*! \brief SVG data for image

		Image can be described by SVG (Scalable Vector Graphic) code. If this property is empty, SchemaItemImage displays
		image specified by <b>Image</b> property, otherwise displays image specified by <b>Svg</b> property.

		// Example:
		\code
		<svg>
		<line x1="0" y1="0" x2="200" y2="200" stroke-width="1" stroke="rgb(0,0,0)"/>
		</svg>
		\endcode
		*/
		Q_PROPERTY(QString Svg READ svgData WRITE setSvgData)

	public:
		SchemaItemImage(void);
		explicit SchemaItemImage(SchemaUnit unit);
		virtual ~SchemaItemImage(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Properties and Data
		//
	public:
		bool allowScale() const;		// Applied only to raster images
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
}
