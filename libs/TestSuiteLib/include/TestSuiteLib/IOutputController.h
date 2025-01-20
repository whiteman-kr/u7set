#pragma once

namespace TestSuite
{
	class IOutputController
	{
	public:
		virtual ~IOutputController() = default;

		virtual bool init(qint64 timeoutMs) = 0;
		virtual bool shutdown() = 0;

		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) = 0;
		virtual bool waitForAllSignalsWritten(qint64 timeoutMs, qint64& timeElapsedMs) const = 0;

		// To do: separate interface for tuning source control to other interface.
		//
		virtual bool tuningSourceIsActive(QString lmEquipmentId) const = 0;
		virtual bool tuningSourceIsInactive(QString lmEquipmentId) const = 0;
		virtual bool activateTuningSource(QString lmEquipmentId, bool activate) = 0;
	};

	class OutputControllerStub : public IOutputController
	{
	public:
		bool init(qint64 /*timeoutMs*/) override { return true; }
		bool shutdown() override { return true; }

		bool writeSignalValue(const QString& /*appSignalId*/, const QVariant& /*value*/) override { return false; }
		bool waitForAllSignalsWritten(qint64 /*timeoutMs*/, qint64& /*timeElapsedMs*/) const override { return false; }

		bool tuningSourceIsActive(QString /*lmEquipmentId*/) const override { return true; }
		bool tuningSourceIsInactive(QString /*lmEquipmentId*/) const override { return true; }
		bool activateTuningSource(QString /*lmEquipmentId*/, bool /*activate*/) override { return true; }
	};
} // namespace TestSuite
