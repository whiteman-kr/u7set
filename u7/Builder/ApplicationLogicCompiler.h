#pragma once

#include <QObject>
#include <QTranslator>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../Builder/ApplicationLogicBuilder.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"
#include "AfblSet.h"
#include "../VFrame30/FblItemRect.h"
#include "../VFrame30/VideoItemSignal.h"



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



/*	class AlgFbParam
	{
	public:
		QString caption;
		int index = 0;
		int size = 16;
		quint16 value = 0;
	};*/


//	typedef QVector<AlgFbParam> AlgFbParamArray;

	class AppFbMap
	{
	public:

	};


	class ApplicationSignal
	{
	private:
		QString m_strID;
		Signal* m_signal = nullptr;
		bool m_calculated = false;
		SignalType m_signalType;

	public:
		ApplicationSignal(const QString& strID) : m_strID(strID) {}

		void setStrID(QString strID) { m_strID = strID; }
		QString strID() const { return m_strID; }

		void setCalculated() { m_calculated = true; }
		bool isCalculated() const { return m_calculated; }

		void setSignal(Signal* signal) { m_signal = signal; }
		Signal* signal() const { return m_signal; }


		void signalType(SignalType signalType) { m_signalType = signalType; }
		SignalType signalType() const { return m_signalType; }

		bool isGhostSignal() { return m_signal == nullptr; }
	};


	typedef QHash<QString, ApplicationSignal> ApplicationSignalsMap;


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

		ApplicationSignalsMap m_appSignals;

		QHash<QString, int> m_signalStrIdMap;

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

		bool copyDiagData();
		bool copyInOutSignals();

		bool generateApplicationLogicCode();

		bool writeResult();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		bool run();
	};
}

