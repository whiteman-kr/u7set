#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemRect : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(double LineWeight READ weight WRITE setWeight)

		Q_PROPERTY(QColor LineColor READ lineColor WRITE setLineColor)
		Q_PROPERTY(QColor FillColor READ fillColor WRITE setFillColor)

		Q_PROPERTY(double Fill READ fill WRITE setFill)
		Q_PROPERTY(double DrawRect READ drawRect WRITE setDrawRect)

		// Text Category Properties

		Q_PROPERTY(QColor TextColor READ textColor WRITE setTextColor)

		Q_PROPERTY(QString Text READ text WRITE setText)

		Q_PROPERTY(E::HorzAlign AlignHorz READ horzAlign WRITE setHorzAlign)
		Q_PROPERTY(E::VertAlign AlignVert READ vertAlign WRITE setVertAlign)

		Q_PROPERTY(QString FontName READ getFontName WRITE setFontName)
		Q_PROPERTY(double FontSize READ getFontSize WRITE setFontSize)
		Q_PROPERTY(bool FontBold READ getFontBold WRITE setFontBold)
		Q_PROPERTY(bool FontItalic READ getFontItalic WRITE setFontItalic)


	public:
		SchemaItemRect(void);
		explicit SchemaItemRect(SchemaUnit unit);
		virtual ~SchemaItemRect(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

		// Properties and Data
		//
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		const QString& text() const;
		void setText(QString value);

		E::HorzAlign horzAlign() const;
		void setHorzAlign(E::HorzAlign align);

		E::VertAlign vertAlign() const;
		void setVertAlign(E::VertAlign align);

		DECLARE_FONT_PROPERTIES(Font);

		bool fill() const;
		void setFill(bool fill);

		bool drawRect() const;
		void setDrawRect(bool value);

	private:
		double m_weight = 0.0;				// Line weight, in pixels or inches depends on UnitDocPt
		QColor m_lineColor;
		QColor m_fillColor;
		QColor m_textColor;
		QString m_text;
		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_fill = true;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
