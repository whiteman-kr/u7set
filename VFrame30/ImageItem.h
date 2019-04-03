#pragma once
#include <QSvgRenderer>
#include <optional>
#include "../lib/PropertyObject.h"
#include "DrawParam.h"

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT ImageItem : public PropertyObject
	{
		Q_OBJECT

	public:
		ImageItem(void);
		ImageItem(ImageItem& src);

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

}

Q_DECLARE_METATYPE(PropertyVector<VFrame30::ImageItem>)
Q_DECLARE_METATYPE(PropertyList<VFrame30::ImageItem>)
