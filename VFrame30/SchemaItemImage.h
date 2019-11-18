#pragma once

#include "PosRectImpl.h"
#include "ImageItem.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemImage : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(double AllowScale READ allowScale WRITE setAllowScale)
		Q_PROPERTY(double KeepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)

		Q_PROPERTY(QString Svg READ svgData WRITE setSvgData)

	public:
		SchemaItemImage(void);
		explicit SchemaItemImage(SchemaUnit unit);
		virtual ~SchemaItemImage(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Properties and Data
		//
	public:
		bool allowScale() const;		// Applied only to raster images
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		const QImage& image() const;
		void setImage(const QImage& image);

		const QString& svgData() const;
		void setSvgData(const QString& data);

	private:
		ImageItem m_image;
	};
}
