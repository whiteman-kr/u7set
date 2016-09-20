#include "AppDataSource.h"


// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------

AppDataSource::AppDataSource()
{
}

bool AppDataSource::getState(Network::AppDataSourceState* protoState) const
{
	if (protoState == nullptr)
	{
		assert(false);
		return false;
	}

	protoState->set_id(m_id);
	protoState->set_uptime(m_uptime);
	protoState->set_receiveddatasize(m_receivedDataSize);
	protoState->set_datareceivingrate(m_dataReceivingRate);
	protoState->set_receivedframescount(m_receivedFramesCount);
	protoState->set_processingenabled(m_dataProcessingEnabled);
	protoState->set_processedpacketcount(m_receivedPacketCount);
	protoState->set_errorprotocolversion(m_errorProtocolVersion);
	protoState->set_errorframesquantity(m_errorFramesQuantity);
	protoState->set_errorframeno(m_errorFrameNo);
	protoState->set_lostedpackets(m_lostedPackets);

	return true;
}


bool AppDataSource::setState(const Network::AppDataSourceState& protoState)
{
	m_id = protoState.id();
	m_uptime = protoState.uptime();
	m_receivedDataSize = protoState.receiveddatasize();
	m_dataReceivingRate = protoState.datareceivingrate();
	m_receivedFramesCount = protoState.receivedframescount();
	m_dataProcessingEnabled = protoState.processingenabled();
	m_receivedPacketCount = protoState.processedpacketcount();
	m_errorProtocolVersion = protoState.errorprotocolversion();
	m_errorFramesQuantity = protoState.errorframesquantity();
	m_errorFrameNo = protoState.errorframeno();
	m_lostedPackets = protoState.lostedpackets();

	return true;
}



// -------------------------------------------------------------------------------
//
// AppDataSources class implementation
//
// -------------------------------------------------------------------------------

AppDataSources::~AppDataSources()
{
	clear();
}


void AppDataSources::clear()
{
	for(AppDataSource* dataSource : *this)
	{
		delete dataSource;
	}

	HashedVector<QString, AppDataSource*>::clear();
}
