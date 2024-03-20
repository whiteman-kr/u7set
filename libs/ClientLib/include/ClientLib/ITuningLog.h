#pragma once


namespace ClientLib
{
	class ITuningLog
	{
	public:
		virtual ~ITuningLog() = default;

		virtual bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue) = 0;
		virtual bool write(const QString& message) = 0;

		virtual void viewTuningLog(QWidget* parent) = 0;
	};


	class TuningLogStub : public ITuningLog
	{
	public:

		virtual bool write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue) override;
		virtual bool write(const QString& message) override;

		virtual void viewTuningLog(QWidget* parent) override;
	};
}
