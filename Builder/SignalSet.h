#pragma once

#include "../lib/AppSignal.h"
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
		SignalSet(VFrame30::BusSet* busSet, std::shared_ptr<BuildResultWriter> resultWriter, IssueLogger* log);
		virtual ~SignalSet();

		bool prepareBusses();
		bool checkSignals();
		bool bindSignalsToLMs(Hardware::EquipmentSet* equipment);
		void initCalculatedSignalsProperties();
		void cacheSpecPropValues();
		bool expandTemplates(Hardware::EquipmentSet* equipment);

		BusShared getBus(const QString & busTypeID) const { return m_busses.getBus(busTypeID); }

		Signal* appendBusChildSignal(const Signal& s, BusShared bus, const BusSignal& busSignal);
		Signal* createBusChildSignal(const Signal& s, BusShared bus, const BusSignal& busSignal);

		void findAndRemoveExcludedFromBuildSignals();

	private:
		QString expandBusSignalCaptionTemplate(const Signal& busParentSignal, BusShared bus, const BusSignal& busSignal) const;

		bool checkSignalPropertiesRanges(const Signal& s);
		bool checkSignalPropertyRanges(const Signal& s, const QString& propertyName);
		bool checkSignalTuningValuesRanges(const Signal& s, const TuningValue& tuningValue, const QString& propertyName);

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		std::shared_ptr<BuildResultWriter> m_resultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		//

		QHash<int, QString> m_busSignals;

		Busses m_busses;
	};

}
