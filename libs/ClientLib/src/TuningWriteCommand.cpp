#include <ClientLib/TuningWriteCommand.h>

namespace ClientLib
{

	//
	// TuningWriteCommand
	//
	bool TuningWriteCommand::toProtoWriteCommand(Network::TuningWriteCommand* message) const
	{
		message->set_signalhash(appSignalHash);
		value.save(message->mutable_value());
		return true;
	}
} // namespace ClientLib
