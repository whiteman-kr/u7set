#pragma once

#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/PosRectImpl.h>
#include <VFrame30/SchemaItemVdu.h>


namespace VFrame30
{
	// class ImageItem;

	class SchemaItemVduImageValue : public PosRectImpl,
									public IMatsSchemaItemAssociations,
									public SchemaItemVduVisitable<SchemaItemVduImageValue>
	{
		Q_OBJECT

	public:
		SchemaItemVduImageValue(void);
		explicit SchemaItemVduImageValue(SchemaUnit unit);
		virtual ~SchemaItemVduImageValue(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		virtual void drawHighlight(CDrawParam* drawParam) const override;

	protected:
		void drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect);

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override;
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		QString signalIdsString(const Context* context) const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		QStringList signalIds(const Context* context) const;
		void setSignalIds(const QStringList& value);

		const PropertyVector<ImageItem>& images() const;
		void setImages(const PropertyVector<ImageItem>& value);

		const QColor& fillColor() const;
		void setFillColor(const QColor& color);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};

		PropertyVector<ImageItem> m_images; // Each image is a std::shared_ptr

		QColor m_fillColor = qRgba(0xE0, 0xE0, 0xE0, 0xFF);
	};
} // namespace VFrame30
