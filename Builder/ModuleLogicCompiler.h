#pragma once

#include "../HardwareLib/DeviceObject.h"
#include "../HardwareLib/ModuleFirmware.h"
#include "../HardwareLib/Connection.h"
#include "../CommonLib/HashedVector.h"
#include "../AppSignalLib/ComparatorSet.h"
#include "../lib/TuningDataStorage.h"

#include "BuildResultWriter.h"
#include "ConnectionStorage.h"
#include "OptoModule.h"
#include "LmMemoryMap.h"
#include "UalItems.h"
#include "MemWriteMap.h"
#include "Loopbacks.h"
#include "SignalSet.h"
#include "CodeChecker.h"
#include "CodeOptimization.h"

class LmDescription;

namespace Builder
{
	class ApplicationLogicCompiler;
	class ModuleLogicCompiler;

	using ProcToCall = std::pair<std::function<bool(Builder::ModuleLogicCompiler*)>, QString>;
	using CodeGenProcToCall = std::pair<std::function<bool(ModuleLogicCompiler*, CodeSnippet*)>, QString>;
	using CodeOptimizationProcToCall = std::pair<std::function<bool(ModuleLogicCompiler*, CodeSnippet&)>, QString>;

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

			QVector<AfblUsageInfo> afblUsageInfo;
		};

		struct Module
		{
			bool isInputModule() const;
			bool isOutputModule() const;
			bool isOptoModule() const;
			Hardware::DeviceModule::FamilyType familyType() const;
			QString equipmentID() const;

			//

			//const Hardware::DeviceModule* device = nullptr;
			std::shared_ptr<const Hardware::DeviceModule> device;

			int place = 0;

			// properties loaded from Hardware::DeviceModule::dynamicProperties
			//
			int txDataSize = 0;			// overall size of data transmitted from module to LM
			int txDiagDataOffset = 0;
			int txDiagDataSize = 0;
			int txAppDataOffset = 0;
			int txAppDataSize = 0;

			int rxDataSize = 0;			// overall size of data transmitted from LM to module
			int rxAppDataOffset = 0;
			int rxAppDataSize = 0;

			// calculated fields
			//
			int moduleDataOffset = 0;	// offset of data received from module or transmitted to module in LM's memory
										// depends of module place in the chassis

			int appRegDataOffset = 0;	// offset of module application data in registration buffer
		};

	private:

		struct FbConv
		{
			QString caption;

			std::shared_ptr<Afb::AfbElement> pointer;
			std::set<const UalAfb*> ualAfbs;			// pointer to ualAfb instances doing conversion

			int x1ParamIndex = -1;
			int x2ParamIndex = -1;
			int y1ParamIndex = -1;
			int y2ParamIndex = -1;

			int inputSignalIndex = -1;
			int inputSignalDataSize = -1;

			int outputSignalIndex = -1;
			int outputSignalDataSize = -1;
		};

		struct BusProcessingStepInfo
		{
			int stepsNumber = 0;
			int currentStep = 0;
			int currentStepSizeBits = 0;			// for now 32 or 16 only
			int currentBusSignalOffsetW = 0;

			bool isLastStep() const { return currentStep == (stepsNumber - 1); }
		};

		// helper structs to copy bits in one word code generation
		//
		struct CopyBitInfo
		{
			// must be initialized before call codeCopyBits

			const UalSignal* ualSignal = nullptr;
			Address16 srcBitAddr;
			bool invertBit = false;
			QString comment;

			// will be set inside codeCopyBits function

			static const int NON_CONST = -1;
			static const int CONST_0 = 0;
			static const int CONST_1 = 1;

			mutable int constValue = NON_CONST;
		};

		using CopyBitsMap = std::map<Address16, CopyBitInfo>;	// destBitAddr => CopyBitInfo;
																// destBitAddr ascending ordered in map
																// destBitAddr.offset() for all in map should have equal!

		class BusFilling
		{
		public:
			BusFilling(BusShared bus);

			void fillWord(int offsetInBus);
			void fillDword(int offsetInBus);
			void fill(int offsetInBus, int sizeW);

			void getUnfilled(std::vector<std::pair<int, int>>* unfilledAreas) const;	// pair == <startInbusOffset, sizeW>

		private:
			std::vector<quint16> m_busArea;
		};

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, const Hardware::DeviceModule* lm);
		~ModuleLogicCompiler();

		SignalSet* signalSet() { return m_signals; }
		AppSignal* getSignal(const QString& appSignalID);

		IssueLogger* log() const { return m_log; }

		bool pass1();
		bool pass2();

		QString lmEquipmentID() const;
		int lmDescriptionNumber() const;
		QString lmDescriptionName() const;

		bool expertMode() const;
		bool generateExtraDebugInfo() const;

		const ResourcesUsageInfo& resourcesUsageInfo() const { return m_resourcesUsageInfo; }

		void setModuleCompilersRef(const QVector<ModuleLogicCompiler*>* moduleCompilers);

		bool getSignalsAndPinsLinkedToItem(const UalItem* item,
										   std::set<QString>* linkedSignals,
										   std::set<const UalItem*>* linkedItems,
										   std::map<QUuid, const UalItem*>* linkedPins);

		std::shared_ptr<Hardware::DeviceModule> getLmSharedPtr();

		std::shared_ptr<const LmDescription> getLmDescription() const;

		BusShared getBusShared(const QString& busTypeID);

		bool getTuningSignalsFramesInfo(std::vector<std::pair<quint32, quint32>>* framesInfo) const;

		Builder::Context* builderContext() const { return m_context; }

		const std::map<int, Module>& modules() const { return  m_modules; }

		bool getLmUsedTuningArea(std::vector<CodeChecker::MemArea>* tuningAreas) const;

		bool getLmAssociatedOptoPortsRxAreas(std::vector<CodeChecker::MemArea>* optoRxAreas) const;
		bool getLmAssociatedOptoPortsTxAreas(std::vector<CodeChecker::MemArea>* optoTxAreas) const;

		const LmMemoryMap& lmMemoryMap() const { return m_memoryMap; }

		const UalAfbs& ualAfbs() const;

		QList<const UalSignal*> getLoopbacksUalSignals() const;

		bool optimizeCode(CodeOptimizationType optimizationType,
						  const CodeSnippet& srcCode,
						  CodeSnippetConstIterator start,
						  CodeSnippetConstIterator end,
						  CodeSnippet& optimizedCode,
						  const CodeSnippet& replacementCode);

		int bitAccumulatorAddress() const;
		Address16 bitAccumulatorAddress16() const;

		int wordAccumulatorAddress() const;
		Address16 wordAccumulatorAddress16() const;
		int wordAccumulator2Address() const;

		Address16 constBit0Addr() const { return m_memoryMap.constBit0Addr(); }
		Address16 constBit1Addr() const { return m_memoryMap.constBit1Addr(); }

		bool addressInBitMemory(const Address16& addr) const { return m_memoryMap.addressInBitMemory(addr.offset()); }
		bool addressInBitMemory(int addr) const { return m_memoryMap.addressInBitMemory(addr); }

		bool addressInWordMemory(const Address16& addr) const { return m_memoryMap.addressInWordMemory(addr.offset()); }
		bool addressInWordMemory(int addr) const { return m_memoryMap.addressInWordMemory(addr); }

		Address16 getDiscreteUalAddrBitConstIncluded(const UalSignal* ualSignal) const;

	private:
		bool getLmAssociatedOptoPortsAreas(std::vector<CodeChecker::MemArea>* optoAreas, bool rx) const;

		// pass #1 compilation functions
		//
		bool loadLMSettings();
		bool loadModulesSettings();

		bool createChassisSignalsMap();

		bool createUalItemsMaps();
		QString getUalItemStrID(const AppLogicItem& appLogicItem) const;

		bool createUalAfbsMap();
		UalAfb* createUalAfb(const UalItem& appItem);

		//

		bool createUalSignals();
		bool writeUalItemsFile();

		bool loopbacksPreprocessing();
		bool findAndProcessSingleItemLoopbacks();
		void getInputsDirectlyConnectedToOutput(const UalItem* ualItem,
										const SchemaPin& output,
										QVector<QUuid>* connectedInputsGuids);
		QString getConnectedLoopbackSourceID(const SchemaPin& output);

		bool findLoopbackSources();
		bool findLoopbackTargets();
		bool findSignalsAndPinsLinkedToLoopbackTargets();

		bool getSignalsAndPinsLinkedToOutPin(	const UalItem* item,
												const SchemaPin& outPin,
												std::set<QString>* linkedSignals,
												std::set<const UalItem*>* linkedItems,
												std::map<QUuid, const UalItem*>* linkedPins);

		bool createUalItemSignalsList();

		bool createUalSignalsFromInputAndTuningAcquiredSignals();

		bool createUalSignalsFromBusComposers();
		bool createUalSignalsFromBusComposer(UalItem* ualItem);
		UalSignal* createBusParentSignal(UalItem* ualItem, const SchemaPin& outPin, AppSignal* s, const QString& busTypeID);
		UalSignal* createBusParentSignalFromBusExtractorConnectedToDiscreteSignal(UalItem* ualItem);

		bool createUalSignalsFromOptoValidity();

		bool createUalSignalsFromReceivers();
		bool createUalSignalsFromReceiver(UalItem* ualItem);
		bool createUalSignalFromReceiverOutput(UalItem* ualItem, const SchemaPin& outPin, const QString& appSignalID, bool isSinglePortConnection);
		bool createUalSignalFromReceiverValidity(UalItem* ualItem, const SchemaPin& validityPin, std::shared_ptr<Hardware::Connection> connection);
		bool getReceiverConnectionID(const SchemaReceiver* receiver, QString* connectionID, const QString& schemaID);

		bool createUalSignalFromSignal(UalItem* ualItem, int passNo);
		bool createUalSignalFromConst(UalItem* ualItem);
		bool createUalSignalsFromAfbOuts(UalItem* ualItem);
		bool linkUalSignalsFromBusExtractor(UalItem* ualItem);

		bool linkConnectedItems(UalItem* srcUalItem, const SchemaPin& outPin, UalSignal* ualSignal);
		bool linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkAfbInput(UalItem* srcItem, UalItem* afbItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkSetFlagsItemInput(UalItem* srcItem, UalItem* setFlagsItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusComposerInput(UalItem* srcItem, UalItem* busComposerItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkBusExtractorInput(UalItem* srcItem, UalItem* busExtractorItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkLoopbackSource(UalItem* loopbackSourceItem, QUuid inPinUuid, UalSignal* ualSignal);

		bool checkLoopbacks();
		bool linkLoopbackTargets();
		bool linkLoopbackTarget(UalItem* loopbackTargetItem);

		bool checkBusProcessingItemsConnections();

		bool processSignalsWithFlags();
		bool processAcquiredIOSignalsValidity();
		bool processAcquiredOptoSignalsValidity();
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

		AppSignal* getCompatibleConnectedSignal(const SchemaPin& outPin, const AfbSignal& outAfbSignal, const QString& busTypeID);
		AppSignal* getCompatibleConnectedSignal(const SchemaPin& outPin, const AfbSignal& outAfbSignal);
		AppSignal* getCompatibleConnectedSignal(const SchemaPin& outPin, const AppSignal& s);
		AppSignal* getCompatibleConnectedBusSignal(const SchemaPin& outPin, const QString& busTypeID);
		bool isCompatible(const AfbSignal& outAfbSignal, const QString& busTypeID, const AppSignal* s);

		bool isConnectedToTerminatorOnly(const SchemaPin& outPin) const;
		bool isOutConnectedToTerminatorOnly(const UalAfb* ualItem) const;
		bool isConnectedToLoopback(const SchemaPin& inPin, std::shared_ptr<Loopback>* loopback);
		bool determineOutBusTypeID(UalAfb* ualAfb, QString* outBusTypeID);
		bool determineBusTypeByInputs(const UalAfb* ualAfb, QString* outBusTypeID);
		bool determineBusTypeByOutput(const UalAfb* ualAfb, QString* outBusTypeID);
		bool isBusTypesAreEqual(const QStringList& busTypes);
		std::optional<int> getOutPinExpectedReadCount(const SchemaPin& outPin);
		int getAfbInPinExpectedReadCount(const UalItem* ualItem, const QUuid& inPinGuid);

		bool checkInOutsConnectedToSignal(UalItem* ualItem, bool shouldConnectToSameSignal);
		bool checkPinsConnectedToSignal(const std::vector<SchemaPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignal);

		bool appendRefPinToSignal(UalItem* ualItem, UalSignal* ualSignal);

		bool checkBusAndAfbInputCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem, QUuid destPinUuid);
		bool checkBusAndSignalCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);
		bool checkBusAndBusExtractorCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);

		bool buildTuningData();
		bool buildTuningSignalsLists(Tuning::TuningDataShared tuningData);
		bool getTuningSettings(bool* tuningPropertyExists, bool* tuningEnabled);

		bool disposeSignalsInHeap();

		bool createSignalLists();

		bool createAcquiredDiscreteInputSignalsList();
		bool createAcquiredDiscreteStrictOutputSignalsList();
		bool createAcquiredDiscreteInternalSignalsList();
		bool createAcquiredDiscreteOptoSignalsList();
		bool createAcquiredDiscreteBusChildSignalsList();
		bool createAcquiredDiscreteTuningSignalsList();
		bool createAcquiredDiscreteConstSignalsList();

		bool createNonAcquiredDiscreteInputSignalsList();
		bool createNonAcquiredDiscreteStrictOutputSignalsList();
		bool createNonAcquiredDiscreteInternalSignalsList();
		bool createDiscreteInvertedOutputSignalsList();

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

		bool createAcquiredInputBusesList();
		bool createAcquiredOutputBusesList();
		bool createAcquiredInternalBusesList();
		bool createAcquiredBusBusChildSignalsList();
		bool createAcquiredOptoBusesList();

		bool createNonAcquiredOutputBusesList();
		bool createNonAcquiredInternalBusesList();

		bool setSignalsCalculatedAttributes();

		bool groupTxSignals();

		bool listsUniquenessCheck() const;
		bool listUniquenessCheck(std::set<UalSignal*>* presentSignalsSet, const QVector<UalSignal*>& signalList) const;

		void sortSignalList(QVector<UalSignal*>& signalList);
		void sortSignalList(QVector<const UalSignal*>& signalList);

		void sortSignalListByUalAddr(QVector<UalSignal*>& signalList);

		bool disposeSignalsInMemory();

		bool calculateIoSignalsAddresses();

		bool disposeTunableSignalsUalAddresses();

		// disposing discrete signals in memory
		//
		bool setDiscreteAndBusInputSignalsUalAddresses();
		bool disposeDiscreteSignalsInBitMemory();
		bool disposeNonAcquiredDiscreteInvertedInputSignals();
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

		bool setSignalsRegValidityAddr();

		bool appendAfbsForInOutSignalsConversion();
		bool findFbsForInOutSignalsConversion();
		bool createAfbForAnalogInputSignalConversion(const AppSignal& signal, UalItem* appItem, bool* needConversion);
		bool createFbForAnalogOutputSignalConversion(const AppSignal& signal, UalItem* appItem, bool* needConversion);
		bool isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceAppSignal& deviceAppSignal, const AppSignal& appSignal);

		bool setOutputSignalsAsComputed();

		bool processTxSignals();
		bool processSinglePortRxSignals();

		bool processTransmitters();
		bool processTransmitter(const UalItem* ualItem);
		bool getConnectedSignals(const UalItem* transmitterItem, QVector<QPair<QString, UalSignal *>>* connectedSignals);

		bool getDirectlyConnectedInSignalID(const SchemaPin& inPin, QString* directlyConnectedInSignalID);
		bool getNearestInSignalIDs(const SchemaPin& inPin, QStringList* nearestSignalIDs);
		bool getNearestInSignalID(const SchemaPin& inPin, QString* nearestSignalID);
		bool getNearestOutSignalIDs(const SchemaPin& outPin, QStringList* nearestSignalIDs);
		bool getNearestOutSignalID(const SchemaPin& outPin, QString* nearestSignalID);
		bool getNearestSignalID(const SchemaPin& inOutPin, QString* nearestSignalID);

		bool processSinglePortReceivers();
		bool processSinglePortReceiver(const UalItem* item);

		bool setOptoRawInSignalsAsComputed();

		bool fillComparatorSet();

		bool findEndpointSignals();

		// pass #2 compilation functions
		//
		bool initComparatorSignals();
		bool finalizeOptoConnectionsProcessing();
		bool setOptoUalSignalsAddresses();

		bool generateIdrPhaseCode();
		bool generateAlpPhaseCode();

		bool makeSourceAppLogicCode();

		bool makeAppLogicCode(AppLogicCode& idrCode,
							  AppLogicCode& alpCode,
							  AppLogicCode* appCode);

		bool checkAppLogicCode();
		bool cleanupHeaps();

		bool optimizeAppLogicCode();
		bool makeOptimizedAppLogicCode();

		bool optimizeSequentialMoves(CodeSnippet& srcCode);
		bool optimizeSequentialConstMoves(CodeSnippet& srcCode);
		bool optimizeSequentialBitMoves(CodeSnippet& srcCode);
		bool optimizeBitFilling(CodeSnippet& srcCode);

		bool writeInfoFilesAfterOptimization();
		bool checkOptimizedAppLogicCode();

		bool generateIdrCodeStart(CodeSnippet* code);
		bool generateCustomCode(CodeSnippet* code);
		bool generateAfbsVersionCheckingCode(CodeSnippet* code);
		bool generateInitAfbsCode(CodeSnippet* code);
		bool generateIDRPhaseInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, const QString& usedBy);
		bool generateInitAppFbParamsCode(CodeSnippet* code, const UalAfb& appFb, bool instantiator);
		bool displayAfbParams(CodeSnippet* code, const UalAfb& appFb);
		bool generateLoopbacksRefreshingCode(CodeSnippet* code);
		bool getRefreshingCode(CodeSnippet* code, const QString& loopbackID, const UalSignal* lbSignal);
		bool generateConstBitsInitialization(CodeSnippet* code);
		bool generateIdrCodeStop(CodeSnippet* code);

		bool copyAcquiredRawDataInRegBuf(CodeSnippet* code);
		bool invertDiscreteInputSignals(CodeSnippet* code);
		bool convertAnalogInputSignals(CodeSnippet* code);

		bool generateAppLogicCode(CodeSnippet* code);

		bool generateAfbCode(CodeSnippet* code, const UalItem* ualItem);
		bool generateSignalsToAfbInputsCode(CodeSnippet* code, const UalAfb* ualAfb,
											const BusProcessingStepInfo& bpStepInfo);

		bool generateSignalToAfbInputCode(CodeSnippet* code, const UalAfb* ualAfb,
										  const AfbSignal& inAfbSignal,
										  const UalSignal* inUalSignal,
										  const BusProcessingStepInfo& bpStepInfo,
										  const Address16& readAddr,
										  bool ignoreTypeChecking);

		bool generateSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
											 const AfbSignal& inAfbSignal,
											 const UalSignal* inUalSignal,
											 const BusProcessingStepInfo& bpStepInfo);

		bool generateDiscreteSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
													 const AfbSignal& inAfbSignal,
													 const UalSignal* inUalSignal,
													 const BusProcessingStepInfo& bpStepInfo);

		bool generateBusSignalToAfbBusInputCode(CodeSnippet* code, const UalAfb* ualAfb,
												const AfbSignal& inAfbSignal, const UalSignal* inUalSignal,
												const BusProcessingStepInfo& bpStepInfo);

		bool startAfb(CodeSnippet* code, const UalAfb* ualAfb, const BusProcessingStepInfo& bpStepInfo);

		bool generateAfbOutputsToSignalsCode(CodeSnippet* code, const UalAfb* ualAfb,
											 const BusProcessingStepInfo& bpStepInfo);

		bool generateAfbOutputToSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
										   const AfbSignal& outAfbSignal,
										   const UalSignal* outUalSignal,
										   const BusProcessingStepInfo& bpStepInfo,
										   const Address16& writeAddr,
										   bool ignoreTypeChecking);

		bool generateAfbBusOutputToBusSignalCode(CodeSnippet* code, const UalAfb* ualAfb,
												 const AfbSignal& outAfbSignal, const UalSignal* outUalSignal,
												 const BusProcessingStepInfo& bpStepInfo);

		// Packed AFBs code generation

		bool generatePackedAfbCode(CodeSnippet* code, const UalAfb* afb);

		bool generateBitAccBasedPackedAfbCode(CodeSnippet* code, const UalAfb* afb,
											  const std::vector<std::pair<const UalSignal*, Address16>>& inSignals,
											  const UalSignal* outSignal, const Address16& outWriteAddr);

		bool generateAfbBasedPackedAfbCode(CodeSnippet* code, const UalAfb* afb,
									 const std::vector<std::pair<const UalSignal*, Address16>>& inSignals,
									 const UalSignal* outSignal, const Address16& outWriteAddr);
		//

		bool generateAfbBitAccCode(CodeSnippet* code, const UalAfb* ualAfb,
									const BusProcessingStepInfo& bpStepInfo, bool* result);

		bool generateAfbBitAccNotCode(CodeSnippet* code, const UalAfb* ualAfb,
										const BusProcessingStepInfo& bpStepInfo, bool* result);

		bool generateAfbBitAcc1NotCode(	CodeSnippet* code, const UalAfb* ualAfb,
										UalSignal* inSignal, UalSignal* outSignal,
										bool* result);

		bool generateAfbBitAccBusNotCode(CodeSnippet* code, const UalAfb* ualAfb,
										 const BusProcessingStepInfo& bpStepInfo,
										 UalSignal* inSignal, UalSignal* outSignal, bool* result);

		bool generateAfbBitAccOrCode(CodeSnippet* code, const UalAfb* ualAfb, bool* result);
		bool generateAfbBitAccAndCode(CodeSnippet* code, const UalAfb* ualAfb, bool* result);

		bool generateInvertDiscreteInputsCode(CodeSnippet* code,
											  const QVector<UalSignal*>& inSignals,
											  const QString& comment);

		bool calcBusProcessingSteps(const UalAfb* ualAfb, std::vector<int>* busProcessingStepsSizes);
		bool getPinsAndSignalsBusSizes(const UalAfb* ualAfb, const std::vector<SchemaPin>& pins,
									   std::vector<std::vector<int>>* pinsSizes, int* signalsSize, bool isInputs,
									   bool* allBusInputsConnectedToDiscretes);
		//

		bool generateBusComposerCode(CodeSnippet* code, const UalItem* ualItem);
		UalSignal* getBusComposerBusSignal(const UalItem* composerItem, bool* connectedToTedrminatorOnly);
		bool generateAnalogSignalToBusAnalogInputCode(CodeSnippet* code,
													  const UalSignal* inputSignal,
													  const UalSignal* busChildSignal,
													  const BusSignal& busSignal,
													  const QString& busComposerLabel,
													  BusFilling* busFilling);

		bool generateInbusConversionCode(CodeSnippet* code,
										const UalSignal* inputSignal,
										const UalSignal* busChildSignal,
										const BusSignal& busSignal,
										const QString& busComposerLabel);

		bool genInbusScalingCode(CodeSnippet* code,
								 const UalSignal* inputSignal,
								 const UalSignal* busChildSignal,
								 const BusSignal& busSignal,
								 const QString& busExtractorLabel,
								 const InbusConvDescription &convDesc,
								 bool readValueFromAccumulator,
								 bool saveResultToAccumulator,
								 const Address16& inbusSignalAddr);

		bool genInbusTypeConversionCode(CodeSnippet* code,
									const UalSignal* inputSignal,
									const UalSignal* busChildSignal,
									const BusSignal& busSignal,
									const QString& busComposerLabel,
									const InbusConvDescription& convDesc,
									bool readValueFromAccumulator,
									bool saveResultToAccumulator,
									const Address16& inbusSignalAddr);

		bool genInbusTconvCode(CodeSnippet* code,
						   const UalSignal* inputSignal,
						   const UalSignal* busChildSignal,
						   const BusSignal& busSignal,
						   const QString& busComposerLabel,
						   const QString& tconvAfbCaption,
						   bool readValueFromAccumulator,
						   bool saveResultToAccumulator,
						   const Address16& inbusSignalAddr);


		bool genInbusByteOrderConversionCode(CodeSnippet* code,
											const UalSignal* inputSignal,
											const UalSignal* busChildSignal,
											const BusSignal& busSignal,
											const QString& busComposerLabel,
											const InbusConvDescription& convDesc,
											bool readValueFromAccumulator,
											bool saveResultToAccumulator,
											const Address16& inbusSignalAddr);

		bool generateDiscreteSignalToBusDiscreteInputCode(CodeSnippet* code,
														  const UalSignal* inputSignal,
														  const UalSignal* busChildSignal,
														  const BusSignal& busSignal);

		bool generateDiscreteSignalsToBusDiscreteInputsCode(CodeSnippet* code,
					const std::map<int, std::map<int, std::pair<UalSignal*, UalSignal*>>>& busDiscretes,
					const UalSignal& busSignal,
					BusFilling* busFilling);

		void clearUnusedBusSpace(CodeSnippet* code,
								const UalSignal& busSignal,
								const BusFilling& busFilling);

		bool generateDiscreteSignalToBusBusInputCode(CodeSnippet* code,
													 UalSignal* inputSignal,
													 UalSignal* busChildSignal,
													 const BusSignal& busSignal,
													 BusFilling* busFilling);

		bool generateBusSignalToBusBusInputCode(CodeSnippet* code,
												UalSignal* inputSignal,
												UalSignal* busChildSignal,
												const BusSignal& busSignal,
												BusFilling* busFilling);

		bool generateBusExtractorCode(CodeSnippet* code, const UalItem* ualItem);
		bool generateBusExtractorCode(CodeSnippet* code, const UalItem* ualItem, UalSignal* inputBusSignal);

		bool generateFrombusConversionCode(CodeSnippet* code,
										   const UalSignal* inputBusSignal,
										   const BusSignal& busSignal,
										   UalSignal* busChildSignal,
										   const QString& busExtractorLabel);

		bool genFrombusByteOrderConversionCode(CodeSnippet* code,
												const UalSignal* inputBusSignal,
												const BusSignal& busSignal,
												const UalSignal* busChildSignal,
												const QString& busExtractorLabel,
												const InbusConvDescription& convDesc,
												bool readValueFromAccumulator,
												bool saveResultToAccumulator,
												const Address16& inbusSignalAddr);

		bool genFrombusTypeConversionCode(CodeSnippet* code,
											const UalSignal* inputBusSignal,
											const BusSignal& busSignal,
											const UalSignal* busChildSignal,
											const QString& busExtractorLabel,
											const InbusConvDescription& convDesc,
											bool readValueFromAccumulator,
											bool saveResultToAccumulator,
											const Address16& inbusSignalAddr);

		bool genFrombusTconvCode(CodeSnippet* code,
								const UalSignal* inputBusSignal,
								const BusSignal& busSignal,
								const UalSignal* busChildSignal,
								const QString& busExtractorLabel,
								const QString& tconvAfbCaption,
								bool readValueFromAccumulator,
								bool saveResultToAccumulator,
								const Address16& inbusSignalAddr);

		bool genFrombusScalingCode(CodeSnippet* code,
								 const UalSignal* inputBusSignal,
								 const BusSignal& busSignal,
								 const UalSignal* busChildSignal,
								 const QString& busExtractorLabel,
								 const InbusConvDescription &convDesc,
								 bool readValueFromAccumulator,
								 bool saveResultToAccumulator,
								 const Address16& inbusSignalAddr);

		bool generateDiscreteSignalToBusExtractorCode(CodeSnippet* code,
													  const UalItem* ualItem,
													  const SchemaPin& inPin,
													  const UalSignal* inputSignal);

		bool generateMemCopyCode(Address16 toAddr, Address16 fromAddr, int sizeW, const QString& comment, CodeSnippet* code);

		UalItem* getInputPinAssociatedOutputPinParent(QUuid appItemUuid, const QString& inPinCaption, QUuid* connectedOutPinUuid) const;
		UalItem* getAssociatedOutputPinParent(const SchemaPin& inputPin, QUuid* connectedOutPinUuid = nullptr) const;
		const UalSignal* getExtractorBusSignal(const UalItem* appBusExtractor);
		bool getConnectedAppItems(const SchemaPin& pin, ConnectedAppItems* connectedAppItems);
		bool getBusProcessingParams(const UalAfb* appFb, bool& isBusProcessingAfb, QString& busTypeID);
		UalSignal* getPinInputAppSignal(const SchemaPin& inPin);

		UalSignal* getUalSignalByPinCaption(const UalItem* ualItem, const QString& pinCaption, bool isInput);

		bool addToComparatorSet(const UalAfb* appFb);
		bool initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* afb);

		bool copyAcquiredAnalogOptoSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredAnalogBusChildSignalsInRegBuf(CodeSnippet* code);

		bool copyAcquiredTuningAnalogSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredTuningDiscreteSignalsInRegBuf(CodeSnippet* code);

		bool copyAcquiredAnalogConstSignalsInRegBuf(CodeSnippet* code);

		bool copyAcquiredInputBusesInRegBuf(CodeSnippet* code);
		bool copyAcquiredBusChildBusesInRegBuf(CodeSnippet* code);
		bool copyAcquiredOptoBusesInRegBuf(CodeSnippet* code);

		bool copyBusesToRegBuf(const QString& comment, const QVector<UalSignal*>& buses, CodeSnippet* code);

		bool checkUalAndRegBufAddrs(const UalSignal* ualSignal) const;
		bool checkUalAndIoBufAddrs(const UalSignal* ualSignal) const;

		bool copyAcquiredDiscreteInputSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteOptoSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteBusChildSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteOutputAndInternalSignalsInRegBuf(CodeSnippet* code);
		bool copyAcquiredDiscreteConstSignalsInRegBuf(CodeSnippet* code);

		bool copyScatteredDiscreteSignalsInRegBuf(CodeSnippet* code, const QVector<UalSignal*>& signalsList, const QString& description);

		bool copyOutputSignalsInOutputModulesMemory(CodeSnippet* code);
		bool initOutputModulesMemory(CodeSnippet* code);
		bool convertAndCopyOutputAnalogSignals(CodeSnippet* code);
		bool copyOutputBusSignals(CodeSnippet* code);
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
		bool detectUsedReservedSignals();
		bool fillAnalogSignalsOnSchemas();
		bool calculateCodeRunTime();

		QString lmSubsystemEquipmentIdPath() const;
		QString getInfoFileName(const QString& fileNameExtension) const;
		QString getSrcInfoFileName(const QString& fileNameExtension) const;

		bool writeInfoFiles();
		bool writeAsmFile(const AppLogicCode& code) const;
		bool writeMemFile() const;
		bool writeStatisticsFile(const AppLogicCode& code,
								 const AppLogicCode& idrCode,
								 const AppLogicCode& alpCode) const;
		bool writeOptimizationReportFile() const;
		void printOptiStatistics(const AppLogicCode& code,
								 const AppLogicCode& optiCode,
								 QStringList* outFile) const;
		void printOptimizationsInfo(QStringList* outFile, int srcCodeSize) const;

		bool writeTuningInfoFile() const;
		bool writeOptoModulesReport() const;
		bool writeLoopbacksReport();
		bool writeHeapsLog();

		bool calcAppDataUID();
		bool calcDiagDataUID();

		bool writeResult();
		bool writeBinCodeForLm(const QByteArray& binCode);

		bool writeOcmRsSignalsXml();

		void printCodeStatistics(const AppLogicCode& code,
								 QStringList& file,
								 bool exludeNotUsedCommands) const;

		void printCodeStatisticsTable(const AppLogicCode& code,
									  const std::vector<CommandStatistics>& stat,
									  QStringList& file,
									  bool exludeNotUsedCommands) const;

		QString getStatStr(const QString& mnemo,
						   int used, float usedPercent,
						   int sizeW, float sizePercent,
						   int execTime, float execPercent, bool isTotal) const;

		bool displayResourcesUsageInfo();
		void calcOptoDiscretesStatistics();
		bool getAfblUsageInfo();
		void cleanup();

		bool checkLoopbackTargetSignalsCompatibility(const AppSignal& srcSignal, QUuid srcSignalUuid, const AppSignal& destSignal, QUuid destSignalUuid);
		bool checkLoopbackTargetSignalsCompatibility(const AppSignal& srcSignal, QUuid srcSignalUuid, const UalAfb& fb, const AfbSignal& afbSignal);

		bool isUsedInUal(const AppSignal* s) const;
		bool isUsedInUal(const QString& appSignalID) const;

		QString getSchemaID(QUuid itemUuid);

		bool getLMIntProperty(const QString& name, int* value);
		bool getLMStrProperty(const QString& name, QString *value);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		std::shared_ptr<Hardware::DeviceObject> getDeviceSharedPtr(const Hardware::DeviceObject* device);
		std::shared_ptr<Hardware::DeviceObject> getDeviceSharedPtr(const QString& deviceEquipmentID);

		void dumpApplicationLogicItems();

		bool writeSignalLists();
		bool writeSignalList(const QVector<UalSignal *> &signalList, QString listName) const;
		bool writeUalSignalsList() const;

		bool runProcs(const std::vector<ProcToCall>& procArray);
		bool runCodeGenProcs(const std::vector<CodeGenProcToCall>& procArray, CodeSnippet* code);

		Address16 getConstBitAddr(UalSignal* constDiscreteUalSignal);

		CodeItem codeSetMemory(int addrFrom, quint16 constValue, int sizeW, const QString& comment = QStringLiteral(""));

		bool codeCopyBits(CodeSnippet* code, int destAddrOffset, const CopyBitsMap& copyBitsMap);

		bool codeNotWord(CodeSnippet* code, const Address16& srcAddr, const QString& srcComment,
						 const Address16& destAddr, const QString& destComment) const;

		bool codeNotBit(CodeSnippet* code, const Address16& srcAddr, const QString& srcComment,
						 const Address16& destAddr, const QString& destComment) const;

		UalSignals& ualSignals() { return m_ualSignals; }

		QString getFormatStr(const Hardware::DeviceAppSignal& ds);
		QString getFormatStr(const AppSignal& s);
		QString getFormatStr(E::SignalType signalType, E::DataFormat dataFormat, int dataSizeBits, E::ByteOrder byteOrder);

		bool partitionOfInteger(int number, const std::vector<int>& availableParts,
								 std::vector<int>* resultPartition);
		bool partitionOfInteger(int number, const QVector<int>& availableParts, QVector<int>* partition);

		void findLogicAfbsForBitAccReplacing(const QString& afbCaption, int logicConf, std::set<QUuid>* guidsMap);

	public:
		static const int MIN_AFB_OPCODE = 1;
		static const int MAX_AFB_OPCODE = 63;

	private:
		// input parameters
		//
		ApplicationLogicCompiler& m_appLogicCompiler;
		Context* m_context = nullptr;
		const Hardware::DeviceModule* m_lm = nullptr;
		DeviceModuleShared m_lmShared;

		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Builder::ConnectionStorage* m_connections = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		ComparatorSet* m_cmpSet = nullptr;

		LmDescriptionConstShared m_lmDescription;
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

		bool m_bitAccAvailable = false;

		quint32 m_rupAppDataUID = 0;			// App data 32-bit UID placed in RUP frame header
		quint32 m_rupDiagDataUID = 0;			// Diag data 32-bit UID placed in RUP frame header
		quint32 m_rupTuningDataUID = 0;			// Tuning data 32-bit UID placed in RUP frame header
		quint64 m_fotipTuningDataUID = 0;		// Tuning data 64-bit UID placed in FOTIP frame header

		// LM's calculated memory offsets and sizes
		//
		LmMemoryMap m_memoryMap;

//		HashedVector<QString, Module> m_modules;		// modules installed in chassis, module EquipmentID => Module

		std::map<int, Module> m_modules;				// modules installed in chassis, module place => Module

		//

		AppLogicCode m_appLogicCode;
		AppLogicCode m_idrCode;
		AppLogicCode m_alpCode;

		AppLogicCode m_optiAppLogicCode;
		AppLogicCode m_optiIdrCode;
		AppLogicCode m_optiAlpCode;

		int m_optimizationNo = 0;
		std::map<CodeOptimizationType, OptimizationInfo> m_optimizationsInfo;

		AfbComponents m_afbComponents;

		UalSignals m_ualSignals;
		UalAfbs m_ualAfbs;
		int m_packedLogicAfbInstance = -1;			// AFB LOGIC instance reserved for packed logic processing

		// maps of OR and AND afbs guids which can be replaced by bit acc commands
		//
		std::set<QUuid> m_afbsOrForBitAccReplacing;
		std::set<QUuid> m_afbsAndForBitAccReplacing;

		// service maps
		//
		HashedVector<QUuid, UalItem*> m_ualItems;				// item GUID => item ptr
		std::map<QUuid, UalItem*> m_pinParent;					// pin GUID => parent item ptr

		std::map<Hash, AppSignal*> m_chassisSignals;			// all signals available in current chassis, calcHash(AppSignalID) => AppSignal*
		std::vector<AppSignal*> m_ioSignals;					// input/output signals of current chassis
		std::map<Hash, AppSignal*> m_equipmentSignals;			// equipment signals to app signals map, calcHash(signal EquipmentID) => AppSignal*
		std::map<Hash, UalSignal*> m_optoPortValiditySignal;	// calcHash(OptoPort EquipmentID) => OptoPort validity signal

		std::map<Hash, std::set<QUuid>> m_ualItemsSignals;		// Hash(appSignalID) => set of UalItem.guid (type Signal) with this appSignalID

		std::set<QString> m_signalsWithFlagsIDs;
		std::set<const UalSignal*> m_signalsWithFlagsAndFlagSignals;

		Loopbacks m_loopbacks;

		QVector<UalSignal*> m_acquiredDiscreteInputSignals;				// acquired discrete input signals, no matter used in UAL or NOT
																		// with property InvertSignal == false

		QVector<UalSignal*> m_acquiredDiscreteInvertedInputSignals;		// same as above but with property InvertSignal == true

		QVector<UalSignal*> m_acquiredDiscreteStrictOutputSignals;		// acquired discrete Strict Output Signals, used in UAL
																		//
																		// Strict Output Signals it is a signals that is not an Input, Tunable, Opto or Const.
																		// I.e. signals that are formed by the app logic

		QVector<UalSignal*> m_acquiredDiscreteInternalSignals;			// acquired discrete internal non tunable signals, used in UAL
		QVector<UalSignal*> m_acquiredDiscreteTuningSignals;			// acquired discrete internal tunable signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredDiscreteConstSignals;
		QVector<UalSignal*> m_acquiredDiscreteOptoSignals;
		QVector<UalSignal*> m_acquiredDiscreteBusChildSignals;

		QVector<UalSignal*> m_nonAcquiredDiscreteInputSignals;			// non acquired discrete input signals, used in UAL, with property InvertSignal == false
		QVector<UalSignal*> m_nonAcquiredDiscreteInvertedInputSignals;	// same as above but with property InvertSignal == true

		QVector<UalSignal*> m_nonAcquiredDiscreteStrictOutputSignals;	// non acquired discrete output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteInternalSignals;		// non acquired discrete internal non tuningbale signals, used in UAL
		QVector<UalSignal*> m_discreteInvertedOutputSignals;			// non acquired discrete internal opto signals, used in UAL

		QVector<UalSignal*> m_acquiredAnalogInputSignals;				// acquired analog input signals, no matter used in UAL or not
		QVector<UalSignal*> m_acquiredAnalogStrictOutputSignals;		// acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogInternalSignals;			// acquired analog internal signals, used in UAL
		QVector<UalSignal*> m_acquiredAnalogOptoSignals;				// acquired analog opto signals (simple copied from opto buffers)
		QVector<UalSignal*> m_acquiredAnalogBusChildSignals;			// acquired analog opto signals (unlike to opto signals may require conversion from inbus format)
		QVector<UalSignal*> m_acquiredAnalogTuningSignals;				// acquired analog internal tunable signals, no matter used in UAL or not

		QMultiHash<int, UalSignal*> m_acquiredAnalogConstIntSignals;
		QMultiHash<float, UalSignal*> m_acquiredAnalogConstFloatSignals;

		QVector<UalSignal*> m_nonAcquiredAnalogInputSignals;			// non acquired analog input signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogStrictOutputSignals;		// non acquired analog strict output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredAnalogInternalSignals;			// non acquired analog internal non tunigable signals, used in UAL

		QVector<AppSignal*> m_analogOutputSignalsToConversion;				// all analog output signals requires conversion

		QVector<UalSignal*> m_acquiredInputBuses;						// acquired entirely Input Buses (in end of ALP phase should be copied from IO modules memory to regBuf)
		QVector<UalSignal*> m_acquiredOutputBuses;						// acquired entirely Output Buses (in end of ALP phase should be copied from regBuf to IO modules memory)
		QVector<UalSignal*> m_acquiredInternalBuses;					// acquired entirely Internal Buses
		QVector<UalSignal*> m_acquiredOptoBuses;						// acquired entirely Opto Buses
		QVector<UalSignal*> m_acquiredBusChildBuses;					// acquired entirely bus child Buses

		QVector<UalSignal*> m_nonAcquiredOutputBuses;
		QVector<UalSignal*> m_nonAcquiredInternalBuses;					// non acquired internal buses AND!

		ResourcesUsageInfo m_resourcesUsageInfo;

		//

		std::map<QString, FbConv> m_fbConv;								// AFB caption => FbConv structure

		QVector<UalItem*> m_scalAppItems;
		QHash<QString, UalAfb*> m_inOutSignalsToScalAppFbMap;

		Tuning::TuningDataShared m_tuningData;

		const QVector<ModuleLogicCompiler*>* m_moduleCompilers = nullptr;

		static const QString EMPTY_STR;
	};


#define CHECK_UAL_ADDR_RETURN_FALSE(ualSignal)	if (ualSignal->ualAddrIsValid() == false)	\
												{ \
													m_log->errALC5105(ualSignal->appSignalID(), \
																	  ualSignal->ualItemGuid(), \
																	  ualSignal->ualItemSchemaID()); \
													return false; \
												}
}
