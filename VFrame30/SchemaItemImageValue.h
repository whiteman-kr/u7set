#pragma once

#include "PosRectImpl.h"
#include "ImageItem.h"

class AppSignalState;
class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{

	class VFRAME30LIBSHARED_EXPORT SchemaItemImageValue : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)
		Q_PROPERTY(QString CurrentImageID READ currentImageId WRITE setCurrentImageId)

	public:
		SchemaItemImageValue(void);
		explicit SchemaItemImageValue(SchemaUnit unit);
		virtual ~SchemaItemImageValue(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

	protected:
		void initDrawingResources() const;
		bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		void drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect);

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Java Script invocables specific for SchemaItemImageValue
		//
	public slots:

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		const QStringList& signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		const PropertyVector<ImageItem>& images() const;
		void setImages(const PropertyVector<ImageItem>& value);

		QString initialImageId() const;
		void setInitialImageId(QString value);

		QString currentImageId() const;
		void setCurrentImageId(QString value);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		PropertyVector<ImageItem> m_images;	// Each image is a std::shared_ptr

		QString m_initialImageId;			// Default value on Monitor start for currentImageId

		// MonitorMode variables
		//
		QString m_currentImageId;
	};
}


