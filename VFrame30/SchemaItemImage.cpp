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
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::allowScale, PropertyNames::appearanceCategory, true, SchemaItemImage::allowScale, SchemaItemImage::setAllowScale);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::keepAspectRatio, PropertyNames::appearanceCategory, true, SchemaItemImage::keepAspectRatio, SchemaItemImage::setKeepAspectRatio);

		ADD_PROPERTY_GETTER_SETTER(QImage, PropertyNames::image, false, SchemaItemImage::image, SchemaItemImage::setImage);
		//ADD_PROPERTY_GETTER_SETTER(QByteArray, PropertyNames::svg, false, SchemaItemImage::svgData, SchemaItemImage::setSvg);

		// --
		//
		m_static = true;
		setItemUnit(unit);
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

		imageMessage->set_allowscale(m_allowScale);
		imageMessage->set_keepaspectratio(m_keepAspectRatio);

		if (m_image.isNull() == false)
		{
			QByteArray ba;
			QBuffer buffer(&ba);
			buffer.open(QIODevice::WriteOnly);

			bool saveOk = m_image.save(&buffer, "PNG");
			if (saveOk == false)
			{
				qDebug() << __FUNCTION__ << " SaveImageResult: False";
			}

			imageMessage->set_imagedata(ba.constData(), ba.size());
		}


		return true;
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
			assert(message.schemaitem().has_image());
			return false;
		}

		const Proto::SchemaItemImage& imageMessage = message.schemaitem().image();

		m_allowScale = imageMessage.allowscale();
		m_keepAspectRatio = imageMessage.keepaspectratio();

		if (imageMessage.has_imagedata() == true)
		{
			const std::string& imageString = imageMessage.imagedata();
			QByteArray ba = QByteArray::fromRawData(imageString.data(), static_cast<int>(imageString.size()));

			bool loadOk = m_image.loadFromData(ba);

			if (loadOk == false)
			{
				qDebug() << __FUNCTION__ << " LoadImageResult: False";
			}
		}
		else
		{
			m_image = QImage();
		}

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemImage::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
						
		// Calculate rectangle
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		// --
		//
		if (m_image.isNull() == true)
		{
			// Image not set, draw rect and information text
			//
			QPen pen(Qt::black);
			pen.setWidthF(drawParam->cosmeticPenWidth());

			p->setPen(pen);
			p->drawRect(r);

			// --
			//
			QFont f;		// Default application font
			p->setFont(f);

			DrawHelper::drawText(p, itemUnit(), QStringLiteral("No Image"), r, Qt::AlignCenter | Qt::AlignVCenter);

			return;
		}

		// Draw Image
		//


		if (m_keepAspectRatio == true)
		{
			QRectF imageRect = r;

			QSizeF imageSize = m_image.size();	// m_image.size() / m_image.devicePixelRatio();
			imageSize.scale(imageRect.width(), imageRect.height(), Qt::KeepAspectRatio);

			imageRect.setSize(imageSize);
			imageRect.translate(std::fabs(r.width() - imageRect.width()) / 2,
								std::fabs(r.height() - imageRect.height()) / 2);

			p->drawImage(imageRect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
		}
		else
		{
			p->drawImage(r, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
		}

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

	// AllowScale property
	//
	bool SchemaItemImage::allowScale() const
	{
		return m_allowScale;
	}

	void SchemaItemImage::setAllowScale(bool value)
	{
		m_allowScale = value;
	}

	// KeepAspectRatio
	//
	bool SchemaItemImage::keepAspectRatio() const
	{
		return m_keepAspectRatio;
	}

	void SchemaItemImage::setKeepAspectRatio(bool value)
	{
		m_keepAspectRatio = value;
	}

	// Image
	//
	QImage SchemaItemImage::image() const
	{
		return m_image;
	}

	void SchemaItemImage::setImage(QImage image)
	{
		//m_image.load("D:\\About.png");
		//bool ok = m_image.load("D:\\ScemaFolder.svg");
		m_image = image;
	}

	QByteArray SchemaItemImage::svgData() const
	{
		return m_svgData;
	}

	void SchemaItemImage::setSvgData(QByteArray data)
	{
		m_svgData = data;
	}
}

