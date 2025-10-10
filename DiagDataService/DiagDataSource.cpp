#include "DiagDataSource.h"

// -------------------------------------------------------------------------------
//
// DiagDataSource class implementation
//
// -------------------------------------------------------------------------------

DiagDataSource::DiagDataSource(const DataSource& dataSource) :
	OnlineDataSource<SimpleDiagSignalState>(dataSource, E::LanControllerType::DiagData)
{
}

DiagDataSource::~DiagDataSource()
{

}

bool DiagDataSource::parseRupData(const Times& time,
	bool isSimPacket,
	quint16 packetNo,
	const char* rupData,
	int rupDataSize)
{
	Q_UNUSED(time);
	Q_UNUSED(isSimPacket);
	Q_UNUSED(packetNo);
	Q_UNUSED(rupData);
	Q_UNUSED(rupDataSize);

	Q_ASSERT(false);		// to do
	SimpleDiagSignalState newState;

	pushSignalState(newState);

	return true;
}

bool DiagDataSource::invalidateAllSignals()
{
	return true;
}

bool DiagDataSource::init(const std::map<Hash, Hardware::DiagSignalType>& diagSignalTypes,
						  const ::Network::AcquiredDiagSignalsAndObjects& diagSignalsAndObjects)
{
	Q_UNUSED(diagSignalTypes);

	bool result = true;

	int lmCount = diagSignalsAndObjects.lmdiagsignals().size();

	std::string dsEquipmentID = moduleEquipmentID().toStdString();

	const ::Network::LmDiagSignals* lmDiagSignals = nullptr;

	for(int i = 0; i < lmCount; i++)
	{
		if (diagSignalsAndObjects.lmdiagsignals()[i].lmequipmentid() == dsEquipmentID)
		{
			lmDiagSignals = &diagSignalsAndObjects.lmdiagsignals()[i];
			break;
		}
	}

	if (lmDiagSignals == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	int signalsCount = lmDiagSignals->diagsignals().size();

	for(int i = 0; i < signalsCount; i++)
	{
		const Network::AcquiredDiagSignal& ds = lmDiagSignals->diagsignals()[i];

		Q_UNUSED(ds);

		// create dynamic signal state
	}

	return result;
}


