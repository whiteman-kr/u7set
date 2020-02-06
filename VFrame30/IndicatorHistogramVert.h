#pragma once
#include "Indicator.h"

namespace VFrame30
{

	class CustomSetPoint : public PropertyObject
	{
		Q_OBJECT

	public:
		CustomSetPoint() = default;
		virtual ~CustomSetPoint() {};

		CustomSetPoint(const CustomSetPoint&) noexcept = default;
		CustomSetPoint(CustomSetPoint&&) noexcept = default;

		CustomSetPoint& operator=(const CustomSetPoint&) noexcept = default;
		CustomSetPoint& operator=(CustomSetPoint&&) noexcept = default;

	public:
		virtual void propertyDemand(const QString&) override;

		bool save(Proto::VFrameSetPoint* message) const;
		bool load(const Proto::VFrameSetPoint& message);

	public slots:
		E::IndicatorSetpointType setpointType() const;
		void setSetpointType(E::IndicatorSetpointType value);

		QColor color() const;
		void setColor(const QColor& value);

		const QString& schemaItemLabel() const;
		void setSchemaItemLabel(const QString& value);

		const QString& outputAppSignalId() const;
		void setOutputAppSignalId(const QString& value);

		double staticValue() const;
		void setStaticValue(double value);

		E::CmpType staticCompareType() const;
		void setStaticCompareType(E::CmpType value);

	private:
		E::IndicatorSetpointType m_setpointType = E::IndicatorSetpointType::Static;
		QColor m_color{Qt::darkBlue};
		/*
				Static						Just show some value with CmpType and color, no real setpoint is involved
				AutoBySchemaItemLabel		Get setpoint by (Comparator) SchemaItem Label, all data (values, type) are taken from setpoint storage
											Color is taken from output AppSignal.AlertColor (UserSpecificProperties), or from m_color if property is not exists
				AutoByOutAppSignalId		Get setpoint by output AppSignalID (internal), all data are taken from setpoint storage
											Color is taken from output AppSignal.AlertColor (UserSpecificProperties), or from m_color if property is not exists
		*/

		// Values for E::IndicatorSetpointType::AutoBySchemaItemLabel
		//
		QString m_schemaItemLabel;

		// Values for E::IndicatorSetpointType::AutoByOutAppSignalId
		//
		QString m_outputAppSignalId;

		// Values for E::IndicatorSetpointType::Static
		//
		double m_staticValue = 0;
		E::CmpType m_staticCompareType = E::CmpType::Greate;
	};


	//
	// Vertical histogram, the base view
	//
	class IndicatorHistogramVert : public Indicator
	{
	public:
		IndicatorHistogramVert() = delete;
		explicit IndicatorHistogramVert(SchemaUnit itemUnit);
		virtual ~IndicatorHistogramVert() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool save(Proto::SchemaItemIndicator* message) const override;
		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* schemaItem) const override;

	private:
		void drawBar(CDrawParam* drawParam, const QRectF& barRect, int signalIndex, const QString appSignalId, QColor barColor, const SchemaItemIndicator* schemaItem) const;

		struct DrawGridStruct
		{
			double gridVertPos;
			double gridWidth;
			QString text;
		};
		void drawGrids(const std::vector<DrawGridStruct> grids, CDrawParam* drawParam, const QRectF barRect, const SchemaItemIndicator* item) const;

		void drawSetpoints(CDrawParam* drawParam, const std::vector<QRectF>& barRects, const SchemaItemIndicator* schemaItem) const;


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

		bool m_drawAutoSetpoints = true;					// Draw all auto generated setpoints
		bool m_drawCustomSetpoints = true;					// Draw custom setpoints

		PropertyVector<CustomSetPoint> m_customSetPoints;	// Custom setpoint list
	};

}

Q_DECLARE_METATYPE(VFrame30::CustomSetPoint)
Q_DECLARE_METATYPE(PropertyVector<VFrame30::CustomSetPoint>)
