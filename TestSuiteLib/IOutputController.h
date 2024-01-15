#pragma once

namespace TestSuite
{
	class IOutputController
	{
	public:
		virtual ~IOutputController() = default;

		virtual bool waitForConnection(qint64 timeoutMs) const = 0;
		virtual bool writeSignalValue(const QString& appSignalId, const QVariant& value) = 0;
		virtual bool waitForAllSignalsWritten(qint64 timeoutMs, quint64& timeElapsedMs) const = 0;

		virtual bool tuningSourceIsActive(QString lmEquipmentId) const = 0;
		virtual bool tuningSourceIsInactive(QString lmEquipmentId) const = 0;
		virtual bool activateTuningSource(QString lmEquipmentId, bool activate) = 0;
	};

	class OutputControllerStub : public IOutputController
	{
	public:
		bool waitForConnection(qint64 /*timeoutMs*/) const override {return false;}
		bool writeSignalValue(const QString& /*appSignalId*/, const QVariant& /*value*/) override {return false;}
		bool waitForAllSignalsWritten(qint64 /*timeoutMs*/, quint64& /*timeElapsedMs*/ ) const override { return false; }

		bool tuningSourceIsActive(QString lmEquipmentId) const override { return true; }
		bool tuningSourceIsInactive(QString lmEquipmentId) const override { return true; }
		bool activateTuningSource(QString lmEquipmentId, bool activate) override { return true; }
	};
}

