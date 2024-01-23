#pragma once

#include "../AppSignalLib/AppSignal.h"
#include "../UtilsLib/WUtils.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/SchemaItemBus.h"
#include "../VFrame30/SchemaItemLoopback.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"
#include "../CommonLib/HashedVector.h"
#include "../HardwareLib/Afb.h"
#include "../HardwareLib/LmDescription.h"

#include "Parser.h"
#include "AppLogicCode.h"
#include "Busses.h"
#include "SignalsHeap.h"

namespace Builder
{
	//
	// Shorter Schema* synonymouses for types incoming from Parser to ModuleLogicCompiler
	//
	using SchemaItem = std::shared_ptr<VFrame30::FblItemRect>;	// generic base class for different schema elements
	using SchemaPin = VFrame30::AfbPin;

	using SchemaConst = VFrame30::SchemaItemConst ;
	using SchemaSignal = VFrame30::SchemaItemSignal;
	using SchemaAfb = VFrame30::SchemaItemAfb;

	using SchemaTransmitter = VFrame30::SchemaItemTransmitter;
	using SchemaReceiver = VFrame30::SchemaItemReceiver;

	using SchemaBusComposer = VFrame30::SchemaItemBusComposer;
	using SchemaBusExtractor = VFrame30::SchemaItemBusExtractor;

	using SchemaLoopbackSource = VFrame30::SchemaItemLoopbackSource;
	using SchemaLoopbackTarget =  VFrame30::SchemaItemLoopbackTarget;

	using AfbSignal = Afb::AfbSignal;							// signal associated with AFB inputs/outputs
	using AfbParam = Afb::AfbParam;

	using AfbElementShared = std::shared_ptr<Afb::AfbElement>;

	class UalItem;
	class UalAfb;
	class ModuleLogicCompiler;

	class AfbComponents
	{
	public:
		struct LogicInfo
		{
			bool isValid() const;

			int opCode = -1;

			int confIndex = -1;
			int operandQuantityIndex = -1;
			int busWidthIndex = -1;
			int firstInIndex = -1;
			int resultIndex = -1;

			int confOr = -1;
			int confAnd = -1;

			int minOperandsCount = -1;
			int maxOperandsCount = -1;
		};

	public:
		AfbComponents();
		virtual ~AfbComponents();

		void init(LmDescriptionConstShared lmDescription);
		bool addInstance(UalAfb* ualAfb, IssueLogger* log);
		bool addInstance(int afbOpcode, int* newInstance, IssueLogger* log);

		int getUsedInstancesCount(int opCode) const;
		bool isBusProcessingAfb(const QString& afbElementStrID) const;

		const LogicInfo& logicInfo() const;

	private:
		struct ComponentInfo
		{
			ComponentInfo(const QString& compCaption,
						  int compMaxInstCount,
						  bool compHasRam);

			QString caption;
			int curInstance = -1;		// valid instance number begins from 0
			int maxInstCount = 0;
			bool hasRam = false;

			//

			std::map<QString, int> nonRamAfbInstantiators;	// Non RAM AFB instantiatorID => AFB instance
		};

	private:
		std::map<int, ComponentInfo> m_componentsInfo;		// AFB opCode => ComponentInfo
		std::set<Hash> m_busProcessingAfbElemets;			// set of calcHash(afbElement->strID())

		LogicInfo m_logicInfo;
	};

	class UalItem : public QObject
	{
		Q_OBJECT

	public:
		UalItem();
		UalItem(const UalItem& ualItem);
		UalItem(const AppLogicItem& appLogicItem);
		UalItem(std::shared_ptr<Afb::AfbElement> afbElement, QString& errorMsg);

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
		bool isPackedLogic() const;

        bool assignFlags(IssueLogger* log) const;

		E::UalItemType type() const;

		bool hasRam() const { return afb().hasRam().value_or(afbComponent()->hasRam()); }
		int maxInstances() const { return afbComponent()->maxInstCount(); }
		int version() const { return afbComponent()->impVersion(); }
		QString componentCaption() const { return afbComponent()->caption(); }

		const std::vector<SchemaPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		std::vector<SchemaPin>& inputs() { return m_appLogicItem.m_fblItem->inputs(); }

		const SchemaPin& input(const QUuid& guid) const { return m_appLogicItem.m_fblItem->input(guid); }
		SchemaPin& input(const QUuid& guid) { return m_appLogicItem.m_fblItem->input(guid); }

		const std::vector<SchemaPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		std::vector<SchemaPin>& outputs() { return m_appLogicItem.m_fblItem->outputs(); }

		const SchemaPin& output(const QUuid& guid) const { return m_appLogicItem.m_fblItem->output(guid); }
		VFrame30::AfbPin& output(const QUuid& guid) { return m_appLogicItem.m_fblItem->output(guid); }

		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.afbElement().params(); }
		std::vector<Afb::AfbParam>& params() { return m_appLogicItem.afbElement().params(); }

		const SchemaConst* schemaConst() const;
		const SchemaSignal* schemaSignal() const;
		const SchemaAfb* schemaAfb() const;
		const SchemaTransmitter* schemaTransmitter() const;
		const SchemaReceiver* schemaReceiver() const;
		const SchemaBusComposer* schemaBusComposer() const;
		const SchemaBusExtractor* schemaBusExtractor() const;
		const SchemaLoopbackSource* schemaLoopbackSource() const;
		const SchemaLoopbackTarget* schemaLoopbackTarget() const;

		const Afb::AfbElement& afb() const { return m_appLogicItem.afbElement(); }
		std::shared_ptr<Afb::AfbComponent> afbComponent() const { return m_appLogicItem.afbComponent(); }

		QString schemaID() const;
		std::shared_ptr<VFrame30::Schema> schema() { return m_appLogicItem.m_schema; }

		QString label() const { return m_appLogicItem.m_fblItem->label(); }
		void setLabel(const QString& label) { m_appLogicItem.m_fblItem->setLabel(label); }

		const SchemaPin* getPin(QUuid pinUuid) const;
		const SchemaPin* getPin(const QString& pinCaption) const;

		bool setParamValueByCaption(const QString& paramCaption, const QVariant& value);

	protected:
		AppLogicItem m_appLogicItem;							// structure from parser

	private:
		mutable E::UalItemType m_type = E::UalItemType::Unknown;

		QHash<QString, int> m_opNameToIndexMap;
	};

	typedef std::map<QUuid, UalItem*> ConnectedAppItems;		// connected pin Uuid => AppItem*

	class AfbParamValue
	{
	public:
		static const int NOT_FB_OPERAND_INDEX = -1;

	public:
		AfbParamValue();
		AfbParamValue(const Afb::AfbParam& afbParam);

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

		void setValue(const QVariant& qv);

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

	class AfbParamValuesArray : public std::vector<AfbParamValue>
	{
	public:
		void insert(const QString& opName, const AfbParamValue& value);
		bool contains(const QString& opName) const;
		bool isEmpty() const;
		bool hasParamsToInitialization() const;

		AfbParamValue& operator [] (const QString& opName);
		const AfbParamValue& operator [] (const QString& opName) const;

	private:
		const AfbParamValue& find(const QString& opName) const;

	private:
		std::map<QString, size_t> m_opNameToIndex;			// opName -> index in vector

		static AfbParamValue m_nullValue;
	};

	class UalAfb : public UalItem
	{
		// Application Functional Block
		// represent all FB items in application logic schemas
		//
	public:
		static const int FOR_USER_ONLY_PARAM_INDEX = -1;				// index of AFB parameters used by user only

	public:
		UalAfb(const UalItem& appItem, bool isBusProcessingAfb);

		int instance() const { return m_instance; }
		quint16 opcode() const { return static_cast<quint16>(afb().opCode()); }		// return FB type
		QString caption() const { return afb().caption(); }
		QString typeCaption() const { return afb().componentCaption(); }
		int number() const { return m_number; }

		bool isConstComaparator() const;
		bool isDynamicComaparator() const;
		bool isComparator() const;
		bool isBusProcessing() const;

		bool isPackedLogic() const;
		bool isPackedOrLogic() const;
		bool isPackedAndLogic() const;
		QString packedLogicID() const;

		int precision() const;

		const QString& instantiatorID() const;

		void setInstance(int instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, AfbParam* afbParam) const;
		const AfbParam* getParamByOpName(const QString& opName) const;

		int getParamIntValueByOpName(const QString& opName, bool* ok) const;

		bool getAfbSignalByPin(const SchemaPin& pin, AfbSignal* afbSignal) const;
		bool getAfbSignalByIndex(int index, AfbSignal* afbSignal) const;
		bool getAfbSignalByPinUuid(QUuid pinUuid, AfbSignal* afbSignal) const;
		bool getAfbSignalByCaption(const QString& caption, AfbSignal* afbSignal) const;

		bool setParamValueByCaption(const QString& paramCaption, const QVariant& value);

		bool calculateFbParamValues(ModuleLogicCompiler* compiler);			// implemented in file FbParamCalculation.cpp

		const AfbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

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
		bool calculate_DEAD_ZONE_paramValues_LM1_SR04();
		bool calculate_DEAD_ZONE_paramValues_LM8_SR10();
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

		bool checkUnsignedInt(const AfbParamValue& paramValue);
		bool checkUnsignedInt16(const AfbParamValue& paramValue);
		bool checkUnsignedInt32(const AfbParamValue& paramValue);
		bool checkSignedInt32(const AfbParamValue& paramValue);
		bool checkFloat32(const AfbParamValue& paramValue);

		QString lmDescriptionName() const;

	private:
		int m_instance = -1;
		int m_number = -1;
		bool m_isBusProcessing = false;
		mutable QString m_instantiatorID;

		AfbParamValuesArray m_paramValuesArray;

		ModuleLogicCompiler* m_compiler = nullptr;
		IssueLogger* m_log = nullptr;

		int m_runTime = 0;

		static std::set<QString> m_lmsWithLessGreateEqMode;
	};

	class UalAfbs
	{
		 // this class owns all created UalAfb
	public:
		UalAfbs();
		virtual ~UalAfbs();

		void clear();

		UalAfb* insert(UalAfb* appFb);
		UalAfb* getAfb(const QUuid& afbGuid) const;
		bool contains(const QUuid& afbGuid) const;

		std::vector<UalAfb*>::iterator begin();
		std::vector<UalAfb*>::const_iterator begin() const;

		std::vector<UalAfb*>::iterator end();
		std::vector<UalAfb*>::const_iterator end() const;

	private:
		std::vector<UalAfb*> m_afbs;
		std::map<QUuid, UalAfb*> m_guidToAfb;
		int m_fbNumber = 1;
	};

	class Loopback;

	class UalSignal
	{
		// Class UalSignal represent all signal in application logic schemas,
		// and signals, which createad in compiling time
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
		bool createRegularSignal(const UalItem* ualItem, const QUuid& outPinGuid, AppSignal* s);

		bool createConstSignal(	const QString& lmEquipmentID,
								const UalItem* ualItem,
								const QString& constSignalID,
								E::SignalType constSignalType,
								E::AnalogAppSignalFormat constAnalogFormat,
								AppSignal** autoSignalPtr);

		bool createAutoSignal(const QString& lmEquipmentID,
								const UalItem* ualItem,
								const QUuid& outPinGuid,
								const QString& signalID,
								E::SignalType signalType,
								E::AnalogAppSignalFormat analogFormat,
								AppSignal** autoSignalPtr);

		bool createBusParentSignal( const QString& lmEquipmentID,
									const UalItem* ualItem,
									const QUuid& outPinGuid,
									AppSignal* busSignal,
									BusShared bus,
									const QString& outPinCaption,
									std::shared_ptr<Hardware::DeviceModule> lm,
									AppSignal** autoSignalPtr);

		friend class UalSignals;

	public:
		bool appendRefAppSignal(AppSignal* s, bool isOptoSignal);
		bool appendBusChildRefSignals(const QString &busSignalID, AppSignal* s);

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

		AppSignal* signal() const;

		E::SignalType signalType() const { return m_refSignals[0]->signalType(); }
		E::SignalInOutType inOutType() const;
		E::AnalogAppSignalFormat analogSignalFormat() const { return m_refSignals[0]->analogSignalFormat(); }
		int dataSize() const { return m_refSignals[0]->dataSize(); }
		int sizeW() const { return m_refSignals[0]->sizeW(); }
		E::DataFormat dataFormat() const { return m_refSignals[0]->dataFormat(); }
		E::ByteOrder byteOrder() const { return m_refSignals[0]->byteOrder(); }

		bool isAnalog() const { return m_refSignals[0]->isAnalog(); }
		bool isDiscrete() const { return m_refSignals[0]->isDiscrete(); }
		bool isBus() const { return m_refSignals[0]->isBus(); }
		bool invertSignal() const { return m_refSignals[0]->invertSignal(); }

		bool isHeapPlaced() const { return m_isHeapPlaced; }

		QString busTypeID() const { return m_refSignals[0]->busTypeID(); }

		QString caption() const { return m_refSignals[0]->caption(); }
		QString customAppSignalID() const { return m_refSignals[0]->customAppSignalID(); }

		const AppSignal& constSignal() { return *m_refSignals[0]; }

		const QVector<AppSignal*>& refSignals() const { return m_refSignals; }
		int refSignalsCount() const { return static_cast<int>(m_refSignals.count()); }

		bool isCompatible(const AppSignal* s, IssueLogger* log) const;
		bool isCanBeConnectedTo(const UalItem &ualItem, const AfbSignal& afbSignal, IssueLogger* log) const;
		bool isCompatible(BusShared bus, const BusSignal& busSignal, IssueLogger* log) const;
		bool isCompatible(const UalSignal* ualSignal, IssueLogger* log) const;
		bool isCanBeConnectedTo(const UalSignal* destSignal, IssueLogger* log) const;

		bool isAutoSignal() const { return m_isAutoSignal; }
		bool isInput() const { return m_isInput; }
		bool isTunable() const { return m_isTunable; }

		bool isOptoSignal() const { return m_isOptoSignal; }
		void setReceivedOptoAppSignalID(const QString& recvAppSignalID, const SchemaReceiver* ualReceiver);
		QString receivedOptoAppSignalID() const { return m_receivedOptoAppSignalID; }
		const SchemaReceiver* ualReceiver() const { return m_ualReceiver; }

		bool isSource() const { return m_isInput || m_isTunable || m_isOptoSignal || m_isConst; }

		bool isOutput() const { return m_isOutput; }
		bool isStrictOutput() const { return m_isOutput == true && isSource() == false && isBusChild() == false; }

		bool isInternal() const { return isSource() == false && m_isOutput == false; }

		bool isAcquired() const { return m_isAcquired; }

		bool isBusChild() const { return m_parentBusSignal != nullptr; }
		void setParentBusSignal(UalSignal* parentBusSignal) { m_parentBusSignal = parentBusSignal; }
		bool isFrombusConversionRequired() const { return m_frombusConversionRequired; }
		void setFrombusConversionRequired(bool required) { m_frombusConversionRequired = required; }
		bool isFrombusConversionCodeAlreadyGenerated() const { return m_frombusConversionCodeIsAlreadyGenerated; }
		void setFrombusConversionCodeIsAlreadyGenerated() { m_frombusConversionCodeIsAlreadyGenerated = true; }

		bool anyParentBusIsAcquired() const;

        bool setLoopback(std::shared_ptr<Loopback> loopback);
		bool isLoopbackSource() const { return !m_loopbacks.empty(); }

		//

		bool isConst() const { return m_isConst; }
		E::SignalType constType() const;
		bool isConstDiscrete() const { return m_isConst && constType() == E::SignalType::Discrete; }
		E::AnalogAppSignalFormat constAnalogFormat() const;
		int constDiscreteValue() const;
		int constAnalogIntValue() const;
		float constAnalogFloatValue() const;
		double constValue() const;
		double constValueIfConst() const;

		void sortRefSignals();

		AppSignal* getInputSignal() const;
		AppSignal* getOutputSignal() const;
		AppSignal* getTunableSignal() const;
		QVector<AppSignal*> getAnalogOutputSignals() const;

		QStringList refSignalIDs() const;
		void refSignalIDs(QStringList* appSignalIDs) const;

		QString refSignalIDsJoined() const;
		QString refSignalIDsJoined(const QString& separator) const;

		QStringList acquiredRefSignalsIDs() const;

		QString optoConnectionID() const;

		void setSourceUalItem(const UalItem* ualItem, const QUuid& outPinGuid);

		const UalItem* ualItem() const;
		QUuid ualItemGuid() const;
		QString ualItemSchemaID() const;
		QString ualItemLabel() const;
		QString ualItemLabelOutPinCaption() const;

		bool appendBusChildSignal(const QString& busSignalID, UalSignal* ualSignal);

		BusShared bus() const { return m_bus; }

		UalSignal* getBusChildSignal(const QString& busSignalID);
		const UalSignal* getParentBusSignal() const { return m_parentBusSignal; }

		void setAcquired(bool acquired);

		bool addStateFlagSignal(const QString& signalWithFlagID, E::AppSignalStateFlagType flagType, const QString& flagSignalID, IssueLogger* log);

	private:
		void preliminarySetHeapPlaced(int expectedHeapReadsCount);
		bool canBePlacedInHeap() const;
		void setHeapPlaced();
		void resetHeapPlaced();
		int expectedHeapReadsCount() const;
		void setAutoSignal(bool autoSignal);

	private:
		bool m_isHeapPlaced = false;
		int m_expectedHeapReadsCount = 0;

		const UalItem* m_ualItem = nullptr;
		QUuid m_outPinGuid;

		QVector<AppSignal*> m_refSignals;							// vector of pointers to signal in m_signalSet

		QHash<QString, UalSignal*> m_busChildSignals;

		QString m_refSignalsIDs;

		//

		bool m_isConst = false;

		int m_constDiscreteValue = 0;
		int m_constIntValue = 0;
		double m_constFloatValue = 0;

		//

		std::set<std::shared_ptr<Loopback>> m_loopbacks;

		//

		BusShared m_bus;

		bool m_isAutoSignal = false;

		bool m_isInput = false;							// signal sources
		bool m_isTunable = false;
		bool m_isOptoSignal = false;
		QString m_receivedOptoAppSignalID;
		const SchemaReceiver* m_ualReceiver = nullptr;

		bool m_isOutput = false;
		bool m_isAcquired = false;

		UalSignal* m_parentBusSignal = nullptr;			// if not nullptr - this ual signal is bus child
		bool m_frombusConversionRequired = false;		// set to corresponding value in bus extractor processing
		bool m_frombusConversionCodeIsAlreadyGenerated = false;

		bool m_computed = false;
		bool m_resultSaved = false;

		Address16 m_ualAddr;
		Address16 m_regBufAddr;							// address in RegBuf (absolute in LM's memory)
		Address16 m_regValueAddr;						// relative address from beginning of RegBuf ()

		friend class UalSignals;
	};

	class UalSignals: public QObject
	{
		Q_OBJECT
	public:
		UalSignals(ModuleLogicCompiler& compiler, IssueLogger* log);
		~UalSignals();

		void clear();

		std::vector<UalSignal*>::iterator begin();
		std::vector<UalSignal*>::const_iterator begin() const;

		std::vector<UalSignal*>::iterator end();
		std::vector<UalSignal*>::const_iterator end() const;

		UalSignal* get(const QString& appSignalID) const;
		bool contains(const QString& appSignalID) const;

		UalSignal* get(const QUuid &pinUuid) const;
		bool contains(QUuid pinUuid) const;

		UalSignal* get(const AppSignal* appSignal) const;

		bool contains(const UalSignal* ualSignal) const;

		//

		UalSignal* createSignal(AppSignal* appSignal);

		UalSignal* createSignal(AppSignal* appSignal, const UalItem* ualItem, QUuid outPinUuid);

		UalSignal* createConstSignal(const UalItem* ualItem,
									 E::SignalType constSignalType,
									 E::AnalogAppSignalFormat constAnalogFormat,
									 QUuid outPinUuid);

		UalSignal* createAutoSignal(const UalItem* ualItem, QUuid outPinUuid,
									const AfbSignal& templateOutAfbSignal,
									std::optional<int> expectedReadCount);

		UalSignal* createAutoSignal(const UalItem* ualItem, QUuid outPinUuid, const AppSignal& templateSignal);

		UalSignal* createBusParentSignal(AppSignal* appBusSignal);
		UalSignal* createBusParentSignal(AppSignal* appBusSignal, BusShared bus, const UalItem* ualItem, QUuid outPinUuid, const QString& outPinCaption);

		bool appendRefPin(const UalItem* ualItem, QUuid pinUuid, UalSignal* ualSignal);
		bool appendRefSignal(AppSignal* s, UalSignal* ualSignal);

		bool getReport(QStringList& report) const;

		//

		void initDiscreteSignalsHeap(int startAddrW, int sizeW);
		int getDiscreteSignalsHeapSizeW() const;

		void initAnalogAndBusSignalsHeap(int startAddrW, int sizeW);
		int getAnalogAndBusSignalsHeapSizeW() const;

		Address16 getSignalWriteAddress(const UalSignal& ualSignal);
		Address16 getSignalReadAddress(const UalSignal& ualSignal, bool decrementReadCount);

		void disposeSignalsInHeaps(const std::set<const UalSignal*> &flagsSignals);

		bool finalizeHeaps();

		const SignalsHeap& discreteSignalsHeap() const { return m_discreteSignalsHeap; }
		const SignalsHeap& analogAndBusSignalsHeap() const { return m_analogAndBusSignalsHeap; }

		void getHeapsLog(QStringList* log) const;

		std::shared_ptr<Hardware::DeviceModule> lm() const;

	private:
		UalSignal* privateCreateAutoSignal(const UalItem* ualItem,
									QUuid outPinUuid,
									E::SignalType signalType,
									E::AnalogAppSignalFormat analogFormat,
									std::optional<int> expectedReadCount);

		bool insertNew(QUuid pinUuid, UalSignal* newUalSignal);
		void appendPinRefToSignal(QUuid pinUuid, UalSignal* ualSignal);

		QString getAutoSignalID(const UalItem* ualItem, const SchemaPin& outputPin);

		bool getAnalogFormat(const AfbSignal& afbSignal, E::AnalogAppSignalFormat* analogFormat);

	private:
		ModuleLogicCompiler& m_compiler;
		IssueLogger* m_log = nullptr;

		//

		std::vector<UalSignal*> m_signals;					// owns all created UalSignals
		std::set<const UalSignal*> m_signalSet;
		std::map<QString, UalSignal*> m_idToSignalMap;
		std::map<QUuid, UalSignal*> m_pinToSignalMap;
		std::map<const AppSignal*, UalSignal*> m_ptrToSignalMap;

		SignalsHeap m_discreteSignalsHeap;
		SignalsHeap m_analogAndBusSignalsHeap;				// for now: Analog and Bus signals
	};
}
