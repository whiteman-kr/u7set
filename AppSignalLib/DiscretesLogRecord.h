#pragma once

namespace Network
{
	class DiscretesLogRecord;
}

struct DiscretesLogRecord
{
	qint64 recordID = 0;
	qint64 recordTime = 0;
	qint64 plantTime = 0;
	qint64 systemTime = 0;
	qint64 localTime = 0;
	Hash signalHash = UNDEFINED_HASH;
	double value = 0;
	quint32 flags = 0;
	bool acknowledged = false;
	qint64 ackTime = 0;
	QString ackSource;
	QString ackUser;

	void saveToProto(Network::DiscretesLogRecord* dlr);
	void loadFromProto(const Network::DiscretesLogRecord& dlr);
};