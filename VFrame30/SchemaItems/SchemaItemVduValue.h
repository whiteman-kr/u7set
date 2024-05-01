#pragma once

#include "PosRectImpl.h"


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

		const QString& fontName() const;
		void setFontName(const QString& value);

		const QString& appSignalId() const;
		void setAppSignalId(const QString& value);

	private:
		int m_weight = 0;                             // Line weight, in pixels
		bool m_drawRect = true;                       // Rect is visible, thickness 0 is possible

		QColor m_lineColor = qRgb(0xFF, 0xFF, 0xFF);
		QColor m_fillColor = qRgb(0x00, 0x00, 0xC0);
		QColor m_textColor = qRgb(0xFF, 0xFF, 0xFF);

		QString m_fontName = QStringLiteral("Arial"); // FontName, should be something like "Arial_16"

		QString m_appSignalId;
	};
} // namespace VFrame30
