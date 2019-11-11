#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"


class AppSignalState;
class AppSignalParam;
class TuningSignalState;


namespace VFrame30
{
	class SchemaItemIndicator;

	//
	// IndicatorComponent base class
	//
	class IndicatorObject
	{
	public:
		IndicatorObject() = delete;
		explicit IndicatorObject(SchemaUnit itemUnit);

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) = 0;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) = 0;
		virtual bool save(Proto::SchemaItemIndicator* message) const = 0;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const = 0;

	public:
		// Get/set are working in regional units, for drawing use variables
		//
		template <typename TYPE>
		TYPE regionalGetter(const TYPE& variable) const;			// Get/set are working in regional units, for drawing use variables

		template <typename TYPE>
		void regionalSetter(TYPE value, TYPE* variable);

		void setUnits(SchemaUnit itemUnit);

	protected:
		SchemaUnit m_itemUnit = SchemaUnit::Display;
	};


	//
	// Vertical histogram, the base view
	//
	class IndicatorHistogramVert : public IndicatorObject
	{
	public:
		IndicatorHistogramVert() = delete;
		explicit IndicatorHistogramVert(SchemaUnit itemUnit);
		virtual ~IndicatorHistogramVert() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool save(Proto::SchemaItemIndicator* message) const override;
		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const override;

	private:
		void drawBar(CDrawParam* drawParam, const QRectF& barRect, int signalIndex, const QString appSignalId, QColor barColor, const SchemaItemIndicator* item) const;
		struct DrawGridStruct
		{
			double gridVertPos;
			double gridWidth;
			QString text;
		};

		void drawGrids(const std::vector<DrawGridStruct> grids, CDrawParam* drawParam, const QRectF barRect, const SchemaItemIndicator* item) const;


	private:
		double m_startValue = 0;				// Start/End Values for top and bottom of the indicator,
		double m_endValue = 100.0;				// this field is common for all signals and all columns
												// so it's not possible to set different scales for several signals

		double m_barWidth;						// Column width, In inches or pixels, getter and setter converts it to regional units
		bool m_drawBarRect = true;

		double m_leftMargin = mm2in(5);			// In inches or pixels, getter and setter converts it to regional units
		double m_topMargin = mm2in(5);			// In inches or pixels, getter and setter converts it to regional units
		double m_rightMargin = mm2in(5);		// In inches or pixels, getter and setter converts it to regional units
		double m_bottomMargin = mm2in(5);		// In inches or pixels, getter and setter converts it to regional units

		bool m_drawGrid = true;					// Draw grids
		bool m_drawGridForAllBars = false;		// Draw grids for all bars
		bool m_drawGridValues = true;			// Draw values for grid (only if m_drawGrid == true, for next bars depends on DrawGridOnlyForFirstBar)
		bool m_drawGridValueForAllBars = false;	// Draw values for grid for all bars (true) or just for the first one (false) (only if drawGrid == true && drawGridValues == true)
		bool m_drawGridValueUnits = true;		// Draw units for limits values (only if DrawGrid == true && DrawGridValues == true)
		double m_gridMainStep = 50.0;			// Step for main grids (only if DrawGrid == true)
		double m_gridSmallStep = 10.0;			// Step for small grids (only if DrawGrid == true)
	};


	//
	// ArrowIndicator
	//
	class IndicatorArrowIndicator : public IndicatorObject
	{
	public:
		IndicatorArrowIndicator() = delete;
		explicit IndicatorArrowIndicator(SchemaUnit itemUnit);
		virtual ~IndicatorArrowIndicator() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;
		virtual bool save(Proto::SchemaItemIndicator* message) const override;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const override;

	public:
		double startValue() const;
		void setStartValue(double value);

		double endValue() const;
		void setEndValue(double value);

		double startAngle() const;
		void setStartAngle(double value);

		double spanAngle() const;
		void setSpanAngle(double value);

	private:
		double m_startValue = 0;
		double m_endValue = 100.0;

		double m_startAngle = 330;			// Zero degrees is at the 9 o'clock position.
		double m_spanAngle = 240;
	};


	//
	// SchemaItemIndicator
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemIndicator : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemIndicator(void);
		explicit SchemaItemIndicator(SchemaUnit unit);
		virtual ~SchemaItemIndicator(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

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

		DECLARE_FONT_PROPERTIES(Font);

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
		using IndicatorObjectPtr = std::unique_ptr<IndicatorObject>;
		static const int MaxSignalsCound = 12;

	private:
		IndicatorObject* indicatorObject();
		const IndicatorObject* indicatorObject() const;

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


