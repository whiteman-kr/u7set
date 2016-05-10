#pragma once

#include <QtCore>
#include <QList>
#include "../include/Signal.h"


class TuningData
{
private:
	class FrameTriple
	{

	};


	QString m_lmEquipmentID;

	int m_tuningFrameSizeBytes = 0;
	int m_tuningFramesCount = 0;

/*	static const int ANALOG_FLOAT = 0;
	static const int ANALOG_INT = 1;
	static const int DISCRETE = 2;

	static const int TYPES_COUNT = 3;			// analog float, analog int, discrete

	static const int typeSizeBit[TYPES_COUNT];

	static const int VALUES_COUNT = 3;			// default value, low bound, high bound

	int	m_framesCount[TYPES_COUNT];

	int getNededTuningFramesCount(int signalsCount, int signalValueSizeBits);*/

	char* m_tuningData = nullptr;
	int m_tuningDataSize = 0;

public:
	TuningData(	QString lmID,
				int tuningFrameSizeB,
				int tuningFramesCount,
				QList<Signal*>& analogFloatSignals,
				QList<Signal*>& analogIntSignals,
				QList<Signal*>& discreteSignals);
	~TuningData();

	QByteArray tuningData() const;
};


class TuningDataStorage : public QHash<QString, TuningData*>
{
public:
	~TuningDataStorage();
};

