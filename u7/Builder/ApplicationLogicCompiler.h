#pragma once

#include <QObject>
#include <QTranslator>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../Builder/BuildResultWriter.h"
#include "../Builder/ApplicationLogicCode.h"


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
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;

		QVector<Hardware::DeviceModule*> m_lm;

		QString msg;

	private:
		void findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool compileModulesLogics();


	public:
		ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();

		friend class ModuleLogicCompiler;
	};


	class ModuleLogicCompiler : public QObject
	{
		Q_OBJECT

	private:
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;

		Hardware::DeviceModule* m_lm = nullptr;
		Hardware::DeviceChassis* m_chassis = nullptr;

		AddrW m_regDataAddress;

		ApplicationLogicCode m_code;

		QString msg;

	private:
		bool getDeviceIntProperty(Hardware::DeviceObject* device, const char* propertyName, int &value);
		bool getLMIntProperty(const char* propertyName, int &value);

		Hardware::DeviceModule* getModuleOnPlace(int place);

		// module logic compilations steps
		//
		bool init();
		bool copyDiagData();
		bool copyInOutSignals();

		bool writeResult();

	public:
		ModuleLogicCompiler(ApplicationLogicCompiler& appLogicCompiler, Hardware::DeviceModule* lm);

		bool run();
	};
}

