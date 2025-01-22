#include <VFrame30/ImageItem.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaView.h>

#include <QPrinter>

namespace VFrame30
{
	//
	//	ImageItem
	//
	ImageItem::ImageItem(void)
	{
		createProperties();
		return;
	}

	ImageItem::ImageItem(ImageItem& src) :
		PropertyObject(src),
		m_allowScale(src.m_allowScale),
		m_keepAspectRatio(src.m_keepAspectRatio),
		m_imageId(src.m_imageId),
		m_image(src.m_image),
		m_imageData(src.m_imageData),
		m_svgData(src.m_svgData)
	// m_svgRenderer(src.m_svgRenderer)		// Cannot be copied ((( that's why class has copy constructor
	{
		createProperties();
		return;
	}

	void ImageItem::createProperties()
	{
		Property* p = nullptr;

		ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::allowScale, true, ImageItem::allowScale, ImageItem::setAllowScale);
		ADD_PROPERTY_GETTER_SETTER(bool, PropertyNames::keepAspectRatio, true, ImageItem::keepAspectRatio, ImageItem::setKeepAspectRatio);
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::imageId, true, ImageItem::imageId, ImageItem::setImageId);

		p = ADD_PROPERTY_GET_SET_CAT(QImage,
									 PropertyNames::image,
									 PropertyNames::imageCategory,
									 true,
									 ImageItem::image,
									 ImageItem::setImage);
		p->setSpecificEditor(E::PropertySpecificEditor::LoadFileDialog);
		p->setValidator(QStringLiteral("Images (*.png *.bmp *.jpg *.jpeg *.gif);; All Files (*.*)"));

		p = ADD_PROPERTY_GET_SET_CAT(QString,
									 PropertyNames::svg,
									 PropertyNames::imageCategory,
									 true,
									 ImageItem::svgData,
									 ImageItem::setSvgData);
		p->setSpecificEditor(E::PropertySpecificEditor::Svg);

		return;
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
		message->set_imageid(m_imageId.toStdString());

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
		m_imageId = QString::fromStdString(message.imageid());

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

	QImage ImageItem::toQImage(const QRectF& rect) const
	{
		if (hasAnyImage() == false)
		{
			return {};
		}

		QImage image{rect.size().toSize(), QImage::Format_ARGB32};
		QPainter painter{&image};

		if (svgData().isEmpty() == false)
		{
			QSvgRenderer svgRenderer{svgData().toUtf8()};
			drawSvg(painter, svgRenderer, rect, 100.0, SchemaUnit::Display);
		}

		if (image.isNull() == false)
		{
			drawRasterImage(painter, rect, 100.0, SchemaUnit::Display);
		}

		return image;
	}

	void ImageItem::drawError(CDrawParam* drawParam, const QRectF& rect, const QString& errorText)
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

		QPainter* painter = drawParam->painter();

		QPen pen(Qt::black);
		pen.setWidthF(drawParam->cosmeticPenWidth());
		painter->setPen(pen);
		painter->setBrush(Qt::NoBrush);

		painter->drawRect(rect);

		QFont f; // Default application font
		painter->setFont(f);

		DrawHelper::drawText(painter, drawParam->schemaUnit(), errorText, rect, Qt::AlignCenter | Qt::AlignVCenter);
		return;
	}

	void ImageItem::drawImage(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (svgData().isEmpty() == false)
		{
			drawSvg(drawParam, rect);
			return;
		}

		if (image().isNull() == false)
		{
			drawRasterImage(drawParam, rect);
		}

		return;
	}

	void ImageItem::drawRasterImage(CDrawParam* drawParam, const QRectF& rect) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

		return drawRasterImage(*drawParam->painter(),
							   rect,
							   drawParam->schemaView()->zoom(),
							   drawParam->schemaUnit(),
							   drawParam->realDpiX(),
							   drawParam->realDpiY());
	}

	void ImageItem::drawRasterImage(QPainter& painter,
									const QRectF& rect,
									[[maybe_unused]] double zoom,
									SchemaUnit units,
									double dpiX,
									double dpiY) const
	{
		if (allowScale() == true)
		{
			if (m_keepAspectRatio == true)
			{
				QRectF imageRect = rect;

				QSizeF imageSize = m_image.size(); // m_image.size() / m_image.devicePixelRatio();
				imageSize.scale(imageRect.width(), imageRect.height(), Qt::KeepAspectRatio);

				imageRect.setSize(imageSize);
				imageRect.translate(std::fabs(rect.width() - imageRect.width()) / 2, std::fabs(rect.height() - imageRect.height()) / 2);

				painter.drawImage(imageRect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
			}
			else
			{
				painter.drawImage(rect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
			}
		}
		else
		{
			QRectF imageRect{rect.left(), rect.top(), static_cast<qreal>(m_image.width()), static_cast<qreal>(m_image.height())};

			switch (units)
			{
			case SchemaUnit::Display:
				// Do nothing
				//
				break;
			case SchemaUnit::Inch:
				// in this case - size of the image depends on monitor DPI and IT CAN LOOK DIFFERENT FOR SEVERAL MONITORS WITH DIFFERENT
				// DPI!!!
				//
				imageRect.setWidth(imageRect.width() / dpiX);
				imageRect.setHeight(imageRect.height() / dpiY);
				break;
			default:
				assert(false);
			}

			painter.drawImage(imageRect, m_image, QRectF(0, 0, m_image.width(), m_image.height()));
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
			QByteArray data = m_svgData.toUtf8();
			m_svgRenderer.emplace(data);
		}

		if (m_svgRenderer->isValid() == false)
		{
			// Image not set, draw rect and information text
			//
			drawError(drawParam, rect, QStringLiteral("Not valid SVG file."));
			return;
		}

		return drawSvg(*drawParam->painter(), *m_svgRenderer, rect, drawParam->schemaView()->zoom(), drawParam->schemaUnit());
	}

	void ImageItem::drawSvg(QPainter& painter, QSvgRenderer& svgRenderer, const QRectF& rect, double zoom, SchemaUnit units) const
	{
		// Keep in mind, auto-scale == false does not work for SVG
		//
		QRectF imageRect = rect;

		if (m_keepAspectRatio == true)
		{
			QSizeF imageSize = svgRenderer.viewBoxF().size();
			imageSize.scale(imageRect.width(), imageRect.height(), Qt::KeepAspectRatio);

			imageRect.setSize(imageSize);
			imageRect.translate(std::fabs(rect.width() - imageRect.width()) / 2, std::fabs(rect.height() - imageRect.height()) / 2);
		}

#if 1
		QPaintDevice* device = painter.device();
		bool directRendering = dynamic_cast<QPrinter*>(device) || dynamic_cast<QPdfWriter*>(device) || dynamic_cast<QImage*>(device);

		if (directRendering == true)
		{
			svgRenderer.render(&painter, imageRect);
		}
		else
		{
			DrawHelper::drawSvgCached(painter, units, imageRect, m_svgData, zoom);
		}
#else
		svgRenderer.render(painter, imageRect);
#endif
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

	// ImageID
	//
	const QString& ImageItem::imageId() const
	{
		return m_imageId;
	}

	void ImageItem::setImageId(const QString& value)
	{
		m_imageId = value;
	}

	// Image
	//
	const QImage& ImageItem::image() const
	{
		return m_image;
	}

	void ImageItem::setImage(QImage image)
	{
		m_image = std::move(image);
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
		if (m_svgData != data)
		{
			m_svgData = data;
			m_image = {};
			m_imageData.clear();
			m_svgRenderer.reset();
		}

		return;
	}

	ScriptImageItem::ScriptImageItem(std::shared_ptr<VFrame30::ImageItem> imageItem) :
		m_imageItem(std::move(imageItem))
	{
		Q_ASSERT(m_imageItem);
	}

	ScriptImageItem::~ScriptImageItem()
	{
		qDebug() << Q_FUNC_INFO;
	}

	bool ScriptImageItem::allowScale() const
	{
		return m_imageItem->allowScale();
	}

	void ScriptImageItem::setAllowScale(bool value)
	{
		m_imageItem->setAllowScale(value);
	}

	bool ScriptImageItem::keepAspectRatio() const
	{
		return m_imageItem->keepAspectRatio();
	}

	void ScriptImageItem::setKeepAspectRatio(bool value)
	{
		m_imageItem->setKeepAspectRatio(value);
	}

	const QString& ScriptImageItem::imageId() const
	{
		return m_imageItem->imageId();
	}

	void ScriptImageItem::setImageId(const QString& value)
	{
		m_imageItem->setImageId(value);
	}

	const QString& ScriptImageItem::svgData() const
	{
		return m_imageItem->svgData();
	}

	void ScriptImageItem::setSvgData(const QString& data)
	{
		m_imageItem->setSvgData(data);
	}
} // namespace VFrame30
