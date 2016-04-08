#include "TuningUdpSocket.h"

namespace Tuning
{
	// -------------------------------------------------------------------------
	//
	//	TuningSocket class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocket::TuningSocket(const TuningSettings& settings) :
		SimpleThread(new TuningSocketWorker(settings))
	{
	}


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketWorker::TuningSocketWorker(const TuningSettings &settings) :
		m_settings(settings)
	{
	}


	void TuningSocketWorker::readTunigData()
	{

	}


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker::Request class implementaton
	//
	// -------------------------------------------------------------------------


	void TuningSocketWorker::Request::initToRead(const TuningSettings& settings, quint64 tuningID, int frameNo)
	{
		header.version = 1;
		header.tuningID = tuningID;
//		header.subsystemKey = subsystemKey;
//		header.operationCode = TO_INT(OperationCode::Read);
		header.flags = 0;
//		header.startAddressW = 0;			//
		header.requestSizeB;			// UDP frame size = 1432 bytes
		header.romSizeB;				// = ROM_SIZE_B
		header.romFrameSizeB;			// = ROM_FRAME_SIZE_B
		header.dataType;				// DataType enum values
	}


}
