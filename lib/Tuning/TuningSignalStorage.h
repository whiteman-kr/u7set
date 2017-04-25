#ifndef TUNINGSIGNALSTORAGE_H
#define TUNINGSIGNALSTORAGE_H

#include "Stable.h"
#include "../lib/Hash.h"
#include "../lib/Signal.h"

class TuningSignalStorage
{

public:

	TuningSignalStorage();

	bool loadSignals(const QByteArray& data, QString *errorCode);

	// Object accessing

	int signalsCount() const;

	bool signalExists(Hash hash) const;

	Signal *signalPtrByIndex(int index) const;

	Signal *signalPtrByHash(Hash hash) const;

private:

	// Signals
	//

	std::map<Hash, int> m_signalsMap;

	std::vector<std::shared_ptr<Signal>> m_signals;

};

#endif
