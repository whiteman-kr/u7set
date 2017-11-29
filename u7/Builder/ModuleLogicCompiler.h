#pragma once

#include "../lib/DeviceObject.h"

#include "../lib/OrderedHash.h"
#include "../lib/ModuleConfiguration.h"

#include "../TuningService/TuningDataStorage.h"

#include "BuildResultWriter.h"
#include "OptoModule.h"
#include "LmMemoryMap.h"
#include "ComparatorStorage.h"
#include "UalItems.h"
#include "MemWriteMap.h"

#include "../u7/Connection.h"

class LmDescription;

namespace Builder
{

	class ApplicationLogicCompiler;
	class ModuleLogicCompiler;

	typedef bool (ModuleLogicCompiler::*ModuleLogicCompilerProc)(void);
	typedef std::pair<ModuleLogicCompilerProc, const char*> ProcToCall;
	typedef std::vector<ProcToCall> ProcsToCallArray;

#define PROC_TO_CALL(procName)		{ &procName, #procName }

	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	public:
		struct AfblUsageInfo
		{
			int opCode = -1;
			QString caption;
			int usedInstances = 0;
			int maxInstances = 0;
			double usagePercent = 0;
			int version = 0;				// version of AFB implementation
		};

		struct ResourcesUsageInfo
		{
			QString lmEquipmentID;

			double codeMemoryUsed = 0;
			double bitMemoryUsed = 0;
			double wordMemoryUsed = 0;

			double idrPhaseTimeUsed = 0;			// Input Data Receive phase time
			double alpPhaseTimeUsed = 0;			// Application Logic Processing phase time

			//

			CodeFragmentMetrics initAfbs;
			CodeFragmentMetrics copyAcquiredRawDataInRegBuf;
			CodeFragmentMetrics convertAnalogInputSignals;
			CodeFragmentMetrics appLogicCode;
			CodeFragmentMetrics copyAcquiredAnalogOptoSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredAnalogBusChildSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredTuningAnalogSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredConstAnalogSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredDiscreteInputSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredTuningDiscreteSignalsToRegBuf;
			CodeFragmentMetrics copyAcquiredDiscreteConstSignalsToRegBuf;
			CodeFragmentMetrics copyOutputSignalsInOutputModulesMemory;
			CodeFragmentMetrics copyOptoConnectionsTxData;

			QVector<AfblUsageInfo> afblUsageInfo;
		};

	private:
		struct Module
		{
			bool isInputModule() const;
			bool isOutputModule() const;
			Hardware::DeviceModule::FamilyType familyType() const;

			//

			const Hardware::DeviceModule* device = nullptr;
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

			// calculated fields
			//
			int moduleDataOffset = 0;	// offset of data received from module or transmitted to module in LM's memory
										// depends of module place in the chassis

			int appRegDataOffset = 0;	// offset of module application data in registration buffer
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

		struct BusComposerInfo
		{
			bool busFillingCodeAlreadyGenerated = false;
			int busContentAddress = BAD_ADDRESS;
		};

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, const Hardware::DeviceModule* lm);
		~ModuleLogicCompiler();

		SignalSet& signalSet() { return *m_signals; }
		Signal* getSignal(const QString& appSignalID);

		IssueLogger* log() { return m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbls.getAfbSignal(afbStrID, signalIndex); }

		bool pass1();
		bool pass2();

		QString lmEquipmentID();
		ResourcesUsageInfo resourcesUsageInfo() { return m_resourcesUsageInfo; }

	private:
		// pass #1 compilation functions
		//
		bool loadLMSettings();
		bool loadModulesSettings();

		bool createChassisSignalsMap();

		bool createUalItemsMaps();
		QString getUalItemStrID(const AppLogicItem& appLogicItem) const;

		bool createUalAfbsMap();

		bool createUalSignals();
		bool createUalSignalsFromBusComposer(UalItem* ualItem);
		bool createUalSignalFromSignal(UalItem* ualItem, int passNo);
		bool createUalSignalFromConst(UalItem* ualItem);
		bool createUalSignalsFromAfbOuts(UalItem* ualItem);
		bool createUalSignalsFromReceiver(UalItem* ualItem);
		bool createUalSignalFromReceiverOutput(UalItem* ualItem, const LogicPin& outPin, const QString& appSignalID);
		bool createUalSignalFromReceiverValidity(UalItem* ualItem, const LogicPin& validityPin, const QString& validitySignalEquipmentID);
		bool linkUalSignalsFromBusExtractor(UalItem* ualItem);

		bool linkConnectedItems(UalItem* srcUalItem, const LogicPin& outPin, UalSignal* ualSignal);
		bool linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkAfbInput(UalItem* srcItem, UalItem* afbItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusComposerInput(UalItem* srcItem, UalItem* busComposerItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusExtractorInput(UalItem* srcItem, UalItem* busExtractorItem, QUuid inPinUuid, UalSignal* ualSignal);

		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal, const QString busTypeID);
		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal);
		Signal* getCompatibleConnectedBusSignal(const LogicPin& outPin, const QString busTypeID);

		bool isConnectedToTerminatorOnly(const LogicPin& outPin);
		bool determineOutBusTypeID(UalAfb* ualAfb, QString* outBusTypeID);

		bool checkInOutsConnectedToSignal(UalItem* ualItem, bool shouldConnectToSameSignal);
		bool checkPinsConnectedToSignal(const std::vector<LogicPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignal);

		bool appendRefPinToSignal(UalItem* ualItem, UalSignal* ualSignal);

		bool checkBusAndAfbInputCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem, QUuid destPinUuid);
		bool checkBusAndSignalCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);
		bool checkBusAndBusExtractorCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);

		bool buildTuningData();

		bool createSignalLists();

		bool createAcquiredDiscreteInputSignalsList();
		bool createAcquiredDiscreteStrictOutputSignalsList();
		bool createAcquiredDiscreteInternalSignalsList();
		bool createAcquiredDiscreteOptoAndBusChildSignalsList();
		bool createAcquiredDiscreteTuningSignalsList();
		bool createAcquiredDiscreteConstSignalsList();

		bool createNonAcquiredDiscreteStrictOutputSignalsList();
		bool createNonAcquiredDiscreteInternalSignalsList();

		bool createAcquiredAnalogInputSignalsList();
		bool createAcquiredAnalogStrictOutputSignalsList();
		bool createAcquiredAnalogInternalSignalsList();
		bool createAcquiredAnalogOptoSignalsList();
		bool createAcquiredAnalogBusChildSignalsList();
		bool createAcquiredAnalogTuninglSignalsList();
		bool createAcquiredAnalogConstSignalsList();

		bool createAnalogOutputSignalsToConversionList();

		bool createNonAcquiredAnalogInputSignalsList();
		bool createNonAcquiredAnalogStrictOutputSignalsList();
		bool createNonAcquiredAnalogInternalSignalsList();

		bool createAcquiredBusSignalsList();
		bool createNonAcquiredBusSignalsList();

		bool groupTxSignals();

		bool appendLinkedValiditySignal(const Signal* s);

		bool listsUniquenessCheck() const;
		bool listUniquenessCheck(QHash<UalSignal*, UalSignal*>& signalsMap, const QVector<UalSignal*>& signalList) const;
		void sortSignalList(QVector<UalSignal*> &signalList);

		bool disposeSignalsInMemory();

		bool calculateIoSignalsAddresses();

		bool setTuningableSignalsUalAddresses();

		// disposing discrete signals in memory
		//
		bool setDiscreteInputSignalsUalAddresses();
		bool disposeDiscreteSignalsInBitMemory();

		// disposing acquired analog, discrete and bus signals in registration buffer (word-addressed memory)
		//
		bool disposeAcquiredRawDataInRegBuf();
		bool disposeAcquiredAnalogSignalsInRegBuf();
		bool disposeAcquiredBusesInRegBuf();
		bool disposeAcquiredDiscreteSignalsInRegBuf();

		// disposing non acquired analog and bus signals in word-addressed memory
		//
		bool disposeNonAcquiredAnalogSignals();
		bool disposeNonAcquiredBuses();

		bool appendAfbsForAnalogInOutSignalsConversion();
		bool findFbsForAnalogInOutSignalsConversion();
		bool createAfbForAnalogInputSignalConversion(Signal& signal, UalItem& appItem);
		bool createFbForAnalogOutputSignalConversion(Signal& signal, UalItem& appItem);
		bool isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceSignal& deviceSignal, const Signal& appSignal);

		UalAfb* createUalAfb(const UalItem& appItem);
		bool setOutputSignalsAsComputed();

		bool processTxSignals();
		bool processSinglePortRxSignals();

		bool processTransmitters();
		bool processTransmitter(const UalItem* ualItem);
		bool getConnectedSignals(const UalItem* transmitterItem, QVector<QPair<QString, UalSignal *>>* connectedSignals);
		bool getNearestSignalID(const LogicPin& inPin, QString* nearestSignalID);

		bool processSinglePortReceivers();
		bool processSinglePortReceiver(const UalItem* item);

		bool setOptoRawInSignalsAsComputed();

		// pass #2 compilation functions
		//
		bool finalizeOptoConnectionsProcessing();
		bool setOptoUalSignalsAddresses();

		bool initAfbs();
		bool initAppFbParams(UalAfb* appFb, bool instantiatorsOnly);
		bool displayAfbParams(const UalAfb& appFb);

		bool startAppLogicCode();

		bool copyAcquiredRawDataInRegBuf();
		bool convertAnalogInputSignals();

		bool generateAppLogicCode();

		bool generateAfbCode(const UalItem* ualItem);

		bool generateSignalsToAfbInputsCode(const UalAfb* ualAfb, int busProcessingStep);
		bool generateSignalToAfbInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep);
		bool generateSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep);
		bool generateDiscreteSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal);
		bool generateBusSignalToAfbBusInputCode(const UalAfb* ualAfb, const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal, int busProcessingStep);

		bool startAfb(const UalAfb* ualAfb, int processingStep, int processingStepsNumber);

		bool generateAfbOutputsToSignalsCode(const UalAfb* ualAfb, int busProcessingStep);
		bool generateAfbOutputToSignalCode(const UalAfb* ualAfb, const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal, int busProcessingStep);
		bool generateAfbBusOutputToBusSignalCode(const UalAfb* ualAfb, const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal, int busProcessingStep);

		bool calcBusProcessingStepsNumber(const UalAfb* ualAfb, int* busProcessingStepsNumber);
		bool getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<LogicPin>& pins, int* pinsSize, int* signalsSize, bool isInputs);
		bool isBusProcessingAfb(const UalAfb* ualAfb, bool* isBusProcessing);

		//

		bool generateBusComposerCode(const UalItem* ualItem);
		UalSignal* getBusComposerBusSignal(const UalItem* composerItem, bool* connectedToTedrminatorOnly);
		bool generateAnalogSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal);
		bool generateDiscreteSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal);
		bool generateBusSignalToBusCode(UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal);

		UalItem* getInputPinAssociatedOutputPinParent(QUuid appItemUuid, const QString& inPinCaption, QUuid* connectedOutPinUuid) const;
		UalItem* getAssociatedOutputPinParent(const LogicPin& inputPin, QUuid* connectedOutPinUuid = nullptr) const;
		const UalSignal *getExtractorBusSignal(const UalItem* appBusExtractor);
		bool getConnectedAppItems(const LogicPin& pin, ConnectedAppItems* connectedAppItems);
		bool getBusProcessingParams(const UalAfb* appFb, bool& isBusProcessingAfb, QString& busTypeID);
		UalSignal* getPinInputAppSignal(const LogicPin& inPin);

		UalSignal* getUalSignalByPinCaption(const UalItem* ualItem, const QString& pinCaption, bool isInput);

		bool isConnectedToTerminator(const LogicPin& outPin);

		bool addToComparatorStorage(const UalAfb *appFb);
		bool initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* appFb);

		bool copyAcquiredAnalogOptoSignalsToRegBuf();
		bool copyAcquiredAnalogBusChildSignalsToRegBuf();

		bool copyAcquiredTuningAnalogSignalsToRegBuf();
		bool copyAcquiredTuningDiscreteSignalsToRegBuf();

		bool copyAcquiredAnalogConstSignalsToRegBuf();

		bool copyAcquiredDiscreteInputSignalsToRegBuf();
		bool copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf();
		bool copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf();
		bool copyAcquiredDiscreteConstSignalsToRegBuf();

		bool copyScatteredDiscreteSignalsInRegBuf(const QVector<UalSignal *> &m_acquiredDiscreteInputSignals, QString description);

		bool copyOutputSignalsInOutputModulesMemory();
		bool initOutputModulesMemory();
		bool conevrtOutputAnalogSignals();
		bool copyOutputDiscreteSignals();

		bool copyOptoConnectionsTxData();

		bool copyOptoPortTxData(Hardware::OptoPortShared port);
		bool copyOptoPortTxRawData(Hardware::OptoPortShared port);
		bool copyOptoPortTxAnalogSignals(Hardware::OptoPortShared port);
		bool copyOptoPortTxBusSignals(Hardware::OptoPortShared port);
		bool copyOptoPortTxDiscreteSignals(Hardware::OptoPortShared port);
		bool isCopyOptimizationAllowed(const Commands& copyCode, int* srcAddr);
		bool copyOptoPortAllNativeRawData(Hardware::OptoPortShared port, int& offset, MemWriteMap& memWriteMap);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, int modulePlace, MemWriteMap& memWriteMap);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, const Hardware::DeviceModule* module, MemWriteMap& memWriteMap);
		bool copyOptoPortTxOptoPortRawData(Hardware::OptoPortShared port, int& offset, const QString& portEquipmentID, MemWriteMap& memWriteMap);
		bool copyOptoPortTxConst16RawData(Hardware::OptoPortShared port, int const16value, int& offset, MemWriteMap& memWriteMap);
		bool copyOptoPortRawTxAnalogSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap);
		bool copyOptoPortRawTxDiscreteSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap);
		bool copyOptoPortRawTxBusSignals(Hardware::OptoPortShared port, MemWriteMap& memWriteMap);

		bool finishAppLogicCode();
		bool setLmAppLANDataSize();
		bool calculateCodeRunTime();

		bool writeResult();
		bool setLmAppLANDataUID(const QByteArray& lmAppCode, quint64 &uniqueID);
		bool writeTuningInfoFile(const QString& subsystemID, int lmNumber);
		bool writeOcmRsSignalsXml();
		void writeLMCodeTestFile();

		bool displayResourcesUsageInfo();
		void calcOptoDiscretesStatistics();
		bool getAfblUsageInfo();
		void cleanup();

		bool checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const Signal& destSignal, QUuid destSignalUuid);
		bool checkSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const LogicAfbSignal& afbSignal);

		bool isUsedInUal(const Signal* s) const;
		bool isUsedInUal(const QString& appSignalID) const;

		QString getSchemaID(QUuid itemUuid);

		bool getLMIntProperty(const QString& name, int* value);
		bool getLMStrProperty(const QString& name, QString *value);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		void dumpApplicationLogicItems();

		const HashedVector<QString, Signal*>& chassisSignals() const { return m_chassisSignals; }

		bool writeSignalLists();
		bool writeSignalList(const QVector<UalSignal *> &signalList, QString listName) const;
		bool writeUalSignalsList() const;

		bool runProcs(const ProcsToCallArray& procArray);

		Address16 constBit0Addr() const { return m_memoryMap.constBit0Addr(); }
		Address16 constBit1Addr() const { return m_memoryMap.constBit1Addr(); }

		Address16 getConstBitAddr(UalSignal* constDiscreteUalSignal);

	private:
		static const int ERR_VALUE = -1;

		// input parameters
		//
		ApplicationLogicCompiler& m_appLogicCompiler;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		ComparatorStorage* m_cmpStorage = nullptr;

		std::shared_ptr<LmDescription> m_lmDescription;
		AppLogicData* m_appLogicData = nullptr;
		AppLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		const Hardware::DeviceModule* m_lm = nullptr;
		const Hardware::DeviceChassis* m_chassis = nullptr;

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

		int m_lmDescriptionNumber = 0;

		// LM's calculated memory offsets and sizes
		//
		LmMemoryMap m_memoryMap;

		HashedVector<QString, Module> m_modules;		// modules installed in chassis, module EquipmentID => Module

		//

		ApplicationLogicCode m_code;

		int m_idrPhaseClockCount = 0;		// input data receive phase clock count
		int m_alpPhaseClockCount = 0;		// application logic processing clock count

		AfblsMap m_afbls;

		UalSignalsMap m_ualSignals;
		UalAfbsMap m_ualAfbs;

		// service maps
		//
		HashedVector<QUuid, UalItem*> m_ualItems;				// item GUID => item ptr
		QHash<QUuid, UalItem*> m_pinParent;						// pin GUID => parent item ptr

		HashedVector<QString, Signal*> m_chassisSignals;		// all signals available in current chassis, AppSignalID => Signal*
		QHash<QString, Signal*> m_ioSignals;					// input/output signals of current chassis, AppSignalID => Signal*
		QHash<QString, Signal*> m_equipmentSignals;				// equipment signals to app signals map, signal EquipmentID => Signal*

		QHash<QString, QString> m_linkedValidtySignalsID;		// device signals with linked validity signals
																// DeviceSignalEquipmentID => LinkedValiditySignalEquipmentID

		QVector<UalSignal*> m_acquiredDiscreteInputSignals;				// acquired discrete input signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredDiscreteStrictOutputSignals;		// acquired discrete strict output signals, used in UAL
		QVector<UalSignal*> m_acquiredDiscreteInternalSignals;			// acquired discrete internal non tuningable signals, used in UAL
		QVector<UalSignal*> m_acquiredDiscreteTuningSignals;			// acquired discrete internal tuningable signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredDiscreteConstSignals;
		QVector<UalSignal*> m_acquiredDiscreteOptoAndBusChildSignals;

		QVector<UalSignal*> m_nonAcquiredDiscreteStrictOutputSignals;	// non acquired discrete output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteInternalSignals;		// non acquired discrete internal non tuningbale signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteOptoSignals;			// non acquired discrete internal opto signals, used in UAL

		QVector<UalSignal*> m_acquiredAnalogInputSignals;				// acquired analog input signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredAnalogStrictOutputSignals;		// acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogInternalSignals;			// acquired analog internal signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogOptoSignals;				// acquired analog opto signals (simple copied from opto buffers)
		QVector<UalSignal*> m_acquiredAnalogBusChildSignals;			// acquired analog opto signals (unlike to opto signals may require conversion from inbus format)
		QVector<UalSignal*> m_acquiredAnalogTuningSignals;				// acquired analog internal tuningable signals, no matter used in UAL or not

		QHash<int, UalSignal*> m_acquiredAnalogConstIntSignals;
		QHash<float, UalSignal*> m_acquiredAnalogConstFloatSignals;

		QVector<UalSignal*> m_nonAcquiredAnalogInputSignals;			// non acquired analog input signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogStrictOutputSignals;		// non acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogInternalSignals;			// non acquired analog internal non tunigable signals, used in UAL

		QVector<Signal*> m_analogOutputSignalsToConversion;				// all analog output signals requires conversion

		QVector<UalSignal*> m_acquiredBuses;							// acquired bus signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredBuses;							// non acquired bus signals, used in UAL

		//QHash<Signal*, Signal*> m_acquiredDiscreteInputSignalsMap;		// is used in conjunction with m_acquiredDiscreteInputSignals
																		// for grant unique records

		QHash<QUuid, QUuid> m_outPinSignal;								// output pin GUID -> signal GUID

		QHash<Hardware::DeviceModule::FamilyType, QString> m_moduleFamilyTypeStr;

		QString msg;

		ResourcesUsageInfo m_resourcesUsageInfo;

		QVector<FbScal> m_fbScal;

		static const int FB_SCALE_16UI_FP_INDEX = 0;
		static const int FB_SCALE_16UI_SI_INDEX = 1;
		static const int FB_SCALE_FP_16UI_INDEX = 2;
		static const int FB_SCALE_SI_16UI_INDEX = 3;

		static const char* INPUT_CONTROLLER_SUFFIX;
		static const char* OUTPUT_CONTROLLER_SUFFIX;
		static const char* PLATFORM_INTERFACE_CONTROLLER_SUFFIX;

		static const char* BUS_COMPOSER_CAPTION;
		static const char* BUS_EXTRACTOR_CAPTION;

		static const char* TEST_DATA_DIR;

		QVector<UalItem*> m_scalAppItems;
		QHash<QString, UalAfb*> m_inOutSignalsToScalAppFbMap;

		Tuning::TuningData* m_tuningData = nullptr;
	};

}
