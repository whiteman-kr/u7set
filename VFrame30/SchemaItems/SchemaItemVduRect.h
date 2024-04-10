#pragma once

#include "../FontParam.h"
#include "PosRectImpl.h"

namespace VFrame30
{
	class SchemaItemVduRect : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemVduRect(void);
		explicit SchemaItemVduRect(SchemaUnit units);
		virtual ~SchemaItemVduRect(void) = default;

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

		bool fill() const;
		void setFill(bool fill);

		bool drawRect() const;
		void setDrawRect(bool value);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		DECLARE_FONT_PROPERTIES(Font)

		const QString& text() const;
		void setText(const QString& value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

	private:
		int m_weight = 0;                             // Line weight, in pixels
		bool m_fill = true;                           // Fill rectangle
		bool m_drawRect = true;                       // Rect is visible, thickness 0 is possible

		QColor m_lineColor = qRgb(0x00, 0x00, 0x00);
		QColor m_fillColor = qRgb(0xC0, 0xC0, 0xC0);
		QColor m_textColor = qRgb(0x00, 0x00, 0x00);

		FontParam m_font;

		QString m_text;

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
	};
} // namespace VFrame30
