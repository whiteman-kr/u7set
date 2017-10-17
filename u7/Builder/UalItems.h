#pragma once

#include <QtCore>

#include "../lib/Signal.h"
#include "../lib/WUtils.h"

#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemBus.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"


#include "Parser.h"
#include "ApplicationLogicCode.h"

namespace Builder
{
	// typedefs Logic* for types defined outside ModuleLogicCompiler
	//
	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::AfbPin LogicPin;

	typedef VFrame30::SchemaItemSignal LogicSignal;
	typedef VFrame30::SchemaItemAfb LogicFb;
	typedef VFrame30::SchemaItemConst UalConst;

	typedef VFrame30::SchemaItemTransmitter LogicTransmitter;
	typedef VFrame30::SchemaItemReceiver LogicReceiver;

	typedef VFrame30::SchemaItemBusComposer UalBusComposer;
	typedef VFrame30::SchemaItemBusExtractor UalBusExtractor;

	typedef Afb::AfbSignal LogicAfbSignal;
	typedef Afb::AfbParam LogicAfbParam;

	class UalItem;
	class ModuleLogicCompiler;

	class Afbl
	{
		// Functional Block Library element
		//
	public:
		Afbl(std::shared_ptr<Afb::AfbElement> afb);
		virtual ~Afbl();

		bool hasRam() const { return m_afb->hasRam(); }
		int opCode() const	{ return m_afb->opCode(); }

		const Afb::AfbElement& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }

		bool isBusProcessingAfb() const;

	private:
		bool isBusProcessingAfbChecking() const;

	private:
		std::shared_ptr<Afb::AfbElement> m_afb;

		mutable int m_isBusProcessingAfb = -1;			// -1 - isBusProcessingAfb() is not previously called
														//  0 - afb is not bus processing element
														//  1 - afb is bus processing element
	};

	typedef QHash<int, int> FblInstanceMap;				// Key is OpCode
	typedef QHash<QString, int> NonRamFblInstanceMap;

	class UalAfb;

	class AfblsMap : public HashedVector<QString, Afbl*>
	{
	public:
		virtual ~AfblsMap();

		bool addInstance(UalAfb* ualAfb);
		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);

	private:

		struct StrIDIndex
		{
			QString strID;		// AfbElement strID()
			int index;			// AfbElementSignal or AfbElementParam index

			operator QString() const { return QString("%1:%2").arg(strID).arg(index); }
		};

		FblInstanceMap m_fblInstance;						// Fbl opCode -> current instance
		NonRamFblInstanceMap m_nonRamFblInstance;			// Non RAM Fbl StrID -> instance

		QHash<QString, LogicAfbSignal> m_afbSignals;
		QHash<QString, LogicAfbParam*> m_afbParams;
	};

	class UalItem : public QObject
	{
		Q_OBJECT

	public:
		enum Type
		{
			Unknown,
			Signal,
			Afb,
			Const,
			Transmitter,
			Receiver,
			Terminator,
			BusComposer,
			BusExtractor,
			LoopbackOutput,
			LoopbackInput
		};

		UalItem();
		UalItem(const UalItem& ualItem);
		UalItem(const AppLogicItem& appLogicItem);
		UalItem(std::shared_ptr<Afb::AfbElement> afbElement, QString &errorMsg);

		bool init(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.m_afbElement.strID(); }
		QString caption() const { return m_appLogicItem.m_afbElement.caption(); }

		QString strID() const;

		bool isSignal() const { return type() == UalItem::Type::Signal; }
		bool isAfb() const { return type() == UalItem::Type::Afb; }
		bool isConst() const { return type() == UalItem::Type::Const; }
		bool isTransmitter() const { return type() == UalItem::Type::Transmitter; }
		bool isReceiver() const { return type() == UalItem::Type::Receiver; }
		bool isTerminator() const { return type() == UalItem::Type::Terminator; }
		bool isBusComposer() const { return type() == UalItem::Type::BusComposer; }
		bool isBusExtractor() const { return type() == UalItem::Type::BusExtractor; }
		bool isLoopbackOutput() const { return type() == UalItem::Type::LoopbackOutput; }
		bool isLoopbackInput() const { return type() == UalItem::Type::LoopbackInput; }

		Type type() const;

		bool hasRam() const { return afb().hasRam(); }

		const std::vector<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::vector<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.m_afbElement.params(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toAfbElement(); }
		const UalConst* ualConst() const { return m_appLogicItem.m_fblItem->toSchemaItemConst(); }
		const LogicTransmitter& logicTransmitter() const { return *m_appLogicItem.m_fblItem->toTransmitterElement(); }
		const LogicReceiver& logicReceiver() const { return *m_appLogicItem.m_fblItem->toReceiverElement(); }

		const UalBusComposer* ualBusComposer() const { return m_appLogicItem.m_fblItem->toBusComposerElement(); }
		const UalBusExtractor* ualBusExtractor() const { return m_appLogicItem.m_fblItem->toBusExtractorElement(); }

		const Afb::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }

		std::shared_ptr<VFrame30::FblItemRect> itemRect() const { return m_appLogicItem.m_fblItem; }

		QString schemaID() const { return m_appLogicItem.m_schema->schemaId(); }

		QString label() const { return m_appLogicItem.m_fblItem->label(); }

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }

	protected:
		AppLogicItem m_appLogicItem;							// structure from parser

	private:
		mutable UalItem::Type m_type = UalItem::Type::Unknown;

		QHash<QString, int> m_opNameToIndexMap;
	};

	typedef std::map<QUuid, UalItem*> ConnectedAppItems;		// connected pin Uuid => AppItem*

	class AppFbParamValue
	{
	public:
		static const int NOT_FB_OPERAND_INDEX = -1;

	public:
		AppFbParamValue();
		AppFbParamValue(const Afb::AfbParam& afbParam);

		bool isUnsignedInt() const { return m_dataFormat == E::DataFormat::UnsignedInt; }
		bool isUnsignedInt16() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_16BIT; }
		bool isUnsignedInt32() const { return m_dataFormat == E::DataFormat::UnsignedInt && m_dataSize == SIZE_32BIT; }
		bool isSignedInt32() const { return m_dataFormat == E::DataFormat::SignedInt && m_dataSize == SIZE_32BIT; }
		bool isFloat32() const { return m_dataFormat == E::DataFormat::Float && m_dataSize == SIZE_32BIT; }
		bool isVisible() const { return m_visible; }
		bool isNoFbOperand() const { return m_operandIndex == NOT_FB_OPERAND_INDEX; }

		bool instantiator() const { return m_instantiator; }

		E::SignalType type() const { return m_type; }
		E::DataFormat dataFormat() const { return m_dataFormat; }
		int dataSize() const { return m_dataSize; }

		int operandIndex() const { return m_operandIndex; }
		QString opName() const { return m_opName; }
		QString caption() const { return m_caption; }

		quint32 unsignedIntValue() const;
		void setUnsignedIntValue(quint32 value);

		qint32 signedIntValue() const;
		void setSignedIntValue(qint32 value);

		double floatValue() const;
		void setFloatValue(double value);

		QString toString() const;

	private:
		E::SignalType m_type = E::SignalType::Discrete;
		E::DataFormat m_dataFormat = E::DataFormat::UnsignedInt;
		bool m_instantiator = false;
		int m_dataSize = 1;
		QString m_opName;
		QString m_caption;
		int m_operandIndex = NOT_FB_OPERAND_INDEX;
		bool m_visible = false;

		quint32 m_unsignedIntValue = 0;
		qint32 m_signedIntValue = 0;
		double m_floatValue = 0;
	};

	typedef HashedVector<QString, AppFbParamValue> AppFbParamValuesArray;

	class ModuleLogicCompiler;

	class UalAfb : public UalItem
	{
		// Application Functional Block
		// represent all FB items in application logic schemas
		//
	public:
		static const int FOR_USER_ONLY_PARAM_INDEX = -1;				// index of FB's parameters used by user only

	public:
		UalAfb(const UalItem &appItem);

		quint16 instance() const { return m_instance; }
		quint16 opcode() const { return afb().opCode(); }		// return FB type
		QString caption() const { return afb().caption(); }
		QString typeCaption() const { return afb().componentCaption(); }
		int number() const { return m_number; }

		bool isConstComaparator() const;
		bool isDynamicComaparator() const;
		bool isComparator() const;

		QString instantiatorID();

		void setInstance(quint16 instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, LogicAfbParam* afbParam) const;
		bool getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const;
		bool getAfbSignalByPin(const LogicPin& pin, LogicAfbSignal* afbSignal) const { return getAfbSignalByIndex(pin.afbOperandIndex(), afbSignal); }
		bool getAfbSignalByPinUuid(QUuid pinUuid, LogicAfbSignal* afbSignal) const;

		bool calculateFbParamValues(ModuleLogicCompiler* compiler);			// implemented in file FbParamCalculation.cpp

		const AppFbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

		int runTime() const { return m_runTime; }

	private:
		bool isBusProcessingAfbChecking() const;

		// FB's parameters values and runtime calculations
		// implemented in file FbParamCalculation.cpp
		//
		bool calculate_LOGIC_paramValues();
		bool calculate_NOT_paramValues();
		bool calculate_FLIP_FLOP_paramValues();
		bool calculate_CTUD_paramValues();
		bool calculate_MAJ_paramValues();
		bool calculate_SRSST_paramValues();
		bool calculate_BCOD_paramValues();
		bool calculate_BDEC_paramValues();
		bool calculate_MATH_paramValues();
		bool calculate_TCT_paramValues();
		bool calculate_BCOMP_paramValues();
		bool calculate_SCALE_paramValues();
		bool calculate_SCALE_P_paramValues();
		bool calculate_DAMPER_paramValues();
		bool calculate_MEM_paramValues();
		bool calculate_FUNC_paramValues();
		bool calculate_INT_paramValues();
		bool calculate_DPCOMP_paramValues();
		bool calculate_MUX_paramValues();
		bool calculate_LATCH_paramValues();
		bool calculate_LIM_paramValues();
		bool calculate_DEAD_ZONE_paramValues();
		bool calculate_POL_paramValues();
		bool calculate_DER_paramValues();
		bool calculate_MISMATCH_paramValues();
		bool calculate_TCONV_paramValues();

		//

		bool checkRequiredParameters(const QStringList& requiredParams);
		bool checkRequiredParameters(const QStringList& requiredParams, bool displayError);

		bool checkUnsignedInt(const AppFbParamValue& paramValue);
		bool checkUnsignedInt16(const AppFbParamValue& paramValue);
		bool checkUnsignedInt32(const AppFbParamValue& paramValue);
		bool checkSignedInt32(const AppFbParamValue& paramValue);
		bool checkFloat32(const AppFbParamValue& paramValue);

	private:
		quint16 m_instance = -1;
		int m_number = -1;
		QString m_instantiatorID;

		AppFbParamValuesArray m_paramValuesArray;

		ModuleLogicCompiler* m_compiler = nullptr;
		IssueLogger* m_log = nullptr;

		int m_runTime = 0;

		const quint16 CONST_COMPARATOR_OPCODE = 10;
		const quint16 DYNAMIC_COMPARATOR_OPCODE = 20;
	};


	class UalAfbsMap: public HashedVector<QUuid, UalAfb*>
	{
	public:
		virtual ~UalAfbsMap();

		UalAfb* insert(UalAfb *appFb);
		void clear();

	private:
		int m_fbNumber = 1;
	};

	class UalSignal
	{
		// Application Signal
		// represent all signal in application logic schemas, and signals, which createad in compiling time
		//
	public:
		UalSignal(Signal* s);
		UalSignal(const QUuid& guid, E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, int dataSize, const UalItem* appItem, const QString& appSignalID);
		UalSignal(const QUuid& guid, const QString &signalID, const QString& busTypeID, int busSizeW);

		~UalSignal();

		bool appendSignalRef(Signal* s);

		void setComputed() { m_computed = true; }
		bool isComputed() const { return m_computed; }

		void setResultSaved() { m_resultSaved = true; }
		bool isResultSaved() const { return m_resultSaved; }

		bool isAutoSignal() const { return m_isAutoSignal; }

		QString appSignalID() const { return m_signals[0]->appSignalID(); }

		Address16 ualAddr() const { return m_ualAddr; }
		Address16 regBufAddr() const { return m_regBufAddr; }

		E::SignalType signalType() const { return m_signals[0]->signalType(); }
		E::AnalogAppSignalFormat analogSignalFormat() const { return m_signals[0]->analogSignalFormat(); }
		int dataSize() const { return m_signals[0]->dataSize(); }

		bool isAnalog() const { return m_signals[0]->isAnalog(); }
		bool isDiscrete() const { return m_signals[0]->isDiscrete(); }
		bool isBus() const { return m_signals[0]->isBus(); }

		bool isAcquired() const { return m_signals[0]->isAcquired(); }
		bool isInternal() const { return m_signals[0]->isInternal(); }
		bool isInput() const { return m_signals[0]->isInput(); }
		bool isOutput() const { return m_signals[0]->isOutput(); }
		bool enableTuning() const { return m_signals[0]->enableTuning(); }

		QString busTypeID() const { return m_signals[0]->busTypeID(); }

		bool isCompatibleDataFormat(const LogicAfbSignal& afbSignal) const;

		const Signal& constSignal() { return *m_signals[0]; }

		Signal* signal() { return m_signals[0]; }
		const Signal* signal() const { return m_signals[0]; }

	private:
		QVector<Signal*> m_signals;							// vector of pointers to signal in m_signalSet

		bool m_isAutoSignal = false;

		bool m_computed = false;
		bool m_resultSaved = false;

		Address16 m_ualAddr;
		Address16 m_regBufAddr;
	};

	class UalSignalsMap: public QObject, private QHash<UalSignal*, UalSignal*>
	{
		Q_OBJECT
	public:
		UalSignalsMap(ModuleLogicCompiler& compiler, IssueLogger* log);
		~UalSignalsMap();

		QHash<UalSignal*, UalSignal*>::iterator begin() { return QHash<UalSignal*, UalSignal*>::begin(); }
		QHash<UalSignal*, UalSignal*>::const_iterator begin() const { return QHash<UalSignal*, UalSignal*>::begin(); }
		QHash<UalSignal*, UalSignal*>::iterator end() { return QHash<UalSignal*, UalSignal*>::end(); }
		QHash<UalSignal*, UalSignal*>::const_iterator end() const { return QHash<UalSignal*, UalSignal*>::end(); }

		UalSignal* createInputSignal(Signal* s, QUuid outPinUuid);

		bool appendLink(QUuid pinUuid, UalSignal* ualSignal);
		bool appendSignalRef(UalSignal* ualSignal, Signal* s);

		UalSignal* get(const QString& appSignalID) const { return m_idToSignalMap.value(appSignalID, nullptr); }
		bool contains(const QString& appSignalID) const { return m_idToSignalMap.contains(appSignalID); }

		UalSignal* get(QUuid pinUuid) const { return m_pinToSignalMap.value(pinUuid, nullptr); }
		bool contains(QUuid pinUuid) const { return m_pinToSignalMap.contains(pinUuid); }



		bool insertUalSignal(const UalItem* appItem);
		bool insertNonBusAutoSignal(const UalAfb* appFb, const LogicPin& outputPin);
		bool insertBusAutoSignal(const UalItem* appItem, const LogicPin& outputPin, BusShared bus);


		void clear();

	private:
		bool insertNew(QUuid pinUuid, UalSignal* newUalSignal);

		QString getAutoSignalID(const UalItem* appItem, const LogicPin& outputPin);

		const UalSignal* value(const UalSignal* &key, const UalSignal* &defaultValue) const;


	private:
		ModuleLogicCompiler& m_compiler;
		IssueLogger* m_log = nullptr;

		//

		QHash<QString, UalSignal*> m_idToSignalMap;
		QHash<QUuid, UalSignal*> m_pinToSignalMap;
	};

}
