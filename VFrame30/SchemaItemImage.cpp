#include "SchemaItemImage.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "QPainter"


namespace VFrame30
{
	SchemaItemImage::SchemaItemImage(void) :
		SchemaItemImage(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
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
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Svg Files (*.svg);; All Files (*.*)"));

		// --
		//
		m_static = true;
		setItemUnit(unit);

		return;
	}

	SchemaItemImage::~SchemaItemImage(void)
	{
	}

	// Serialization
	//
	bool SchemaItemImage::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
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
		bool result = PosRectImpl::LoadData(message);
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
	void SchemaItemImage::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
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
}

