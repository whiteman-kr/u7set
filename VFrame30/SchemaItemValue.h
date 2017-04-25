#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

struct AppSignalState;
class AppSignalParam;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemValue : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(QString Text READ textAnalog WRITE setTextAnalog)

		Q_PROPERTY(QString textNonValid READ textNonValid WRITE setTextNonValid)
		Q_PROPERTY(QString TextNonValid READ textNonValid WRITE setTextNonValid)

		Q_PROPERTY(QString textAnalog READ textAnalog WRITE setTextAnalog)
		Q_PROPERTY(QString TextAnalog READ textAnalog WRITE setTextAnalog)

		Q_PROPERTY(QString textDiscreteNo READ textDiscreteNo WRITE setTextDiscreteNo)
		Q_PROPERTY(QString TextDiscreteNo READ textDiscreteNo WRITE setTextDiscreteNo)

		Q_PROPERTY(QString textDiscreteYes READ textDiscreteYes WRITE setTextDiscreteYes)
		Q_PROPERTY(QString TextDiscreteTes READ textDiscreteYes WRITE setTextDiscreteYes)

		Q_PROPERTY(double lineWeight READ lineWeight WRITE setLineWeight)
		Q_PROPERTY(double LineWeight READ lineWeight WRITE setLineWeight)

		Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)

		Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

		Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
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
		void drawLogic(CDrawParam* drawParam, const QRectF& rect, const AppSignalParam& signal, const AppSignalState& signalState) const;

		QString parseText(QString text, const AppSignalParam& signal, const AppSignalState& signalState) const;
		QString formatNumber(double value, const AppSignalParam& signal) const;

		// Text search
		//
	public:
		virtual bool searchText(const QString& textAnalog) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Properties and Data
		//
	public:
		double lineWeight() const;
		void setLineWeight(double lineWeight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		const QColor& fillColorNonValid0() const			{	return m_fillColorNonValid0;	}
		void setFillColorNonValid0(const QColor& value)		{	m_fillColorNonValid0 = value;	}

		const QColor& fillColorNonValid1() const			{	return m_fillColorNonValid1;	}
		void setFillColorNonValid1(const QColor& value)		{	m_fillColorNonValid1 = value;	}

		const QColor& textColorNonValid0() const			{	return m_textColorNonValid0;	}
		void setTextColorNonValid0(const QColor& value)		{	m_textColorNonValid0 = value;	}

		const QColor& textColorNonValid1() const			{	return m_textColorNonValid1;	}
		void setTextColorNonValid1(const QColor& value)		{	m_textColorNonValid1 = value;	}

//		const QColor& fillColorOverflow0() const			{	return m_fillColorOverflow0;	}
//		void setFillColorOverflow0(const QColor& value)		{	m_fillColorOverflow0 = value;	}

//		const QColor& fillColorOverflow1() const			{	return m_fillColorOverflow1;	}
//		void setFillColorOverflow1(const QColor& value)		{	m_fillColorOverflow1 = value;	}

//		const QColor& textColorOverflow0() const			{	return m_textColorOverflow0;	}
//		void setTextColorOverflow0(const QColor& value)		{	m_textColorOverflow0 = value;	}

//		const QColor& textColorOverflow1() const			{	return m_textColorOverflow1;	}
//		void setTextColorOverflow1(const QColor& value)		{	m_textColorOverflow1 = value;	}

//		const QColor& fillColorUnderflow0() const			{	return m_fillColorUnderflow0;	}
//		void setFillColorUnderflow0(const QColor& value)	{	m_fillColorUnderflow0 = value;	}

//		const QColor& fillColorUnderflow1() const			{	return m_fillColorUnderflow1;	}
//		void setFillColorUnderflow1(const QColor& value)	{	m_fillColorUnderflow1 = value;	}

//		const QColor& textColorUnderflow0() const			{	return m_textColorUnderflow0;	}
//		void setTextColorUnderflow0(const QColor& value)	{	m_textColorUnderflow0 = value;	}

//		const QColor& textColorUnderflow1() const			{	return m_textColorUnderflow1;	}
//		void setTextColorUnderflow1(const QColor& value)	{	m_textColorUnderflow1 = value;	}

		const QColor& fillColorAnalog0() const				{	return m_fillColorAnalog0;		}
		void setFillColorAnalog0(const QColor& value)		{	m_fillColorAnalog0 = value;		}

		const QColor& fillColorAnalog1() const				{	return m_fillColorAnalog1;		}
		void setFillColorAnalog1(const QColor& value)		{	m_fillColorAnalog1 = value;		}

		const QColor& textColorAnalog0() const				{	return m_textColorAnalog0;		}
		void setTextColorAnalog0(const QColor& value)		{	m_textColorAnalog0 = value;		}

		const QColor& textColorAnalog1() const				{	return m_textColorAnalog1;		}
		void setTextColorAnalog1(const QColor& value)		{	m_textColorAnalog1 = value;		}

		const QColor& fillColorDiscrYes0() const			{	return m_fillColorDiscrYes0;	}
		void setFillColorDiscrYes0(const QColor& value)		{	m_fillColorDiscrYes0 = value;	}

		const QColor& fillColorDiscrYes1() const			{	return m_fillColorDiscrYes1;	}
		void setFillColorDiscrYes1(const QColor& value)		{	m_fillColorDiscrYes1 = value;	}

		const QColor& textColorDiscrYes0() const			{	return m_textColorDiscrYes0;	}
		void setTextColorDiscrYes0(const QColor& value)		{	m_textColorDiscrYes0 = value;	}

		const QColor& textColorDiscrYes1() const			{	return m_textColorDiscrYes1;	}
		void setTextColorDiscrYes1(const QColor& value)		{	m_textColorDiscrYes1 = value;	}

		const QColor& fillColorDiscrNo0() const				{	return m_fillColorDiscrNo0;		}
		void setFillColorDiscrNo0(const QColor& value)		{	m_fillColorDiscrNo0 = value;	}

		const QColor& fillColorDiscrNo1() const				{	return m_fillColorDiscrNo1;		}
		void setFillColorDiscrNo1(const QColor& value)		{	m_fillColorDiscrNo1 = value;	}

		const QColor& textColorDiscrNo0() const				{	return m_textColorDiscrNo0;		}
		void setTextColorDiscrNo0(const QColor& value)		{	m_textColorDiscrNo0 = value;	}

		const QColor& textColorDiscrNo1() const				{	return m_textColorDiscrNo1;		}
		void setTextColorDiscrNo1(const QColor& value)		{	m_textColorDiscrNo1 = value;	}

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		DECLARE_FONT_PROPERTIES(Font);

		bool drawRect() const;
		void setDrawRect(bool value);

		QString signalId() const;
		void setSignalId(const QString& value);

		E::SignalSource signalSource() const;
		void setSignalSource(E::SignalSource value);

		const QString& textAnalog() const;
		void setTextAnalog(QString value);

		const QString& textDiscreteNo() const;
		void setTextDiscreteNo(QString value);

		const QString& textDiscreteYes() const;
		void setTextDiscreteYes(QString value);

		const QString& textNonValid() const;
		void setTextNonValid(QString value);

		int precision() const;
		void setPrecision(int value);

	private:
		double m_lineWeight = 0.0;
		QColor m_lineColor = {qRgb(0x00, 0x00, 0x00)};

		QColor m_fillColor = {qRgb(0xE0, 0xE0, 0xE0)};	// EditMode colors
		QColor m_textColor = {qRgb(0x00, 0x00, 0x00)};

		// Colors: By priorities
		//
		// Discrete + Analog
		//		NonValid0, NonValid1
		//
		// Analog:
		//		Overflow0, Overflow1
		//		Underflow0, Underflow1
		//		Normal0, Normal1
		//
		// Discrete:
		//		StateNo0, StateNo1
		//		StateYes0, StateYes1
		//
		QColor m_fillColorNonValid0 = {qRgb(0xC0, 0x00, 0x00)};		// Back red, text white
		QColor m_fillColorNonValid1 = {qRgb(0xC0, 0x00, 0x00)};
		QColor m_textColorNonValid0 = {qRgb(0xFF, 0xFF, 0xFF)};
		QColor m_textColorNonValid1 = {qRgb(0xFF, 0xFF, 0xFF)};

//		QColor m_fillColorOverflow0 = {qRgb(0xC0, 0x00, 0x00)};		// Back red, text white
//		QColor m_fillColorOverflow1 = {qRgb(0xC0, 0x00, 0x00)};
//		QColor m_textColorOverflow0 = {qRgb(0xFF, 0xFF, 0xFF)};
//		QColor m_textColorOverflow1 = {qRgb(0xFF, 0xFF, 0xFF)};

//		QColor m_fillColorUnderflow0 = {qRgb(0xC0, 0x00, 0x00)};	// Back red, text white
//		QColor m_fillColorUnderflow1 = {qRgb(0xC0, 0x00, 0x00)};
//		QColor m_textColorUnderflow0 = {qRgb(0xFF, 0xFF, 0xFF)};
//		QColor m_textColorUnderflow1 = {qRgb(0xFF, 0xFF, 0xFF)};

		QColor m_fillColorAnalog0 = {qRgb(0x00, 0x00, 0xC0)};		// Back dark blue, text white
		QColor m_fillColorAnalog1 = {qRgb(0x00, 0x00, 0xC0)};
		QColor m_textColorAnalog0 = {qRgb(0xFF, 0xFF, 0xFF)};
		QColor m_textColorAnalog1 = {qRgb(0xFF, 0xFF, 0xFF)};

		QColor m_fillColorDiscrYes0 = {qRgb(0x00, 0x00, 0xC0)};		// Back dark blue, text white
		QColor m_fillColorDiscrYes1 = {qRgb(0x00, 0x00, 0xC0)};
		QColor m_textColorDiscrYes0 = {qRgb(0xFF, 0xFF, 0xFF)};
		QColor m_textColorDiscrYes1 = {qRgb(0xFF, 0xFF, 0xFF)};

		QColor m_fillColorDiscrNo0 = {qRgb(0x00, 0x00, 0xC0)};		// Back dark blue, text white
		QColor m_fillColorDiscrNo1 = {qRgb(0x00, 0x00, 0xC0)};
		QColor m_textColorDiscrNo0 = {qRgb(0xFF, 0xFF, 0xFF)};
		QColor m_textColorDiscrNo1 = {qRgb(0xFF, 0xFF, 0xFF)};

		// --
		//
		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		QString m_signalId = {"#APPSIGNALID"};
		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		QString m_textAnalog = {"$(value)"};		// $(value)			: signal value
		QString m_textDiscreteNo = {"$(value)"};	// $(caption)		: caption
		QString m_textDiscreteYes = {"$(value)"};	// $(signalid)		: SignalID (CustomSignalID)
		QString m_textNonValid = {"?"};				// $(appsignalid)	: AppSignalID (#APPSIGANLID)
													// $(equipmentid)	: Signal EquipmentID (LM for internal signals, input/output equipment port for IO signals)
													// $(highlimit)		: High limit
													// $(lowlimit)		: Low limit

		int m_precision = -1;		// decimal places after period, -1 means take value from signal

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
