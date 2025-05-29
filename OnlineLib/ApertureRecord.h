#pragma once

#include <Network.pb.h>

struct ApertureRecord
{
	QString signalID;
	E::ApertureType apertureType = E::ApertureType::RangePercent;
	double coarseAperture = 0;
	double fineAperture = 0;
	bool setDefault = false;

	QString toString() const;
	void fromString(const QString& str);

	void saveToProto(Network::ApertureRecord* ar) const;
	void readFromProto(const Network::ApertureRecord& ar);
};
