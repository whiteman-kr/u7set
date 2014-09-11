#pragma once

#include "PosRectImpl.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CVideoItemRect : public CPosRectImpl
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemRect>;
#endif

	private:
		CVideoItemRect(void);
	public:
		explicit CVideoItemRect(SchemeUnit unit);
		virtual ~CVideoItemRect(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(::Proto::Envelope* message) const override;
		virtual bool LoadData(const ::Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const override;

		// Properties and Data
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

	private:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QString m_text;
		QRgb m_textColor;
		FontParam m_font;
		bool m_fill;

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
