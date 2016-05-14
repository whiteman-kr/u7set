#include <QtEndian>
#include "../include/Crc.h"
#include "TuningDataStorage.h"
#include "../TuningService/TuningDataSource.h"


// -------------------------------------------------------------------------------------
//
// TuningSignalsData class implementation
//
// -------------------------------------------------------------------------------------

TuningFramesData::TuningFramesData()
{
}


TuningFramesData::~TuningFramesData()
{
	if (m_framesData != nullptr)
	{
		delete [] m_framesData;
	}
}


void TuningFramesData::init(int firstFrameNo, int tuningFrameSizeBytes, int signalSizeBits, int signalCount)
{
	m_firstFrameNo = firstFrameNo;
	m_tuningFrameSizeBytes = tuningFrameSizeBytes;
	m_tuningFrameSizeBits = m_tuningFrameSizeBytes * BITS_8;
	m_signalSizeBits = signalSizeBits;
	m_signalCount = signalCount;

	// allocate framesData memory
	//
	int neededSize = m_signalSizeBits * m_signalCount;

	m_tripleFramesCount = neededSize / m_tuningFrameSizeBits + (neededSize % m_tuningFrameSizeBits ? 1 : 0);

	m_usedFramesCount = m_tripleFramesCount * FRAMES_3;

	m_framesData = new char [m_usedFramesCount * m_tuningFrameSizeBytes];
	memset(m_framesData, 0, m_usedFramesCount * m_tuningFrameSizeBytes);
}


void TuningFramesData::setFramesDataBit(int offset, int bit, int value)
{
	if (m_framesData == nullptr)
	{
		assert(false);		// call TuningSignalsData::init first
		return;
	}

	if (offset >= framesDataSize())
	{
		assert(false);		// offset out of range
		return;
	}

	if (bit < 0 || bit > 15)
	{
		assert(false);		// bit out of range
		return;
	}

	quint16* ptr = reinterpret_cast<quint16*>(m_framesData + offset);

	if (value == 0)
	{
		quint16 v = ~0x0001;

		v <<= bit;

		*ptr &= v;
	}
	else
	{
		quint16 v = 0x0001;

		v <<= bit;

		*ptr |= v;
	}
}


void TuningFramesData::copySignalsData(QList<Signal*> signalsList)
{
	if (signalsList.count() != m_signalCount )
	{
		assert(false);
		return;
	}

	if (m_framesData == nullptr)
	{
		assert(false);		// call TuningSignalsData::init first
		return;
	}

	int writeOffsetBytes = 0;
	int bit = 0;

	for(Signal* signal : signalsList)
	{
		if (signal == nullptr)
		{
			assert(false);
			continue;
		}

		if (signal->dataSize() != m_signalSizeBits)
		{
			assert(false);
			continue;
		}

		if (signal->isAnalog())
		{
			if (signal->dataFormat() == E::DataFormat::Float)
			{
				float* defaultValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes);
				*defaultValuePtr = static_cast<float>(signal->tuningDefaultValue());

				float* lowBoundValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes);
				*lowBoundValuePtr = static_cast<float>(signal->lowLimit());

				float* highBoundValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes * 2);
				*highBoundValuePtr = static_cast<float>(signal->highLimit());
			}
			else
			{
				assert(signal->dataFormat() == E::DataFormat::SignedInt);

				qint32* defaultValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes);
				*defaultValuePtr = static_cast<qint32>(signal->tuningDefaultValue());

				qint32* lowBoundValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes);
				*lowBoundValuePtr = static_cast<qint32>(signal->lowLimit());

				qint32* highBoundValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes * 2);
				*highBoundValuePtr = static_cast<qint32>(signal->highLimit());
			}

			Address16 tuningAddr;

			tuningAddr.setOffset((m_firstFrameNo * m_tuningFrameSizeBytes + writeOffsetBytes) / 2);	// offset in words!!!
			tuningAddr.setBit(0);

			signal->setTuningAddr(tuningAddr);

			writeOffsetBytes += m_signalSizeBits / BITS_8;

			if ((writeOffsetBytes % m_tuningFrameSizeBytes) == 0)
			{
				writeOffsetBytes += m_tuningFrameSizeBytes * 2;
			}
		}
		else
		{
			setFramesDataBit(writeOffsetBytes, bit, signal->tuningDefaultValue() == 0.0 ? 0 : 1);
			setFramesDataBit(writeOffsetBytes + m_tuningFrameSizeBytes, bit, 0);					// low bound
			setFramesDataBit(writeOffsetBytes + m_tuningFrameSizeBytes * 2, bit, 1);				// high bound

			Address16 tuningAddr;

			tuningAddr.setOffset((m_firstFrameNo * m_tuningFrameSizeBytes + writeOffsetBytes) / 2);	// offset in words!!!
			tuningAddr.setBit(bit);

			signal->setTuningAddr(tuningAddr);

			bit++;

			if (bit == 16)
			{
				writeOffsetBytes += sizeof(quint16);
				bit = 0;
			}

			if ((writeOffsetBytes % m_tuningFrameSizeBytes) == 0 && bit == 0)
			{
				writeOffsetBytes += m_tuningFrameSizeBytes * 2;
			}
		}
	}
}


void TuningFramesData::converToBigEndian()
{
	if (m_framesData == nullptr)
	{
		assert(false);		// call TuningSignalsData::init first
		return;
	}

	int size = m_usedFramesCount * m_tuningFrameSizeBytes;

	if (m_signalSizeBits == 32)
	{
		for(int offset = 0; offset < size; offset += (32 / BITS_8))
		{
			qint32* ptr = reinterpret_cast<qint32*>(m_framesData + offset);

			*ptr = qToBigEndian<qint32>(*ptr);
		}
	}
	else
	{
		if (m_signalSizeBits == 1)
		{
			for(int offset = 0; offset < size; offset += (16 / BITS_8))
			{
				qint16* ptr = reinterpret_cast<qint16*>(m_framesData + offset);

				*ptr = qToBigEndian<qint16>(*ptr);
			}
		}
		else
		{
			assert(false);	// unknown size !!!
		}
	}
}


void TuningFramesData::setFrameData(int frameNo, const char* fotipData)
{
	if (frameNo < 0 || frameNo >= m_usedFramesCount)
	{
		assert(false);
		return;
	}

	if (m_framesData == nullptr)
	{
		assert(false);
		return;
	}

	memcpy(m_framesData + frameNo * FOTIP_TX_RX_DATA_SIZE, fotipData, FOTIP_TX_RX_DATA_SIZE);
}


bool TuningFramesData::getSignalState(const Signal* signal, TuningSignalState* tss)
{
	int offset = signal->tuningAddr().offset() * sizeof(quint16) - m_firstFrameNo * m_tuningFrameSizeBytes;

	if (signal->isAnalog() && signal->dataFormat() == E::DataFormat::Float)
	{
		float value = qToLittleEndian<float>(*reinterpret_cast<float*>(m_framesData + offset));
		tss->currentValue = static_cast<double>(value);

		float lowLimit = qToLittleEndian<float>(*reinterpret_cast<float*>(m_framesData + offset + m_tuningFrameSizeBytes));
		tss->lowLimit = static_cast<double>(lowLimit);

		float highLimit = qToLittleEndian<float>(*reinterpret_cast<float*>(m_framesData + offset + m_tuningFrameSizeBytes * 2));
		tss->lowLimit = static_cast<double>(highLimit);

		return true;
	}

	if (signal->isAnalog() && signal->dataFormat() == E::DataFormat::SignedInt)
	{
		qint32 value = qToLittleEndian<qint32>(*reinterpret_cast<qint32*>(m_framesData + offset));
		tss->currentValue = static_cast<double>(value);

		qint32 lowLimit = qToLittleEndian<qint32>(*reinterpret_cast<qint32*>(m_framesData + offset + m_tuningFrameSizeBytes));
		tss->lowLimit = static_cast<double>(lowLimit);

		qint32 highLimit = qToLittleEndian<qint32>(*reinterpret_cast<qint32*>(m_framesData + offset + m_tuningFrameSizeBytes * 2));
		tss->lowLimit = static_cast<double>(highLimit);

		return true;
	}

	if (signal->isDiscrete())
	{
		int bit = signal->tuningAddr().bit();

		quint16 value = qToLittleEndian<quint16>(*reinterpret_cast<quint16*>(m_framesData + offset));
		value = (value >> bit) & 0x0001;
		tss->currentValue = static_cast<double>(value);

		quint16 lowLimit = qToLittleEndian<quint16>(*reinterpret_cast<quint16*>(m_framesData + offset + m_tuningFrameSizeBytes));
		lowLimit = (lowLimit >> bit) & 0x0001;
		tss->lowLimit = static_cast<double>(lowLimit);

		quint16 highLimit = qToLittleEndian<quint16>(*reinterpret_cast<quint16*>(m_framesData + offset + m_tuningFrameSizeBytes * 2));
		highLimit = (highLimit >> bit) & 0x0001;
		tss->lowLimit = static_cast<double>(highLimit);

		return true;
	}

	assert(false);
	return false;
}

// -------------------------------------------------------------------------------------
//
// TuningData class implementation
//
// -------------------------------------------------------------------------------------

const char* TuningData::TUNING_DATA_ELEMENT = "TuningData";
const char* TuningData::LM_ID = "LmID";
const char* TuningData::UNIQUE_ID = "UniqueID";
const char* TuningData::TUNING_FRAME_SIZE_BYTES = "FrameSizeBytes";
const char* TuningData::TUNING_FRAMES_COUNT = "FramesCount";
const char* TuningData::TUNING_USED_FRAMES_COUNT = "UsedFramesCount";
const char* TuningData::TUNING_ALL_SIGNALS_COUNT = "TuningSignalsCount";
const char* TuningData::TUNING_ANALOG_FLOAT_SIGNALS = "AnalogFloatSignals";
const char* TuningData::TUNING_ANALOG_INT_SIGNALS = "AnalogIntSignals";
const char* TuningData::TUNING_DISCRETE_SIGNALS = "DiscreteSignals";
const char* TuningData::TUNING_SIGNALS_COUNT = "Count";


TuningData::TuningData()
{
}


TuningData::TuningData(QString lmID,
						int tuningFrameSizeBytes,
						int tuningFramesCount) :
	m_lmEquipmentID(lmID),
	m_tuningFrameSizeBytes(tuningFrameSizeBytes),
	m_tuningFramesCount(tuningFramesCount)
{
}


TuningData::~TuningData()
{
	if (m_deleteSignals == false)
	{
		return;
	}

	for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
	{
		for(Signal* signal : m_tuningSignals[type])
		{
			qDebug() << "Delete " << signal->appSignalID();
			delete signal;
		}

		m_tuningSignals[type].clear();
	}
}


bool TuningData::buildTuningSignalsLists(HashedVector<QString, Signal *> lmAssociatedSignals, Builder::IssueLogger* log)
{
	bool result = true;

	for(QList<Signal*>& signalList : m_tuningSignals)
	{
		signalList.clear();
	}

	for(Signal* signal : lmAssociatedSignals)
	{
		if (signal->enableTuning() == false)
		{
			continue;
		}

		if (signal->isAnalog())
		{
			if (signal->dataSize() != 32)
			{
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
					  QString(tr("Signal '%1' for tuning must have 32-bit dataSize")).
					  arg(signal->appSignalID()));
				result = false;
			}
			else
			{
				if (signal->dataFormat() == E::DataFormat::Float)
				{
					m_tuningSignals[TYPE_ANALOG_FLOAT].append(signal);
				}
				else
				{
					if (signal->dataFormat() == E::DataFormat::SignedInt)
					{
						m_tuningSignals[TYPE_ANALOG_INT].append(signal);
					}
					else
					{
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
							  QString(tr("Signal '%1' for tuning must have Float or Signed Int data format")).
							  arg(signal->appSignalID()));
						result = false;
					}
				}
			}
		}
		else
		{
			if (signal->dataSize() != 1)
			{
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
					  QString(tr("Signal '%1' for tuning must have 1-bit dataSize")).
					  arg(signal->appSignalID()));
				result = false;
				continue;
			}
			else
			{
				m_tuningSignals[TYPE_DISCRETE].append(signal);
			}
		}
	}

	return result;
}



bool TuningData::buildTuningData()
{
	m_usedFramesCount = 0;

	for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
	{
		TuningFramesData& framesData = m_tuningFramesData[type];
		QList<Signal*>& tuningSignals = m_tuningSignals[type];

		framesData.init(m_usedFramesCount, m_tuningFrameSizeBytes, signalValueSizeBits(type), tuningSignals.count());
		framesData.copySignalsData(tuningSignals);
		framesData.converToBigEndian();

		m_usedFramesCount += framesData.usedFramesCount();
	}

	return true;
}


bool TuningData::initTuningData()
{
	int firstFrame = 0;

	for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
	{
		TuningFramesData& framesData = m_tuningFramesData[type];
		QList<Signal*>& tuningSignals = m_tuningSignals[type];

		framesData.init(firstFrame, m_tuningFrameSizeBytes, signalValueSizeBits(type), tuningSignals.count());

		firstFrame += framesData.usedFramesCount();

		for(Signal* signal : tuningSignals)
		{
			m_id2SignalMap.insert(signal->appSignalID(), signal);
		}
	}

	if (firstFrame != m_usedFramesCount)
	{
		assert(false);
	}

	return true;
}


int TuningData::signalValueSizeBits(int type)
{
	switch(type)
	{
	case TYPE_ANALOG_FLOAT:
		return sizeof(float) * TuningFramesData::BITS_8;

	case TYPE_ANALOG_INT:
		return sizeof(qint32) * TuningFramesData::BITS_8;

	case TYPE_DISCRETE:
		return 1;

	default:
		assert(false);
	}

	return 0;
}


quint64 TuningData::generateUniqueID(const QString& lmEquipmentID)
{
	Crc64 crc;

	crc.add(lmEquipmentID);

	for(QList<Signal*>& signalList : m_tuningSignals)
	{
		for(Signal* signal : signalList)
		{
			crc.add(signal->appSignalID());
			crc.add(signal->equipmentID());
			crc.add(signal->tuningDefaultValue());
			crc.add(signal->lowLimit());
			crc.add(signal->highLimit());
		}
	}

	m_uniqueID = crc.result();

	return m_uniqueID;
}


void TuningData::getTuningData(QByteArray* tuningData) const
{
	if (tuningData == nullptr)
	{
		assert(false);
		return;
	}

	for(int i = 0; i < TYPES_COUNT; i++)
	{
		tuningData->append(m_tuningFramesData[i].framesData(), m_tuningFramesData[i].framesDataSize());
	}
}


void TuningData::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(TUNING_DATA_ELEMENT);

	xml.writeStringAttribute(LM_ID, m_lmEquipmentID);
	xml.writeUInt64Attribute(UNIQUE_ID, m_uniqueID, true);
	xml.writeIntAttribute(TUNING_FRAME_SIZE_BYTES, m_tuningFrameSizeBytes);
	xml.writeIntAttribute(TUNING_FRAMES_COUNT, m_tuningFramesCount);
	xml.writeIntAttribute(TUNING_USED_FRAMES_COUNT, m_usedFramesCount);

	int signalCount = 0;

	for(QList<Signal*>& signalList : m_tuningSignals)
	{
		signalCount += signalList.count();
	}

	xml.writeIntAttribute(TUNING_ALL_SIGNALS_COUNT, signalCount);

	const char* typeSection[TYPES_COUNT] =
	{
		TUNING_ANALOG_FLOAT_SIGNALS,
		TUNING_ANALOG_INT_SIGNALS,
		TUNING_DISCRETE_SIGNALS
	};

	for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
	{
		QList<Signal*>& tuningSignals = m_tuningSignals[type];

		xml.writeStartElement(typeSection[type]);
		xml.writeIntAttribute(TUNING_SIGNALS_COUNT, tuningSignals.count());

		for(Signal* signal : tuningSignals)
		{
			signal->writeToXml(xml);
		}

		xml.writeEndElement();		//	</typeSection[type]>
	}

	xml.writeEndElement();		//	</TUNING_DATA_ELEMENT>
}


bool TuningData::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	if (xml.findElement(TUNING_DATA_ELEMENT) == false)
	{
		return false;
	}

	result &= xml.readStringAttribute(LM_ID, &m_lmEquipmentID);
	result &= xml.readUInt64Attribute(UNIQUE_ID, &m_uniqueID);
	result &= xml.readIntAttribute(TUNING_FRAME_SIZE_BYTES, &m_tuningFrameSizeBytes);
	result &= xml.readIntAttribute(TUNING_FRAMES_COUNT, &m_tuningFramesCount);
	result &= xml.readIntAttribute(TUNING_USED_FRAMES_COUNT, &m_usedFramesCount);

	int totalSignalsCount = 0;

	result &= xml.readIntAttribute(TUNING_ALL_SIGNALS_COUNT, &totalSignalsCount);

	m_deleteSignals = true;		// !

	const char* typeSection[TYPES_COUNT] =
	{
		TUNING_ANALOG_FLOAT_SIGNALS,
		TUNING_ANALOG_INT_SIGNALS,
		TUNING_DISCRETE_SIGNALS
	};

	for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
	{
		if (xml.findElement(typeSection[type]) == false)
		{
			return false;
		}

		int count = 0;

		result &= xml.readIntAttribute(TUNING_SIGNALS_COUNT, &count);

		for(int i = 0; i < count; i++)
		{
			if (xml.findElement("Signal") == false)
			{
				result = false;
				break;
			}

			Signal* signal = new Signal(false);

			result &= signal->readFromXml(xml);

			m_tuningSignals[type].append(signal);
		}
	}

	return result;
}


void TuningData::getSignals(QList<Signal*>& signalList)
{
	signalList.clear();

	for(QList<Signal*>& list : m_tuningSignals)
	{
		signalList.append(list);
	}
}


void TuningData::setFrameData(int frameNo, const char* fotipData)
{
	for(int i = 0; i < TYPES_COUNT; i++ )
	{
		int firstFrameNo = m_tuningFramesData[i].firstFrameNo();
		int usedFramesCount = m_tuningFramesData[i].usedFramesCount();

		if (frameNo >= firstFrameNo + usedFramesCount)
		{
			continue;
		}

		m_tuningFramesData[i].setFrameData(frameNo - firstFrameNo, fotipData);
		break;
	}
}


bool TuningData::getSignalState(const QString& appSignalID, TuningSignalState* tss)
{
	if (m_id2SignalMap.contains(appSignalID) == false)
	{
		return false;
	}

	Signal* signal = m_id2SignalMap[appSignalID];

	int type = getSignalType(signal);

	if (type == -1)
	{
		return false;
	}

	return m_tuningFramesData[type].getSignalState(signal, tss);
}


int TuningData::getSignalType(const Signal* signal)
{
	if (signal->isAnalog() && signal->dataFormat() == E::DataFormat::Float)
	{
		return TYPE_ANALOG_FLOAT;
	}

	if (signal->isAnalog() && signal->dataFormat() == E::DataFormat::SignedInt)
	{
		return TYPE_ANALOG_INT;
	}

	if (signal->isDiscrete())
	{
		return TYPE_DISCRETE;
	}

	assert(false);
	return -1;
}

// -------------------------------------------------------------------------------------
//
// TuningDataStorage class implementation
//
// -------------------------------------------------------------------------------------

TuningDataStorage::~TuningDataStorage()
{
	for(TuningData* tuningData : *this)
	{
		delete tuningData;
	}

	QHash<QString, TuningData*>::clear();
}
