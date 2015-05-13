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
#include "AfblSet.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/VideoItemSignal.h"
#include "../VFrame30/VideoItemFblElement.h"
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
		AfblSet* m_afbl = nullptr;
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
		ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, AfblSet* afblSet, ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();

		friend class ModuleLogicCompiler;
	};


	typedef VFrame30::FblItemRect LogicItem;
	typedef VFrame30::VideoItemSignal LogicSignal;
	typedef VFrame30::VideoItemFblElement LogicFb;
	typedef VFrame30::CFblConnectionPoint LogicPin;

	class AppItem;

	class ModuleLogicCompiler;

	// Functional Block Library element
	//

	typedef QHash<CommandCodes, int> CommandCodesInstanceMap;
	typedef QHash<QString, int> NonRamFblInstanceMap;

	class Fbl
	{
	private:
		AfbElement* m_afbElement = nullptr;
		quint16 m_currentInstance = 0;

		// dynamically created maps
		//
		static int m_refCount;
		static CommandCodesInstanceMap* m_commandCodesInstance;		// CommandCodes -> current instance
		static NonRamFblInstanceMap* m_nonRamFblInstance;			// Non RAM Fbl StrID -> instance

	public:
		Fbl(AfbElement* afbElement);
		~Fbl();

		quint16 addInstance();

		bool hasRam() const { return m_afbElement->hasRam(); }

		const AfbElement& afbElement() const { return *m_afbElement; }

		QUuid guid() const { return m_afbElement->guid(); }
		QString strID() const { return m_afbElement->strID(); }
	};


	class FblsMap : public HashedVector<QUuid, Fbl*>
	{
	public:
		~FblsMap() { clear(); }

		int addInstance(AppItem *appItem);

		void insert(AfbElement* afbElement);
		void clear();
	};


	// Base class for AppFb & AppSignal
	// contains pointer to AppLogicItem
	//

	class AppItem
	{
	protected:
		const VFrame30::FblItemRect* m_fblItem = nullptr;
		const VFrame30::LogicScheme* m_scheme = nullptr;
		const Afbl::AfbElement* m_afbElement = nullptr;

	public:
		AppItem(const AppItem& appItem);
		AppItem(const AppLogicItem* appLogicItem);

		QUuid guid() const { return m_fblItem->guid(); }
		QUuid afbGuid() const { return m_afbElement->guid(); }

		QString strID() { return m_fblItem->toSignalElement()->signalStrIds(); }

		bool isSignal() const { return m_fblItem->isSignalElement(); }
		bool isFb() const { return m_fblItem->isFblElement(); }

		bool afbInitialized() const { return m_afbElement != nullptr; }

		const std::list<LogicPin>& inputs() const { return m_fblItem->inputs(); }
		const std::list<LogicPin>& outputs() const { return m_fblItem->outputs(); }

		const Afbl::AfbElement& afb() const { return *m_afbElement; }
		//const LogicItem& logic() const { return *m_fblItem; }
	};


	// Application Functional Block
	// represent all FB items in application logic schemes
	//
	class AppFb : public AppItem
	{
	private:
		const LogicFb* m_logicFb = nullptr;
		quint16 m_instance = -1;

	public:
		AppFb(AppItem* appItem, int instance);

		quint16 instance() const { return m_instance; }

		const LogicFb& logicFb() const { return *m_logicFb; }
	};


	class AppFbsMap : public HashedVector<QUuid, AppFb*>
	{
	public:
		~AppFbsMap() { clear(); }

		void insert(AppItem* appItem, int instance);
		void clear();
	};


	// Application Signal
	// represent all signal in application logic schemes, and signals, which createad in compiling time
	//

	class AppSignal
	{
	private:
		AppItem* m_appItem = nullptr;

		QUuid m_guid;
		QString m_strID;
		Signal* m_signal = nullptr;
		bool m_calculated = false;
		SignalType m_signalType;

	public:
		AppSignal(const QUuid& guid, const QString& strID, AppItem* appItem);

		AppItem& appItem() const;

		void setStrID(QString strID) { m_strID = strID; }
		QString strID() const { return m_strID; }

		void setCalculated() { m_calculated = true; }
		bool isCalculated() const { return m_calculated; }

		void setSignal(Signal* signal) { m_signal = signal; }
		Signal* signal() const { return m_signal; }


		void signalType(SignalType signalType) { m_signalType = signalType; }
		SignalType signalType() const { return m_signalType; }

		bool isShadowSignal() { return m_appItem == nullptr; }
	};


	class AppSignalsMap : public HashedVector<QUuid, AppSignal*>
	{
	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;

		void insert(const QUuid& guid, const QString& strID, AppItem* appItem);

	public:
		~AppSignalsMap();

		void insert(AppItem* appItem);
		void insert(const QUuid& ouPinGuid);
		void clear();

		void bindRealSignals(SignalSet* signalSet);
	};



	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	private:

		// input parameters
		//

		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		AfblSet* m_afbl = nullptr;
		ApplicationLogicData* m_appLogicData = nullptr;
		ApplicationLogicModule* m_moduleLogic = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;
		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;

		//

		AddrW m_regDataAddress;

		ApplicationLogicCode m_code;

		FblsMap m_fbls;

		AppSignalsMap m_appSignals;
		AppFbsMap m_appFbs;

		// service maps
		//
		HashedVector<QUuid, AppItem*> m_appItems;			// item GUID -> item ptr
		QHash<QUuid, AppItem*> m_pinParent;			// pin GUID -> parent item ptr

		QString msg;

	private:
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const char* propertyName, int &value);
		bool getLMIntProperty(const char* propertyName, int &value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		// module logic compilations steps
		//
		bool init();

		bool createAppSignalsMap();

		bool afbInitialization();
		bool initializeAppFbConstParams(AppFb* appFb);
		bool initializeAppFbVariableParams(AppFb* appFb);

		bool getUsedAfbs();
		//bool generateAfbInitialization(int fbType, int fbInstance, AlgFbParamArray& params);


		void buildServiceMaps();

		bool copyDiagData();
		bool copyInOutSignals();

		bool generateApplicationLogicCode();

		bool writeResult();

		void cleanup();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		bool run();
	};
}

