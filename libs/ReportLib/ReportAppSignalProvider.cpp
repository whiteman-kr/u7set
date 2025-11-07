#include <ReportLib/ReportAppSignalProvider.h>

#include "../AppSignalLib/IAppSignalManager.h"

namespace ReportLib
{
	//
	//
	// ReportSchemaAppSignalProvider - this class is used to provide app signals for drawing schemas, showing and getting signal ids,
	// description, precision, etc...
	//
	//

	ReportAppSignalProvider::ReportAppSignalProvider(const AppSignalSet* signalSet) :
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

	std::vector<Hash> ReportAppSignalProvider::signalHashes() const
	{
		// Unlikely this function required for schema editing
		//
		Q_ASSERT(false);
		return {};
	}

	bool ReportAppSignalProvider::signalExists(Hash hash) const
	{
		return m_signalSet->contains(hash);
	}

	bool ReportAppSignalProvider::signalsExist(const QStringList& signalIds) const
	{
		return std::all_of(signalIds.begin(),
						   signalIds.end(),
						   [this](const QString& appSignalId)
						   {
							   return m_signalSet->contains(appSignalId);
						   });
	}

	std::optional<AppSignalParam> ReportAppSignalProvider::signalParam(Hash signalHash) const
	{
		auto signal = m_signalSet->getSignalByHash(signalHash);

		if (signal != nullptr)
		{
			return AppSignalParam{*signal};
		}

		return std::nullopt;
	}

	std::optional<AppSignalState> ReportAppSignalProvider::signalState(Hash signalHash) const
	{
		bool exists = signalExists(signalHash);
		if (exists == false)
		{
			return std::nullopt;
		}

		AppSignalState result;
		result.m_flags.valid = 1;
		result.m_value = 0;

		return result;
	}

	std::optional<AppSignalState> ReportAppSignalProvider::signalState(Hash signalHash, Hash /*dataServerHash*/) const
	{
		return signalState(signalHash);
	}

	void ReportAppSignalProvider::signalState(std::span<const Hash> appSignalHashes,
											  std::vector<std::optional<AppSignalState>>* result) const
	{
		if (result == nullptr)
		{
			Q_ASSERT(result);
			return;
		}

		result->clear();
		result->reserve(appSignalHashes.size());

		std::transform(appSignalHashes.begin(),
					   appSignalHashes.end(),
					   std::back_inserter(*result),
					   [this](Hash hash)
					   {
						   return signalState(hash);
					   });

		return;
	}

	void ReportAppSignalProvider::signalState(std::span<const Hash> appSignalHashes,
											  Hash /*dataServerHash*/,
											  std::vector<std::optional<AppSignalState>>* result) const
	{
		return signalState(appSignalHashes, result);
	}

	QStringList ReportAppSignalProvider::signalTags(Hash signalHash) const
	{
		const AppSignal* s = m_signalSet->getSignalByHash(signalHash);

		if (s != nullptr)
		{
			return s->tags();
		}

		return {};
	}

	bool ReportAppSignalProvider::signalHasTag(Hash signalHash, const QString& tag) const
	{
		return signalTags(signalHash).contains(tag, Qt::CaseInsensitive);
	}

	QStringList ReportAppSignalProvider::signalIdsByTag(const QString& /*tag*/) const
	{
		// No simulation of this function in edit schema mode
		//
		Q_ASSERT(false);
		return {};
	}

	E::SignalType ReportAppSignalProvider::signalType(Hash signalHash, bool* found) const
	{
		auto sp = signalParam(signalHash);
		if (found != nullptr)
		{
			*found = sp.has_value();
		}
		return sp.has_value() ? sp->type() : E::SignalType::Analog;
	}

	QString ReportAppSignalProvider::equipmentToAppSignalId(const QString& equipmentId) const
	{
		return equipmentId;
	}

	std::vector<std::shared_ptr<Comparator>> ReportAppSignalProvider::setpointsByInput(const QString& appSignalId) const
	{
		// No simulation of this function in edit schema mode
		//
		Q_UNUSED(appSignalId);
		return {};
	}

	std::shared_ptr<Comparator> ReportAppSignalProvider::setpointByOutput(const QString& appSignalId) const
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

} // namespace ReportLib
