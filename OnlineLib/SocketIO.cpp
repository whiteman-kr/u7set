#ifndef __ONLINE_LIB__
#error Don't include this file in the project! Link OnlineLib instead.
#endif

#include "SocketIO.h"
#include <assert.h>

QString getNetworkErrorStr(NetworkError err)
{
	switch(err)
	{
	case NetworkError::Success:
		return QString("NetworkError::Success");

	case NetworkError::WrongPartNo:
		return QString("NetworkError::WrongPartNo");

	case NetworkError::RequestParamExceed:
		return QString("NetworkError::RequestParamExceed");

	case NetworkError::RequestStateExceed:
		return QString("NetworkError::RequestStateExceed");

	case NetworkError::ParseRequestError:
		return QString("NetworkError::ParseRequestError");

	case NetworkError::RequestDataSourcesStatesExceed:
		return QString("NetworkError::RequestDataSourcesStatesExceed");

	case NetworkError::UnitsExceed:
		return QString("NetworkError::UnitsExceed");

	case NetworkError::UnknownTuningClientID:
		return QString("NetworkError::UnknownTuningClientID");

	case NetworkError::UnknownSignalHash:
		return QString("NetworkError::UnknownSignalHash");

	case NetworkError::InternalError:
		return QString("NetworkError::InternalError");

	case NetworkError::ArchiveError:
		return QString("NetworkError::ArchiveError");

	case NetworkError::WrongTuningValueType:
		return QString("NetworkError::WrongTuningValueType");

	case NetworkError::TuningValueOutOfRange:
		return QString("NetworkError::TuningValueOutOfRange");

	case NetworkError::SingleLmControlDisabled:
		return QString("NetworkError::SingleLmControlDisabled");

	case NetworkError::LmControlIsNotActive:
		return QString("NetworkError::LmControlIsNotActive");

	case NetworkError::ClientIsNotActive:
		return QString("NetworkError::ClientIsNotActive");

	case NetworkError::TuningNoReply:
		return QString("NetworkError::TuningNoReply");

	case NetworkError::TuningValueCorrupted:
		return QString("NetworkError::TuningValueCorrupted");

	default:
		assert(false);			// unknown err value
	}

	return QString("Unknown NetworkError ???");
}



