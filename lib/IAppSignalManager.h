#ifndef IAPPSIGNALMANAGER_H
#define IAPPSIGNALMANAGER_H

#include "AppSignal.h"

class IAppSignalManager
{
public:
	virtual bool signalExists(Hash hash) const = 0;
	virtual bool signalExists(const QString& appSignalId) const = 0;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const = 0;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const = 0;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const = 0;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const = 0;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const = 0;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const = 0;
};

#endif // IAPPSIGNALMANAGER_H
