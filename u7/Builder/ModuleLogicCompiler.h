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

#include "../u7/Connection.h"

class LogicModule;

namespace Builder
{

	class ApplicationLogicCompiler;

	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

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
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);
		~ModuleLogicCompiler();

		SignalSet& signalSet() { return *m_signals; }
		Signal* getSignal(const QString& appSignalID);

		IssueLogger* log() { return m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbls.getAfbSignal(afbStrID, signalIndex); }

		bool pass1();
		bool pass2();

		QString lmEquipmentID();

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
		bool createUalSignalFromSignal(UalItem* ualItem);
		bool createUalSignalFromConst(UalItem* ualItem);
		bool createUalSignalsFromAfbOuts(UalItem* ualItem);

		bool linkConnectedItems(UalItem* srcUalItem, const LogicPin& outPin, UalSignal* ualSignal);
		bool linkSignal(UalItem* srcItem, UalItem* signalItem, QUuid inPinUuid, UalSignal* ualSignal);
		bool linkAfbInput(UalItem* srcItem, UalItem* afbItem, QUuid inPinUuid, UalSignal* ualSignal);

		bool detectConstSignalType(const LogicPin& outPin, E::SignalType* constSignalType, E::AnalogAppSignalFormat* constAnalogFormat);

		Signal* getCompatibleConnectedSignal(const LogicPin& outPin, const LogicAfbSignal& outAfbSignal);
		bool isConnectedToTerminatorOnly(const LogicPin& outPin);

		bool checkInOutsConnectedToSignal(UalItem* ualItem, bool shouldConnectToSameSignal);
		bool checkPinsConnectedToSignal(const std::vector<LogicPin>& pins, bool shouldConnectToSameSignal, UalSignal** sameSignal);

		bool appendUalSignals();
		bool appendSignalsFromAppItems();

		bool appendAfbOutputsAutoSignals(UalItem* appItem);
		bool appendAfbNonBusOutputsAutoSignals(UalItem* appItem, const LogicPin& outPin, const ConnectedAppItems& connectedAppItems);
		bool appendAfbBusOutputsAutoSignals(UalItem* appItem, const LogicPin& outPin, const ConnectedAppItems& connectedAppItems, BusShared bus);

		bool appendBusComposerOutputAutoSignal(UalItem* busComposer);

		bool checkBusAndAfbInputCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem, QUuid destPinUuid);
		bool checkBusAndSignalCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);
		bool checkBusAndBusExtractorCompatibility(UalItem* srcAppItem, BusShared bus, UalItem* destAppItem);

		bool buildTuningData();

		bool createSignalLists();

		bool createAcquiredDiscreteInputSignalsList();
		bool createAcquiredDiscreteStrictOutputSignalsList();
		bool createAcquiredDiscreteInternalSignalsList();
		bool createAcquiredDiscreteTuningSignalsList();
		bool createAcquiredDiscreteConstSignalsList();

		bool createNonAcquiredDiscreteInputSignalsList();
		bool createNonAcquiredDiscreteOutputSignalsList();
		bool createNonAcquiredDiscreteInternalSignalsList();
		bool createNonAcquiredDiscreteTuningSignalsList();

		bool createAcquiredAnalogInputSignalsList();
		bool createAcquiredAnalogOutputSignalsList();
		bool createAcquiredAnalogInternalSignalsList();
		bool createAcquiredAnalogTuninglSignalsList();

		bool createNonAcquiredAnalogInputSignalsList();
		bool createNonAcquiredAnalogOutputSignalsList();
		bool createNonAcquiredAnalogInternalSignalsList();
		bool createNonAcquiredAnalogTuningSignalsList();

		bool createAcquiredBusSignalsList();
		bool createNonAcquiredBusSignalsList();

		bool appendLinkedValiditySignal(const Signal* s);

		bool listsUniquenessCheck() const;
		bool listUniquenessCheck(QHash<UalSignal*, UalSignal*>& signalsMap, const QVector<UalSignal*>& signalList) const;
		void sortSignalList(QVector<UalSignal*> &signalList);

		bool disposeSignalsInMemory();

		bool calculateIoSignalsAddresses();

		// disposing discrete signals in bit-addressed memory
		//
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

		bool appendFbsForAnalogInOutSignalsConversion();
		bool findFbsForAnalogInOutSignalsConversion();
		bool createFbForAnalogInputSignalConversion(Signal& signal, UalItem& appItem);
		bool createFbForAnalogOutputSignalConversion(Signal& signal, UalItem& appItem);
		bool isDeviceAndAppSignalsIsCompatible(const Hardware::DeviceSignal& deviceSignal, const Signal& appSignal);

		UalAfb* createUalAfb(const UalItem& appItem);
		bool setOutputSignalsAsComputed();

		bool processTxSignals();
		bool processSerialRxSignals();

		bool processTransmitters();
		bool processTransmitter(const UalItem *item);
		bool getSignalsConnectedToTransmitter(const LogicTransmitter &transmitter, QVector<QPair<QString, QUuid>>& connectedSignals);

		bool processSerialReceivers();
		bool processSerialReceiver(const UalItem* item);

		bool setOptoRawInSignalsAsComputed();

		// pass #2 compilation functions
		//
		bool finalizeOptoConnectionsProcessing();
		bool generateAppStartCommand();

		bool initAfbs();
		bool initAppFbParams(UalAfb* appFb, bool instantiatorsOnly);
		bool displayAfbParams(const UalAfb& appFb);

		bool startAppLogicCode();

		bool copyAcquiredRawDataInRegBuf();
		bool convertAnalogInputSignals();

		bool copySerialRxSignals();
		bool copySerialRxAnalogSignal(Hardware::OptoPortShared port, Hardware::TxRxSignalShared rxSignal);
		bool copySerialRxDiscreteSignal(Hardware::OptoPortShared port, Hardware::TxRxSignalShared rxSignal);

		bool generateAppLogicCode();

		bool generateAppSignalCode(const UalItem* appItem);
		bool generateWriteConstToSignalCode(UalSignal& appSignal, const UalConst* constItem);
		bool generateWriteReceiverToSignalCode(const LogicReceiver& receiver, UalSignal& appSignal, const QUuid& pinGuid);
		bool generateWriteBusExtractorToSignalCode(UalSignal& appSignal, const UalItem* appBusExtractor, QUuid extractorOutPinUuid);
		bool generateWriteSignalToSignalCode(UalSignal& appSignal, QUuid srcSignalGuid);

		bool generateFbCode(const UalItem* appItem);
		bool writeFbInputSignals(const UalAfb *appFb);
		bool generateWriteConstToFbCode(const UalAfb& appFb, const LogicPin& inPin, const UalConst* constItem);
		bool genearateWriteReceiverToFbCode(const UalAfb &appFb, const LogicPin& inPin, const LogicReceiver& receiver, const QUuid& receiverPinGuid);
		bool generateWriteSignalToFbCode(const UalAfb& appFb, const LogicPin& inPin, const UalSignal& appSignal);
		bool startFb(const UalAfb* appFb);
		bool readFbOutputSignals(const UalAfb *appFb);
		bool generateReadFuncBlockToSignalCode(const UalAfb& appFb, const LogicPin& outPin, const QUuid& signalGuid);

		bool generateBusComposerCode(const UalItem* composer);
		bool generateBusComposerToSignalCode(const UalItem* composer, QUuid signalUuid, BusComposerInfo* composerInfo);
		bool fillAnalogBusSignals(const UalItem *composer, const Signal* destSignal);
		bool generateAnalogSignalToBusCode(const UalItem *composer, const BusSignal& busInputSignal, const Signal* busSignal, QUuid connectedSignalGuid);
		bool generateAnalogConstToBusCode(const BusSignal& busInputSignal, const Signal* busSignal, const UalItem* constAppItem);
		bool fillDiscreteBusSignals(const UalItem* composer, const Signal* busSignal);
		bool generateDiscreteSignalToBusCode(const UalItem* composer, const BusSignal& busInputSignal, const Signal* busSignal, QUuid connectedSignalGuid, Commands& fillingCode);
		bool generateDiscreteConstToBusCode(const BusSignal& busInputSignal, const Signal* busSignal, const UalItem* constAppItem, Commands& fillingCode);

		UalItem* getInputPinAssociatedOutputPinParent(QUuid appItemUuid, const QString& inPinCaption, QUuid* connectedOutPinUuid) const;
		UalItem* getAssociatedOutputPinParent(const LogicPin& inputPin, QUuid* connectedOutPinUuid = nullptr) const;
		const UalSignal *getExtractorBusSignal(const UalItem* appBusExtractor);
		bool getConnectedAppItems(const LogicPin& pin, ConnectedAppItems* connectedAppItems);
		bool getBusProcessingParams(const UalAfb* appFb, bool& isBusProcessingAfb, QString& busTypeID);
		UalSignal* getPinInputAppSignal(const LogicPin& inPin);

		bool addToComparatorStorage(const UalAfb *appFb);
		bool initComparator(std::shared_ptr<Comparator> cmp, const UalAfb* appFb);

		bool copyAcquiredTuningAnalogSignalsToRegBuf();
		bool copyAcquiredTuningDiscreteSignalsToRegBuf();

		bool copyAcquiredDiscreteInputSignalsToRegBuf();
		bool copyAcquiredDiscreteOutputAndInternalSignalsToRegBuf();
		bool copyAcquiredDiscreteConstSignalsToRegBuf();

		bool copyOutputSignalsInOutputModulesMemory();
		bool initOutputModulesMemory();
		bool conevrtOutputAnalogSignals();
		bool copyOutputDiscreteSignals();

		bool copyOptoConnectionsTxData();

		bool copyOptoPortTxData(Hardware::OptoPortShared port);
		bool copyOptoPortTxRawData(Hardware::OptoPortShared port);
		bool copyOptoPortTxAnalogSignals(Hardware::OptoPortShared port);
		bool copyOptoPortTxDiscreteSignals(Hardware::OptoPortShared port);
		bool copyOptoPortAllNativeRawData(Hardware::OptoPortShared port, int& offset);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, int modulePlace);
		bool copyOptoPortTxModuleRawData(Hardware::OptoPortShared port, int& offset, const Hardware::DeviceModule* module);
		bool copyOptoPortTxOptoPortRawData(Hardware::OptoPortShared port, int& offset, const QString& portEquipmentID);
		bool copyOptoPortTxConst16RawData(Hardware::OptoPortShared port, int const16value, int& offset);
		bool copyOptoPortRawTxAnalogSignals(Hardware::OptoPortShared port);
		bool copyOptoPortRawTxDiscreteSignals(Hardware::OptoPortShared port);

		bool finishAppLogicCode();
		bool setLmAppLANDataSize();
		bool calculateCodeRunTime();

		bool writeResult();
		bool setLmAppLANDataUID(const QByteArray& lmAppCode, quint64 &uniqueID);
		bool writeTuningInfoFile(const QString& lmCaption, const QString& subsystemID, int lmNumber);
		bool writeOcmRsSignalsXml();
		void writeLMCodeTestFile();

		void displayUsedMemoryInfo();
		void displayTimingInfo();
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

		std::shared_ptr<LogicModule> m_lmDescription;
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

		QVector<UalSignal*> m_nonAcquiredDiscreteInputSignals;			// non acquired discrete input signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteStrictOutputSignals;	// non acquired discrete output signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteInternalSignals;		// non acquired discrete internal non tuningbale signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteTuningSignals;			// non acquired discrete internal tuningable signals, used in UAL
		QVector<UalSignal*> m_nonAcquiredDiscreteOptoSignals;			// non acquired discrete internal opto signals, used in UAL

		QVector<Signal*> m_acquiredAnalogInputSignals;			// acquired analog input signals, no matter used in UAL or not
		QVector<Signal*> m_acquiredAnalogOutputSignals;			// acquired analog output signals, used in UAL
		QVector<Signal*> m_acquiredAnalogInternalSignals;		// acquired analog internal signals, used in UAL
		QVector<Signal*> m_acquiredAnalogTuningSignals;			// acquired analog internal tuningable signals, no matter used in UAL or not

		QVector<Signal*> m_nonAcquiredAnalogInputSignals;		// non acquired analog input signals, used in UAL
		QVector<Signal*> m_nonAcquiredAnalogOutputSignals;		// non acquired analog output signals, used in UAL
		QVector<Signal*> m_nonAcquiredAnalogInternalSignals;	// non acquired analog internal non tunigable signals, used in UAL
		QVector<Signal*> m_nonAcquiredAnalogTuningSignals;		// non acquired analog internal tuningable signals, used in UAL

		QVector<Signal*> m_acquiredBuses;						// acquired bus signals, used in UAL
		QVector<Signal*> m_nonAcquiredBuses;					// non acquired bus signals, used in UAL

		//QHash<Signal*, Signal*> m_acquiredDiscreteInputSignalsMap;		// is used in conjunction with m_acquiredDiscreteInputSignals
																		// for grant unique records

		QHash<QUuid, QUuid> m_outPinSignal;								// output pin GUID -> signal GUID

		QHash<Hardware::DeviceModule::FamilyType, QString> m_moduleFamilyTypeStr;

		QString msg;

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
