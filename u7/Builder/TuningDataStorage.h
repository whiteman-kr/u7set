#pragma once

#include <QtCore>
#include <QList>
#include "../include/Signal.h"
#include "IssueLogger.h"


class TuningSignalsData
{
public:
	static const int BITS_8 = 8;
	static const int FRAMES_3 = 3;

private:
	int m_firstFrameNo = 0;
	int m_tuningFrameSizeBytes = 0;
	int m_tuningFrameSizeBits = 0;

	int m_signalSizeBits = 0;
	int m_signalCount = 0;

	//

	int m_tripleFramesCount = 0;
	int m_totalFramesCount = 0;

	char* m_framesData = nullptr;

	void setFramesDataBit(int offset, int bit, int value);

public:
	TuningSignalsData();
	virtual ~TuningSignalsData();

	void init(int firstFrameNo, int tuningFrameSizeBytes, int signalSizeBits, int signalCount);

	void copySignalsData(QList<Signal*> signalsList);

	int totalFramesCount() const { return m_totalFramesCount; }
	int framesDataSize() const { return m_totalFramesCount * m_tuningFrameSizeBytes; }

	const char* framesData() const { return m_framesData; }

	void converToBigEndian();
};


class TuningData : public QObject
{
	Q_OBJECT

private:
	QString m_lmEquipmentID;

	int m_tuningFrameSizeBytes = 0;
	int m_tuningFramesCount = 0;
	quint64 m_uninqueID = 0;

	static const int TYPE_ANALOG_FLOAT = 0;
	static const int TYPE_ANALOG_INT = 1;
	static const int TYPE_DISCRETE = 2;

	static const int TYPES_COUNT = 3;			// analog float, analog int, discrete

	TuningSignalsData m_tuningSignalsData[TYPES_COUNT];

	QList<Signal*> m_tuningAnalogFloat;
	QList<Signal*> m_tuningAnalogInt;
	QList<Signal*> m_tuningDiscrete;

	bool m_deleteSignals = false;

	// XML serialization constants
	//
	static const char* TUNING_DATA_ELEMENT;
	static const char* LM_ID;
	static const char* TUNING_FRAME_SIZE_BYTES;
	static const char* TUNING_FRAMES_COUNT;
	static const char* TUNING_ALL_SIGNALS_COUNT;
	static const char* TUNING_ANALOG_FLOAT_SIGNALS;
	static const char* TUNING_ANALOG_INT_SIGNALS;
	static const char* TUNING_DISCRETE_SIGNALS;
	static const char* TUNING_SIGNALS_COUNT;

public:
	TuningData();
	TuningData(	QString lmID,
				int tuningFrameSizeBytes,
				int tuningFramesCount);

	~TuningData();

	bool buildTuningSignalsLists(HashedVector<QString, Signal*> lmAssociatedSignals, Builder::IssueLogger* log);

	bool buildTuningData();

	quint64 uniqueID() const { return m_uninqueID; }
	void getTuningData(QByteArray* tuningData) const;

	int totalFramesCount() const;

	void writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	void getSignals(QList<Signal *>& signalList);

	QList<Signal*> tuningAnalogFloatSignals() const { return m_tuningAnalogFloat; }
	QList<Signal*> tuningAnalogIntSignals() const { return m_tuningAnalogInt; }
	QList<Signal*> tuningDiscreteSignals() const { return m_tuningDiscrete; }
};


class TuningDataStorage : public QHash<QString, TuningData*>
{
public:
	~TuningDataStorage();
};

