#include <QtEndian>
#include "../lib/Crc.h"
#include "../lib/WUtils.h"
#include "TuningIPENDataStorage.h"
#include "TuningIPENSource.h"


namespace  TuningIPEN
{

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


	void TuningFramesData::copySignalsData(const QVector<Signal*>& signalsList, std::vector<QVariantList>& metadata)
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

			QVariantList data;

			data.append(QVariant(signal->appSignalID()));
			data.append(QVariant(signal->customAppSignalID()));
			data.append(QVariant(signal->caption()));

			if (signal->isAnalog() == true)
			{
				if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
				{
					data.append(QVariant(QString("AnalogFloat")));

					float* defaultValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes);
					*defaultValuePtr = signal->tuningDefaultValue().floatValue();

					data.append(QVariant(*defaultValuePtr));

					float* lowBoundValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes);
					*lowBoundValuePtr = static_cast<float>(signal->lowEngeneeringUnits());

					data.append(QVariant(*lowBoundValuePtr));

					float* highBoundValuePtr = reinterpret_cast<float*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes * 2);
					*highBoundValuePtr = static_cast<float>(signal->highEngeneeringUnits());

					data.append(QVariant(*highBoundValuePtr));
				}
				else
				{
					if (signal->analogSignalFormat() != E::AnalogAppSignalFormat::SignedInt32)
					{
						assert(false);
						continue;
					}

					data.append(QVariant(QString("AnalogInt")));

					qint32* defaultValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes);
					*defaultValuePtr = signal->tuningDefaultValue().int32Value();

					data.append(QVariant(*defaultValuePtr));

					qint32* lowBoundValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes);
					*lowBoundValuePtr = static_cast<qint32>(signal->lowEngeneeringUnits());

					data.append(QVariant(*lowBoundValuePtr));

					qint32* highBoundValuePtr = reinterpret_cast<qint32*>(m_framesData + writeOffsetBytes + m_tuningFrameSizeBytes * 2);
					*highBoundValuePtr = static_cast<qint32>(signal->highEngeneeringUnits());

					data.append(QVariant(*highBoundValuePtr));
				}

				Address16 tuningAddr;

				tuningAddr.setOffset((m_firstFrameNo * m_tuningFrameSizeBytes + writeOffsetBytes) / 2);	// offset in words!!!
				tuningAddr.setBit(0);

				signal->setTuningAddr(tuningAddr);

				data.append(QVariant(tuningAddr.offset()));
				data.append(QVariant(tuningAddr.bit()));

				writeOffsetBytes += m_signalSizeBits / BITS_8;

				if ((writeOffsetBytes % m_tuningFrameSizeBytes) == 0)
				{
					writeOffsetBytes += m_tuningFrameSizeBytes * 2;
				}
			}
			else
			{
				if (signal->isDiscrete() == false)
				{
					assert(false);
					continue;
				}

				data.append(QVariant(QString("Discrete")));

				setFramesDataBit(writeOffsetBytes, bit, signal->tuningDefaultValue().discreteValue());
				setFramesDataBit(writeOffsetBytes + m_tuningFrameSizeBytes, bit, 0);					// low bound
				setFramesDataBit(writeOffsetBytes + m_tuningFrameSizeBytes * 2, bit, 1);				// high bound

				data.append(QVariant(signal->tuningDefaultValue().discreteValue()));
				data.append(QVariant(static_cast<int>(0)));
				data.append(QVariant(static_cast<int>(1)));

				Address16 tuningAddr;

				tuningAddr.setOffset((m_firstFrameNo * m_tuningFrameSizeBytes + writeOffsetBytes) / 2);	// offset in words!!!
				tuningAddr.setBit(bit);

				signal->setTuningAddr(tuningAddr);

				data.append(QVariant(tuningAddr.offset()));
				data.append(QVariant(15 - tuningAddr.bit()));			// conversion to big endian is reverse bits !!!

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

			metadata.push_back(data);
		}

		converToBigEndian();		// !!!
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

				*ptr = reverseBytes<qint32>(*ptr);
			}
		}
		else
		{
			if (m_signalSizeBits == 1)
			{
				for(int offset = 0; offset < size; offset += (16 / BITS_8))
				{
					qint16* ptr = reinterpret_cast<qint16*>(m_framesData + offset);

					*ptr = reverseBytes<qint16>(*ptr);
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

		char* valuePointer = m_framesData + offset;

		// Data in m_framesData is in Big Endian format!
		//
		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
		{
			float value = *reinterpret_cast<float*>(valuePointer);
			value = reverseBytes<float>(value);
			tss->currentValue = static_cast<double>(value);

			float lowLimit = *reinterpret_cast<float*>(valuePointer + m_tuningFrameSizeBytes);
			lowLimit = reverseBytes<float>(lowLimit);
			tss->lowLimit = static_cast<double>(lowLimit);

			float highLimit = *reinterpret_cast<float*>(valuePointer + m_tuningFrameSizeBytes * 2);
			highLimit = reverseBytes<float>(highLimit);
			tss->highLimit = static_cast<double>(highLimit);

			tss->valid = true;

			return true;
		}

		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
		{
			qint32 value = reverseBytes<qint32>(*reinterpret_cast<qint32*>(valuePointer));
			tss->currentValue = static_cast<double>(value);

			qint32 lowLimit = reverseBytes<qint32>(*reinterpret_cast<qint32*>(valuePointer + m_tuningFrameSizeBytes));
			tss->lowLimit = static_cast<double>(lowLimit);

			qint32 highLimit = reverseBytes<qint32>(*reinterpret_cast<qint32*>(valuePointer + m_tuningFrameSizeBytes * 2));
			tss->highLimit = static_cast<double>(highLimit);

			tss->valid = true;

			return true;
		}

		if (signal->isDiscrete())
		{
			int bit = signal->tuningAddr().bit();

			quint16 value = reverseBytes<quint16>(*reinterpret_cast<quint16*>(valuePointer));
			value = (value >> bit) & 0x0001;
			tss->currentValue = static_cast<double>(value);

			quint16 lowLimit = reverseBytes<quint16>(*reinterpret_cast<quint16*>(valuePointer + m_tuningFrameSizeBytes));
			lowLimit = (lowLimit >> bit) & 0x0001;
			tss->lowLimit = static_cast<double>(lowLimit);

			quint16 highLimit = reverseBytes<quint16>(*reinterpret_cast<quint16*>(valuePointer + m_tuningFrameSizeBytes * 2));
			highLimit = (highLimit >> bit) & 0x0001;
			tss->highLimit = static_cast<double>(highLimit);

			tss->valid = true;

			return true;
		}

		assert(false);
		return false;
	}


	bool TuningFramesData::setSignalState(const Signal* signal, double value, SocketRequest* sr)
	{
		if (sr == nullptr)
		{
			assert(false);
			return false;
		}

		int signalOffsetBytes = signal->tuningAddr().offset() * sizeof(quint16);

		int frameNo = signalOffsetBytes / m_tuningFrameSizeBytes;

		assert(frameNo >= m_firstFrameNo);

		sr->startAddressW = (frameNo * m_tuningFrameSizeBytes )/ sizeof(quint16);

		memcpy(sr->fotipData, m_framesData + (frameNo - m_firstFrameNo) * m_tuningFrameSizeBytes, m_tuningFrameSizeBytes);

		int inFrameOffset = signalOffsetBytes - frameNo * m_tuningFrameSizeBytes;

		char* valuePointer = sr->fotipData + inFrameOffset;

		// Data in m_framesData is in Big Endian format!
		//
		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
		{
			float floatValue = reverseBytes<float>(static_cast<float>(value));	// to Big Endian

			*reinterpret_cast<float*>(valuePointer) = floatValue;

			return true;
		}

		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
		{
			qint32 intValue = reverseBytes<qint32>(static_cast<qint32>(value));	// to Big Endian

			*reinterpret_cast<qint32*>(valuePointer) = intValue;

			return true;
		}

		if (signal->isDiscrete())
		{
			quint16 val = value == 0 ? 0 : 1;

			quint16 mask = 0x0001 << signal->tuningAddr().bit();

			quint16 currentValue = reverseBytes<quint16>(*reinterpret_cast<quint16*>(valuePointer));

			if (val == 0)
			{
				currentValue &= ~mask;
			}
			else
			{
				currentValue |= mask;
			}

			*reinterpret_cast<quint16*>(valuePointer) = reverseBytes<quint16>(currentValue);

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

	TuningData::TuningData()
	{
	}


	TuningData::TuningData(QString lmID,
							int tuningFrameSizeBytes,
							int tuningFramesCount) :
		Tuning::TuningData(lmID, 0, tuningFrameSizeBytes, tuningFramesCount)
	{
	}


	TuningData::~TuningData()
	{
	}


	bool TuningData::buildTuningData()
	{
		m_usedFramesCount = 0;

		m_metadata.clear();

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			TuningFramesData& framesData = m_tuningFramesData[type];
			const QVector<Signal*>& tuningSignals = m_tuningSignals[type];

			framesData.init(m_usedFramesCount, m_tuningFramePayloadBytes, signalValueSizeBits(type), tuningSignals.count());
			framesData.copySignalsData(tuningSignals, m_metadata);

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
			const QVector<Signal*>& tuningSignals = m_tuningSignals[type];

			framesData.init(firstFrame, m_tuningFramePayloadBytes, signalValueSizeBits(type), tuningSignals.count());

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

		if (signal == nullptr)
		{
			assert(false);
			return false;
		}

		int type = getSignalType(signal);

		if (type == -1)
		{
			return false;
		}

		return m_tuningFramesData[type].getSignalState(signal, tss);
	}


	bool TuningData::setSignalState(const QString& appSignalID, double value, SocketRequest* sr)
	{
		if (m_id2SignalMap.contains(appSignalID) == false)
		{
			return false;
		}

		Signal* signal = m_id2SignalMap[appSignalID];

		if (signal == nullptr)
		{
			assert(false);
			return false;
		}

		int type = getSignalType(signal);

		if (type == -1)
		{
			return false;
		}

		switch(type)
		{
		case TYPE_ANALOG_FLOAT:
			sr->dataType = FotipDataType::AnalogFloat;
			break;

		case TYPE_ANALOG_INT:
			sr->dataType = FotipDataType::AnalogSignedInt;
			break;

		case TYPE_DISCRETE:
			sr->dataType = FotipDataType::Discrete;
			break;

		default:
			assert(false);
		}

		return m_tuningFramesData[type].setSignalState(signal, value, sr);
	}

}
