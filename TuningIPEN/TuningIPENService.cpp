#include "TuningIPENService.h"
#include "../include/WUtils.h"


TuningIPENServiceWorker::TuningIPENServiceWorker(const QString& serviceStrID,
													const QString& cfgServiceIP1,
													const QString& cfgServiceIP2,
													const QString& cfgFileName) :
	TuningServiceWorker(serviceStrID, cfgServiceIP1, cfgServiceIP2, cfgFileName)
{
}


TuningServiceWorker* TuningIPENServiceWorker::createInstance()
{
	TuningIPENServiceWorker* worker = new TuningIPENServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2(), m_cfgFileName);

	worker->setTuningService(m_tuningService);

	m_tuningService->setTuningServiceWorker(worker);

	return worker;			// cast to TuningServiceWorker* - OK
}


void TuningIPENServiceWorker::requestPreprocessing(Tuning::SocketRequest& sr)
{
	if (sr.operation != Tuning::OperationCode::Write)
	{
		return;
	}

	sr.dataType = Tuning::DataType::Discrete;			// turn off limits control!

	quint16* ptr = reinterpret_cast<quint16*>(sr.fotipData);

	for(int i = 0; i < sizeof(sr.fotipData) / sizeof(quint16); i++)
	{
		*ptr = reverseBytes<quint16>(*ptr);

		ptr++;
	}
}


void TuningIPENServiceWorker::replyPreprocessing(Tuning::SocketReply& sr)
{
	quint16* ptr = reinterpret_cast<quint16*>(sr.fotipData);

	for(int i = 0; i < sizeof(sr.fotipData) / sizeof(quint16); i++)
	{
		*ptr = reverseBytes<quint16>(*ptr);

		ptr++;
	}
}
