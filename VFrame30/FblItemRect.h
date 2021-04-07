#pragma once

#include "PosRectImpl.h"
#include "FblItem.h"
#include "FontParam.h"
#include "../lib/CUtils.h"

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
	class SchemaItemTerminator;
	class SchemaItemBusComposer;
	class SchemaItemBusExtractor;
	class SchemaItemLoopbackSource;
	class SchemaItemLoopbackTarget;
}

namespace VFrame30
{
	static const double BusSideLineWidth = mm2in(0.4);


	class FblItemRect : public PosRectImpl, public FblItem
	{
		Q_OBJECT

	public:
		FblItemRect(void);
		FblItemRect(SchemaUnit itemunit);
		virtual ~FblItemRect(void);

	protected:
		virtual void propertyDemand(const QString& prop) override;

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
		virtual void draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* layer) const override;

		// Draw item's label
		//
		virtual void drawLabel(CDrawParam* drawParam) const override;

		// Draw debug info
		//
		virtual void drawDebugInfo(CDrawParam* drawParam, const QString& runOrderIndex) const override;

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
	protected:
		void drawMultichannelSlashLines(CDrawParam* drawParam, QPen& linePen) const;

		// Other public methods
		//
	public:
		QRectF itemRectWithPins(CDrawParam* drawParam) const;								// Get item rect with inputs and outputs
		QRectF itemRectPinIndent(CDrawParam* drawParam) const;		// Get item rect without inputs and outputs

		Q_INVOKABLE void adjustHeight(double gridSize = -1, int pinGridStep = -1);

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		virtual void dump() const override;

		// Properties and Data
		//
	public:
		virtual void setNewGuid() override;		// set new GUID for item, for it's pins etc, useful for copy (mousemove + ctrl)

		bool isInputSignalElement() const;
		bool isOutputSignalElement() const;
		bool isInOutSignalElement() const;
		bool isSignalElement() const;
		bool isConstElement() const;
		bool isAfbElement() const;
		bool isConnectionElement() const;
		bool isReceiverElement() const;
		bool isTransmitterElement() const;
		bool isTerminatorElement() const;
		bool isBusComposerElement() const;
		bool isBusExtractorElement() const;
		bool isLoopbackSourceElement() const;
		bool isLoopbackTargetElement() const;

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

		VFrame30::SchemaItemTerminator* toTerminatorElement();
		const VFrame30::SchemaItemTerminator* toTerminatorElement() const;

		VFrame30::SchemaItemBusComposer* toBusComposerElement();
		const VFrame30::SchemaItemBusComposer* toBusComposerElement() const;

		VFrame30::SchemaItemBusExtractor* toBusExtractorElement();
		const VFrame30::SchemaItemBusExtractor* toBusExtractorElement() const;

		VFrame30::SchemaItemLoopbackSource* toLoopbackSourceElement();
		const VFrame30::SchemaItemLoopbackSource* toLoopbackSourceElement() const;

		VFrame30::SchemaItemLoopbackTarget* toLoopbackTargetElement();
		const VFrame30::SchemaItemLoopbackTarget* toLoopbackTargetElement() const;

		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

		QColor fillColor() const;
		void setFillColor(QColor color);

		QColor textColor() const;
		void setTextColor(QColor color);

		DECLARE_FONT_PROPERTIES(Font)

		QString userText() const;
		void setUserText(QString value);

		E::TextPos userTextPos() const;
		void setUserTextPos(E::TextPos value);

	protected:
		// m_gridSize and m_pingGridStep are cached values from Schema, they set in CalcPointPos.
		// We need these variables in case we call functions and do not have schema pointer.
		// This is not good, it is subject to change.
		//
		mutable double m_cachedGridSize = -1;			// -1 means it is not iniotialized
		mutable int m_cachedPinGridStep = 0;

		double m_weight;								// Line weight, is kept in pixels or inches depends on UnitDocPt
		QRgb m_lineColor;
		QRgb m_fillColor;
		QRgb m_textColor;
		FontParam m_font;

		QString m_userText;
		E::TextPos m_userTextPos = E::TextPos::Top;
	};
}


