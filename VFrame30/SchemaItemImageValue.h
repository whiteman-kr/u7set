#pragma once

#include "PosRectImpl.h"
#include <QSvgRenderer>
#include <optional>

class AppSignalState;
class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemImageValue : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)

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

		//QString parseText(QString text, const AppSignalParam& signal, const AppSignalState& signalState) const;
		//QString formatNumber(double value, const AppSignalParam& signal) const;

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

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

//		QImage image() const;
//		void setImage(QImage image);

//		QString svgData() const;
//		void setSvgData(QString data);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		bool m_keepAspectRatio = true;

//		QImage m_image;
//		QString m_svgData;

		// Drawing resources
		//
//		mutable std::optional<QSvgRenderer> m_svgRenderer;
	};
}
