#pragma once

#include "../lib/Signal.h"
#include "../VFrame30/Bus.h"

#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "Busses.h"

namespace Builder
{

	class SignalSet : public QObject, public ::SignalSet
	{
		Q_OBJECT

	public:
		SignalSet(VFrame30::BusSet* busSet, BuildResultWriter* resultWriter, IssueLogger* log);
		virtual ~SignalSet();

		bool prepareBusses();
		bool checkSignals();
		bool expandBusSignals();
		bool bindSignalsToLMs(Hardware::EquipmentSet* equipment);
		void initCalculatedSignalsProperties();

		BusShared getBus(const QString & busTypeID) const { return m_busses.getBus(busTypeID); }

		static QString buildBusSignalCaption(const QString& busParentSignalCaption,
												 const QString& busTypeID,
												 const QString& busParentSignalCustomID,
												 const QString& busChildSignalID,
												 const QString& busChildSignalCaption) const;

	private:
		bool appendBusSignal(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal);
		QString buildBusSignalCaption(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal);

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		//

		int m_maxSignalID = -1;
		QHash<int, QString> m_busSignals;

		Busses m_busses;
	};

}
