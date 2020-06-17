#pragma once

#include "../lib/Signal.h"
#include "../lib/WUtils.h"

#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemBus.h"
#include "../VFrame30/SchemaItemLoopback.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"

#include "Parser.h"
#include "ApplicationLogicCode.h"
#include "Busses.h"
#include "SignalsHeap.h"

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
	typedef VFrame30::SchemaItemReceiver UalReceiver;

	typedef VFrame30::SchemaItemBusComposer UalBusComposer;
	typedef VFrame30::SchemaItemBusExtractor UalBusExtractor;

	typedef VFrame30::SchemaItemLoopbackSource UalLoopbackSource;
	typedef VFrame30::SchemaItemLoopbackTarget UalLoopbackTarget;

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

		int opCode() const	{ return m_afb->opCode(); }

		const Afb::AfbElement& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }

		bool isBusProcessingAfb() const;

		int maxInstances() const { return m_afb->component()->maxInstCount(); }
		int version() const { return m_afb->component()->impVersion(); }
		QString componentCaption() const { return m_afb->component()->caption(); }

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

		bool addInstance(UalAfb* ualAfb, IssueLogger *log);
		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);

		int getUsedInstances(int opCode) const;

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
		UalItem();
		UalItem(const UalItem& ualItem);
		UalItem(const AppLogicItem& appLogicItem);
		UalItem(std::shared_ptr<Afb::AfbElement> afbElement, QString &errorMsg);

		bool init(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.afbElement().strID(); }
		QString caption() const { return m_appLogicItem.afbElement().caption(); }

		QString strID() const;

		bool isSignal() const { return type() == E::UalItemType::Signal; }
		bool isAfb() const { return type() == E::UalItemType::Afb; }
		bool isConst() const { return type() == E::UalItemType::Const; }
		bool isTransmitter() const { return type() == E::UalItemType::Transmitter; }
		bool isReceiver() const { return type() == E::UalItemType::Receiver; }
		bool isTerminator() const { return type() == E::UalItemType::Terminator; }
		bool isBusComposer() const { return type() == E::UalItemType::BusComposer; }
		bool isBusExtractor() const { return type() == E::UalItemType::BusExtractor; }
		bool isLoopbackSource() const { return type() == E::UalItemType::LoopbackSource; }
		bool isLoopbackTarget() const { return type() == E::UalItemType::LoopbackTarget; }
		bool isSetFlagsItem() const;
		bool isSimLockItem() const;
		bool isMismatchItem() const;

		bool assignFlags() const;

		E::UalItemType type() const;

		bool hasRam() const { return afb().hasRam().value_or(afbComponent()->hasRam()); }
		int maxInstances() const { return afbComponent()->maxInstCount(); }
		int version() const { return afbComponent()->impVersion(); }
		QString componentCaption() const { return afbComponent()->caption(); }

		const std::vector<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		std::vector<LogicPin>& inputs() { return m_appLogicItem.m_fblItem->inputs(); }

		const LogicPin& input(const QUuid& guid) const { return m_appLogicItem.m_fblItem->input(guid); }
		LogicPin& input(const QUuid& guid) { return m_appLogicItem.m_fblItem->input(guid); }

		const std::vector<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		std::vector<LogicPin>& outputs() { return m_appLogicItem.m_fblItem->outputs(); }

		const LogicPin& output(const QUuid& guid) const { return m_appLogicItem.m_fblItem->output(guid); }
		VFrame30::AfbPin& output(const QUuid& guid) { return m_appLogicItem.m_fblItem->output(guid); }

		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.afbElement().params(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toAfbElement(); }
		const UalConst* ualConst() const { return m_appLogicItem.m_fblItem->toSchemaItemConst(); }
		const LogicTransmitter& logicTransmitter() const { return *m_appLogicItem.m_fblItem->toTransmitterElement(); }
		const UalReceiver& logicReceiver() const { return *m_appLogicItem.m_fblItem->toReceiverElement(); }
		const UalReceiver* ualReceiver() const { return m_appLogicItem.m_fblItem->toReceiverElement(); }
		const UalLoopbackSource* ualLoopbackSource() const { return m_appLogicItem.m_fblItem->toLoopbackSourceElement(); }
		const UalLoopbackTarget* ualLoopbackTarget() const { return m_appLogicItem.m_fblItem->toLoopbackTargetElement(); }

		const UalBusComposer* ualBusComposer() const { return m_appLogicItem.m_fblItem->toBusComposerElement(); }
		const UalBusExtractor* ualBusExtractor() const { return m_appLogicItem.m_fblItem->toBusExtractorElement(); }

		const Afb::AfbElement& afb() const { return m_appLogicItem.afbElement(); }
		std::shared_ptr<Afb::AfbComponent> afbComponent() const { return m_appLogicItem.afbComponent(); }

		std::shared_ptr<VFrame30::FblItemRect> itemRect() const { return m_appLogicItem.m_fblItem; }

		QString schemaID() const { return m_appLogicItem.m_schema->schemaId(); }
		std::shared_ptr<VFrame30::Schema> schema() { return m_appLogicItem.m_schema; }

		QString label() const { return m_appLogicItem.m_fblItem->label(); }
		void setLabel(const QString& label) { m_appLogicItem.m_fblItem->setLabel(label); }

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }

		const LogicPin* getPin(QUuid pinUuid) const;
		const LogicPin* getPin(const QString& pinCaption) const;

	protected:
		AppLogicItem m_appLogicItem;							// structure from parser

	private:
		mutable E::UalItemType m_type = E::UalItemType::Unknown;

		QHash<QString, int> m_opNameToIndexMap;

		static const QString SET_FLAGS_ITEM_CAPTION;
		static const QString SIM_LOCK_ITEM_CAPTION;
		static const QString MISMATCH_ITEM_CAPTION;
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

		float floatValue() const;
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

		static const QString IN_PIN_CAPTION;
		static const QString OUT_PIN_CAPTION;

		static const QString IN_1_PIN_CAPTION;
		static const QString IN_2_PIN_CAPTION;
		static const QString IN_3_PIN_CAPTION;
		static const QString IN_4_PIN_CAPTION;

		static const QString OUT_1_PIN_CAPTION;
		static const QString OUT_2_PIN_CAPTION;
		static const QString OUT_3_PIN_CAPTION;
		static const QString OUT_4_PIN_CAPTION;

		static const QString SIMLOCK_SIM_PIN_CAPTION;
		static const QString SIMLOCK_BLOCK_PIN_CAPTION;

		static const QString VALIDITY_PIN_CAPTION;
		static const QString SIMULATED_PIN_CAPTION;
		static const QString BLOCKED_PIN_CAPTION;
		static const QString MISMATCH_PIN_CAPTION;
		static const QString HIGH_LIMIT_PIN_CAPTION;
		static const QString LOW_LIMIT_PIN_CAPTION;

	public:
		UalAfb(const UalItem &appItem, bool isBusProcessingAfb);

		int instance() const { return m_instance; }
		quint16 opcode() const { return static_cast<quint16>(afb().opCode()); }		// return FB type
		QString caption() const { return afb().caption(); }
		QString typeCaption() const { return afb().componentCaption(); }
		int number() const { return m_number; }

		bool isConstComaparator() const;
		bool isDynamicComaparator() const;
		bool isComparator() const;
		bool isBusProcessing() const;

		QString instantiatorID() const;

		void setInstance(int instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, LogicAfbParam* afbParam) const;
		bool getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const;
		bool getAfbSignalByPin(const LogicPin& pin, LogicAfbSignal* afbSignal) const { return getAfbSignalByIndex(pin.afbOperandIndex(), afbSignal); }
		bool getAfbSignalByPinUuid(QUuid pinUuid, LogicAfbSignal* afbSignal) const;

		bool calculateFbParamValues(ModuleLogicCompiler* compiler);			// implemented in file FbParamCalculation.cpp

		const AppFbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

		int runTime() const { return m_runTime; }

	private:
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
		bool calculate_DEAD_ZONE_paramValues_ldn4();
		bool calculate_POL_paramValues();
		bool calculate_DERIV_paramValues();
		bool calculate_MISMATCH_paramValues();
		bool calculate_TCONV_paramValues();
		bool calculate_INDICATION_paramValues();
		bool calculate_PULSE_GENERATOR_paramValues();

		//

		bool checkRequiredParameters(const QStringList& requiredParams);
		bool checkRequiredParameters(const QStringList& requiredParams, bool displayError);
		bool checkRequiredParameter(const QString& requiredParam, bool displayError);

		bool checkUnsignedInt(const AppFbParamValue& paramValue);
		bool checkUnsignedInt16(const AppFbParamValue& paramValue);
		bool checkUnsignedInt32(const AppFbParamValue& paramValue);
		bool checkSignedInt32(const AppFbParamValue& paramValue);
		bool checkFloat32(const AppFbParamValue& paramValue);

		int lmDescriptionNumber() const;

	private:
		int m_instance = -1;
		int m_number = -1;
		bool m_isBusProcessing = false;
		mutable QString m_instantiatorID;

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

	class Loopback;

	class UalSignal
	{
		// Application Signal
		// represent all signal in application logic schemas, and signals, which createad in compiling time
		//
	public:
		static const QString AUTO_CONST_SIGNAL_ID_PREFIX;
		static const QString AUTO_SIGNAL_ID_PREFIX;
		static const QString AUTO_BUS_ID_PREFIX;

	public:
		UalSignal();
		UalSignal(const UalSignal&) = delete;

		~UalSignal();

	private:
		// private intializers can be used by UalSignalsMap only
		//
		bool createRegularSignal(const UalItem* ualItem, Signal* s);

		bool createConstSignal(const UalItem* ualItem,
								const QString& constSignalID,
								E::SignalType constSignalType,
								E::AnalogAppSignalFormat constAnalogFormat,
								Signal** autoSignalPtr);

		bool createAutoSignal(const UalItem* ualItem,
								const QString& signalID,
								E::SignalType signalType,
								E::AnalogAppSignalFormat analogFormat,
								Signal** autoSignalPtr);

		bool createBusParentSignal(const UalItem* ualItem,
									Signal* busSignal,
									BusShared bus,
									const QString& outPinCaption,
									std::shared_ptr<Hardware::DeviceModule> lm,
									Signal** autoSignalPtr);

		friend class UalSignalsMap;

	public:
		bool appendRefSignal(Signal* s, bool isOptoSignal);
		bool appendBusChildRefSignals(const QString &busSignalID, Signal* s);

		void setComputed() { m_computed = true; }
		bool isComputed() const { return m_computed; }

		void setResultSaved() { m_resultSaved = true; }
		bool isResultSaved() const { return m_resultSaved; }

		QString appSignalID() const { return m_refSignals[0]->appSignalID(); }

		Address16 ualAddr() const;
		Address16 ualAddrWithoutChecks() const;
		bool setUalAddr(Address16 ualAddr);

		bool ualAddrIsValid() const;

		bool checkUalAddr() const;

		Address16 regBufAddr() const { return m_regBufAddr; }
		bool setRegBufAddr(Address16 regBufAddr);

		bool checkRegBufAddr() const;

		Address16 regValueAddr() const { return m_regValueAddr; }
		bool setRegValueAddr(Address16 regValueAddr);

		Address16 ioBufAddr() const;

		bool checkIoBufAddr() const;

		Signal* signal() const;

		E::SignalType signalType() const { return m_refSignals[0]->signalType(); }
		E::SignalInOutType inOutType() const { return m_refSignals[0]->inOutType(); }
		E::AnalogAppSignalFormat analogSignalFormat() const { return m_refSignals[0]->analogSignalFormat(); }
		int dataSize() const { return m_refSignals[0]->dataSize(); }
		int sizeW() const { return m_refSignals[0]->sizeW(); }
		E::DataFormat dataFormat() const { return m_refSignals[0]->dataFormat(); }
		E::ByteOrder byteOrder() const { return m_refSignals[0]->byteOrder(); }

		bool isAnalog() const { return m_refSignals[0]->isAnalog(); }
		bool isDiscrete() const { return m_refSignals[0]->isDiscrete(); }
		bool isBus() const { return m_refSignals[0]->isBus(); }

		bool isHeapPlaced() const { return m_isHeapPlaced; }

		QString busTypeID() const { return m_refSignals[0]->busTypeID(); }

		QString caption() const { return m_refSignals[0]->caption(); }
		QString customAppSignalID() const { return m_refSignals[0]->customAppSignalID(); }

		const Signal& constSignal() { return *m_refSignals[0]; }

		const QVector<Signal*>& refSignals() const { return m_refSignals; }
		int refSignalsCount() const { return m_refSignals.count(); }

		bool isCompatible(const Signal* s, IssueLogger* log) const;
		bool isCanBeConnectedTo(const UalItem &ualItem, const LogicAfbSignal& afbSignal, IssueLogger* log) const;
		bool isCompatible(BusShared bus, const BusSignal& busSignal, IssueLogger* log) const;
		bool isCompatible(const UalSignal* ualSignal, IssueLogger* log) const;
		bool isCanBeConnectedTo(const UalSignal* destSignal, IssueLogger* log) const;

		bool isInput() const { return m_isInput; }
		bool isTunable() const { return m_isTunable; }

		bool isOptoSignal() const { return m_isOptoSignal; }
		void setReceivedOptoAppSignalID(const QString& recvAppSignalID);
		QString receivedOptoAppSignalID() const { return m_receivedOptoAppSignalID; }

		bool isSource() const { return m_isInput || m_isTunable || m_isOptoSignal || m_isConst; }

		bool isOutput() const { return m_isOutput; }
		bool isStrictOutput() const { return m_isOutput == true && isSource() == false && isBusChild() == false; }

		bool isInternal() const { return isSource() == false && m_isOutput == false; }

		bool isAcquired() const { return m_isAcquired; }

		bool isBusChild() const { return m_parentBusSignal != nullptr; }
		void setParentBusSignal(UalSignal* parentBusSignal) { m_parentBusSignal = parentBusSignal; }

		bool anyParentBusIsAcquired() const;

		bool isLoopbackSource() const { return m_loopback != nullptr; }

		void setLoopback(std::shared_ptr<Loopback> loopback);
		std::shared_ptr<Loopback> loopback() const;
		QString loopbackID() const;

		//

		bool isConst() const { return m_isConst; }
		E::SignalType constType() const;
		E::AnalogAppSignalFormat constAnalogFormat() const;
		int constDiscreteValue() const;
		int constAnalogIntValue() const;
		float constAnalogFloatValue() const;
		double constValue() const;
		double constValueIfConst() const;

		void sortRefSignals();

		Signal* getInputSignal() const;
		Signal* getOutputSignal() const;
		Signal* getTunableSignal() const;
		QVector<Signal*> getAnalogOutputSignals() const;

		QStringList refSignalIDs() const;
		void refSignalIDs(QStringList* appSignalIDs) const;

		QString refSignalIDsJoined() const;
		QString refSignalIDsJoined(const QString& separator) const;

		QStringList acquiredRefSignalsIDs() const;

		QString optoConnectionID() const;

		void setUalItem(const UalItem* ualItem);

		const UalItem* ualItem() const { return m_ualItem; }
		QUuid ualItemGuid() const;
		QString ualItemSchemaID() const;
		QString ualItemLabel() const;

		bool appendBusChildSignal(const QString& busSignalID, UalSignal* ualSignal);

		BusShared bus() const { return m_bus; }

		UalSignal* getBusChildSignal(const QString& busSignalID);

		void setAcquired(bool acquired);

		bool addStateFlagSignal(const QString& signalWithFlagID, E::AppSignalStateFlagType flagType, const QString& flagSignalID, IssueLogger* log);

	private:
		void preliminarySetHeapPlaced(int expectedHeapReadsCount);
		bool canBePlacedInHeap() const;
		void setHeapPlaced();
		void resetHeapPlaced();
		int expectedHeapReadsCount() const;

	private:
		bool m_isHeapPlaced = false;
		int m_expectedHeapReadsCount = 0;

		const UalItem* m_ualItem = nullptr;

		QVector<Signal*> m_refSignals;							// vector of pointers to signal in m_signalSet

		QHash<QString, UalSignal*> m_busChildSignals;

		QString m_refSignalsIDs;

		//

		bool m_isConst = false;

		int m_constDiscreteValue = 0;
		int m_constIntValue = 0;
		double m_constFloatValue = 0;

		//

		std::shared_ptr<Loopback> m_loopback;

		//

		BusShared m_bus;

		bool m_isInput = false;							// signal sources
		bool m_isTunable = false;
		bool m_isOptoSignal = false;
		QString m_receivedOptoAppSignalID;

		Signal* m_optoSignalNativeCopy = nullptr;		// delete this!

		bool m_isOutput = false;
		bool m_isAcquired = false;

		UalSignal* m_parentBusSignal = nullptr;			// if not nullptr - this ual signal is bus child

		bool m_computed = false;
		bool m_resultSaved = false;

		Address16 m_ualAddr;
		Address16 m_regBufAddr;							// address in RegBuf (absolute in LM's memory)
		Address16 m_regValueAddr;						// relative address from beginning of RegBuf ()

		friend class UalSignalsMap;
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

		UalSignal* createSignal(Signal* appSignal);

		UalSignal* createSignal(Signal* appSignal, const UalItem* ualItem, QUuid outPinUuid);

		UalSignal* createConstSignal(const UalItem* ualItem,
									 E::SignalType constSignalType,
									 E::AnalogAppSignalFormat constAnalogFormat,
									 QUuid outPinUuid);

		UalSignal* createAutoSignal(const UalItem* ualItem, QUuid outPinUuid,
									const LogicAfbSignal& templateOutAfbSignal,
									std::optional<int> expectedReadCount);

		UalSignal* createAutoSignal(const UalItem* ualItem, QUuid outPinUuid, const Signal& templateSignal);

		UalSignal* createBusParentSignal(Signal* appBusSignal);
		UalSignal* createBusParentSignal(Signal* appBusSignal, BusShared bus, const UalItem* ualItem, QUuid outPinUuid, const QString& outPinCaption);

		bool appendRefPin(const UalItem* ualItem, QUuid pinUuid, UalSignal* ualSignal);
		bool appendRefSignal(Signal* s, UalSignal* ualSignal);

		UalSignal* get(const QString& appSignalID) const { return m_idToSignalMap.value(appSignalID, nullptr); }
		bool contains(const QString& appSignalID) const { return m_idToSignalMap.contains(appSignalID); }

		UalSignal* get(QUuid pinUuid) const { return m_pinToSignalMap.value(pinUuid, nullptr); }
		bool contains(QUuid pinUuid) const { return m_pinToSignalMap.contains(pinUuid); }

		void clear();

		bool getReport(QStringList& report) const;

		//

		void initDiscreteSignalsHeap(int startAddrW, int sizeW);
		int getDiscreteSignalsHeapSizeW() const;

		void initAnalogAndBusSignalsHeap(int startAddrW, int sizeW);
		int getAnalogAndBusSignalsHeapSizeW() const;

		Address16 getSignalWriteAddress(const UalSignal& ualSignal);
		Address16 getSignalReadAddress(const UalSignal& ualSignal, bool decrementReadCount);

		void disposeSignalsInHeaps(const std::unordered_set<UalSignal*>& flagsSignals);

		void finalizeHeaps();

		void getHeapsLog(QStringList* log) const;

	private:
		UalSignal* privateCreateAutoSignal(const UalItem* ualItem,
									QUuid outPinUuid,
									E::SignalType signalType,
									E::AnalogAppSignalFormat analogFormat,
									std::optional<int> expectedReadCount);

		bool insertNew(QUuid pinUuid, UalSignal* newUalSignal);
		void appendPinRefToSignal(QUuid pinUuid, UalSignal* ualSignal);

		QString getAutoSignalID(const UalItem* appItem, const LogicPin& outputPin);

		const UalSignal* value(const UalSignal* &key, const UalSignal* &defaultValue) const;

		bool getAnalogFormat(const LogicAfbSignal& afbSignal, E::AnalogAppSignalFormat* analogFormat);

	private:
		ModuleLogicCompiler& m_compiler;
		IssueLogger* m_log = nullptr;

		//

		QHash<QString, UalSignal*> m_idToSignalMap;
		QHash<QUuid, UalSignal*> m_pinToSignalMap;
		QHash<UalSignal*, QUuid> m_signalToPinsMap;
		QHash<Signal*, UalSignal*> m_ptrToSignalMap;

		SignalsHeap m_discreteSignalsHeap;
		SignalsHeap m_analogAndBusSignalsHeap;				// for now: Analog and Bus signals
	};

}
