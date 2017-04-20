#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemValue : public PosRectImpl
	{
		Q_OBJECT

		Q_PROPERTY(QString Text READ analogText WRITE setAnalogText)
		Q_PROPERTY(QString analogText READ analogText WRITE setAnalogText)

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

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Text search
		//
	public:
		virtual bool searchText(const QString& analogText) const override;

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

		const QString& analogText() const;
		void setAnalogText(QString value);

	private:
		double m_lineWeight = 0.0;
		QColor m_lineColor = {qRgb(0x00, 0x00, 0x00)};
		QColor m_fillColor = {qRgb(0xE0, 0xE0, 0xE0)};
		QColor m_textColor = {qRgb(0x00, 0x00, 0x00)};

		QString m_analogText = {"%v"};

		E::HorzAlign m_horzAlign = E::HorzAlign::AlignHCenter;
		E::VertAlign m_vertAlign = E::VertAlign::AlignVCenter;
		FontParam m_font;
		bool m_drawRect = true;				// Rect is visible, thikness 0 is possible

		QString m_signalId;

		E::SignalSource m_signalSource = E::SignalSource::AppDataService;

		// FillColor 1/2
		// TextColor 1/2
		// DiscreteText 0/1
		//

		// Drawing resources
		//
		mutable std::shared_ptr<QPen> m_rectPen;
		mutable std::shared_ptr<QBrush> m_fillBrush;
	};
}
