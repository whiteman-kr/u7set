#pragma once

#include "PosRectImpl.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemRect : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemRect(void);
		explicit SchemaItemRect(SchemaUnit unit);
		virtual ~SchemaItemRect(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

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
