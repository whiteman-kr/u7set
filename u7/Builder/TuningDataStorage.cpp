#include "TuningDataStorage.h"
#include <QtEndian>
#include "../include/Crc.h"


// -------------------------------------------------------------------------------------
//
// TuningSignalsData class implementation
//
// -------------------------------------------------------------------------------------

TuningSignalsData::TuningSignalsData()
{
}


TuningSignalsData::~TuningSignalsData()
{
	if (m_framesData != nullptr)
	{
		delete [] m_framesData;
	}
}


void TuningSignalsData::init(int firstFrameNo, int tuningFrameSizeBytes, int signalSizeBits, int signalCount)
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

	m_totalFramesCount = m_tripleFramesCount * FRAMES_3;

	m_framesData = new char [m_totalFramesCount * m_tuningFrameSizeBytes];
	memset(m_framesData, 0, m_totalFramesCount * m_tuningFrameSizeBytes);
}


void TuningSignalsData::setFramesDataBit(int offset, int bit, int value)
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


void TuningSignalsData::copySignalsData(QList<Signal*> signalsList)
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


void TuningSignalsData::converToBigEndian()
{
	if (m_framesData == nullptr)
	{
		assert(false);		// call TuningSignalsData::init first
		return;
	}

	int size = m_totalFramesCount * m_tuningFrameSizeBytes;

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

	for(Signal* signal : m_tuningAnalogFloat)
	{
		delete signal;
	}

	for(Signal* signal : m_tuningAnalogInt)
	{
		delete signal;
	}

	for(Signal* signal : m_tuningDiscrete)
	{
		delete signal;
	}
}


bool TuningData::buildTuningSignalsLists(HashedVector<QString, Signal *> lmAssociatedSignals, Builder::IssueLogger* log)
{
	bool result = true;

	m_tuningAnalogFloat.clear();
	m_tuningAnalogInt.clear();
	m_tuningDiscrete.clear();

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
					m_tuningAnalogFloat.append(signal);
				}
				else
				{
					if (signal->dataFormat() == E::DataFormat::SignedInt)
					{
						m_tuningAnalogInt.append(signal);
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
				m_tuningDiscrete.append(signal);
			}
		}
	}

	return result;
}



bool TuningData::buildTuningData()
{
	TuningSignalsData& analogFloatSignalsData = m_tuningSignalsData[TYPE_ANALOG_FLOAT];

	int firstFrame = 0;

	analogFloatSignalsData.init(firstFrame, m_tuningFrameSizeBytes, sizeof(float) * TuningSignalsData::BITS_8, m_tuningAnalogFloat.count());
	analogFloatSignalsData.copySignalsData(m_tuningAnalogFloat);
	analogFloatSignalsData.converToBigEndian();

	firstFrame += analogFloatSignalsData.totalFramesCount();

	TuningSignalsData& analogIntSignalsData = m_tuningSignalsData[TYPE_ANALOG_INT];

	analogIntSignalsData.init(firstFrame, m_tuningFrameSizeBytes, sizeof(qint32) * TuningSignalsData::BITS_8, m_tuningAnalogInt.count());
	analogIntSignalsData.copySignalsData(m_tuningAnalogInt);
	analogIntSignalsData.converToBigEndian();

	firstFrame += analogIntSignalsData.totalFramesCount();

	TuningSignalsData& discreteSignalsData = m_tuningSignalsData[TYPE_DISCRETE];

	discreteSignalsData.init(firstFrame, m_tuningFrameSizeBytes, 1, m_tuningDiscrete.count());
	discreteSignalsData.copySignalsData(m_tuningDiscrete);
	discreteSignalsData.converToBigEndian();

	return true;
}


quint64 TuningData::generateUniqueID(const QString& lmEquipmentID)
{
	Crc64 crc;

	crc.add(lmEquipmentID);

	for (Signal* signal : m_tuningAnalogFloat)
	{
		crc.add(signal->appSignalID());
		crc.add(signal->equipmentID());
		crc.add(signal->tuningDefaultValue());
		crc.add(signal->lowLimit());
		crc.add(signal->highLimit());
	}

	for(Signal* signal : m_tuningAnalogInt)
	{
		crc.add(signal->appSignalID());
		crc.add(signal->equipmentID());
		crc.add(signal->tuningDefaultValue());
		crc.add(signal->lowLimit());
		crc.add(signal->highLimit());
	}

	for(Signal* signal : m_tuningDiscrete)
	{
		crc.add(signal->appSignalID());
		crc.add(signal->equipmentID());
		crc.add(signal->tuningDefaultValue());
		crc.add(signal->lowLimit());
		crc.add(signal->highLimit());
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
		tuningData->append(m_tuningSignalsData[i].framesData(), m_tuningSignalsData[i].framesDataSize());
	}
}


int TuningData::totalFramesCount() const
{
	int framesCount = 0;

	for(int i = 0; i < TYPES_COUNT; i++)
	{
		framesCount += m_tuningSignalsData[i].totalFramesCount();
	}

	return framesCount;
}


void TuningData::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(TUNING_DATA_ELEMENT);

	xml.writeStringAttribute(LM_ID, m_lmEquipmentID);
	xml.writeUInt64Attribute(UNIQUE_ID, m_uniqueID, true);
	xml.writeIntAttribute(TUNING_FRAME_SIZE_BYTES, m_tuningFrameSizeBytes);
	xml.writeIntAttribute(TUNING_FRAMES_COUNT, m_tuningFramesCount);
	xml.writeIntAttribute(TUNING_ALL_SIGNALS_COUNT, m_tuningAnalogFloat.count() +
													m_tuningAnalogInt.count() +
													m_tuningDiscrete.count());

	xml.writeStartElement(TUNING_ANALOG_FLOAT_SIGNALS);
	xml.writeIntAttribute(TUNING_SIGNALS_COUNT, m_tuningAnalogFloat.count());

	for(Signal* signal : m_tuningAnalogFloat)
	{
		signal->writeToXml(xml);
	}

	xml.writeEndElement();		//	</TUNING_ANALOG_FLOAT_SIGNALS>


	xml.writeStartElement(TUNING_ANALOG_INT_SIGNALS);
	xml.writeIntAttribute(TUNING_SIGNALS_COUNT, m_tuningAnalogInt.count());

	for(Signal* signal : m_tuningAnalogInt)
	{
		signal->writeToXml(xml);
	}

	xml.writeEndElement();		//	</TUNING_ANALOG_INT_SIGNALS>

	xml.writeStartElement(TUNING_DISCRETE_SIGNALS);
	xml.writeIntAttribute(TUNING_SIGNALS_COUNT, m_tuningDiscrete.count());

	for(Signal* signal : m_tuningDiscrete)
	{
		signal->writeToXml(xml);
	}

	xml.writeEndElement();		//	</TUNING_DISCRETE_SIGNALS>

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

	int totalSignalsCount = 0;

	result &= xml.readIntAttribute(TUNING_ALL_SIGNALS_COUNT, &totalSignalsCount);

	m_deleteSignals = true;		// !

	//

	if (xml.findElement(TUNING_ANALOG_FLOAT_SIGNALS) == false)
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
		m_tuningAnalogFloat.append(signal);
	}

	//

	if (xml.findElement(TUNING_ANALOG_INT_SIGNALS) == false)
	{
		return false;
	}

	count = 0;

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
		m_tuningAnalogInt.append(signal);
	}

	//

	if (xml.findElement(TUNING_DISCRETE_SIGNALS) == false)
	{
		return false;
	}

	count = 0;

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
		m_tuningDiscrete.append(signal);
	}

	return result;
}


void TuningData::getSignals(QList<Signal*>& signalList)
{
	signalList.clear();

	signalList.append(m_tuningAnalogFloat);
	signalList.append(m_tuningAnalogInt);
	signalList.append(m_tuningDiscrete);
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
