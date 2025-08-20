#include "DiscretesLogRecord.h"

// ------------------------------------------------------------------------------------------
//
// DiscretesLogRecord struct implementation
//
// ------------------------------------------------------------------------------------------

void DiscretesLogRecord::saveToProto(Network::DiscretesLogRecord* dlr)
{
	if (dlr == nullptr) 
	{
		Q_ASSERT(dlr);
		return;
	}

	dlr->set_recordid(recordID);
	dlr->set_recordtime(recordTime);
	dlr->set_planttime(plantTime);
	dlr->set_systemtime(systemTime);
	dlr->set_localtime(localTime);
	dlr->set_signalhash(signalHash);
	dlr->set_value(value);
	dlr->set_flags(flags);
	dlr->set_acknowledged(acknowledged);
	dlr->set_acktime(ackTime);
	dlr->set_acksource(ackSource.toStdString());
	dlr->set_ackuser(ackUser.toStdString());

	return;
}

void DiscretesLogRecord::loadFromProto(const Network::DiscretesLogRecord& dlr)
{
	recordID = dlr.recordid();
	recordTime = dlr.recordtime();
	plantTime = dlr.planttime();
	systemTime = dlr.systemtime();
	localTime = dlr.localtime();
	signalHash = dlr.signalhash();
	value = dlr.value();
	flags = dlr.flags();
	acknowledged = dlr.acknowledged();
	ackTime = dlr.acktime();
	ackSource = QString::fromStdString(dlr.acksource());
	ackUser = QString::fromStdString(dlr.ackuser());

	return;
}
