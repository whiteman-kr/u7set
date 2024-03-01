#include "SchemaItemImage.h"
#include "DrawParam.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "QPainter"


namespace VFrame30
{
	SchemaItemImage::SchemaItemImage(void) :
		SchemaItemImage(SchemaUnit::Inch)
	{
		// This constructor can be called only from SchemaItemImage(SchemaUnit unit) constructor.
		// After this call all properties are initialized and can be used.
		//
	}

	SchemaItemImage::SchemaItemImage(SchemaUnit unit)
	{
		Property* p;

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::allowScale, PropertyNames::appearanceCategory, true, SchemaItemImage::allowScale, SchemaItemImage::setAllowScale);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::keepAspectRatio, PropertyNames::appearanceCategory, true, SchemaItemImage::keepAspectRatio, SchemaItemImage::setKeepAspectRatio);

		p = ADD_PROPERTY_GET_SET_CAT(QImage, PropertyNames::image, PropertyNames::imageCategory, true, SchemaItemImage::image, SchemaItemImage::setImage);
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.gif);; All Files (*.*)"));

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::svg, PropertyNames::imageCategory, true, SchemaItemImage::svgData, SchemaItemImage::setSvgData);
		p->setSpecificEditor(E::PropertySpecificEditor::Svg);

		// --
		//
		m_static = true;
		setItemUnit(unit);

		return;
	}

	// Serialization
	//
	bool SchemaItemImage::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectRotatable::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemImage* imageMessage = message->mutable_schemaitem()->mutable_image();

		bool ok = m_image.save(imageMessage->mutable_image());
		return ok;
	}

	bool SchemaItemImage::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosRectRotatable::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_image() == false)
		{
			Q_ASSERT(message.schemaitem().has_image());
			return false;
		}

		const Proto::SchemaItemImage& imageMessage = message.schemaitem().image();

		bool ok = m_image.load(imageMessage.image());

		return ok;
	}

	// Drawing Functions
	//
	void SchemaItemImage::draw(CDrawParam* drawParam) const
	{
		auto drawPrivate = [this](CDrawParam* drawParam)
		{
			QRectF rect = boundingRectInDocPt(drawParam);

			if (m_image.hasAnyImage() == false)
			{
				// Image not set, draw rect and information text
				//
				m_image.drawError(drawParam, rect, QStringLiteral("No Image"));
				return;
			}

			// Draw Image
			//
			m_image.drawImage(drawParam, rect);
			return;
		};

		return drawRotated(drawParam, [drawParam, this, &drawPrivate]()
						   {
							   return drawPrivate(drawParam);
						   });

		return;
	}


	double SchemaItemImage::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemImage::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// Properties and Data
	//

	// AllowScale
	//
	bool SchemaItemImage::allowScale() const
	{
		return m_image.allowScale();
	}

	void SchemaItemImage::setAllowScale(bool value)
	{
		m_image.setAllowScale(value);
	}

	// KeepAspectRatio
	//
	bool SchemaItemImage::keepAspectRatio() const
	{
		return m_image.keepAspectRatio();
	}

	void SchemaItemImage::setKeepAspectRatio(bool value)
	{
		m_image.setKeepAspectRatio(value);
	}

	// Image
	//
	const QImage& SchemaItemImage::image() const
	{
		return m_image.image();
	}

	void SchemaItemImage::setImage(const QImage& image)
	{
		m_image.setImage(image);
	}

	// Svg
	//
	const QString& SchemaItemImage::svgData() const
	{
		return m_image.svgData();
	}

	void SchemaItemImage::setSvgData(const QString& data)
	{
		m_image.setSvgData(data);
	}
} // namespace VFrame30
