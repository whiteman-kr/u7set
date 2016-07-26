#pragma once

#include "PosRectImpl.h"
#include "FblItem.h"

namespace VFrame30
{
	class SchemaItemSignal;
	class SchemaItemInput;
	class SchemaItemOutput;
	class SchemaItemConst;
	class SchemaItemAfb;
	class SchemaItemInOut;
	class SchemaItemConnection;
	class SchemaItemReceiver;
	class SchemaItemTransmitter;
}

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT FblItemRect : public PosRectImpl, public FblItem
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<FblItemRect>;
#endif

	protected:
		FblItemRect(void);
		FblItemRect(SchemaUnit itemunit);
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
		virtual void Draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* layer) const override;

		// Draw debug info
		//
		virtual void DrawDebugInfo(CDrawParam* drawParam, const QString& runOrderIndex) const override;

		// Вычислить координаты точки
		//
		virtual void SetConnectionsPos(double gridSize, int pinGridStep) override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const override;

		// Вычисление координат точки, для прямоугольного Fbl элемента
		//
		SchemaPoint CalcPointPos(const QRectF& fblItemRect,
								 const AfbPin& connection,
								 int pinCount,
								 int index,
								 double gridSize, int pinGridStep) const;

		// Other public methods
		//
	public:
		QRectF itemRectWithPins() const;		// Get item rect with inputs and outputs
		QRectF itemRectPinIndent(QPaintDevice* paintDevice) const;		// Get item rect without inputs and outputs

		Q_INVOKABLE void adjustHeight();

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		virtual void debug() const override;

		// Properties and Data
		//
	public:
		virtual void setNewGuid() override;		// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		virtual bool IsFblItem() const override;

		bool isInputSignalElement() const;
		bool isOutputSignalElement() const;
		bool isInOutSignalElement() const;
		bool isSignalElement() const;
		bool isConstElement() const;
		bool isAfbElement() const;
		bool isConnectionElement() const;
		bool isReceiverElement() const;
		bool isTransmitterElement() const;

		VFrame30::SchemaItemSignal* toSignalElement();
		const VFrame30::SchemaItemSignal* toSignalElement() const;

		VFrame30::SchemaItemInput* toInputSignalElement();
		const VFrame30::SchemaItemInput* toInputSignalElement() const;

		VFrame30::SchemaItemOutput* toOutputSignalElement();
		const VFrame30::SchemaItemOutput* toOutputSignalElement() const;

		VFrame30::SchemaItemConst* toSchemaItemConst();
		const VFrame30::SchemaItemConst* toSchemaItemConst() const;

		SchemaItemAfb* toAfbElement();
		const VFrame30::SchemaItemAfb* toAfbElement() const;

		VFrame30::SchemaItemInOut* toInOutSignalElement();
		const VFrame30::SchemaItemInOut* toInOutSignalElement() const;

		VFrame30::SchemaItemReceiver* toReceiverElement();
		const VFrame30::SchemaItemReceiver* toReceiverElement() const;

		VFrame30::SchemaItemTransmitter* toTransmitterElement();
		const VFrame30::SchemaItemTransmitter* toTransmitterElement() const;

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
		// m_gridSize and m_pingGridStep are cached values from Schema, they set in CalcPointPos.
		// We need these variables in case we call functions and do not have schema pointer.
		// This is not good, it is subject to change.
		//
		mutable double m_cachedGridSize = -1;			// -1 means it is not iniotialized
		mutable int m_cachedPinGridStep = 0;

		double m_weight;					// Толщина линии, хранится в точках или дюймах в зависимости от UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QRgb m_textColor;
		FontParam m_font;
	};
}


