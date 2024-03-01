#pragma once
#include <QSvgRenderer>
#include <optional>
#include "../DrawParam.h"

namespace VFrame30
{
	class ImageItem : public PropertyObject
	{
		Q_OBJECT

	public:
		ImageItem(void);
		ImageItem(ImageItem& src);

	private:
		void createProperties();

	public:
		bool save(Proto::ImageItem* message) const;
		bool load(const Proto::ImageItem& message);

		bool hasAnyImage() const;

		static void drawError(CDrawParam* drawParam, const QRectF& rect, const QString& errorText);
		void drawImage(CDrawParam* drawParam, const QRectF& rect) const;
		void drawRasterImage(CDrawParam* drawParam, const QRectF& rect) const;
		void drawSvg(CDrawParam* drawParam, const QRectF& rect) const;

	public:
		bool allowScale() const;
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		const QString& imageId() const;
		void setImageId(const QString& value);

		const QImage& image() const;
		void setImage(QImage image);

		const QString& svgData() const;
		void setSvgData(const QString& data);

	private:
		// Class has COPY constructor, keep in mind when adding new members!!!
		//
		bool m_allowScale = true;
		bool m_keepAspectRatio = true;
		QString m_imageId = "IMAGEID";

		QImage m_image;
		mutable QByteArray m_imageData;							// To prevent from compressing image again and again if it was not changed

		QString m_svgData;
		mutable std::optional<QSvgRenderer> m_svgRenderer;		// Drawing resources

		// Class has COPY constructor, keep in mind when adding new members!!!
		//
	};

	/*! \class ScriptImageItem
		\ingroup dynamicSchemaItems
		\brief This class is used to describe images for \ref VFrame30::SchemaItemImageValue "SchemaItemImageValue".
	*/
	class ScriptImageItem : public QObject
	{
		Q_OBJECT

		/// \brief Scale image to the size of object.
		Q_PROPERTY(bool AllowScale READ allowScale WRITE setAllowScale)
		Q_PROPERTY(bool allowScale READ allowScale WRITE setAllowScale)

		/// \brief Keep apect ration of the image.
		Q_PROPERTY(bool KeepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
		Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)

		/// \brief Image identifier, use it to get image item from \ref VFrame30::SchemaItemImageValue "SchemaItemImageValue".
		Q_PROPERTY(QString ImageId READ imageId WRITE setImageId)
		Q_PROPERTY(QString imageId READ imageId WRITE setImageId)

		/// \brief Svg file data.
		Q_PROPERTY(QString Svg READ svgData WRITE setSvgData)
		Q_PROPERTY(QString svg READ svgData WRITE setSvgData)

	public:
		ScriptImageItem(std::shared_ptr<VFrame30::ImageItem> imageItem);
		virtual ~ScriptImageItem();

	public:
		bool allowScale() const;
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		const QString& imageId() const;
		void setImageId(const QString& value);

		const QString& svgData() const;
		void setSvgData(const QString& data);

	private:
		std::shared_ptr<VFrame30::ImageItem> m_imageItem;
	};

}

Q_DECLARE_METATYPE(PropertyVector<VFrame30::ImageItem>)
Q_DECLARE_METATYPE(PropertyList<VFrame30::ImageItem>)
