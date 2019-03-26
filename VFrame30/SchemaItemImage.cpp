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

		if (m_svgData.isEmpty() == false)
		{
			imageMessage->set_svgdata(m_svgData.toStdString());
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

		if (imageMessage.has_svgdata() == true)
		{
			m_svgData = QString::fromStdString(imageMessage.svgdata());
		}
		else
		{
			m_svgData.clear();
		}

		m_svgRenderer.reset();

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemImage::Draw(CDrawParam* drawParam, const Schema* /*schema*/, const SchemaLayer* /*layer*/) const
	{
		QRectF rect = boundingRectInDocPt();

		// --
		//
		if (m_image.isNull() == true && m_svgData.isEmpty() == true)
		{
			// Image not set, draw rect and information text
			//
			drawError(drawParam, rect, QStringLiteral("No Image"));
			return;
		}

		// Draw Image
		//
		if (m_svgData.isEmpty() == false)
		{
			drawSvg(drawParam, rect);
			return;
		}

		if (m_image.isNull() == false)
		{
			drawImage(drawParam, rect);
		}

		return;
	}

	void SchemaItemImage::drawImage(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (m_keepAspectRatio == true)
		{
			QRectF imageRect = rect;

			QSizeF imageSize = m_image.size();	// m_image.size() / m_image.devicePixelRatio();
			imageSize.scale(imageRect.width(), imageRect.height(), Qt::KeepAspectRatio);

			imageRect.setSize(imageSize);
			imageRect.translate(std::fabs(rect.width() - imageRect.width()) / 2,
								std::fabs(rect.height() - imageRect.height()) / 2);

			drawParam->painter()->drawImage(imageRect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
		}
		else
		{
			drawParam->painter()->drawImage(rect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
		}

		return;
	}

	void SchemaItemImage::drawSvg(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (m_svgData.isEmpty() == true)
		{
			return;
		}

		if (m_svgRenderer.has_value() == false)
		{
			QByteArray data = m_svgData.toLocal8Bit();
			m_svgRenderer.emplace(data);
		}

		if (m_svgRenderer->isValid() == false)
		{
			// Image not set, draw rect and information text
			//
			drawError(drawParam, rect, QStringLiteral("Not valid SVG file."));
			return;
		}

		QRectF imageRect = rect;

		if (m_keepAspectRatio == true)
		{
			QSizeF imageSize = m_svgRenderer->viewBoxF().size();
			imageSize.scale(imageRect.width(), imageRect.height(), Qt::KeepAspectRatio);

			imageRect.setSize(imageSize);
			imageRect.translate(std::fabs(rect.width() - imageRect.width()) / 2,
								std::fabs(rect.height() - imageRect.height()) / 2);
		}

		m_svgRenderer->render(drawParam->painter(), imageRect);

		return;
	}

	void SchemaItemImage::drawError(CDrawParam* drawParam, const QRectF& rect, QString errorText) const
	{
		QPen pen(Qt::black);
		pen.setWidthF(drawParam->cosmeticPenWidth());

		QPainter* painter = drawParam->painter();

		painter->setPen(pen);
		painter->drawRect(rect);

		QFont f;		// Default application font
		painter->setFont(f);

		DrawHelper::drawText(painter, itemUnit(), errorText, rect, Qt::AlignCenter | Qt::AlignVCenter);

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
		m_image = image;
		m_svgData.clear();
		m_svgRenderer.reset();
	}

	// Svg
	//
	QString SchemaItemImage::svgData() const
	{
		return m_svgData;
	}

	void SchemaItemImage::setSvgData(QString data)
	{
		m_svgData = data;
		m_image = {};
		m_svgRenderer.reset();
	}
}

