#include "PacketsMessages.h"

const char* errorCodeToString(ErrorCode code)
{
	switch (code)
	{
	case ErrorCode::Success:
		return "Success";
	case ErrorCode::NoConnection:
		return "NoConnection";
	case ErrorCode::SnapshotNotFound:
		return "SnapshotNotFound";
	case ErrorCode::SignalNotFound:
		return "SignalNotFound";
	case ErrorCode::OutOfRange:
		return "OutOfRange";
	case ErrorCode::CannotWrite:
		return "CannotWrite";
	default:
		return "Unknown Error Code";
	}
}

const char* simStateToString(SimulatorStateCode code)
{
	switch (code)
	{
	case SimulatorStateCode::Unavailable:
		return "Unavailable";
	case SimulatorStateCode::Stopped:
		return "Stopped";
	case SimulatorStateCode::Running:
		return "Running";
	case SimulatorStateCode::Paused:
		return "Paused";
	default:
		return "Unknown State Code";
	}
}