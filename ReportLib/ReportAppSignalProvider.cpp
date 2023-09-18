#include "ReportAppSignalProvider.h"

namespace ReportLib
{
	//
	//
	// ReportSchemaAppSignalProvider - this calss is used to provide app signals for drawing schemas, showing and getting signal ids, description, preciosion, etc...
	//
	//

	ReportAppSignalProvider::ReportAppSignalProvider(const AppSignalSet *signalSet) :
		m_signalSet(signalSet)
	{
		Q_ASSERT(signalSet);
	}

	int ReportAppSignalProvider::signalsCount() const
	{
		// Unlikely this function required for schema editing
		//
		Q_ASSERT(false);
		return 0;
	}

	std::vector<AppSignalParam> ReportAppSignalProvider::signalList() const
	{
		// Unlikely this function required for schema editing
		//
		Q_ASSERT(false);
		return {};
	}

	bool ReportAppSignalProvider::signalExists(Hash hash) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(hash);
		Q_ASSERT(false);
		return {};
	}

	bool ReportAppSignalProvider::signalExists(const QString& appSignalId) const
	{
		return m_signalSet->contains(appSignalId);
	}

	bool ReportAppSignalProvider::signalsExist(const QStringList& signalIds) const
	{
		return std::all_of(signalIds.begin(), signalIds.end(), [this](const QString& appSignalId) {
			return m_signalSet->contains(appSignalId);
		});
	}

	AppSignalParam ReportAppSignalProvider::signalParam(Hash signalHash, bool* found) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(signalHash);
		Q_UNUSED(found);
		Q_ASSERT(false);
		return {};
	}

	AppSignalParam ReportAppSignalProvider::signalParam(const QString& appSignalId, bool* found) const
	{
		AppSignalParam result;

		const AppSignal* s = m_signalSet->getSignal(appSignalId);

		if (found != nullptr)
		{
			*found = s != nullptr;
		}

		if (s != nullptr)
		{
			result.load(*s);
		}

		return result;
	}

	AppSignalState ReportAppSignalProvider::signalState(Hash signalHash, bool* found) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(signalHash);
		Q_UNUSED(found);
		Q_ASSERT(false);
		return {};
	}

	AppSignalState ReportAppSignalProvider::signalState(const QString& appSignalId, bool* found) const
	{
		AppSignalState result;
		result.m_hash = ::calcHash(appSignalId);

		bool exists = signalExists(appSignalId);
		if (found != nullptr)
		{
			*found = exists;
		}

		if (exists == true)
		{
			result.m_flags.valid = 1;
			result.m_value = 0;

			//		result.m_time.plant = TimeStamp{QDateTime::currentDateTime()};
			//		result.m_time.local = result.m_time.plant;
			//		result.m_time.system = TimeStamp{QDateTime::currentDateTimeUtc()};
		}

		return result;
	}

	AppSignalState ReportAppSignalProvider::signalState(Hash signalHash, Hash /*dataServerHash*/, bool* found) const
	{
		return signalState(signalHash, found);
	}

	AppSignalState ReportAppSignalProvider::signalState(const QString& appSignalId, const QString& /*dataServerId*/, bool* found) const
	{
		return signalState(appSignalId, found);

	}

	void ReportAppSignalProvider::signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(appSignalHashes);
		Q_UNUSED(result);
		Q_UNUSED(found);
		Q_ASSERT(false);
		return;
	}

	void ReportAppSignalProvider::signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const
	{
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return;
		}

		if (found != nullptr)
		{
			*found = 0;
		}

		result->clear();
		result->reserve(appSignalIds.size());

		for (const QString& id : appSignalIds)
		{
			bool signalFound = false;

			result->emplace_back(this->signalState(id, &signalFound));

			if (signalFound && found != nullptr)
			{
				(*found)++;
			}
		}

		return;
	}

	void ReportAppSignalProvider::signalState(const std::vector<Hash>& appSignalHashes, Hash /*dataServerHash*/, std::vector<AppSignalState>* result, int* found) const
	{
		signalState(appSignalHashes, result, found);
	}

	void ReportAppSignalProvider::signalState(const std::vector<QString>& appSignalIds, const QString& /*dataServerId*/, std::vector<AppSignalState>* result, int* found) const
	{
		signalState(appSignalIds, result, found);
	}


	QStringList ReportAppSignalProvider::signalTags(Hash signalHash) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(signalHash);
		Q_ASSERT(false);
		return {};
	}

	QStringList ReportAppSignalProvider::signalTags(const QString& appSignalId) const
	{
		const AppSignal* s = m_signalSet->getSignal(appSignalId);

		if (s != nullptr)
		{
			return s->tags();
		}

		return {};
	}

	bool ReportAppSignalProvider::signalHasTag(Hash signalHash, const QString& tag) const
	{
		// Unlikely this function required for schema editing
		//
		Q_UNUSED(signalHash);
		Q_UNUSED(tag);
		Q_ASSERT(false);
		return false;
	}

	bool ReportAppSignalProvider::signalHasTag(const QString& appSignalId, const QString& tag) const
	{
		return signalTags(appSignalId).contains(tag, Qt::CaseInsensitive);
	}

	E::SignalType ReportAppSignalProvider::signalType(Hash signalHash, bool* found) const
	{
		Q_UNUSED(signalHash);
		Q_UNUSED(found);
		Q_ASSERT(false);	// to do
		return E::SignalType::Analog;
	}

	QStringList ReportAppSignalProvider::signalIdsByTag(const QString& /*tag*/) const
	{
		// No simulation of this function in edit schema mode
		//
		Q_ASSERT(false);
		return {};
	}

	E::SignalType ReportAppSignalProvider::signalType(const QString& appSignalId, bool* found) const
	{
		return signalType(::calcHash(appSignalId), found);
	}

	QString ReportAppSignalProvider::equipmentToAppSignalId(const QString& /*equipmentId*/) const
	{
		Q_ASSERT(false);	// todo
		return {};
	}

	std::vector<std::shared_ptr<Comparator>> ReportAppSignalProvider::setpointsByInputSignalId(const QString& appSignalId) const
	{
		// No simulation of this function in edit schema mode
		//
		Q_UNUSED(appSignalId);
		return {};
	}

	QStringList ReportAppSignalProvider::tags() const
	{
		// No simulation of this function in edit schema mode
		//
		return {};
	}

}
