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
		Afbl::AfbElementCollection* m_afbl = nullptr;
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
								 Afbl::AfbElementCollection* afblSet, ApplicationLogicData* appLogicData,
								 BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();

		friend class ModuleLogicCompiler;
	};


	// typedefs Logic* for types defined outside ApplicationLogicCompiler
	//

	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::SchemeItemSignal LogicSignal;
	typedef VFrame30::SchemeItemAfb LogicFb;
	typedef VFrame30::CFblConnectionPoint LogicPin;
	typedef VFrame30::SchemeItemConst LogicConst;
	typedef Afbl::AfbElement LogicAfb;
	typedef Afbl::AfbSignal LogicAfbSignal;
	typedef Afbl::AfbParam LogicAfbParam;

	class AppItem;
	class ModuleLogicCompiler;


	// Functional Block Library element
	//

	class Afb
	{
	private:
		std::shared_ptr<LogicAfb> m_afb;

	public:
		Afb(std::shared_ptr<LogicAfb> afb);
		~Afb();

		bool hasRam() const { return m_afb->hasRam(); }

		const LogicAfb& afb() const { return *m_afb; }

		QString strID() const { return m_afb->strID(); }
		int opcode() const { return m_afb->opcode(); }
	};


	typedef QHash<int, int> FblInstanceMap;
	typedef QHash<QString, int> NonRamFblInstanceMap;


	class AfbMap: public HashedVector<QString, Afb*>
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

		void insert(std::shared_ptr<LogicAfb> logicAfb);
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
		const Afbl::AfbElement& afb() const { return m_appLogicItem.m_afbElement; }
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
		quint16 opcode() const { return afb().opcode(); }		// return FB type
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

			// calculated fields
			//
			int rxTxDataOffset = 0;				// offset of data received from module or transmitted to module in LM's memory
												// depends of module place in the chassis

			int moduleAppDataOffset = 0;		// offset of module application data in LM's memory
												// moduleAppDataOffset == rxTxDataOffset + appLogicDataOffset

			int appDataOffset = 0;				// offset of module application data for processing (in registration buffer)

			bool isInputModule();
			bool isOutputModue();
			Hardware::DeviceModule::FamilyType familyType();
		};

		// input parameters
		//

		ApplicationLogicCompiler& m_appLogicCompiler;
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		Afbl::AfbElementCollection* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		ApplicationLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;

		// LM's and modules settings
		//
		int	m_moduleDataOffset = 0;
		int m_moduleDataSize = 0;

		int m_optoInterfaceDataOffset = 0;
		int m_optoInterfaceDataSize = 0;

		int m_appLogicBitDataOffset = 0;
		int	m_appLogicBitDataSize = 0;

		int m_tuningDataOffset = 0;
		int m_tuningDataSize = 0;

		int m_appLogicWordDataOffset = 0;
		int m_appLogicWordDataSize = 0;

		int m_lmDiagDataOffset = 0;
		int m_lmDiagDataSize = 0;

		int m_lmIntOutDataOffset = 0;
		int m_lmIntOutDataSize = 0;

		int m_lmAppLogicFrameSize = 0;
		int m_lmAppLogicFrameCount = 0;

		int m_lmCycleDuration = 0;

		// LM's calculated memory offsets and sizes
		//

		int m_registeredInternalAnalogSignalsOffset = 0;	// offset of internal analog signals (in registration buffer)
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
		int m_internalDiscreteSignalsCount = 0;				// count of nternal discrete signals

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

		bool copyLMDiagDataToRegBuf();
		bool copyInModulesAppLogicDataToRegBuf();
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

		bool finishAppLogicCode();

		bool buildServiceMaps();
		bool createAppSignalsMap();

		bool initAppFbParams(AppFb* appFb, bool instantiatorOnly);
		bool calculateFbAnalogIntegralParamValue(AppFb* appFb, const Afbl::AfbParam& param, int paramIntValue, quint16* paramValue);

		bool calculate_TCT_AnalogIntegralParamValue(AppFb* appFb, const Afbl::AfbParam& param, int paramIntValue, quint16* paramValue);

		bool getUsedAfbs();
		QString getAppLogicItemStrID(const AppLogicItem& appLogicItem) const { AppItem appItem(appLogicItem); return appItem.strID(); }

		bool copyInOutSignalsToRegistration();

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

