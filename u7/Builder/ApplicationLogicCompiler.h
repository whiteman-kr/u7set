#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
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


	// Functional Block Library element
	//

	class Fbl
	{
	private:
		AfbElement* m_afbElement = nullptr;
		bool m_singleInstance = true;
		quint16 m_currentInstance = 0;

	public:
		Fbl(AfbElement* afbElement);

		quint16 addInstance();

		const AfbElement& afbElement() const { return *m_afbElement; }

		bool isSingleInstance() const { return m_singleInstance; }

		QUuid guid() const { return m_afbElement->guid(); }
		QString strID() const { return m_afbElement->strID(); }
	};


	class FblsMap : public QHash<QUuid, Fbl*>
	{
	public:
		~FblsMap() { clear(); }

		int addInstance(LogicFb* logicFb);

		void insert(AfbElement* afbElement);
		void clear();
	};


	// Application Functional Block
	// represent all FB items in application logic schemes
	//

	class AppFb
	{
	private:
		LogicFb* m_logicFb;
		quint16 m_instance = -1;

	public:
		AppFb(LogicFb* logicFb, int instance) : m_logicFb(logicFb), m_instance(instance) {}

		QUuid afbGuid() const { return m_logicFb->afbGuid(); }
		quint16 instance() const { return m_instance; }
	};


	class AppFbsMap : public QHash<QUuid, AppFb*>
	{
	private:
		QVector<AppFb*> m_appFbs;

	public:
		~AppFbsMap() { clear(); }

		void insert(LogicFb* logicFb, int instance);
		void clear();
	};


	// Application Signal
	// represent all signal in application logic schemes, and signals, which createad in compiling time
	//

	class AppSignal
	{
	private:
		QString m_strID;
		Signal* m_signal = nullptr;
		bool m_calculated = false;
		SignalType m_signalType;

		bool m_shadowSignal = false;

	public:
		AppSignal(const QString& strID, bool isShadowSignal) : m_strID(strID), m_shadowSignal(isShadowSignal) {}

		void setStrID(QString strID) { m_strID = strID; }
		QString strID() const { return m_strID; }

		void setCalculated() { m_calculated = true; }
		bool isCalculated() const { return m_calculated; }

		void setSignal(Signal* signal) { m_signal = signal; }
		Signal* signal() const { return m_signal; }


		void signalType(SignalType signalType) { m_signalType = signalType; }
		SignalType signalType() const { return m_signalType; }

		bool isShadowSignal() { return m_shadowSignal; }
	};


	class AppSignalsMap : public QHash<QUuid, AppSignal*>
	{
	private:
		QHash<QString, AppSignal*> m_signalStrIdMap;
		QVector<AppSignal*> m_appSignals;

	public:
		~AppSignalsMap();

		void insert(QUuid guid, const QString& signalStrID, bool isShadowSignal);
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
		QHash<QUuid, LogicItem*> m_logicItems;			// item GUID -> item ptr
		QHash<QUuid, LogicItem*> m_itemsPins;			// pin GUID -> parent item ptr

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

