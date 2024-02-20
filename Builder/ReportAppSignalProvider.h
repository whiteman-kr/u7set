#pragma once

#include "../AppSignalLib/IAppSignalManager.h"

namespace Builder
{
//
//
// ReportAppSignalProvider - this class is used to provide app signals for drawing schemas, showing and getting signal ids, description, preciosion, etc...
//
//
class ReportAppSignalProvider final : public IAppSignalManager
{
public:
	ReportAppSignalProvider() = delete;
	ReportAppSignalProvider(const AppSignalSet* signalSet);

	// IAppSignalManager implementation
	//
public:
	virtual int signalsCount() const override;
	virtual std::vector<AppSignalParam> signalList() const override;

	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;
	virtual bool signalsExist(const QStringList& signalIds) const override;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const override;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;
	virtual AppSignalState signalState(Hash signalHash, Hash dataServerHash, bool* found) const override;
	virtual AppSignalState signalState(const QString& appSignalId, const QString& dataServerId, bool* found) const override;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<Hash>& appSignalHashes, Hash dataServerHash, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<QString>& appSignalIds, const QString& dataServerId, std::vector<AppSignalState>* result, int* found) const override;

	virtual QStringList signalTags(Hash signalHash) const override;
	virtual QStringList signalTags(const QString& appSignalId) const override;

	virtual bool signalHasTag(Hash signalHash, const QString& tag) const override;
	virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override;

	virtual QStringList signalIdsByTag(const QString& tag) const override;

	virtual E::SignalType signalType(Hash signalHash, bool* found) const override;
	virtual E::SignalType signalType(const QString& appSignalId, bool* found) const override;

	virtual QString equipmentToAppSignalId(const QString& equipmentId) const override;

	// Setpoints
	//
	virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override;

	// Tags
	//
	virtual QStringList tags() const override;

private:
	const AppSignalSet* m_signalSet = nullptr;
};
}
