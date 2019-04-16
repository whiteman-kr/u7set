#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

class AppSignalState;
class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemValue : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(QStringList SignalIDs READ signalIds WRITE setSignalIds)
		Q_PROPERTY(QString Text READ text WRITE setText)

		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)
		Q_PROPERTY(QColor TextColor READ textColor WRITE setTextColor)

	public:
		SchemaItemValue(void);
		explicit SchemaItemValue(SchemaUnit unit);
		virtual ~SchemaItemValue(void);

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
		void drawText(CDrawParam* drawParam, const QRectF& rect) const;

		bool getSignalState(CDrawParam* drawParam, AppSignalParam* signalParam, AppSignalState* appSignalState, TuningSignalState* tuningSignalState) const;

		QString parseText(QString text, CDrawParam* drawParam, const AppSignalParam& signal, const AppSignalState& signalState) const;
		QString formatNumber(double value, const AppSignalParam& signal) const;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Java Script invocables specific for SchemaItemValue
		//
	public:

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		void setSignalIds(const QStringList& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		double lineWeight() const;
		void setLineWeight(double lineWeight);

		const QColor& lineColor() const;
		void setLineColor(const QColor& color);

		const QColor& fillColor() const;
		void setFillColor(const QColor& color);

		const QColor& textColor() const;
		void setTextColor(const QColor& color);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		DECLARE_FONT_PROPERTIES(Font);

		bool drawRect() const;
		void setDrawRect(bool value);

		const QString& text() const;
		void setText(QString value);

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		double m_lineWeight = 0.0;

		QColor m_lineColor = {qRgb(0x00, 0x00, 0x00)};
		QColor m_fillColor = {qRgb(0x00, 0x00, 0xC0)};
		QColor m_textColor = {qRgb(0xF0, 0xF0, 0xF0)};

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_drawRect = false;		// Rect is visible, thikness 0 is possible

		QString m_text = {"$(value)"};	// $(value)			: signal value
										// $(caption)		: caption
										// $(signalid)		: SignalID (CustomSignalID)
										// $(appsignalid)	: AppSignalID (#APPSIGANLID)
										// $(equipmentid)	: Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)
										// $(highlimit)		: High limit
										// $(lowlimit)		: Low limit
										// $(units)			: Signal units

		int m_precision = -1;			// decimal places, -1 means take value from Signal
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;

		// Drawing resources
		//
		mutable std::unique_ptr<QPen> m_rectPen;
		mutable std::unique_ptr<QBrush> m_fillBrush;
	};
}
