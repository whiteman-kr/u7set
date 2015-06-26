#pragma once

#include "PosRectImpl.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemeItemRect : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)
		Q_PROPERTY(bool Fill READ fill WRITE setFill)
		//Q_PROPERTY(QRgb LineColor READ lineColor WRITE setLineColor)
		//Q_PROPERTY(QRgb FillColor....)
		Q_PROPERTY(QString Text READ text WRITE setText)
		//Q_PROPERTY(QRgb TextColor....)
		Q_PROPERTY(bool DrawRect READ drawRect WRITE setDrawRect)

		Q_PROPERTY(QString FontName READ getFontName WRITE setFontName)
		Q_PROPERTY(double FontSize READ getFontSize WRITE setFontSize)
		Q_PROPERTY(bool FontBold READ getFontBold WRITE setFontBold)
		Q_PROPERTY(bool FontItalic READ getFontItalic WRITE setFontItalic)

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemRect>;
#endif

		SchemeItemRect(void);

	public:
		explicit SchemeItemRect(SchemeUnit unit);
		virtual ~SchemeItemRect(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const override;

		// Properties and Data
		//
	public:
		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

		QRgb fillColor() const;
		void setFillColor(QRgb color);

		const QString& text() const;
		void setText(QString& value);

		QRgb textColor() const;
		void setTextColor(QRgb color);
		
		DECLARE_FONT_PROPERTIES(Font);

		bool fill() const;
		void setFill(bool fill);

		bool drawRect() const;
		void setDrawRect(bool value);

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QString m_text;
		QRgb m_textColor;
		FontParam m_font;
		bool m_fill;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
