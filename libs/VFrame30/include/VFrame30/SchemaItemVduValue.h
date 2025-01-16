#pragma once

#include <VFrame30/FontParam.h>
#include <VFrame30/PosRectImpl.h>
#include <VFrame30/SchemaItemVdu.h>

namespace VFrame30
{
	class SchemaItemVduValue : public PosRectImpl,
							   public SchemaItemVdu
	{
		Q_OBJECT

	public:
		SchemaItemVduValue(void);
		explicit SchemaItemVduValue(SchemaUnit units);
		virtual ~SchemaItemVduValue(void) = default;

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

	private:
		QString parseText(QStringView text, const VFrame30::AppSignalController* appSignalController) const;

		// VduItemVisitor
		//
	public:
		void accept(VduItemVisitor& visitor) const override;

		// Properties and Data
		//
	public:
		int weight() const;
		void setWeight(int weight);

		bool drawRect() const;
		void setDrawRect(bool value);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		const QString& text() const;
		void setText(const QString& value);

		QString appSignalIdsString() const;
		void setAppSignalIdsString(const QString& value);

		QStringList appSignalIds() const;
		void setAppSignalIds(const QStringList& value);

		int precision() const;
		void setPrecision(int value);

		DECLARE_FONT_PROPERTIES(Font)

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

	private:
		int m_weight = 0;       // Line weight, in pixels
		bool m_drawRect = true; // Rect is visible, thickness 0 is possible

		QColor m_lineColor = qRgb(0xFF, 0xFF, 0xFF);
		QColor m_fillColor = qRgb(0x00, 0x00, 0xC0);
		QColor m_textColor = qRgb(0xFF, 0xFF, 0xFF);

		// clang-format off
		QString m_text; // Text to display, may contain placeholders:
						// Example: "Value %i: %E %u" -> "Value YCB10B23: 1.0E-11 kg"
						// %% - Percent
						// %i - CustomAppSignalID
						// %c - Signal caption
						// %v - Signal value
						// %V - Signal value + unit
						// %s - +/- signal value
						// %S - +/- signal value + unit
						// %u - unit
						// %e - Value in exponential form (1.0e-11)
						// %E - Value in exponential form (1.0E-11)
						// %x - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).
						// %X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).
		// clang-format on

		QStringList m_appSignalIds;

		int m_precision = 0; // Number of digits after the decimal point

		FontParam m_font;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
	};
} // namespace VFrame30
