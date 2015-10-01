#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../include/OrderedHash.h"
#include "../include/ModuleConfiguration.h"
#include "../Builder/ApplicationLogicBuilder.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/SchemeItemSignal.h"
#include "../VFrame30/SchemeItemAfb.h"
#include "../VFrame30/SchemeItemConst.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/FblItem.h"
#include "Subsystem.h"

namespace Builder
{

	// Signals properties
	//
	const char* const VALUE_OFFSET = "ValueOffset";
	const char* const VALUE_BIT = "ValueBit";

	const char* const SECTION_MEMORY_SETTINGS = "MemorySettings";
	const char* const SECTION_FLASH_MEMORY = "FlashMemory";
	const char* const SECTION_LOGIC_UNIT = "LogicUnit";

	const char* const PARAM_TEST_START_COUNT = "test_start_count";

	// Constants
	//

	// FB SCALE

	const char* const FB_SCAL_16UI_32FP_CAPTION = "scal_16ui_32fp";
	const char* const FB_SCAL_16UI_32SI_CAPTION = "scal_16ui_32si";

	const int FB_SCAL_K1_PARAM_INDEX = 1;
	const int FB_SCAL_K2_PARAM_INDEX = 3;

	//

	const int ERR_VALUE = -1;

	const int NOT_FB_OPERAND_INDEX = -1;

	const int	LM1_PLACE = 0,
				LM2_PLACE = 15,

				FIRST_MODULE_PLACE = 1,
				LAST_MODULE_PLACE = 14,

				MODULES_COUNT = 14,

				OPTO_INTERFACE_COUNT = 3;

	const int	WORD_SIZE = 16;

	const int	ANALOG_SIZE_W = 2;


	class AddrW
	{
	private:
		int m_base = 0;
		int m_offset = 0;

	public:
		AddrW() {}

		void setBase(int base) { m_base = base; }

		int base() { return m_base; }
		int offset() { return m_offset; }
		int address() { return m_base + m_offset; }

		void reset() { m_base = m_offset = 0; }

		void addWord() { m_offset++; }
		void addWord(int n) { m_offset += n; }
	};


	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		Afb::AfbElementCollection* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;

		QVector<Hardware::DeviceModule*> m_lm;

		QHash<QString, Hardware::ModuleFirmware*> m_subsystemModuleFirmware;

		QString msg;

	private:
		void findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool compileModulesLogics();
		bool writeModuleLogicCompilerResult(QString subsysStrID, QString lmCaption, int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode);
		bool saveModulesLogicsFiles();

	public:
		ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems, Hardware::DeviceObject* equipment, SignalSet* signalSet,
								 Afb::AfbElementCollection* afblSet, ApplicationLogicData* appLogicData,
								 BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();

		friend class ModuleLogicCompiler;
	};


	// typedefs Logic* for types defined outside ApplicationLogicCompiler
	//

	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::SchemeItemSignal LogicSignal;
	typedef VFrame30::SchemeItemAfb LogicFb;
	typedef VFrame30::AfbPin LogicPin;
	typedef VFrame30::SchemeItemConst LogicConst;
	//typedef Afb::AfbElement LogicAfb;
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

		int addInstance(AppItem *appItem);

		void insert(std::shared_ptr<Afb::AfbElement> logicAfb);
		void clear();

		const LogicAfbSignal getAfbSignal(const QString &afbStrID, int signalIndex);
	};


	// Base class for AppFb & AppSignal
	// contains pointer to AppLogicItem
	//

	class AppItem
	{
	protected:
		AppLogicItem m_appLogicItem;

	public:
		AppItem(const AppItem& appItem);
		AppItem(const AppLogicItem& appLogicItem);

		QUuid guid() const { return m_appLogicItem.m_fblItem->guid(); }
		QString afbStrID() const { return m_appLogicItem.m_afbElement.strID(); }

		QString strID() const;

		bool isSignal() const { return m_appLogicItem.m_fblItem->isSignalElement(); }
		bool isFb() const { return m_appLogicItem.m_fblItem->isFblElement(); }
		bool isConst() const { return m_appLogicItem.m_fblItem->isConstElement(); }

		bool hasRam() const { return afb().hasRam(); }

		const std::list<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::list<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toFblElement(); }
		const LogicConst& logicConst() const { return *m_appLogicItem.m_fblItem->toSchemeItemConst(); }
		const Afb::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }
		//const LogicItem& logic() const { return *m_fblItem; }

		const LogicSignal& signal() { return *(m_appLogicItem.m_fblItem->toSignalElement()); }
	};


	// Application Functional Block
	// represent all FB items in application logic schemes
	//
	class AppFb : public AppItem
	{
	private:
		quint16 m_instance = -1;
		int m_number = -1;

	public:
		AppFb(AppItem* appItem, int instance, int number);

		quint16 instance() const { return m_instance; }
		quint16 opcode() const { return afb().type().toOpCode(); }		// return FB type
		QString caption() const { return afb().caption().toUpper(); }
		QString typeCaption() const { return afb().type().text(); }
		int number() const { return m_number; }

		LogicAfbParam getAfbParamByIndex(int index) const;
		LogicAfbSignal getAfbSignalByIndex(int index) const;
	};


	class AppFbMap: public HashedVector<QUuid, AppFb*>
	{
	private:
		int m_fbNumber = 1;

	public:
		~AppFbMap() { clear(); }

		void insert(AppItem* appItem, int instance);
		void clear();
	};


	// Application Signal
	// represent all signal in application logic schemes, and signals, which createad in compiling time
	//

	class AppSignal : public Signal
	{
	private:
		const AppItem* m_appItem = nullptr;					// application signals pointer (for real signals)
															// application sunctional block pointer (for shadow signals)
		QUuid m_guid;

		bool m_isShadowSignal = false;

		bool m_computed = false;

	public:
		AppSignal(const Signal* signal, const AppItem* appItem);
		AppSignal(const QUuid& guid, SignalType signalType, int dataSize, const AppItem* appItem, const QString& strID);

		const AppItem &appItem() const;

		void setComputed() { m_computed = true; }
		bool isComputed() const { return m_computed; }


		bool isShadowSignal() { return m_isShadowSignal; }

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
		bool insert(const AppFb* appFb, const LogicPin& outputPin);

		AppSignal* getByStrID(const QString& strID);

		void clear();
	};


	struct MemoryArea
	{
	private:
		int m_startAddress = 0;
		int m_sizeW = 0;

		bool m_locked = false;

	public:
		void setStartAddress(int startAddress) { assert(m_locked == false); m_startAddress = startAddress; }
		void setSizeW(int sizeW) { assert(m_locked == false); m_sizeW = sizeW; }

		int startAddress() const { return m_startAddress; }
		int sizeW() const { return m_sizeW; }

		int* ptrStartAddress() { return &m_startAddress; }
		int* ptrSizeW() { return &m_sizeW; }

		void lock() { m_locked = true; }
		void unlock() { m_locked = false; }

		int nextAddress() const { return m_startAddress + m_sizeW; }

		MemoryArea& operator = (const MemoryArea& ma)
		{
			assert(m_locked == false);

			m_startAddress = ma.m_startAddress;
			m_sizeW = ma.m_sizeW;

			return *this;
		}
	};


	class LmMemoryMap : public QObject
	{
		Q_OBJECT

	private:

		struct
		{
			MemoryArea memory;

			MemoryArea module[MODULES_COUNT];
		} m_modules;

		struct
		{
			MemoryArea memory;

			MemoryArea channel[OPTO_INTERFACE_COUNT];
			MemoryArea result;

		} m_optoInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea regDiscretSignals;
			MemoryArea nonRegDiscretSignals;

			int regDiscreteSignalCount;
			int nonRegDiscreteSignalCount;

			int regDiscreteSignalsSizeW() const
			{
				return regDiscreteSignalCount % WORD_SIZE ? regDiscreteSignalCount / WORD_SIZE + 1 : regDiscreteSignalCount / WORD_SIZE;
			}

			int nonRegDiscreteSignalsSizeW() const
			{
				return nonRegDiscreteSignalCount % WORD_SIZE ? nonRegDiscreteSignalCount / WORD_SIZE + 1 : nonRegDiscreteSignalCount / WORD_SIZE;
			}

		} m_appBitAdressed;

		struct
		{
			MemoryArea memory;
		} m_tuningInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea lmDiagnostics;					// copying from this->lmDiagnostics
			MemoryArea lmInputs;						// reads from this->m_lmInOuts area
			MemoryArea lmOutputs;						// writes to this->m_lmInOuts ares
			MemoryArea module[MODULES_COUNT];			// depends from chassis	configuration
			MemoryArea regAnalogSignals;
			MemoryArea regDiscreteSignals;				// copying from this->appBitAdressed.regDiscretSignals
			MemoryArea nonRegAnalogSignals;

			int regAnalogSignalCount;
			int nonRegAnalogSignalCount;

			int regAnalogSignalsSizeW()
			{
				return regAnalogSignalCount * ANALOG_SIZE_W;
			}

			int nonRegAnalogSignalsSizeW()
			{
				return nonRegAnalogSignalCount * ANALOG_SIZE_W;
			}
		} m_appWordAdressed;

		struct
		{
			MemoryArea memory;
		} m_lmDiagnostics;

		struct
		{
			MemoryArea memory;
		} m_lmInOuts;

		bool recalculateAddresses();

		OutputLog* m_log = nullptr;

		void addSection(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);

	public:

		LmMemoryMap(OutputLog* log);

		bool init(	const MemoryArea& moduleData,
					const MemoryArea& optoInterfaceData,
					const MemoryArea& appLogicBitData,
					const MemoryArea& tuningData,
					const MemoryArea& appLogicWordData,
					const MemoryArea& lmDiagData,
					const MemoryArea& lmIntOutData);

		int lmDiagnosticsAddress() const { return m_lmDiagnostics.memory.startAddress(); }
		int lmDiagnosticsSizeW() const { return m_lmDiagnostics.memory.sizeW(); }

		int lmInOutsAddress() const { return m_lmInOuts.memory.startAddress(); }
		int lmInOutsSizeW() const { return m_lmInOuts.memory.sizeW(); }

		int regDiscreteSignalsAddress() const { return m_appBitAdressed.regDiscretSignals.startAddress(); }
		int regDiscreteSignalsSizeW() const { return m_appBitAdressed.regDiscreteSignalsSizeW(); }

		// rb_* - adrresses and sizes in Registration Buffer
		//
		int rb_lmDiagnosticsAddress() const { return m_appWordAdressed.lmDiagnostics.startAddress(); }
		int rb_regDiscreteSignalsAddress() const { return m_appWordAdressed.regDiscreteSignals.startAddress(); }

		int rb_lmInputsAddress() const { return m_appWordAdressed.lmInputs.startAddress(); }
		int rb_lmOutputsAddress() const { return m_appWordAdressed.lmOutputs.startAddress(); }

		//

		int getModuleDataOffset(int place);

		int getModuleRegDataOffset(int place);

		int addModule(int place, int moduleAppRegDataSize);

		void getFile(QStringList& memFile);

		/*Address16 addRegDiscreteSignal();
		Address16 addNonRegDiscreteSignal();

		Address16 addRegAnalogSignal();
		Address16 addNonRegAnalogSignal();*/
	};


	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	private:

		struct PropertyNameVar
		{
			const char* name = nullptr;
			int* var = nullptr;

			PropertyNameVar(const char* n, int* v) : name(n), var(v) {}
		};

		struct Module
		{
			Hardware::DeviceModule* device = nullptr;
			int place = 0;

			// properties loaded from Hardware::DeviceModule::dynamicProperties
			//
			int txDataSize = 0;					// size of data transmitted to LM
			int rxDataSize = 0;					// size of data received from LM

			int diagDataOffset = 0;
			int diagDataSize = 0;

			int appLogicDataOffset = 0;
			int appLogicDataSize = 0;
			int appLogicDataSizeWithReserve = 0;
			int appLogicRegDataSize = 0;

			// calculated fields
			//
			int rxTxDataOffset = 0;				// offset of data received from module or transmitted to module in LM's memory
												// depends of module place in the chassis

			int moduleAppDataOffset = 0;		// offset of module application data in LM's memory
												// moduleAppDataOffset == rxTxDataOffset + appLogicDataOffset

			int appRegDataOffset = 0;			// offset of module application data for processing (in registration buffer)

			bool isInputModule() const;
			bool isOutputModule() const;
			Hardware::DeviceModule::FamilyType familyType() const;
		};

		// input parameters
		//

		ApplicationLogicCompiler& m_appLogicCompiler;
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		Afb::AfbElementCollection* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		ApplicationLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;

		// LM's and modules settings
		//
		MemoryArea m_moduleData;
		MemoryArea m_optoInterfaceData;
		MemoryArea m_appLogicBitData;
		MemoryArea m_tuningData;
		MemoryArea m_appLogicWordData;
		MemoryArea m_lmDiagData;
		MemoryArea m_lmIntOutData;

		int m_lmAppLogicFrameSize = 0;
		int m_lmAppLogicFrameCount = 0;

		int m_lmCycleDuration = 0;

		// LM's calculated memory offsets and sizes
		//

		LmMemoryMap m_memoryMap;

/*		int m_registeredInternalAnalogSignalsOffset = 0;	// offset of internal analog signals (in registration buffer)
		int m_registeredInternalAnalogSignalsSize = 0;		// size of internal analog signals (in words)

		int m_internalAnalogSignalsOffset = 0;				// offset of internal analog signals (in registration buffer)
		int m_internalAnalogSignalsSize = 0;				// size of internal analog signals (in words)

		int m_registeredInternalDiscreteSignalsOffset = 0;	// offset of internal discrete signals (in bit-addressed memory)
		int m_registeredInternalDiscreteSignalsSize = 0;	// size of internal discrete signals (in words)
		int m_registeredInternalDiscreteSignalsCount = 0;	// count of nternal discrete signals

		int m_regBufferInternalDiscreteSignalsOffset = 0;	// offset of internal discrete signals (in registration buffer)
		int m_regBufferInternalDiscreteSignalsSize = 0;		// size of internal discrete signals (in words)

		int m_internalDiscreteSignalsOffset = 0;			// offset of internal discrete signals (in bit-addressed memory)
		int m_internalDiscreteSignalsSize = 0;				// size of internal discrete signals (in words)
		int m_internalDiscreteSignalsCount = 0;				// count of nternal discrete signals*/

		QVector<Module> m_modules;

		//

		AddrW m_regDataAddress;

		ApplicationLogicCode m_code;

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

		std::shared_ptr<Afb::AfbElement> m_scal_16ui_32fp;
		std::shared_ptr<Afb::AfbElement> m_scal_16ui_32si;

	private:
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const QString& section, const QString& name, int* value);
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const QString& name, int* value);

		bool getLMIntProperty(const QString& section, const QString& name, int* value);
		bool getLMIntProperty(const QString& name, int* value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		QString getModuleFamilyTypeStr(Hardware::DeviceModule::FamilyType familyType);

		// module logic compilations steps
		//
		bool loadLMSettings();
		bool loadModulesSettings();

		bool prepareAppLogicGeneration();

		bool generateAppStartCommand();
		bool generateFbTestCode();
		bool finishTestCode();

		bool startAppLogicCode();

		bool initAfbs();

		bool copyLMDataToRegBuf();
		bool copyInModulesAppLogicDataToRegBuf();

		bool copyLmOutSignalsToModuleMemory();

		void copyDimDataToRegBuf(const Module& module);
		void copyAimDataToRegBuf(const Module& module);

		bool initOutModulesAppLogicDataInRegBuf();

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

		bool copyDiscreteSignalsToRegBuf();

		bool copyOutModulesAppLogicDataToModulesMemory();

		void copyDomDataToModuleMemory(const Module& module);
		void copyAomDataToModuleMemory(const Module& module);

		bool finishAppLogicCode();

		bool appendFbsForAnalogInOutSignalsConversion();
		bool appendFbForAnalogInputSignalConversion(const Signal &signal);

		bool buildServiceMaps();
		bool createAppSignalsMap();

		bool initAppFbParams(AppFb* appFb, bool instantiatorOnly);
		bool calculateFbAnalogIntegralParamValue(AppFb* appFb, const Afb::AfbParam& param, int paramIntValue, quint16* paramValue);

		bool calculate_TCT_AnalogIntegralParamValue(AppFb* appFb, const Afb::AfbParam& param, int paramIntValue, quint16* paramValue);

		bool getUsedAfbs();
		QString getAppLogicItemStrID(const AppLogicItem& appLogicItem) const { AppItem appItem(appLogicItem); return appItem.strID(); }

		bool copyInOutSignalsToRegistration();

		bool calculateLmMemoryMap();
		bool calculateInOutSignalsAddresses();
		bool calculateInternalSignalsAddresses();

		bool generateApplicationLogicCode();

		bool writeResult();

		void writeLMCodeTestFile();

		void cleanup();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		const SignalSet& signalSet() { return *m_signals; }
		const Signal* getSignal(const QString& strID);

		OutputLog& log() { return *m_log; }

		const LogicAfbSignal getAfbSignal(const QString& afbStrID, int signalIndex) { return m_afbs.getAfbSignal(afbStrID, signalIndex); }

		bool run();
	};
}

