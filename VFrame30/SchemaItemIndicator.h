#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"
#include "Indicator.h"


class AppSignalState;
class AppSignalParam;
class TuningSignalState;


namespace VFrame30
{
	class Indicator;

	//
	// SchemaItemIndicator
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemIndicator : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemIndicator(void);
		explicit SchemaItemIndicator(SchemaUnit unit);
		virtual ~SchemaItemIndicator(void) = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		std::set<QString> getSignalTags(CDrawParam* drawParam, const QString& appSignalId) const;
		bool getSignalParam(CDrawParam* drawParam, AppSignalParam* signalParam) const;
		bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;
		std::optional<double> getSignalState(CDrawParam* drawParam, const QString& appSignalId) const;

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		int precision() const;
		void setPrecision(int value);

		DECLARE_FONT_PROPERTIES(Font)

		FontParam& font();
		const FontParam& font() const;

		bool drawRect() const;
		void setDrawRect(bool value);

		double lineWeight() const;
		double lineWeightDraw() const noexcept;
		void setLineWeight(double weight);

		const QColor& backgroundColor() const;
		void setBackgroundColor(const QColor& color);

		const QColor& lineColor() const;
		void setLineColor(const QColor& color);

		const QVector<QColor>& signalColors() const;
		void setSignalColors(const QVector<QColor>& value);

		E::IndicatorType indicatorType() const;
		void setIndicatorType(E::IndicatorType value);

		// --
		//
		using IndicatorObjectPtr = std::unique_ptr<Indicator>;
		static const int MaxSignalsCound = 12;

	private:
		Indicator* indicatorObject();
		const Indicator* indicatorObject() const;

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		int m_precision = 2;

		FontParam m_font;

		bool m_drawRect = false;
		double m_lineWeight = 0.0;				// Line weight, in pixels or inches depends on UnitDocPt

		QColor m_backgroundColor{Qt::lightGray};
		QColor m_lineColor{Qt::black};

		QVector<QColor> m_signalColors = {Qt::darkBlue};

		E::IndicatorType m_indicatorType = E::IndicatorType::HistogramVert;

		// Do not remove any items (even obsolete) from the next array, do not change its orded,
		// only add new items after adding them to E::IndicatorType
		//
		std::array<IndicatorObjectPtr, E::IndicatorTypeCount> m_indicatorObjects;
	};

}


