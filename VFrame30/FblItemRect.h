#pragma once

#include "PosRectImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class VideoItemSignal;
	class VideoItemInputSignal;
	class VideoItemOutputSignal;
	class SchemeItemConst;
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
		friend ::Factory<SchemeItem>::DerivedType<FblItemRect>;
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
		virtual void SetConnectionsPos(double gridSize, int pinGridStep) override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemePoint* pResult, double gridSize, int pinGridStep) const override;

		///<summary> 
		/// Вычисление координат точки, для прямоугольного Fbl элемента
		///</summary>
		SchemePoint CalcPointPos(const QRectF& fblItemRect,
									const CFblConnectionPoint& connection,
									int pinCount,
									int index,
									double gridSize, int pinGridStep) const;

		// Other public methods
		//
	public:
		Q_INVOKABLE void adjustHeight();

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const;

		// Properties and Data
		//
	public:
		virtual void setNewGuid() override;		// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		virtual bool IsFblItem() const override;

		bool isInputSignalElement() const;
		bool isOutputSignalElement() const;
		bool isSignalElement() const;
		bool isConstElement() const;
		bool isFblElement() const;

		VFrame30::VideoItemSignal *toSignalElement();
		const VFrame30::VideoItemSignal* toSignalElement() const;

		VFrame30::VideoItemInputSignal* toInputSignalElement();
		const VFrame30::VideoItemInputSignal* toInputSignalElement() const;

		VFrame30::VideoItemOutputSignal* toOutputSignalElement();
		const VFrame30::VideoItemOutputSignal* toOutputSignalElement() const;

		VFrame30::SchemeItemConst* toSchemeItemConst();
		const VFrame30::SchemeItemConst* toSchemeItemConst() const;

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
		// m_gridSize and m_pingGridStep are cached values from Scheme, they set in CalcPointPos.
		// We need these variables in case we call functions and do not have scheme pointer.
		// This is not good, it is subject to change.
		//
		mutable double m_cachedGridSize = -1;			// -1 means it is not iniotialized
		mutable double m_cachedPinGridStep = 0;

		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QRgb m_textColor;
		FontParam m_font;
	};
}


