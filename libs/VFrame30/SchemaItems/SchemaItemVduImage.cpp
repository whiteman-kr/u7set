#include <VFrame30/DrawParam.h>
#include <VFrame30/ImageItem.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduImage.h>


namespace VFrame30
{
	SchemaItemVduImage::SchemaItemVduImage(void) :
		SchemaItemVduImage(SchemaUnit::Display)
	{
		// This constructor can be called only from SchemaItemVduImage(SchemaUnit unit) constructor.
		// After this call all properties are initialized and can be used.
		//
	}

	SchemaItemVduImage::SchemaItemVduImage(SchemaUnit units) :
		m_image{std::make_unique<ImageItem>()}
	{
		assert(units == SchemaUnit::Display);

		Property* p;

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::allowScale,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduImage::allowScale,
								 SchemaItemVduImage::setAllowScale);
		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::keepAspectRatio,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduImage::keepAspectRatio,
								 SchemaItemVduImage::setKeepAspectRatio);

		p = ADD_PROPERTY_GET_SET_CAT(QImage,
									 PropertyNames::image,
									 PropertyNames::imageCategory,
									 true,
									 SchemaItemVduImage::image,
									 SchemaItemVduImage::setImage);
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.gif);; All Files (*.*)"));

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::svg,
									 PropertyNames::imageCategory,
									 true,
									 SchemaItemVduImage::svgData,
									 SchemaItemVduImage::setSvgData);
		p->setSpecificEditor(E::PropertySpecificEditor::Svg);

		// --
		//
		m_static = true;
		setItemUnit(units);

		return;
	}

	SchemaItemVduImage::~SchemaItemVduImage() = default;

	// Serialization
	//
	bool SchemaItemVduImage::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemVduImage* imageMessage = message->MutableExtension(Proto::schemaitem)->mutable_vduimage();

		bool ok = m_image->save(imageMessage->mutable_image());
		return ok;
	}

	bool SchemaItemVduImage::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
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
		if (message.GetExtension(Proto::schemaitem).has_vduimage() == false)
		{
			Q_ASSERT(message.GetExtension(Proto::schemaitem).has_vduimage());
			return false;
		}

		const Proto::SchemaItemVduImage& imageMessage = message.GetExtension(Proto::schemaitem).vduimage();

		bool ok = m_image->load(imageMessage.image());

		return ok;
	}

	// Drawing Functions
	//
	void SchemaItemVduImage::draw(CDrawParam* drawParam) const
	{
		QRectF rect = boundingRectInDocPt(drawParam);

		if (m_image->hasAnyImage() == false)
		{
			// Image not set, draw rect and information text
			//
			m_image->drawError(drawParam, rect, QStringLiteral("No Image"));
			return;
		}

		// Draw Image
		//
		m_image->drawImage(drawParam, rect);
		return;
	}


	double SchemaItemVduImage::minimumPossibleHeightDocPt(double gridSize, [[maybe_unused]] int pinGridStep) const
	{
		return gridSize;
	}

	double SchemaItemVduImage::minimumPossibleWidthDocPt(double gridSize, [[maybe_unused]] int pinGridStep) const
	{
		return gridSize;
	}

	bool SchemaItemVduImage::accept(VduItemVisitor& visitor) const
	{
		return visitor.visit(*this);
	}

	// Properties and Data
	//

	// AllowScale
	//
	bool SchemaItemVduImage::allowScale() const
	{
		return m_image->allowScale();
	}

	void SchemaItemVduImage::setAllowScale(bool value)
	{
		m_image->setAllowScale(value);
	}

	// KeepAspectRatio
	//
	bool SchemaItemVduImage::keepAspectRatio() const
	{
		return m_image->keepAspectRatio();
	}

	void SchemaItemVduImage::setKeepAspectRatio(bool value)
	{
		m_image->setKeepAspectRatio(value);
	}

	// Image
	//
	const QImage& SchemaItemVduImage::image() const
	{
		return m_image->image();
	}

	void SchemaItemVduImage::setImage(const QImage& image)
	{
		m_image->setImage(image);
	}

	// Svg
	//
	const QString& SchemaItemVduImage::svgData() const
	{
		return m_image->svgData();
	}

	void SchemaItemVduImage::setSvgData(const QString& data)
	{
		m_image->setSvgData(data);
	}

	QImage SchemaItemVduImage::toQImage(const QRectF& rect) const
	{
		return m_image->toQImage(rect);
	}
} // namespace VFrame30
