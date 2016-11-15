#pragma once

#include "../lib/WUtils.h"
#include "../lib/DeviceObject.h"
#include "../lib/Signal.h"
#include "../lib/OrderedHash.h"
#include "../lib/ModuleConfiguration.h"

#include "../Builder/Parser.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"
#include "../Builder/OptoModule.h"
#include "../Builder/LmMemoryMap.h"
#include "../TuningService/TuningDataStorage.h"

#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemAfb.h"
#include "../VFrame30/SchemaItemConst.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/LogicSchema.h"

#include "../u7/Connection.h"

namespace Builder
{

	// typedefs Logic* for types defined outside ModuleLogicCompiler
	//

	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::SchemaItemSignal LogicSignal;
	typedef VFrame30::SchemaItemAfb LogicFb;
	typedef VFrame30::AfbPin LogicPin;
	typedef VFrame30::SchemaItemConst LogicConst;
	typedef VFrame30::SchemaItemTransmitter LogicTransmitter;
	typedef VFrame30::SchemaItemReceiver LogicReceiver;
	typedef Afb::AfbSignal LogicAfbSignal;
	typedef Afb::AfbParam LogicAfbParam;

	class AppItem;
	class ModuleLogicCompiler;


	// Functional Block Library element
	//

	class LogicAfb
	{
	private:
		std::shared_ptr<Afb::AfbElement> m_afb;

	public:
		LogicAfb(std::shared_ptr<Afb::AfbElement> afb);
		~LogicAfb();

		bool hasRam() const { return m_afb->hasRam(); }

		const Afb::AfbElement& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }
		Afb::AfbType type() const { return m_afb->type(); }
	};


	typedef QHash<int, int> FblInstanceMap;				// Key is OpCode
	typedef QHash<QString, int> NonRamFblInstanceMap;


	class AppFb;

	class AfbMap: public HashedVector<QString, LogicAfb*>
	{
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

	public:
		~AfbMap() { clear(); }

		bool addInstance(AppFb *appFb);

		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);
	};


	// Base class for AppFb & AppSignal
	// contains pointer to AppLogicItem
	//

	class AppItem : public QObject
	{
		Q_OBJECT

	protected:
		AppLogicItem m_appLogicItem;

	private:
		QHash<QString, int> m_opNameToIndexMap;

	public:
		AppItem(const AppItem& appItem);
		AppItem(const AppLogicItem& appLogicItem);
		AppItem(std::shared_ptr<Afb::AfbElement> afbElement, QString &errorMsg);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.m_afbElement.strID(); }
		QString caption() const { return m_appLogicItem.m_afbElement.caption(); }

		QString strID() const;

		bool isSignal() const { return m_appLogicItem.m_fblItem->isSignalElement(); }
		bool isFb() const { return m_appLogicItem.m_fblItem->isAfbElement(); }
		bool isConst() const { return m_appLogicItem.m_fblItem->isConstElement(); }
		bool isTransmitter() const { return m_appLogicItem.m_fblItem->isTransmitterElement(); }
		bool isReceiver() const { return m_appLogicItem.m_fblItem->isReceiverElement(); }
		bool isTerminator() const { return m_appLogicItem.m_fblItem->isTerminatorElement(); }

		bool hasRam() const { return afb().hasRam(); }

		const std::vector<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::vector<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }
		const std::vector<Afb::AfbParam>& params() const { return m_appLogicItem.m_afbElement.params(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toAfbElement(); }
		const LogicConst& logicConst() const { return *m_appLogicItem.m_fblItem->toSchemaItemConst(); }
		const LogicTransmitter& logicTransmitter() const { return *m_appLogicItem.m_fblItem->toTransmitterElement(); }
		const LogicReceiver& logicReceiver() const { return *m_appLogicItem.m_fblItem->toReceiverElement(); }
		const Afb::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }

		QString schemaID() const { return m_appLogicItem.m_schema->schemaID(); }

		QString label() const;

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }
	};


	class AppFbParamValue
	{
	public:
		static const int NOT_FB_OPERAND_INDEX = -1;

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

	public:
		AppFbParamValue() {}
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
	};

	typedef HashedVector<QString, AppFbParamValue> AppFbParamValuesArray;


	// Application Functional Block
	// represent all FB items in application logic schemas
	//

	class ModuleLogicCompiler;

	class AppFb : public AppItem
	{
	private:
		quint16 m_instance = -1;
		int m_number = -1;
		QString m_instantiatorID;

		AppFbParamValuesArray m_paramValuesArray;

		ModuleLogicCompiler* m_compiler = nullptr;
		IssueLogger* m_log = nullptr;

		int m_runTime = 0;

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

		//

		bool checkRequiredParameters(const QStringList& requiredParams);
		bool checkRequiredParameters(const QStringList& requiredParams, bool displayError);


		bool checkUnsignedInt(const AppFbParamValue& paramValue);
		bool checkUnsignedInt16(const AppFbParamValue& paramValue);
		bool checkUnsignedInt32(const AppFbParamValue& paramValue);
		bool checkSignedInt32(const AppFbParamValue& paramValue);
		bool checkFloat32(const AppFbParamValue& paramValue);

		//

	public:
		AppFb(const AppItem &appItem);

		quint16 instance() const { return m_instance; }
		quint16 opcode() const { return afb().type().toOpCode(); }		// return FB type
		QString caption() const { return afb().caption(); }
		QString typeCaption() const { return afb().type().text(); }
		int number() const { return m_number; }

		QString instantiatorID();

		void setInstance(quint16 instance) { m_instance = instance; }
		void setNumber(int number) { m_number = number; }

		bool getAfbParamByIndex(int index, LogicAfbParam* afbParam) const;
		bool getAfbSignalByIndex(int index, LogicAfbSignal* afbSignal) const;

		bool calculateFbParamValues(ModuleLogicCompiler *compiler);			// implemented in file FbParamCalculation.cpp

		const AppFbParamValuesArray& paramValuesArray() const { return m_paramValuesArray; }

		int runTime() const { return m_runTime; }
	};


	class AppFbMap: public HashedVector<QUuid, AppFb*>
	{
	private:
		int m_fbNumber = 1;

	public:
		~AppFbMap() { clear(); }

		AppFb* insert(AppFb *appFb);
		void clear();
	};


	// Application Signal
	// represent all signal in application logic schemas, and signals, which createad in compiling time
	//

	class AppSignal //: public Signal
	{
	private:
		Signal* m_signal = nullptr;							// pointer to signal in m_signalSet

		const AppItem* m_appItem = nullptr;					// application signals pointer (for real signals)
															// application functional block pointer (for shadow signals)
		QUuid m_guid;

		bool m_isShadowSignal = false;

		bool m_computed = false;
		bool m_resultSaved = false;

	public:
		AppSignal(Signal* signal, const AppItem* appItem);
		AppSignal(const QUuid& guid, E::SignalType signalType, E::AnalogAppSignalFormat analogSignalFormat, int dataSize, const AppItem* appItem, const QString& appSignalID);

		~AppSignal();

		const AppItem &appItem() const;

		void setComputed() { m_computed = true; }
		bool isComputed() const;

		void setResultSaved() { m_resultSaved = true; }
		bool isResultSaved() const { return m_resultSaved; }

		bool isShadowSignal() const { return m_isShadowSignal; }

		QString appSignalID() const { return m_signal->appSignalID(); }

		QUuid guid() const;

		const Address16& ramAddr() const { return m_signal->ramAddr(); }
		const Address16& regAddr() const { return m_signal->regValueAddr(); }

		Address16& ramAddr() { return m_signal->ramAddr(); }
		Address16& regAddr() { return m_signal->regValueAddr(); }

		E::SignalType signalType() const { return m_signal->signalType(); }
		E::AnalogAppSignalFormat analogSignalFormat() const { return m_signal->analogSignalFormat(); }
		int dataSize() const { return m_signal->dataSize(); }

		bool isAnalog() const { return m_signal->isAnalog(); }
		bool isDiscrete() const { return m_signal->isDiscrete(); }
		bool isRegistered() const { return m_signal->isRegistered(); }
		bool isInternal() const { return m_signal->isInternal(); }
		bool isInput() const { return m_signal->isInput(); }
		bool isOutput() const { return m_signal->isOutput(); }

		bool isCompatibleDataFormat(const LogicAfbSignal& afbSignal) const;

		const Signal& constSignal() { return *m_signal; }

		Signal* signal() { return m_signal; }

		QString schemaID() const;
	};


	class AppSignalMap: public QObject, public HashedVector<QUuid, AppSignal*>
	{
		Q_OBJECT

	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;

		ModuleLogicCompiler& m_compiler;

		// counters for Internal signals only
		//
		int m_registeredAnalogSignalCount = 0;
		int m_registeredDiscreteSignalCount = 0;

		int m_notRegisteredAnalogSignalCount = 0;
		int m_notRegisteredDiscreteSignalCount = 0;

		void incCounters(const AppSignal* appSignal);

		QString getShadowSignalStrID(const AppFb* appFb, const LogicPin& outputPin);

	public:
		AppSignalMap(ModuleLogicCompiler& compiler);
		~AppSignalMap();

		bool insert(const AppItem* appItem);
		bool insert(const AppFb* appFb, const LogicPin& outputPin, IssueLogger* log);

		AppSignal* getByStrID(const QString& strID);

		void clear();
	};


	class ApplicationLogicCompiler;

	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	public:
		static const int FOR_USER_ONLY_PARAM_INDEX = -1;		// index of FB's parameters used by user only

	private:
		static const int ERR_VALUE = -1;

		struct Module
		{
			Hardware::DeviceModule* device = nullptr;
			int place = 0;

			// properties loaded from Hardware::DeviceModule::dynamicProperties
			//
			int txDataSize = 0;			// size of data transmitted from module to LM
			int txDiagDataOffset = 0;
			int txDiagDataSize = 0;
			int txAppDataOffset = 0;
			int txAppDataSize = 0;

			int rxDataSize = 0;			// size of data transmitted from LM to module
			int rxAppDataOffset = 0;
			int rxAppDataSize = 0;

			int appRegDataSize = 0;		// size of module data in registartion buffer

			// calculated fields
			//
			int moduleDataOffset = 0;	// offset of data received from module or transmitted to module in LM's memory
										// depends of module place in the chassis

			int appRegDataOffset = 0;	// offset of module application data in registration buffer

			bool isInputModule() const;
			bool isOutputModule() const;
			Hardware::DeviceModule::FamilyType familyType() const;
		};


		struct FbScal
		{
			QString caption;

			std::shared_ptr<Afb::AfbElement> pointer;

			int x1ParamIndex = -1;
			int x2ParamIndex = -1;
			int y1ParamIndex = -1;
			int y2ParamIndex = -1;

			int inputSignalIndex = -1;
			int outputSignalIndex = -1;
		};

		// input parameters
		//

		ApplicationLogicCompiler& m_appLogicCompiler;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;

		HashedVector<QString, Signal*> m_lmAssociatedSignals;

		Afb::AfbElementCollection* m_afbl = nullptr;
		AppLogicData* m_appLogicData = nullptr;
		AppLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		const Hardware::DeviceModule* m_lm = nullptr;
		const Hardware::DeviceChassis* m_chassis = nullptr;

		// compiler settings
		//

		bool m_convertUsedInOutAnalogSignalsOnly = false;

		// LM's and modules settings
		//
		int m_lmAppLogicFrameSize = 0;
		int m_lmAppLogicFrameCount = 0;

		int m_lmCycleDuration = 0;

		int m_lmClockFrequency = 96000000;
		int m_lmALPPhaseTime = 1000;
		int m_lmIDRPhaseTime = 2500;

		QString m_lmSubsystemID;
		int m_lmSubsystemKey = 0;
		int m_lmNumber = 0;
		int m_lmChannel = 0;

		// LM's calculated memory offsets and sizes
		//

		LmMemoryMap m_memoryMap;

		int m_bitAccumulatorAddress = 0;

		QVector<Module> m_modules;

		//

		ApplicationLogicCode m_code;

		int m_idrPhaseClockCount = 0;		// input data receive phase clock count
		int m_alpPhaseClockCount = 0;		// application logic processing clock count

		AfbMap m_afbs;

		AppSignalMap m_appSignals;
		AppFbMap m_appFbs;

		// service maps
		//
		HashedVector<QUuid, AppItem*> m_appItems;			// item GUID -> item ptr
		QHash<QUuid, AppItem*> m_pinParent;					// pin GUID -> parent item ptr
		QHash<QString, Signal*> m_signalsStrID;				// signals StrID -> Signal ptr
		QHash<QString, Signal*> m_deviceBoundSignals;		// device signal strID -> Signal ptr
		QHash<QUuid, QUuid> m_outPinSignal;					// output pin GUID -> signal GUID

		QHash<Hardware::DeviceModule::FamilyType, QString> m_moduleFamilyTypeStr;

		QString msg;

		QVector<FbScal> m_fbScal;

		const int FB_SCALE_16UI_FP_INDEX = 0;
		const int FB_SCALE_16UI_SI_INDEX = 1;
		const int FB_SCALE_FP_16UI_INDEX = 2;
		const int FB_SCALE_SI_16UI_INDEX = 3;

		static const char* INPUT_CONTROLLER_SUFFIX;
		static const char* OUTPUT_CONTROLLER_SUFFIX;
		static const char* PLATFORM_INTERFACE_CONTROLLER_SUFFIX;

		QVector<AppItem*> m_scalAppItems;
		QHash<QString, AppFb*> m_inOutSignalsToScalAppFbMap;

		Tuning::TuningData* m_tuningData = nullptr;

	private:

		bool getLMIntProperty(const QString& name, int* value);
		bool getLMStrProperty(const QString& name, QString *value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		// pass #1 compilation functions
		//
		bool getLMChassis();
		bool loadLMSettings();
		bool loadModulesSettings();

		bool prepareAppLogicGeneration();

		bool getLmAssociatedSignals();
		bool buildServiceMaps();
		bool createDeviceBoundSignalsMap();
		bool createAppSignalsMap();

		bool buildRS232SignalLists();
		bool buildOptoPortsSignalLists();

		// pass #2 compilation functions
		//

		bool generateAppStartCommand();
		bool generateFbTestCode();
		bool finishTestCode();
		bool startAppLogicCode();

		bool initAfbs();

		bool copyLMDataToRegBuf();
		bool copyInModulesAppLogicDataToRegBuf();

		bool copyLmOutSignalsToModuleMemory();

		bool copyDimDataToRegBuf(const Module& module);
		bool copyAimDataToRegBuf(const Module& module);
		bool copyAifmDataToRegBuf(const Module& module);
		bool copyMps17DataToRegBuf(const Module& module);

		bool initOutModulesAppLogicDataInRegBuf();
		bool initDOMAppLogicDataInRegBuf(const Module& module);
		bool initAOMAppLogicDataInRegBuf(const Module& module);

		bool generateAppLogicCode();
		bool generateAppSignalCode(const AppItem* appItem);
		bool generateFbCode(const AppItem *appItem);

		bool writeFbInputSignals(const AppFb *appFb);
		bool startFb(const AppFb* appFb);
		bool readFbOutputSignals(const AppFb *appFb);

		bool generateReadFuncBlockToSignalCode(const AppFb& appFb, const LogicPin& outPin, const QUuid& signalGuid);

		bool generateWriteConstToSignalCode(AppSignal& appSignal, const LogicConst& constItem);
		bool generateWriteSignalToSignalCode(AppSignal &appSignal, const AppSignal& srcSignal);

		bool generateWriteConstToFbCode(const AppFb& appFb, const LogicPin& inPin, const LogicConst& constItem);
		bool generateWriteSignalToFbCode(const AppFb& appFb, const LogicPin& inPin, const AppSignal& appSignal);

		bool genearateWriteReceiverToFbCode(const AppFb &appFb, const LogicPin& inPin, const LogicReceiver& receiver, const QUuid& receiverPinGuid);
		bool generateWriteReceiverToSignalCode(const LogicReceiver& receiver, AppSignal& appSignal, const QUuid& pinGuid);

		bool copyDiscreteSignalsToRegBuf();

		bool copyOutModulesAppLogicDataToModulesMemory();
		bool setLmAppLANDataSize();
		bool setLmAppLANDataUID(const QByteArray& lmAppCode, quint64 &uniqueID);

		bool generateRS232ConectionCode();
		bool generateRS232ConectionCode(std::shared_ptr<Hardware::Connection> connection, Hardware::OptoModule *optoModule, Hardware::OptoPort *optoPort);

		bool copyOptoConnectionsTxData();
		bool copyOptoPortTxData(Hardware::OptoPort* port);

		bool copyOptoPortTxRawDataData(Hardware::OptoPort* port);
		bool copyOptoPortTxAnalogSignals(Hardware::OptoPort* port);
		bool copyOptoPortTxDiscreteSignals(Hardware::OptoPort* port);

		bool copyOptoPortAllNativeRawData(Hardware::OptoPort* port, int& offset);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPort* port, int& offset, int modulePlace);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPort* port, int& offset, const Hardware::DeviceModule* module);
		bool copyOptoPortTxOptoPortRawData(Hardware::OptoPort* port, int& offset, const QString& portEquipmentID);

		bool copyRS232Signals();
		bool copyPortRS232Signals(Hardware::OptoModule* module, Hardware::OptoPort* rs232Port);
		bool copyPortRS232AnalogSignals(int portDataAddress, Hardware::OptoPort* rs232Port, QXmlStreamWriter& xmlWriter);
		bool copyPortRS232DiscreteSignals(int portDataAddress, Hardware::OptoPort* rs232Port, QXmlStreamWriter& xmlWriter);
		bool writeSignalsToSerialXml(QXmlStreamWriter& xmlWriter, QVector<Hardware::OptoPort::TxSignal> &txSignals);

		int getNededTuningFramesCount(int tuningFrameSizeBytes, int signalsCount, int signalValueSizeBits);

		bool copyDomDataToModuleMemory(const Module& module);
		bool copyAomDataToModuleMemory(const Module& module);

		bool buildTuningData();
		bool writeTuningInfoFile(const QString& lmCaption, const QString& subsystemID, int lmNumber);

		bool calculateCodeRunTime();

		bool finishAppLogicCode();

		bool findFbsForAnalogInOutSignalsConversion();
		bool appendFbsForAnalogInOutSignalsConversion();
		AppItem* createFbForAnalogInputSignalConversion(const Signal &signal);
		AppItem* createFbForAnalogOutputSignalConversion(const Signal &signal);

		bool createAppFbsMap();
		AppFb* createAppFb(const AppItem& appItem);

		bool initAppFbParams(AppFb* appFb, bool instantiatorsOnly);
		bool displayAfbParams(const AppFb& appFb);

		bool getUsedAfbs();
		QString getAppLogicItemStrID(const AppLogicItem& appLogicItem) const { AppItem appItem(appLogicItem); return appItem.strID(); }

		bool calculateLmMemoryMap();
		bool calculateInOutSignalsAddresses();
		bool calculateInternalSignalsAddresses();
		bool setOutputSignalsAsComputed();
		bool createOptoExchangeLists();

		bool buildOptoModulesStorage();

		bool generateApplicationLogicCode();

		bool writeResult();

		void displayTimingInfo();

		void writeLMCodeTestFile();

		bool checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const Signal& destSignal, QUuid destSignalUuid);
		bool checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const AppFb& fb, const LogicAfbSignal& afbSignal);

		bool writeOcmRsSignalsXml();

		void cleanup();

		void dumApplicationLogicItems();

		QString getSchemaID(const LogicConst& constItem);

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);
		~ModuleLogicCompiler();

		const SignalSet& signalSet() { return *m_signals; }
		Signal* getSignal(const QString& strID);

		IssueLogger* log() { return m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbs.getAfbSignal(afbStrID, signalIndex); }

		bool pass1();
		bool pass2();

		int lmCycleDuration() const { return m_lmCycleDuration; }
	};


}
