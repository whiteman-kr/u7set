#pragma once

#include "../include/DataChannel.h"
#include "AppSignalState.h"


class AppDataChannel : public DataChannel
{
	Q_OBJECT
public:
	AppDataChannel(int channel, const HostAddressPort& dataReceivingIP);

signals:

public slots:
};



class AppDataChannelThread : public DataChannelThread
{
private:
	DataChannel* m_dataChannel = nullptr;

public:
	AppDataChannelThread(int channel, const HostAddressPort& dataRecievingIP);

	void prepare(QVector<Signal>& appSignals, QHash<QString, int> appSignalID2IndexMap, AppSignalState* signalStates);
};

