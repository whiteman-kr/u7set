#pragma once

#include "PosRectImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class VideoItemInputSignal;
	class VideoItemOutputSignal;
	class VideoItemFblElement;
}

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FblItemRect : public PosRectImpl, public FblItem
	{
		Q_OBJECT

		Q_PROPERTY(QString FontName READ getFontName WRITE setFontName)
		Q_PROPERTY(double FontSize READ getFontSize WRITE setFontSize)
		Q_PROPERTY(bool FontBold READ getFontBold WRITE setFontBold)
		Q_PROPERTY(bool FontItalic READ getFontItalic WRITE setFontItalic)

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<VideoItem>::DerivedType<FblItemRect>;
#endif

	protected:
		FblItemRect(void);
		FblItemRect(SchemeUnit itemunit);
	public:
		virtual ~FblItemRect(void);
		
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
		virtual void Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* layer) const override;

		// Вычислить координаты точки
		//
		virtual void SetConnectionsPos() override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const override;

		///<summary> 
		/// Вычисление координат точки, для прямоугольного Fbl элемента
		///</summary>
		VideoItemPoint CalcPointPos(const QRectF& fblItemRect, const CFblConnectionPoint& connection, int pinCount, int index) const;


		// Properties and Data
		//
	public:
		virtual bool IsFblItem() const override;

		bool isInputSignalElement() const;
		bool isOutputSignalElement() const;
		bool isSignalElement() const;
		bool isFblElement() const;

		VFrame30::VideoItemInputSignal* toInputSignalElement();
		const VFrame30::VideoItemInputSignal* toInputSignalElement() const;

		VFrame30::VideoItemOutputSignal* toOutputSignalElement();
		const VFrame30::VideoItemOutputSignal* toOutputSignalElement() const;

		VideoItemFblElement* toFblElement();
		const VFrame30::VideoItemFblElement* toFblElement() const;

		double weight() const;
		void setWeight(double weight);

		QRgb lineColor() const;
		void setLineColor(QRgb color);

		QRgb fillColor() const;
		void setFillColor(QRgb color);

		QRgb textColor() const;
		void setTextColor(QRgb color);

		DECLARE_FONT_PROPERTIES(Font);
		
	protected:
		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QRgb m_textColor;
		FontParam m_font;
	};
}


