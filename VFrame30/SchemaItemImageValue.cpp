#include "SchemaItemImageValue.h"
#include "../AppSignalLib/TuningSignalState.h"
#include "AppSignalController.h"
#include "DrawParam.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "Schema.h"
#include "SchemaView.h"
#include "TuningController.h"


namespace VFrame30
{
	//
	//	SchemaItemImageValue
	//
	SchemaItemImageValue::SchemaItemImageValue(void) :
		SchemaItemImageValue(SchemaUnit::Inch)
	{
		// This constructor can called while serialization
		//
	}

	SchemaItemImageValue::SchemaItemImageValue(SchemaUnit unit)
	{
		// Functional
		//
		auto strIdProperty = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemImageValue::signalIdsString, SchemaItemImageValue::setSignalIdsString);
		strIdProperty->setValidator(PropertyNames::appSignalIDsOrReferenceValidator);

		// ADD_PROPERTY_GET_SET_CAT(E::SignalSource, PropertyNames::signalSource, PropertyNames::functionalCategory, true, SchemaItemImageValue::signalSource, SchemaItemImageValue::setSignalSource);

		ADD_PROPERTY_GET_SET_CAT(PropertyVector<ImageItem>, PropertyNames::images, PropertyNames::functionalCategory, true, SchemaItemImageValue::images, SchemaItemImageValue::setImages);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::currentImageId, PropertyNames::functionalCategory, true, SchemaItemImageValue::currentImageId, SchemaItemImageValue::setCurrentImageId);

		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemImageValue::lineWeight, SchemaItemImageValue::setLineWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemImageValue::lineColor, SchemaItemImageValue::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemImageValue::fillColor, SchemaItemImageValue::setFillColor);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemImageValue::drawRect, SchemaItemImageValue::setDrawRect);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fill, PropertyNames::appearanceCategory, true, SchemaItemImageValue::fillRect, SchemaItemImageValue::setFillRect);

		// --
		//
		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemImageValue::~SchemaItemImageValue(void)
	{
	}

	// Serialization
	//
	bool SchemaItemImageValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectRotatable::SaveData(message);
		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemImageValue* valueMessage = message->mutable_schemaitem()->mutable_imagevalue();

		valueMessage->set_signalids(signalIdsString(nullptr).toStdString()); // nullptr avoid macro expansion
		valueMessage->set_signalsource(static_cast<int32_t>(m_signalSource));

		for (const auto& image : m_images)
		{
			image->save(valueMessage->add_images());
		}

		valueMessage->set_lineweight(m_lineWeight);
		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_drawrect(m_drawRect);
		valueMessage->set_fillrect(m_fillRect);
		valueMessage->set_currentimageid(m_currentImageId.toStdString());

		return true;
	}

	bool SchemaItemImageValue::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_imagevalue() == false)
		{
			assert(message.schemaitem().has_imagevalue());
			return false;
		}

		const Proto::SchemaItemImageValue& valueMessage = message.schemaitem().imagevalue();

		setSignalIdsString(valueMessage.signalids().data());
		m_signalSource = static_cast<E::SignalSource>(valueMessage.signalsource());

		m_images.clear();
		bool loadOk = true;
		for (int i = 0; i < valueMessage.images_size(); i++)
		{
			auto& image = m_images.emplace_back(std::make_shared<ImageItem>());
			loadOk &= image->load(valueMessage.images(i));
		}

		m_lineWeight = valueMessage.lineweight();
		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_drawRect = valueMessage.drawrect();
		m_fillRect = valueMessage.fillrect();
		m_currentImageId = valueMessage.currentimageid().data();

		return loadOk;
	}

	// Drawing Functions
	//
	void SchemaItemImageValue::draw(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return drawPrivate(drawParam);
						   });
	}

	void SchemaItemImageValue::drawPrivate(CDrawParam* drawParam) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
		initDrawingResources();

		// Calculate rectangle
		//
		QRectF rect = boundingRectInDocPt(drawParam);

		// Drawing background
		//
		if (m_fillRect == true)
		{
			m_fillBrush->setColor(m_fillColor);
			drawParam->painter()->fillRect(rect, *m_fillBrush);
		}

		// Draw Image
		//
		std::shared_ptr<ImageItem> currentImage;

		for (const auto& i : m_images)
		{
			if (i->imageId() == currentImageId())
			{
				currentImage = i;
				break;
			}
		}

		if (currentImage == nullptr)
		{
			ImageItem::drawError(drawParam, rect, tr("Image %1 not found.").arg(currentImageId()));
		}
		else
		{
			if (currentImage->hasAnyImage() == false)
			{
				ImageItem::drawError(drawParam, rect, QStringLiteral("No Image"));
			}
			else
			{
				currentImage->drawImage(drawParam, rect);
			}
		}

		// Drawing frame rect
		//
		if (drawRect() == true)
		{
			m_rectPen->setWidthF(m_lineWeight == 0.0 ? drawParam->cosmeticPenWidth() : m_lineWeight);

			p->setPen(*m_rectPen);
			p->drawRect(rect);
		}

		return;
	}

	void SchemaItemImageValue::drawHighlight(CDrawParam* drawParam) const
	{
		return drawRotated(drawParam, [drawParam, this]()
						   {
							   return drawHighlightPrivate(drawParam);
						   });
	}

	void SchemaItemImageValue::drawHighlightPrivate(CDrawParam* drawParam) const
	{
		// Draw highlights by signals
		//
		for (const QString& appSignalId : m_signalIds)
		{
			if (drawParam->highlightIds().contains(appSignalId) == true)
			{
				QRectF highlightRect = boundingRectInDocPt(drawParam);
				drawHighlightRect(drawParam, highlightRect);
				break;
			}
		}

		return;
	}

	void SchemaItemImageValue::initDrawingResources() const
	{
		if (m_rectPen.get() == nullptr)
		{
			m_rectPen = std::make_unique<QPen>();
		}

		if (m_rectPen->color() != lineColor())
		{
			m_rectPen->setColor(lineColor());
		}

		// --
		//
		if (m_fillBrush.get() == nullptr)
		{
			m_fillBrush = std::make_unique<QBrush>(Qt::SolidPattern);
		}

		return;
	}

	void SchemaItemImageValue::drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect)
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

	double SchemaItemImageValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemImageValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	QObject* SchemaItemImageValue::imageItem(QString imageId)
	{
		for (std::shared_ptr<VFrame30::ImageItem>& i : m_images)
		{
			if (i->imageId() == imageId)
			{
				return new ScriptImageItem{i};
			}
		}

		return nullptr;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemImageValue::associatedAppSignalIds() const
	{
		return signalIds();
	}

	QStringList SchemaItemImageValue::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemImageValue::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemImageValue::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemImageValue::associatedSchemaItemLabels() const
	{
		return {};
	}

	QString SchemaItemImageValue::signalIdsString() const
	{
		std::shared_ptr<Context> context = this->context();
		return signalIdsString(context.get());
	}

	QString SchemaItemImageValue::signalIdsString(const Context* context) const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable
		//
		if (context != nullptr &&
			context->appSignalController() != nullptr &&
			context->viewVariables() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, context, nullptr, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = context->appSignalController()->appSignalManager().equipmentToAppSignalId(s);
				}
			}
		}

		return resultList.join(QChar::LineFeed);
	}

	void SchemaItemImageValue::setSignalIdsString(const QString& value)
	{
		m_signalIds = value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	}

	QStringList SchemaItemImageValue::signalIds() const
	{
		std::shared_ptr<Context> context = this->context();
		return signalIds(context.get());
	}

	QStringList SchemaItemImageValue::signalIds(const Context* context) const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode, if applicable
		//
		if (context != nullptr &&
			context->appSignalController() != nullptr &&
			context->viewVariables() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, context, nullptr, this);

			for (QString& s : resultList)
			{
				if (s.startsWith('@') == true)
				{
					s = context->appSignalController()->appSignalManager().equipmentToAppSignalId(s);
				}
			}
		}

		return resultList;
	}

	void SchemaItemImageValue::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;
	}

	E::SignalSource SchemaItemImageValue::signalSource() const
	{
		return m_signalSource;
	}

	void SchemaItemImageValue::setSignalSource(E::SignalSource value)
	{
		m_signalSource = value;
	}

	// Images
	//
	const PropertyVector<ImageItem>& SchemaItemImageValue::images() const
	{
		return m_images;
	}

	void SchemaItemImageValue::setImages(const PropertyVector<ImageItem>& value)
	{
		m_images = value;
	}

	QString SchemaItemImageValue::currentImageId() const
	{
		return m_currentImageId;
	}

	void SchemaItemImageValue::setCurrentImageId(QString value)
	{
		m_currentImageId = value;
	}

	// Weight property
	//
	double SchemaItemImageValue::lineWeight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return VFrame30::RoundDisplayPoint(m_lineWeight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(m_lineWeight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = VFrame30::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemImageValue::setLineWeight(double weight)
	{
		if (weight < 0)
		{
			weight = 0;
		}

		if (itemUnit() == SchemaUnit::Display)
		{
			m_lineWeight = VFrame30::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			m_lineWeight = pt;
		}
	}

	// LineColor property
	//
	const QColor& SchemaItemImageValue::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemImageValue::setLineColor(const QColor& color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	const QColor& SchemaItemImageValue::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemImageValue::setFillColor(const QColor& color)
	{
		m_fillColor = color;
	}

	bool SchemaItemImageValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemImageValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	bool SchemaItemImageValue::fillRect() const
	{
		return m_fillRect;
	}

	void SchemaItemImageValue::setFillRect(bool value)
	{
		m_fillRect = value;
	}
} // namespace VFrame30
