#include "ImageItem.h"
#include "PropertyNames.h"
#include "Schema.h"

namespace VFrame30
{
	//
	//	ImageItem
	//
	ImageItem::ImageItem(void)
	{
		Property* p = nullptr;

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::allowScale, PropertyNames::appearanceCategory, true, ImageItem::allowScale, ImageItem::setAllowScale);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::keepAspectRatio, PropertyNames::appearanceCategory, true, ImageItem::keepAspectRatio, ImageItem::setKeepAspectRatio);

		p = ADD_PROPERTY_GET_SET_CAT(QImage, PropertyNames::image, PropertyNames::imageCategory, true, ImageItem::image, ImageItem::setImage);
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.gif);; All Files (*.*)"));

		p = ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::svg, PropertyNames::imageCategory, true, ImageItem::svgData, ImageItem::setSvgData);
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Svg Files (*.svg);; All Files (*.*)"));

		return;
	}

	ImageItem::ImageItem(ImageItem& src) :
		PropertyObject(src),
		m_allowScale(src.m_allowScale),
		m_keepAspectRatio(src.m_keepAspectRatio),
		m_image(src.m_image),
		m_imageData(src.m_imageData),
		m_svgData(src.m_svgData)
		//m_svgRenderer(src.m_svgRenderer)		// Cannot be copied ((( that's why class has copy constructor
	{
	}

	bool ImageItem::save(Proto::ImageItem* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		message->set_allowscale(m_allowScale);
		message->set_keepaspectratio(m_keepAspectRatio);

		if (m_image.isNull() == false)
		{
			if (m_imageData.isEmpty() == true)
			{
				QBuffer buffer(&m_imageData);
				buffer.open(QIODevice::WriteOnly);

				bool saveOk = m_image.save(&buffer, "PNG");
				if (saveOk == false)
				{
					qDebug() << __FUNCTION__ << " SaveImageResult: False";
				}
			}

			message->set_imagedata(m_imageData.constData(), m_imageData.size());
		}
		else
		{
			m_imageData.clear();
		}

		if (m_svgData.isEmpty() == false)
		{
			message->set_svgdata(m_svgData.toStdString());
		}

		return true;
	}

	bool ImageItem::load(const Proto::ImageItem& message)
	{
		m_allowScale = message.allowscale();
		m_keepAspectRatio = message.keepaspectratio();

		if (message.has_imagedata() == true)
		{
			const std::string& imageString = message.imagedata();
			m_imageData = QByteArray{imageString.data(), static_cast<int>(imageString.size())};

			bool loadOk = m_image.loadFromData(m_imageData);
			if (loadOk == false)
			{
				qDebug() << __FUNCTION__ << " LoadImageResult: False";
			}
		}
		else
		{
			m_image = QImage();
			m_imageData.clear();
		}

		if (message.has_svgdata() == true)
		{
			m_svgData = QString::fromStdString(message.svgdata());
		}
		else
		{
			m_svgData.clear();
		}

		m_svgRenderer.reset();

		return true;
	}

	bool ImageItem::hasAnyImage() const
	{
		return m_image.isNull() == false || m_svgData.isEmpty() == false;
	}

	void ImageItem::drawError(CDrawParam* drawParam, const QRectF& rect, const QString& errorText) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

		QPen pen(Qt::black);
		pen.setWidthF(drawParam->cosmeticPenWidth());

		QPainter* painter = drawParam->painter();

		painter->setPen(pen);
		painter->drawRect(rect);

		QFont f;		// Default application font
		painter->setFont(f);

		DrawHelper::drawText(painter, drawParam->schema()->unit(), errorText, rect, Qt::AlignCenter | Qt::AlignVCenter);
		return;
	}

	void ImageItem::drawImage(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

		if (allowScale() == true)
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
		}
		else
		{
			QRectF imageRect{rect.left(), rect.top(), static_cast<qreal>(m_image.width()), static_cast<qreal>(m_image.height())};

			switch (drawParam->schema()->unit())
			{
			case SchemaUnit::Display:
				// Do nothing
				//
				break;
			case SchemaUnit::Inch:
				// in this case - size of the image depends on monitor DPI and IT CAN LOOK DIFFERENT FOR SEVERAL MONITORS WITH DIFFERENT DPI!!!
				//
				imageRect.setWidth(imageRect.width() / drawParam->dpiX());
				imageRect.setHeight(imageRect.height() / drawParam->dpiY());
				break;
			default:
				assert(false);
			}

			drawParam->painter()->drawImage(imageRect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
		}

		return;
	}

	void ImageItem::drawSvg(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

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

		// Keepn in midnd, autoscale == false does not work for SVG
		//
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

	// Properties and Data
	//

	// AllowScale
	//
	bool ImageItem::allowScale() const
	{
		return m_allowScale;
	}

	void ImageItem::setAllowScale(bool value)
	{
		m_allowScale = value;
	}

	// KeepAspectRatio
	//
	bool ImageItem::keepAspectRatio() const
	{
		return m_keepAspectRatio;
	}

	void ImageItem::setKeepAspectRatio(bool value)
	{
		m_keepAspectRatio = value;
	}

	// Image
	//
	const QImage& ImageItem::image() const
	{
		return m_image;
	}

	void ImageItem::setImage(QImage image)
	{
		m_image = image;
		m_imageData.clear();
		m_svgData.clear();
		m_svgRenderer.reset();
	}

	const QString& ImageItem::svgData() const
	{
		return m_svgData;
	}

	void ImageItem::setSvgData(const QString& data)
	{
		m_svgData = data;
		m_image = {};
		m_imageData.clear();
		m_svgRenderer.reset();
	}
}
