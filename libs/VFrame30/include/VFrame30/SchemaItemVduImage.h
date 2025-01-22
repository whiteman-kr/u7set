#pragma once

#include <VFrame30/PosRectImpl.h>
#include <VFrame30/SchemaItemVdu.h>

#include <memory>

namespace VFrame30
{
	class ImageItem;


	class SchemaItemVduImage final : public PosRectImpl,
									 public SchemaItemVdu
	{
		Q_OBJECT

	public:
		SchemaItemVduImage(void);
		explicit SchemaItemVduImage(SchemaUnit units);

		~SchemaItemVduImage();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// VduItemVisitor
		//
	public:
		bool accept(VduItemVisitor& visitor) const override;

		// Properties and Data
		//
	public:
		bool allowScale() const; // Applied only to raster images
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		const QImage& image() const;
		void setImage(const QImage& image);

		const QString& svgData() const;
		void setSvgData(const QString& data);

		QImage toQImage(const QRectF& rect) const;

	private:
		std::unique_ptr<::VFrame30::ImageItem> m_image;
	};
} // namespace VFrame30
