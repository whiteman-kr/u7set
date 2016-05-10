#include "TuningDataStorage.h"


//const int TuningData::typeSizeBit[TYPES_COUNT] = { sizeof(float)*8, sizeof(qint32)*8, 1 };


TuningData::TuningData(	QString lmID,
						int tuningFrameSizeB,
						int tuningFramesCount,
						QList<Signal*>& analogFloatSignals,
						QList<Signal*>& analogIntSignals,
						QList<Signal*>& discreteSignals) :
	m_lmEquipmentID(lmID),
	m_tuningFrameSizeBytes(tuningFrameSizeB),
	m_tuningFramesCount(tuningFramesCount)
{
/*	m_analogFloatFramesCount = getNededTuningFramesCount(analogFloatSignals.count(), sizeof(float) * 8);
	m_analogIntFramesCount = getNededTuningFramesCount(analogIntSignals.count(), sizeof(qint32) * 8);
	m_discreteFramesCount = getNededTuningFramesCount(discreteSignals.count(), 1);

	int allocSize = (m_analogFloatFramesCount + m_analogIntFramesCount + m_discreteFramesCount) * 3 * m_tuningFrameSizeBytes;

	m_tuningdata = new char[allocSize];

	m_analogFloatData = reinterpret_cast<float*>(m_tuningData);
	m_analogIntData = reinterpret_cast<qint32*>(m_tuningData + (m_analogFloatFramesCount * 3 * m_tuningFrameSizeBytes);
	m_discreteData = reinterpret_cast<quint16*>(m_tuningData + ((m_analogFloatFramesCount + m_analogIntFramesCount) * 3 * m_tuningFrameSizeBytes);*/
}

TuningData::~TuningData()
{
	if (m_tuningData != nullptr)
	{
		delete [] m_tuningData;
	}
}


QByteArray TuningData::tuningData() const
{
	return QByteArray(m_tuningData, m_tuningDataSize);
}

/*
int TuningData::getNededTuningFramesCount(int signalsCount, int signalValueSizeBits)
{
	int nededSizeBits = signalsCount * signalValueSizeBits;
	return nededSizeBits / m_tuningFrameSizeBits + (nededSizeBits % tuningFrameSizeBits ? 1 : 0);
}


int TuningData::getFrameNo(int signalIndex, int signalValueSizeBits)
{
	return nededSizeBits / m_tuningFrameSizeBits + (nededSizeBits % tuningFrameSizeBits ? 1 : 0);
}*/


TuningDataStorage::~TuningDataStorage()
{
	for(TuningData* tuningData : *this)
	{
		delete tuningData;
	}

	QHash<QString, TuningData*>::clear();
}
