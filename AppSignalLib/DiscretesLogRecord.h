#pragma once

namespace Network
{
	class DiscretesLogRecord;
}

struct DiscretesLogRecord
{
	qint64 recordID;
	qint64 recordTime;
	qint64 plantTime;
	qint64 systemTime;
	qint64 localTime;
	Hash signalHash;
	double value;
	quint32 flags;
	bool acknowledged;
	qint64 ackTime;
	QString ackSource;
	QString ackUser;

	void saveToProto(Network::DiscretesLogRecord* dlr);
	void loadFromProto(const Network::DiscretesLogRecord& dlr);
};