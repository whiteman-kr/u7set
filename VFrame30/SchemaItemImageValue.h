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
		Q_PROPERTY(QStringList AppSignalIDs READ signalIds WRITE setSignalIds)

		Q_PROPERTY(QString CurrentImageID READ currentImageId WRITE setCurrentImageId)

		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)
		Q_PROPERTY(bool Fill READ fillRect WRITE setFillRect)

		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

	public:
		SchemaItemImageValue(void);
		explicit SchemaItemImageValue(SchemaUnit unit);
		virtual ~SchemaItemImageValue(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		void initDrawingResources() const;
		//bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		void drawImage(CDrawParam* drawParam, const QString& imageId, const QRectF& rect);

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Java Script invocables specific for SchemaItemImageValue
		//
	public slots:

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		const PropertyVector<ImageItem>& images() const;
		void setImages(const PropertyVector<ImageItem>& value);

		QString currentImageId() const;
		void setCurrentImageId(QString value);

		double lineWeight() const;
		void setLineWeight(double lineWeight);

		const QColor& lineColor() const;
		void setLineColor(const QColor& color);

		const QColor& fillColor() const;
		void setFillColor(const QColor& color);

		bool drawRect() const;
		void setDrawRect(bool value);

		bool fillRect() const;
		void setFillRect(bool value);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		PropertyVector<ImageItem> m_images;	// Each image is a std::shared_ptr

		// MonitorMode variables
		//
		QString m_currentImageId;

		// --
		//
		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgba(0x00, 0x00, 0x00, 0xFF)};
		QColor m_fillColor = {qRgba(0x00, 0x00, 0xC0, 0xFF)};

		bool m_drawRect = false;							// Rect is visible, thikness 0 is possible
		bool m_fillRect = false;

		// Drawing resources
		//
		mutable std::unique_ptr<QPen> m_rectPen;
		mutable std::unique_ptr<QBrush> m_fillBrush;
	};
}


