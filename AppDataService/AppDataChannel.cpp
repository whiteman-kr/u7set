#include "AppDataChannel.h"


// -------------------------------------------------------------------------------
//
// AppDataChannel class implementation
//
// -------------------------------------------------------------------------------

AppDataChannel::AppDataChannel(int channel, const HostAddressPort& dataReceivingIP) :
	DataChannel(channel, DataSource::DataType::App, dataReceivingIP)
{
}


// -------------------------------------------------------------------------------
//
// AppDataChannelThread class implementation
//
// -------------------------------------------------------------------------------

AppDataChannelThread::AppDataChannelThread(int channel, const HostAddressPort& dataRecievingIP)
{
	m_dataChannel = new AppDataChannel(channel, dataRecievingIP);
	addWorker(m_dataChannel);
}


void AppDataChannelThread::prepare(QVector<Signal>& appSignals, QHash<QString, int> appSignalID2IndexMap, AppSignalState* signalStates)
{
}
