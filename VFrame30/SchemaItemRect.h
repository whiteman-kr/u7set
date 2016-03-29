#pragma once

#include "PosRectImpl.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemRect : public PosRectImpl
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemRect>;
#endif

		SchemaItemRect(void);

	public:
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

		// Properties and Data
		//
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		const QString& text() const;
		void setText(QString value);

		QColor textColor() const;
		void setTextColor(QColor color);
		
		DECLARE_FONT_PROPERTIES(Font);

		bool fill() const;
		void setFill(bool fill);

		bool drawRect() const;
		void setDrawRect(bool value);

	private:
		double m_weight;					// ������� �����, �������� � ������ ��� ������ � ����������� �� UnitDocPt
		QColor m_lineColor;
		QColor m_fillColor;
		QColor m_textColor;
		QString m_text;
		FontParam m_font;
		bool m_fill;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
