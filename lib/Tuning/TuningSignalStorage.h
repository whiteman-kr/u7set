#ifndef TUNINGSIGNALSTORAGE_H
#define TUNINGSIGNALSTORAGE_H

#include "../lib/Hash.h"
#include "../lib/AppSignal.h"

class TuningSignalStorage
{

public:
	TuningSignalStorage();

public:
	bool loadSignals(const QByteArray& data, QString* errorCode);
	bool addSignal(const AppSignalParam& param);

	// Object accessing
	//
	int signalsCount() const;

	bool signalExists(Hash hash) const;

	AppSignalParam* signalPtrByIndex(int index) const;
	AppSignalParam* signalPtrByHash(Hash hash) const;

private:
	// Signals
	//
	std::map<Hash, int> m_signalsMap;								// Key is Hash from AppSignalID, value is signal index in m_signals
	std::vector<std::shared_ptr<AppSignalParam>> m_signals;
};

#endif
