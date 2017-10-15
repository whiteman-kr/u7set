#include "TuningSignalStorage.h"
#include "../Proto/serialization.pb.h"


//
// TuningSignalStorage
//

TuningSignalStorage::TuningSignalStorage()
{

}


bool TuningSignalStorage::loadSignals(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	m_signals.clear();
	m_signalsMap.clear();

	::Proto::AppSignalSet set;
	bool result = set.ParseFromArray(data.data(), data.size());

	if (result == false)
	{
		assert(false);
		*errorCode = QObject::tr("Failed to load tuning signals file.");
		return false;
	}

	// Read signals
	//
	for (int i = 0; i < set.appsignal_size(); i++)
	{
		std::shared_ptr<AppSignalParam> asp = std::make_shared<AppSignalParam>();

		if (asp->load(set.appsignal(i)) == false)
		{
			assert(false);
			*errorCode = QObject::tr("Failed to load tuning signal #%1 from file.").arg(i);
			return false;
		}

		if (m_signalsMap.find(asp->hash()) != m_signalsMap.end())
		{
			assert(false);
			*errorCode = QObject::tr("Tuning signal #%1 has duplicate hash: %2.").arg(i).arg(asp->hash());
			return false;
		}

		m_signals.push_back(asp);

		m_signalsMap[asp->hash()] = static_cast<int>(m_signals.size()) - 1;
	}

	assert(m_signalsMap.size() == m_signals.size());

	return true;
}

bool TuningSignalStorage::addSignal(const AppSignalParam& param)
{
	std::shared_ptr<AppSignalParam> asp = std::make_shared<AppSignalParam>(param);

	if (m_signalsMap.find(asp->hash()) != m_signalsMap.end())
	{
		assert(false);
		return false;
	}

	m_signals.push_back(asp);

	m_signalsMap[asp->hash()] = static_cast<int>(m_signals.size()) - 1;

	assert(m_signalsMap.size() == m_signals.size());

	return true;
}


int TuningSignalStorage::signalsCount() const
{
	return static_cast<int>(m_signals.size());

}

bool TuningSignalStorage::signalExists(Hash hash) const
{
	return (m_signalsMap.find(hash) != m_signalsMap.end());
}

AppSignalParam* TuningSignalStorage::signalPtrByIndex(int index) const
{
	if (index < 0 || index >= m_signals.size())
	{
		assert(false);
		return nullptr;
	}

	return m_signals[index].get();
}

AppSignalParam* TuningSignalStorage::signalPtrByHash(Hash hash) const
{
	const auto it = m_signalsMap.find(hash);

	if (it == m_signalsMap.end())
	{
		assert(false);
		return nullptr;
	}

	return signalPtrByIndex(it->second);
}
