#pragma once

#include "../AppSignalLib/Bus.h"

#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "Busses.h"

namespace Hardware
{
	class EquipmentSet;
}

namespace Builder
{
	typedef std::shared_ptr<Hardware::DeviceModule> DeviceModuleShared;

	class SignalSet : public QObject, public ::AppSignalSet
	{
		Q_OBJECT

	public:
		SignalSet(AppSignalLib::BusSet* busSet, std::shared_ptr<BuildResultWriter> resultWriter, IssueLogger* log);
		virtual ~SignalSet();

		const ::AppSignalSet* appSignalSet() const;

		bool prepareBusses();
		bool checkSignals(bool isSafetyProject);
		bool checkSignalsIDsAndHashes();
		bool bindSignalsToLMs(Hardware::EquipmentSet* equipment);
		void initCalculatedSignalsProperties();
		void cacheSpecPropValues();
		bool expandTemplates(Hardware::EquipmentSet* equipment);

		BusShared getBus(const QString & busTypeID) const { return m_busses.getBus(busTypeID); }

		AppSignal* appendBusChildSignal(const AppSignal& s, BusShared bus, const BusSignal& busSignal, DeviceModuleShared lm);
		AppSignal* createBusChildSignal(const AppSignal& s, BusShared bus, const BusSignal& busSignal);

		void findAndRemoveExcludedFromBuildSignals();

		void linkSignalToLm(AppSignal* appSignal, DeviceModuleShared lm);

		void append(AppSignal* signal) = delete;			// Hiding AppSignalSet::append(AppSignal*) of base class
															// All signals that will be added to this signal set
															// should be use append(AppSignal*, DeviceModuleShared)

		void append(AppSignal* appSignal, DeviceModuleShared lm);		// Here SignalSet take on ownership of appSignal object

		DeviceModuleShared getAppSignalLm(const AppSignal* appSignal) const;
		DeviceModuleShared getAppSignalLm(const QString& appSignalID) const;

		bool isSignalExists(const QString& appSignalID) const;

		void resetAddresses();

	private:
		QString expandBusSignalCaptionTemplate(const AppSignal& busParentSignal, BusShared bus, const BusSignal& busSignal) const;

		bool checkSignalPropertiesRanges(const AppSignal& s);
		bool checkSignalPropertyRanges(const AppSignal& s, const QString& propertyName);
		bool checkSignalTuningValuesRanges(const AppSignal& s, const TuningValue& tuningValue, const QString& propertyName);

	private:
		AppSignalLib::BusSet* m_busSet = nullptr;
		std::shared_ptr<BuildResultWriter> m_resultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		Busses m_busses;

		std::map<const AppSignal*, DeviceModuleShared> m_signalToLm;
	};

	using SignalSetShared = std::shared_ptr<SignalSet>;

}
