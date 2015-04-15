#pragma once

#include <QObject>

#include "../include/DeviceObject.h"
#include "../include/Signal.h"
#include "../Builder/BuildResultWriter.h"


namespace Builder
{

	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	private:
		Hardware::DeviceObject* m_equipment = nullptr;
		SignalSet* m_signals = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		OutputLog* m_log = nullptr;

		QVector<Hardware::DeviceObject*> m_lm;

		QString msg;

	private:
		void findLMs(Hardware::DeviceObject* startFromDevice);

	public:
		ApplicationLogicCompiler(Hardware::DeviceObject* equipment, SignalSet* signalSet, BuildResultWriter* buildResultWriter, OutputLog* log);

		bool run();
	};

}

