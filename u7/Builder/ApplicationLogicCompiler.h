#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../include/OrderedHash.h"
#include "../Builder/ApplicationLogicBuilder.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/VideoItemSignal.h"
#include "../VFrame30/VideoItemFblElement.h"
#include "../VFrame30/FblItem.h"
#include "../VFrame30/FblItem.h"


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
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		Afbl::AfbElementCollection* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;

		QVector<Hardware::DeviceModule*> m_lm;

		QString msg;

	private:
		void findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool compileModulesLogics();


	public:
		ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, Afbl::AfbElementCollection* afblSet, ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();

		friend class ModuleLogicCompiler;
	};


	// typedefs Logic* for types defined outside ApplicationLogicCompiler
	//

	typedef std::shared_ptr<VFrame30::FblItemRect> LogicItem;
	typedef VFrame30::VideoItemSignal LogicSignal;
	typedef VFrame30::VideoItemFblElement LogicFb;
	typedef VFrame30::CFblConnectionPoint LogicPin;
	typedef Afbl::AfbElement LogicAfb;
	typedef Afbl::AfbElementSignal LogicAfbSignal;
	typedef Afbl::AfbElementParam LogicAfbParam;


	class AppItem;
	class ModuleLogicCompiler;


	// Functional Block Library element
	//

	class Afb
	{
	private:
		std::shared_ptr<LogicAfb> m_afb;
		int m_instance = 0;					// for Fbls with RAM

	public:
		Afb(std::shared_ptr<LogicAfb> afb);
		~Afb();

		//quint16 addInstance();

		bool hasRam() const { return m_afb->hasRam(); }

		int incInstance() { return (++m_instance); }
		int instance() const { return m_instance; }

		const LogicAfb& afb() const { return *m_afb; }

		QUuid guid() const { return m_afb->guid(); }
		QString strID() const { return m_afb->strID(); }
		int opcode() const { return m_afb->opcode(); }
	};


	typedef QHash<int, int> FblInstanceMap;
	typedef QHash<QString, int> NonRamFblInstanceMap;


	class AfbMap: public HashedVector<QUuid, Afb*>
	{
	private:

		struct GuidIndex
		{
			QUuid guid;			// AfbElement guid()
			int index;			// AfbElementSignal or AfbElementParam index

			operator QString() const { return QString("%1:%2").arg(guid.toString()).arg(index); }
		};

		FblInstanceMap m_fblInstance;						// Fbl opCode -> current instance
		NonRamFblInstanceMap m_nonRamFblInstance;			// Non RAM Fbl StrID -> instance

		QHash<QString, LogicAfbSignal*> m_afbSignals;
		QHash<QString, LogicAfbParam*> m_afbParams;

	public:
		~AfbMap() { clear(); }

		int addInstance(AppItem *appItem);

		void insert(std::shared_ptr<LogicAfb> logicAfb);
		void clear();

		const LogicAfbSignal* getAfbSignal(const QUuid& afbGuid, int signalIndex);
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
		QUuid afbGuid() const { return m_appLogicItem.m_afbElement.guid(); }

		QString strID() const { return m_appLogicItem.m_fblItem->toSignalElement()->signalStrIds(); }

		bool isSignal() const { return m_appLogicItem.m_fblItem->isSignalElement(); }
		bool isFb() const { return m_appLogicItem.m_fblItem->isFblElement(); }

		const std::list<LogicPin>& inputs() const { return m_appLogicItem.m_fblItem->inputs(); }
		const std::list<LogicPin>& outputs() const { return m_appLogicItem.m_fblItem->outputs(); }

		const LogicFb& logicFb() const { return *m_appLogicItem.m_fblItem->toFblElement(); }
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

	public:
		AppFb(AppItem* appItem, int instance);

		quint16 instance() const { return m_instance; }
	};


	class AppFbMap: public HashedVector<QUuid, AppFb*>
	{
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

		bool m_calculated = false;

	public:
		AppSignal(const Signal* signal, const AppItem* appItem);
		AppSignal(const QUuid& guid, SignalType signalType, int dataSize, const AppItem* appItem);

		const AppItem &appItem() const;

		void setCalculated() { m_calculated = true; }
		bool isCalculated() const { return m_calculated; }


		bool isShadowSignal() { return m_appItem == nullptr; }

	};


	class AppSignalMap: public QObject, public HashedVector<QUuid, AppSignal*>
	{
		Q_OBJECT

	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;

		void insert(const QUuid& guid, const QString& strID, SignalType signalType, const Signal* signal, const AppItem* appItem);

		ModuleLogicCompiler& m_compiler;

	public:
		AppSignalMap(ModuleLogicCompiler& compiler);
		~AppSignalMap();

		void insert(const AppItem* appItem);
		void insert(const AppItem* appItem, const LogicPin& outputPin);

		void clear();
	};


	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	private:

		// input parameters
		//

		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		Afbl::AfbElementCollection* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		ApplicationLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;

		// memory addresses and sizes
		//

		int m_addrAppLogicBit = 0;			// address of bit-addressed application logic memory
		int m_sizeAppLogicBit = 0;			// size of bit-addressed application logic memory, in bits

		int m_addrAppLogicW = 0;			// address of word-addressed application logic memory
		int m_sizeAppLogicW = 0;			// size of word-addressed application logic memory

		int m_addrLMDiagData = 0;			// address of LM's diagnostics data
		int m_sizeLMDiagData = 0;			// size of LM's diagnostics data

		int m_addrRegData = 0;				// address of registration data

		int m_sizeIOModulesRegData = 0;		// size of IO modules data in registration buffer
		int m_sizeAnalogSignals = 0;		// size of memory allocated to analog signals
		int m_sizeDiscreteSignals = 0;		// size of memory allocated to discrete signals, in bits

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
		//QHash<QUuid, AppItem*> m_pinTypes;					// pin GUID -> parent item ptr
		QHash<QString, Signal*> m_signalsStrID;				// signals StrID -> Signal ptr
		QHash<QString, Signal*> m_deviceBoundSignals;			// device signal strID -> Signal ptr

		QHash<Hardware::DeviceModule::FamilyType, QString> m_moduleFamilyTypeStr;

		QString msg;

	private:
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const char* propertyName, int &value);
		bool getLMIntProperty(const char* propertyName, int &value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		// module logic compilations steps
		//
		bool init();

		bool initMemoryAddressedAndSizes();
		bool buildServiceMaps();
		bool createAppSignalsMap();

		bool afbInitialization();
		bool initializeAppFbConstParams(AppFb* appFb);
		bool initializeAppFbVariableParams(AppFb* appFb);

		bool getUsedAfbs();
		//bool generateAfbInitialization(int fbType, int fbInstance, AlgFbParamArray& params);

		bool copyDiagDataToRegistration();
		bool copyInOutSignalsToRegistration();

		bool calculateInOutSignalsAddresses();
		bool calculateSignalsAddresses();

		bool generateApplicationLogicCode();

		bool writeResult();

		void cleanup();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		const SignalSet& signalSet() { return *m_signals; }
		const Signal* getSignal(const QString& strID);

		OutputLog& log() { return *m_log; }

		const LogicAfbSignal* getAfbSignal(const QUuid& afbGuid, int signalIndex) { return m_afbs.getAfbSignal(afbGuid, signalIndex); }

		bool run();
	};
}

