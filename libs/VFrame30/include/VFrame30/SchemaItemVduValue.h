#pragma once

#include <VFrame30/FontParam.h>
#include <VFrame30/PosRectImpl.h>

namespace VFrame30
{
	class SchemaItemVduValue : public PosRectImpl
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

		const QString& appSignalId() const;
		void setAppSignalId(const QString& value);

		int precision() const;
		void setPrecision(int value);

		DECLARE_FONT_PROPERTIES(Font)

	private:
		int m_weight = 0;                             // Line weight, in pixels
		bool m_drawRect = true;                       // Rect is visible, thickness 0 is possible

		QColor m_lineColor = qRgb(0xFF, 0xFF, 0xFF);
		QColor m_fillColor = qRgb(0x00, 0x00, 0xC0);
		QColor m_textColor = qRgb(0xFF, 0xFF, 0xFF);

		QString m_appSignalId;

		int m_precision	= 0;                          // Number of digits after the decimal point

		FontParam m_font;
	};
} // namespace VFrame30
