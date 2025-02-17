#include <VFrame30/DrawParam.h>
#include <VFrame30/ImageItem.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduImageValue.h>


namespace VFrame30
{
	//
	//	SchemaItemImageValue
	//
	SchemaItemVduImageValue::SchemaItemVduImageValue(void) :
		SchemaItemVduImageValue(SchemaUnit::Inch)
	{
		// This constructor can called while serialization
		//
	}

	SchemaItemVduImageValue::SchemaItemVduImageValue(SchemaUnit unit)
	{
		// Functional
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::appSignalIDs,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemVduImageValue::signalIdsString,
								 SchemaItemVduImageValue::setSignalIdsString)
			->setValidator(PropertyNames::appSignalIDsOrReferenceValidator)
			.setEssential(true);

		ADD_PROPERTY_GET_SET_CAT(PropertyVector<ImageItem>,
								 PropertyNames::images,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemVduImageValue::images,
								 SchemaItemVduImageValue::setImages)
			->setEssential(true);

		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::fillColor,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduImageValue::fillColor,
								 SchemaItemVduImageValue::setFillColor);

		// --
		//
		m_static = false;
		setItemUnit(unit);

		return;
	}

	SchemaItemVduImageValue::~SchemaItemVduImageValue(void) = default;

	// Serialization
	//
	bool SchemaItemVduImageValue::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemVduImageValue* valueMessage = message->MutableExtension(Proto::schemaitem)->mutable_vduimagevalue();

		valueMessage->set_signalids(signalIdsString(nullptr).toStdString()); // nullptr avoid macro expansion

		for (const auto& image : m_images)
		{
			image->save(valueMessage->add_images());
		}

		valueMessage->set_fillcolor(m_fillColor.rgba());

		return true;
	}

	bool SchemaItemVduImageValue::LoadData(const Proto::Envelope& message)
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
		if (message.GetExtension(Proto::schemaitem).has_vduimagevalue() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_vduimagevalue());
			return false;
		}

		const Proto::SchemaItemVduImageValue& valueMessage = message.GetExtension(Proto::schemaitem).vduimagevalue();

		setSignalIdsString(valueMessage.signalids().data());

		m_images.clear();
		bool loadOk = true;
		for (int i = 0; i < valueMessage.images_size(); i++)
		{
			auto& image = m_images.emplace_back(std::make_shared<ImageItem>());
			loadOk &= image->load(valueMessage.images(i));
		}

		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());

		return loadOk;
	}

	// Drawing Functions
	//
	void SchemaItemVduImageValue::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();

		// Calculate rectangle
		//
		QRectF rect = boundingRectInDocPt(drawParam);

		// Drawing background
		//
		painter->fillRect(rect, m_fillColor);

		// Draw Image
		//
		if (m_images.empty() == true)
		{
			ImageItem::drawError(drawParam, rect, QString("Image is not assigned."));
		}
		else
		{
			std::shared_ptr<ImageItem> image = m_images.front();

			if (image->hasAnyImage() == false)
			{
				ImageItem::drawError(drawParam, rect, QString("No image assigned to ID %1.").arg(image->imageId()));
			}
			else
			{
				image->drawImage(drawParam, rect);
			}
		}

		return;
	}

	void SchemaItemVduImageValue::drawHighlight(CDrawParam* drawParam) const
	{
		bool highlight = drawParam->highlightIds().contains(label());

		// Draw highlights by signals
		//
		for (const QString& appSignalId : m_signalIds)
		{
			if (drawParam->highlightIds().contains(appSignalId) == true)
			{
				highlight = true;
				break;
			}
		}

		if (highlight == true)
		{
			QRectF highlightRect = boundingRectInDocPt(drawParam);
			drawHighlightRect(drawParam, highlightRect);
		}

		return;
	}

	void SchemaItemVduImageValue::drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect)
	{
		for (const std::shared_ptr<ImageItem>& image : m_images)
		{
			if (image->imageId() == imageId)
			{
				if (image->hasAnyImage() == false)
				{
					ImageItem::drawError(drawParam, rect, QString("ImageID %1 has no image.").arg(imageId));
				}
				else
				{
					image->drawImage(drawParam, rect);
				}

				return;
			}
		}

		ImageItem::drawError(drawParam, rect, QString("ImageID %1 not found.").arg(imageId));
		return;
	}

	double SchemaItemVduImageValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemVduImageValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemVduImageValue::associatedDiagObjectIds() const
	{
		return {};
	};

	QStringList SchemaItemVduImageValue::associatedAppSignalIds() const
	{
		return signalIds();
	}

	QStringList SchemaItemVduImageValue::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemVduImageValue::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemVduImageValue::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemVduImageValue::associatedSchemaItemLabels() const
	{
		return {};
	}

	QString SchemaItemVduImageValue::signalIdsString() const
	{
		std::shared_ptr<Context> context = this->context();
		return signalIdsString(context.get());
	}

	QString SchemaItemVduImageValue::signalIdsString(const Context* /*context*/) const
	{
		QStringList resultList = m_signalIds;
		return resultList.join(QChar::LineFeed);
	}

	void SchemaItemVduImageValue::setSignalIdsString(const QString& value)
	{
		m_signalIds = value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	}

	QStringList SchemaItemVduImageValue::signalIds() const
	{
		std::shared_ptr<Context> context = this->context();
		return signalIds(context.get());
	}

	QStringList SchemaItemVduImageValue::signalIds(const Context* /*context*/) const
	{
		QStringList resultList = m_signalIds;
		return resultList;
	}

	void SchemaItemVduImageValue::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;
	}

	// Images
	//
	const PropertyVector<ImageItem>& SchemaItemVduImageValue::images() const
	{
		return m_images;
	}

	void SchemaItemVduImageValue::setImages(const PropertyVector<ImageItem>& value)
	{
		m_images = value;

		// Limit m_images.size() to 12
		//
		while (m_images.size() > 12)
		{
			m_images.pop_back();
		}
	}

	// FillColor property
	//
	const QColor& SchemaItemVduImageValue::fillColor() const
	{
		return m_fillColor;
	}

	void SchemaItemVduImageValue::setFillColor(const QColor& color)
	{
		m_fillColor = color;
	}
} // namespace VFrame30
