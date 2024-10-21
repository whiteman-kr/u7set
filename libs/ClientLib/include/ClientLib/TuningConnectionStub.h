#pragma once
// #include <ClientLib/ITuningAuthorization.h>
#include <ClientLib/ITuningConnection.h>


namespace ClientLib
{
	class TuningConnectionStub : public ITuningConnection
	{
	public:
		bool writeTuningSignal(const QString& /*appSignalId*/, const TuningValue& /*value*/) override
		{
			assert(false);
			return false;
		}

		bool writeTuningSignal(const QString& /*appSignalId*/, QVariant /*value*/) override
		{
			assert(false);
			return false;
		}

		void applyTuningSignals() override { assert(false); }
	};
} // namespace ClientLib