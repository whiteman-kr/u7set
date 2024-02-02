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

bool DiagDataSource::parseRupData(	const Times& time,
									bool isSimPacket,
									quint16 packetNo,
									const char* rupData,
									int rupDataSize,
									const QThread* thread)
{
	Q_UNUSED(time);
	Q_UNUSED(isSimPacket);
	Q_UNUSED(packetNo);
	Q_UNUSED(rupData);
	Q_UNUSED(rupDataSize);
	Q_UNUSED(thread);

	Q_ASSERT(false);		// to do
	SimpleDiagSignalState newState;

	pushSignalState(newState, thread);

	return true;
}

bool DiagDataSource::invalidateAllSignals(const QThread* thread)
{
	Q_UNUSED(thread);
	return true;
}

