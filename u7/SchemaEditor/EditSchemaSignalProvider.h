#pragma once
#include "../../lib/IAppSignalManager.h"
#include "../../lib/Tuning/ITuningSignalManager.h"

class AppSignalSetProvider;

//
//
// EditSchemaAppSignalProvider - this calss is used to provide app signals for drawing schemas, showing and getting signal ids, description, preciosion, etc...
//
//
class EditSchemaAppSignalProvider final : public IAppSignalManager
{
public:
	EditSchemaAppSignalProvider() = delete;
	EditSchemaAppSignalProvider(AppSignalSetProvider* signalSetProvider);

	// IAppSignalManager implementation
	//
public:
	virtual std::vector<AppSignalParam> signalList() const override;

	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;

	virtual AppSignalParam signalParam(Hash signalHash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual AppSignalState signalState(Hash signalHash, bool* found) const override;
	virtual AppSignalState signalState(const QString& appSignalId, bool* found) const override;

	virtual void signalState(const std::vector<Hash>& appSignalHashes, std::vector<AppSignalState>* result, int* found) const override;
	virtual void signalState(const std::vector<QString>& appSignalIds, std::vector<AppSignalState>* result, int* found) const override;

	virtual QStringList signalTags(Hash signalHash) const override;
	virtual QStringList signalTags(const QString& appSignalId) const override;

	virtual bool signalHasTag(Hash signalHash, const QString& tag) const override;
	virtual bool signalHasTag(const QString& appSignalId, const QString& tag) const override;

	virtual E::SignalType signalType(Hash signalHash, bool* found) const final;
	virtual E::SignalType signalType(const QString& appSignalId, bool* found) const final;

	virtual QString equipmentToAppSiganlId(const QString& equipmentId) const final;

	// Setpoints
	//
	virtual std::vector<std::shared_ptr<Comparator>> setpointsByInputSignalId(const QString& appSignalId) const override;

private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
};


//
//
// EditSchemaTuningSignalProvider - this calss is used to provide tuning signals for drawing schemas, showing and getting signal ids, description, preciosion, etc...
//
//
class EditSchemaTuningSignalProvider : public ITuningSignalManager
{
public:
	EditSchemaTuningSignalProvider() = delete;
	EditSchemaTuningSignalProvider(AppSignalSetProvider* signalSetProvider);

	// ITuningSignalManager implementation
	//
public:
	virtual bool signalExists(Hash hash) const override;
	virtual bool signalExists(const QString& appSignalId) const override;

	virtual AppSignalParam signalParam(Hash hash, bool* found) const override;
	virtual AppSignalParam signalParam(const QString& appSignalId, bool* found) const override;

	virtual bool signalParam(Hash hash, AppSignalParam* result) const override;
	virtual bool signalParam(const QString& appSignalId, AppSignalParam* result) const override;

	virtual TuningSignalState state(Hash hash, bool* found) const override;
	virtual TuningSignalState state(const QString& appSignalId, bool* found) const override;


private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
};
