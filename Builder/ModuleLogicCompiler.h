#pragma once

#include "../lib/DeviceObject.h"

#include "../lib/OrderedHash.h"
#include "../lib/ModuleFirmware.h"

#include "../TuningService/TuningDataStorage.h"

#include "BuildResultWriter.h"
#include "OptoModule.h"
#include "LmMemoryMap.h"
#include "../lib/ComparatorSet.h"
#include "UalItems.h"
#include "MemWriteMap.h"
#include "Loopbacks.h"

#include "../lib/Connection.h"
#include "../lib/AppSignalStateFlags.h"

class LmDescription;

namespace Builder
{

	class ApplicationLogicCompiler;
	class ModuleLogicCompiler;

	typedef bool (ModuleLogicCompiler::*ModuleLogicCompilerProc)(void);
	typedef std::pair<ModuleLogicCompilerProc, const char*> ProcToCall;
	typedef std::vector<ProcToCall> ProcsToCallArray;

#define PROC_TO_CALL(procName)		{ &procName, #procName }

	typedef bool (ModuleLogicCompiler::*ModuleLogicCompilerCodeGenProc)(CodeSnippet*);
	typedef std::pair<ModuleLogicCompilerCodeGenProc, const char*> CodeGenProcToCall;
	typedef std::vector<CodeGenProcToCall> CodeGenProcsToCallArray;

#define CODE_GEN_PROC_TO_CALL(procName)		{ &procName, #procName }

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

			CodeSnippetMetrics initAfbs;
			CodeSnippetMetrics copyAcquiredRawDataInRegBuf;
			CodeSnippetMetrics convertAnalogInputSignals;
			CodeSnippetMetrics appLogicCode;
			CodeSnippetMetrics copyAcquiredAnalogOptoSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredAnalogBusChildSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredTuningAnalogSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredConstAnalogSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredDiscreteInputSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredTuningDiscreteSignalsToRegBuf;
			CodeSnippetMetrics copyAcquiredDiscreteConstSignalsToRegBuf;
			CodeSnippetMetrics copyOutputSignalsInOutputModulesMemory;
			CodeSnippetMetrics copyOptoConnectionsTxData;

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

		struct BusProcessingStepInfo
		{
			int stepsNumber = 0;
			int currentStep = 0;
			int currentStepSizeBits = 0;			// for now 32 or 16 only
			int currentBusSignalOffsetW = 0;

			bool isLastStep() const { return currentStep == (stepsNumber - 1); }
		};

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, const Hardware::DeviceModule* lm);
		~ModuleLogicCompiler();

		SignalSet* signalSet() { return m_signals; }
		Signal* getSignal(const QString& appSignalID);

		IssueLogger* log() { return m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbls.getAfbSignal(afbStrID, signalIndex); }

		bool pass1();
		bool pass2();

		QString lmEquipmentID() const;
		int lmDescriptionNumber() const;

		bool expertMode() const;
		bool generateExtraDebugInfo() const;

		const ResourcesUsageInfo& resourcesUsageInfo() { return m_resourcesUsageInfo; }

		void setModuleCompilersRef(const QVector<ModuleLogicCompiler*>* moduleCompilers);

		bool getSignalsAndPinsLinkedToItem(const UalItem* item,
										   std::set<QString>* linkedSignals,
										   std::set<const UalItem*>* linkedItems,
										   std::map<QUuid, const UalItem*>* linkedPins);
	private:
		// pass #1 compilation functions
		//
		bool loadLMSettings();
		bool loadModulesSettings();

		bool createChassisSignalsMap();

		bool createUalItemsMaps();
		QString getUalItemStrID(const AppLogicItem& appLogicItem) const;

		bool createUalAfbsMap();

		//

		bool createUalSignals();
		bool writeUalItemsFile();

		bool loopbacksPreprocessing();
		bool findAndProcessSingleItemLoopbacks();
		void getInputsDirectlyConnectedToOutput(const UalItem* ualItem,
										const LogicPin& output,
										QVector<QUuid>* connectedInputsGuids);
		QString getConnectedLoopbackSourceID(const LogicPin& output);

		bool findLoopbackSources();
		bool findLoopbackTargets();
		bool findSignalsAndPinsLinkedToLoopbackTargets();

		bool getSignalsAndPinsLinkedToOutPin(	const UalItem* item,
												const LogicPin& outPin,
												std::set<QString>* linkedSignals,
												std::set<const UalItem*>* linkedItems,
												std::map<QUuid, const UalItem*>* linkedPins);

		bool createUalSignalsFromInputAndTuningAcquiredSignals();

		bool createUalSignalsFromBusComposers();
		bool createUalSignalsFromBusComposer(UalItem* ualItem);
		UalSignal* createBusParentSignal(UalItem* ualItem, const LogicPin& outPin, Signal* s, const QString& busTypeID);
		UalSignal* createBusParentSignalFromBusExtractorConnectedToDiscreteSignal(UalItem* ualItem);

		bool createUalSignalsFromReceivers();
		bool createUalSignalsFromReceiver(UalItem* ualItem);
		bool createUalSignalFromReceiverOutput(UalItem* ualItem, const LogicPin& outPin, const QString& appSignalID, bool isSinglePortConnection);
		bool createUalSignalFromReceiverValidity(UalItem* ualItem, const LogicPin& validityPin, std::shared_ptr<Hardware::Connection> connection);
		bool getReceiverConnectionID(const UalReceiver* receiver, QString* connectionID, const QString& schemaID);

		bool createUalSignalFromSignal(UalItem* ualItem, int passNo);
		bool createUalSignalFromConst(UalItem* ualItem);
		bool createUalSignalsFromAfbOuts(UalItem* ualItem);
		bool linkUalSignalsFromBusExtractor(UalItem* ualItem);

		bool linkConnectedItems(UalItem* srcUalItem, const LogicPin& outPin, UalSignal* ualSignal);
		bool linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkAfbInput(UalItem* srcItem, UalItem* afbItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkSetFlagsItemInput(UalItem* srcItem, UalItem* setFlagsItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusComposerInput(UalItem* srcItem, UalItem* busComposerItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusExtractorInput(UalItem* srcItem, UalItem* busExtractorItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkLoopbackSource(UalItem* loopbackSourceItem, QUuid inPinUuid, UalSignal* ualSignal);

		bool checkLoopbacks();
		bool linkLoopbackTargets();
		bool linkLoopbackTarget(UalItem* loopbackTargetItem);
		bool removeLoopbackSignalsFromHeap();

		bool checkBusProcessingItemsConnections();

		bool processSignalsWithFlags();
		bool processAcquiredIOSignalsValidity();
		bool processSimlockItems();
		bool processMismatchItems();
		bool processSetFlagsItems();

		bool appendFlagToSignal(const QString& signalWithFlagID,
								E::AppSignalStateFlagType flagType,
								const QString& flagSignalID,
								const UalItem* setFlagsItem);

		bool appendFlagToSignalFromPin(const UalItem* ualItem,
								const QString& pinCaption,
								bool pinShouldBeExist,
								E::AppSignalStateFlagType flagType,
								const QString& signalWithFlagID,
								bool* flagIsSet);

		bool setAcquiredForFlagSignals();
		bool checkSignalsWithFlags();
		void writeSignalsWithFlagsReport();

		bool sortUalSignals();

		//

		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal, const QString& busTypeID);
		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal);
		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const Signal& s);
		Signal* getCompatibleConnectedBusSignal(const LogicPin& outPin, const QString busTypeID);
		bool isCompatible(const LogicAfbSignal& outAfbSignal, const QString& busTypeID, const Signal* s);

		bool isConnectedToTerminatorOnly(const LogicPin& outPin);
		bool isConnectedToLoopback(const LogicPin& inPin, std::shared_ptr<Loopback>* loopback);
		bool determineOutBusTypeID(UalAfb* ualAfb, QString* outBusTypeID);
		bool determineBusTypeByInputs(const UalAfb* ualAfb, QString* outBusTypeID);
		bool determineBusTypeByOutput(const UalAfb* ualAfb, QString* outBusTypeID);
		bool isBusTypesAreEqual(const QStringList& busTypes);
		std::optional<int> getOutPinExpectedReadCount(const LogicPin& outPin);

		bool checkInOutsConnectedToSignal(UalItem* ualItem, bool shouldConnectToSameSignal);
		bool checkPinsConnectedToSignal(const std::vector<LogicPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignal);

		bool appendRefPinToSignal(UalItem* ualItem, UalSignal* ualSignal);

		bool checkBusAndAfbInputCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem, QUuid destPinUuid);
		bool checkBusAndSignalCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);
		bool checkBusAndBusExtractorCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);

		bool buildTuningData();
		bool getTuningSettings(bool* tuningPropertyExists, bool* tuningEnabled);

		bool createSignalLists();

		bool createAcquiredDiscreteInputSignalsList();
		bool createAcquiredDiscreteStrictOutputSignalsList();
		bool createAcquiredDiscreteInternalSignalsList();
		bool createAcquiredDiscreteOptoAndBusChildSignalsList();
		bool createAcquiredDiscreteTuningSignalsList();
		bool createAcquiredDiscreteConstSignalsList();

		bool createNonAcquiredDiscreteInputSignalsList();
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

		bool setSignalsCalculatedAttributes();

		bool groupTxSignals();

		bool listsUniquenessCheck() const;
		bool listUniquenessCheck(QHash<UalSignal*, UalSignal*>& signalsMap, const QVector<UalSignal*>& signalList) const;
		void sortSignalList(QVector<UalSignal*> &signalList);

		bool disposeSignalsInMemory();

		bool calculateIoSignalsAddresses();

		bool setTunableSignalsUalAddresses();

		// disposing discrete signals in memory
		//
		bool setDiscreteInputSignalsUalAddresses();
		bool disposeDiscreteSignalsInBitMemory();
		bool disposeDiscreteSignalsHeap();

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

		bool disposeAnalogAndBusSignalsHeap();

		bool appendAfbsForAnalogInOutSignalsConversion();
		bool findFbsForAnalogInOutSignalsConversion();
		bool createAfbForAnalogInputSignalConversion(const Signal& signal, UalItem* appItem, bool* needConversion);
		bool createFbForAnalogOutputSignalConversion(const Signal& signal, UalItem* appItem, bool* needConversion);
		bool isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceSignal& deviceSignal, const Signal& appSignal);

		UalAfb* createUalAfb(const UalItem& appItem);
		bool setOutputSignalsAsComputed();

		bool processTxSignals();
		bool processSinglePortRxSignals();

		bool processTransmitters();
		bool processTransmitter(const UalItem* ualItem);
		bool getConnectedSignals(const UalItem* transmitterItem, QVector<QPair<QString, UalSignal *>>* connectedSignals);

		bool getDirectlyConnectedInSignalID(const LogicPin& inPin, QString* directlyConnectedInSignalID);
		bool getNearestInSignalIDs(const LogicPin& inPin, QStringList* nearestSignalIDs);
		bool getNearestInSignalID(const LogicPin& inPin, QString* nearestSignalID);
		bool getNearestOutSignalIDs(const LogicPin& outPin, QStringList* nearestSignalIDs);
		bool getNearestOutSignalID(const LogicPin& outPin, QString* nearestSignalID);
		bool getNearestSignalID(const LogicPin& inOutPin, QString* nearestSignalID);


		bool processSinglePortReceivers();
		bool processSinglePortReceiver(const UalItem* item);

		bool setOptoRawInSignalsAsComputed();

		bool fillComparatorSet();

		// pass #2 compilation functions
		//
		bool initComparatorSignals();
		bool finalizeOptoConnectionsProcessing();
		bool setOptoUalSignalsAddresses();

		bool generateIdrPhaseCode();
		bool generateAlpPhaseCode();
		bool makeAppLogicCode();
		bool finalizeAppLogicCodeGeneration();

		bool generateAfbsVersionCheckingCode(CodeSnippet* code);
		bool generateInitAfbsCode(CodeSnippet* code);
		bool generateInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, const QString& usedBy);
		bool displayAfbParams(CodeSnippet* code, const UalAfb& appFb);
		bool generateLoopbacksRefreshingCode(CodeSnippet* code);
		bool getRefreshingCode(CodeSnippet* code, const QString& loopbackID, const UalSignal* lbSignal);
		bool constBitsInitialization(CodeSnippet*code);

		bool copyAcquiredRawDataInRegBuf(CodeSnippet* code);
		bool convertAnalogInputSignals(CodeSnippet* code);

		bool generateAppLogicCode(CodeSnippet* code);

		bool generateAfbCode(CodeSnippet* code, const UalItem* ualItem);
		bool generateSignalsToAfbInputsCode(CodeSnippet* code, const UalAfb* ualAfb,
											const BusProcessingStepInfo& bpStepInfo);

		bool generateSignalToAfbInputCode(CodeSnippet* code, const UalAfb* ualAfb,
										  const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
										  const BusProcessingStepInfo& bpStepInfo);

		bool generateSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
											 const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
											 const BusProcessingStepInfo& bpStepInfo);

		bool generateDiscreteSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
													 const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
													 const BusProcessingStepInfo& bpStepInfo);

		bool generateBusSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
												const LogicAfbSignal& inAfbSignal, const UalSignal* inUalSignal,
												const BusProcessingStepInfo& bpStepInfo);

		bool startAfb(CodeSnippet* code, const UalAfb* ualAfb, const BusProcessingStepInfo& bpStepInfo);

		bool generateAfbOutputsToSignalsCode(CodeSnippet* code, const UalAfb* ualAfb,
											 const BusProcessingStepInfo& bpStepInfo);

		bool generateAfbOutputToSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
										   const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal,
										   const BusProcessingStepInfo& bpStepInfo);

		bool generateAfbBusOutputToBusSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
												 const LogicAfbSignal& outAfbSignal, const UalSignal* outUalSignal,
												 const BusProcessingStepInfo& bpStepInfo);

		bool calcBusProcessingSteps(const UalAfb* ualAfb, std::vector<int>* busProcessingStepsSizes);
		bool getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<LogicPin>& pins,
									   std::vector<std::vector<int>>* pinsSizes, int* signalsSize, bool isInputs,
									   bool* allBusInputsConnectedToDiscretes);
		bool isBusProcessingAfb(const UalAfb* ualAfb, bool* isBusProcessing);

		//

		bool generateBusComposerCode(CodeSnippet* code, const UalItem* ualItem);
		UalSignal* getBusComposerBusSignal(const UalItem* composerItem, bool* connectedToTedrminatorOnly);
		bool generateAnalogSignalToBusCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal);
		bool getAnalogSignalToInbusSignalConversionCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal);
		bool get_SignedInt32_To_Unsigned16_BE_NoScale_inbusSignalCoversionCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal);
		bool generateDiscreteSignalToBusCode(CodeSnippet* code, const UalSignal* inputSignal, const UalSignal* busChildSignal, const BusSignal& busSignal);
		bool generateBusSignalToBusCode(CodeSnippet* code, UalSignal* inputSignal, UalSignal* busChildSignal, const BusSignal& busSignal);

		bool generateDiscreteSignalToBusExtractorCode(CodeSnippet* code, const UalItem* ualItem);

		UalItem* getInputPinAssociatedOutputPinParent(QUuid appItemUuid, const QString& inPinCaption, QUuid* connectedOutPinUuid) const;
		UalItem* getAssociatedOutputPinParent(const LogicPin& inputPin, QUuid* connectedOutPinUuid = nullptr) const;
		const UalSignal *getExtractorBusSignal(const UalItem* appBusExtractor);
		bool getConnectedAppItems(const LogicPin& pin, ConnectedAppItems* connectedAppItems);
		bool getBusProcessingParams(const UalAfb* appFb, bool& isBusProcessingAfb, QString& busTypeID);
		UalSignal* getPinInputAppSignal(const LogicPin& inPin);

		UalSignal* getUalSignalByPinCaption(const UalItem* ualItem, const QString& pinCaption, bool isInput);

		bool addToComparatorSet(const UalAfb *appFb);
		bool initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* appFb);

		bool copyAcquiredAnalogOptoSignalsToRegBuf(CodeSnippet* code);
		bool copyAcquiredAnalogBusChildSignalsToRegBuf(CodeSnippet* code);

		bool copyAcquiredTuningAnalogSignalsToRegBuf(CodeSnippet* code);
		bool copyAcquiredTuningDiscreteSignalsToRegBuf(CodeSnippet* code);

		bool copyAcquiredAnalogConstSignalsToRegBuf(CodeSnippet* code);

		bool copyAcquiredDiscreteInputSignalsToRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteOptoAndBusChildSignalsToRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteConstSignalsToRegBuf(CodeSnippet* code);

		bool copyScatteredDiscreteSignalsInRegBuf(CodeSnippet* code, const QVector<UalSignal *>& signalsList, const QString& description);

		bool copyOutputSignalsInOutputModulesMemory(CodeSnippet* code);
		bool initOutputModulesMemory(CodeSnippet* code);
		bool conevrtOutputAnalogSignals(CodeSnippet* code);
		bool copyOutputDiscreteSignals(CodeSnippet* code);

		bool copyOptoConnectionsTxData(CodeSnippet* code);

		bool copyOptoPortTxData(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortTxRawData(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortTxAnalogSignals(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortTxBusSignals(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortTxDiscreteSignals(CodeSnippet* code, Hardware::OptoPortShared port);
		bool isCopyOptimizationAllowed(const CodeSnippet& copyCode, int* srcAddr);
		bool copyOptoPortAllNativeRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset);
		bool copyOptoPortTxModuleOnPlaceRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, int modulePlace);
		bool copyOptoPortTxModuleRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, const Hardware::DeviceModule* module);
		bool copyOptoPortTxOptoPortRawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, const QString& portEquipmentID);
		bool copyOptoPortTxConst16RawData(CodeSnippet* code, Hardware::OptoPortShared port, int* rawDataOffset, int const16value);
		bool copyOptoPortRawTxAnalogSignals(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortRawTxDiscreteSignals(CodeSnippet* code, Hardware::OptoPortShared port);
		bool copyOptoPortRawTxBusSignals(CodeSnippet* code, Hardware::OptoPortShared port);

		bool setLmAppLANDataSize();
		bool detectUnusedSignals();
		bool calculateCodeRunTime();

		QString lmSubsystemEquipmentIdPath() const;

		bool writeResult();
		bool writeBinCodeForLm();
		bool calcAppLogicUniqueID(const QByteArray& lmAppCode);
		bool writeTuningInfoFile();
		bool writeOcmRsSignalsXml();
		bool writeLooopbacksReport();
		bool writeHeapsLog();

		bool displayResourcesUsageInfo();
		void calcOptoDiscretesStatistics();
		bool getAfblUsageInfo();
		void cleanup();

		bool checkLoopbackTargetSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const Signal& destSignal, QUuid destSignalUuid);
		bool checkLoopbackTargetSignalsCompatibility(const Signal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const LogicAfbSignal& afbSignal);

		bool isUsedInUal(const Signal* s) const;
		bool isUsedInUal(const QString& appSignalID) const;

		QString getSchemaID(QUuid itemUuid);

		bool getLMIntProperty(const QString& name, int* value);
		bool getLMStrProperty(const QString& name, QString *value);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		std::shared_ptr<Hardware::DeviceObject> getDeviceSharedPtr(const Hardware::DeviceObject* device);
		std::shared_ptr<Hardware::DeviceObject> getDeviceSharedPtr(const QString& deviceEquipmentID);
		std::shared_ptr<Hardware::DeviceModule> getLmSharedPtr();

		void dumpApplicationLogicItems();

		const HashedVector<QString, Signal*>& chassisSignals() const { return m_chassisSignals; }

		bool writeSignalLists();
		bool writeSignalList(const QVector<UalSignal *> &signalList, QString listName) const;
		bool writeUalSignalsList() const;

		bool runProcs(const ProcsToCallArray& procArray);
		bool runCodeGenProcs(const CodeGenProcsToCallArray& procArray, CodeSnippet* code);

		Address16 constBit0Addr() const { return m_memoryMap.constBit0Addr(); }
		Address16 constBit1Addr() const { return m_memoryMap.constBit1Addr(); }

		Address16 getConstBitAddr(UalSignal* constDiscreteUalSignal);

		CodeItem codeSetMemory(int addrFrom, quint16 constValue, int sizeW, const QString& comment);

		UalSignalsMap& ualSignals() { return m_ualSignals; }

		QString getFormatStr(const Hardware::DeviceSignal& ds);
		QString getFormatStr(const Signal& s);
		QString getFormatStr(E::SignalType signalType, E::DataFormat dataFormat, int dataSizeBits, E::ByteOrder byteOrder);

	private:
		// input parameters
		//
		ApplicationLogicCompiler& m_appLogicCompiler;
		Context* m_context = nullptr;
		const Hardware::DeviceModule* m_lm = nullptr;

		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		ComparatorSet* m_cmpSet = nullptr;

		std::shared_ptr<LmDescription> m_lmDescription;
		AppLogicData* m_appLogicData = nullptr;
		AppLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		mutable IssueLogger* m_log = nullptr;

		const Hardware::DeviceChassis* m_chassis = nullptr;

		// LM's and modules settings
		//
		int m_lmCodeMemorySize = 0;
		int m_lmAppMemorySize = 0;
		int m_lmAppLogicFramePayload = 0;
		int m_lmAppLogicFrameCount = 0;

		int m_lmCycleDuration = 0;

		int m_lmClockFrequency = 96000000;
		int m_lmALPPhaseTime = 1000;
		int m_lmIDRPhaseTime = 2500;

		QString m_lmSubsystemID;
		int m_lmSubsystemKey = 0;
		int m_lmNumber = 0;
		int m_lmChannel = 0;

		quint64 m_appLogicUniqueID = 0;

		// LM's calculated memory offsets and sizes
		//
		LmMemoryMap m_memoryMap;

		HashedVector<QString, Module> m_modules;		// modules installed in chassis, module EquipmentID => Module

		//

		ApplicationLogicCode m_code;

		CodeSnippet m_idrCode;
		CodeSnippet m_alpCode;

		int m_idrPhaseClockCount = 0;		// input data receive phase clock count
		int m_alpPhaseClockCount = 0;		// application logic processing clock count

		AfblsMap m_afbls;

		UalSignalsMap m_ualSignals;
		UalAfbsMap m_ualAfbs;

		QHash<UalSignal*, UalSignal*> m_outUalSignals;		// output UAL signals map: outUalSignal -> sourceUalSignal

		// service maps
		//
		HashedVector<QUuid, UalItem*> m_ualItems;				// item GUID => item ptr
		QHash<QUuid, UalItem*> m_pinParent;						// pin GUID => parent item ptr

		HashedVector<QString, Signal*> m_chassisSignals;		// all signals available in current chassis, AppSignalID => Signal*
		QHash<QString, Signal*> m_ioSignals;					// input/output signals of current chassis, AppSignalID => Signal*
		QHash<QString, Signal*> m_equipmentSignals;				// equipment signals to app signals map, signal EquipmentID => Signal*

		::std::set<QString> m_signalsWithFlagsIDs;

		Loopbacks m_loopbacks;

		QVector<UalSignal*> m_acquiredDiscreteInputSignals;				// acquired discrete input signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredDiscreteStrictOutputSignals;		// acquired discrete strict output signals, used in UAL
		QVector<UalSignal*> m_acquiredDiscreteInternalSignals;			// acquired discrete internal non tunable signals, used in UAL
		QVector<UalSignal*> m_acquiredDiscreteTuningSignals;			// acquired discrete internal tunable signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredDiscreteConstSignals;
		QVector<UalSignal*> m_acquiredDiscreteOptoAndBusChildSignals;

		QVector<UalSignal*> m_nonAcquiredDiscreteInputSignals;			// non acquired discrete input signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteStrictOutputSignals;	// non acquired discrete output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteInternalSignals;		// non acquired discrete internal non tuningbale signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteOptoSignals;			// non acquired discrete internal opto signals, used in UAL

		QVector<UalSignal*> m_acquiredAnalogInputSignals;				// acquired analog input signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredAnalogStrictOutputSignals;		// acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogInternalSignals;			// acquired analog internal signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogOptoSignals;				// acquired analog opto signals (simple copied from opto buffers)
		QVector<UalSignal*> m_acquiredAnalogBusChildSignals;			// acquired analog opto signals (unlike to opto signals may require conversion from inbus format)
		QVector<UalSignal*> m_acquiredAnalogTuningSignals;				// acquired analog internal tunable signals, no matter used in UAL or not

		QHash<int, UalSignal*> m_acquiredAnalogConstIntSignals;
		QHash<float, UalSignal*> m_acquiredAnalogConstFloatSignals;

		QVector<UalSignal*> m_nonAcquiredAnalogInputSignals;			// non acquired analog input signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogStrictOutputSignals;		// non acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogInternalSignals;			// non acquired analog internal non tunigable signals, used in UAL

		QVector<Signal*> m_analogOutputSignalsToConversion;				// all analog output signals requires conversion

		QVector<UalSignal*> m_acquiredBuses;							// acquired bus signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredBuses;							// non acquired bus signals, used in UAL

		QHash<QUuid, QUuid> m_outPinSignal;								// output pin GUID -> signal GUID

		ResourcesUsageInfo m_resourcesUsageInfo;

		QVector<FbScal> m_fbScal;

		static const int FB_SCALE_16UI_FP_INDEX = 0;
		static const int FB_SCALE_16UI_SI_INDEX = 1;
		static const int FB_SCALE_FP_16UI_INDEX = 2;
		static const int FB_SCALE_SI_16UI_INDEX = 3;
		static const int FB_TCONV_FP_SI_INDEX = 4;
		static const int FB_TCONV_SI_FP_INDEX = 5;

		static const char* INPUT_CONTROLLER_SUFFIX;
		static const char* OUTPUT_CONTROLLER_SUFFIX;
		static const char* PLATFORM_INTERFACE_CONTROLLER_SUFFIX;

		static const char* BUS_COMPOSER_CAPTION;
		static const char* BUS_EXTRACTOR_CAPTION;

		static const char* TEST_DATA_DIR;

		QVector<UalItem*> m_scalAppItems;
		QHash<QString, UalAfb*> m_inOutSignalsToScalAppFbMap;

		Tuning::TuningData* m_tuningData = nullptr;

		const QVector<ModuleLogicCompiler*>* m_moduleCompilers = nullptr;
	};
}
