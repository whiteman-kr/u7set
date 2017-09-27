#pragma once

#include "../lib/Signal.h"
#include "../VFrame30/Bus.h"

#include "IssueLogger.h"

namespace Builder
{

	class SignalSet : public QObject, public ::SignalSet
	{
		Q_OBJECT

	public:
		SignalSet(VFrame30::BusSet* busSet, IssueLogger* log);
		virtual ~SignalSet();

		bool checkSignals();
		bool expandBusSignals();
		bool bindSignalsToLMs(Hardware::EquipmentSet* equipment);
		void initCalculatedSignalsProperties();

	private:
		bool appendBusSignal(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal);
		QString buildBusSignalCaption(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal);

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		IssueLogger* m_log = nullptr;

		int m_maxSignalID = -1;
		QHash<int, QString> m_busSignals;
	};

}
