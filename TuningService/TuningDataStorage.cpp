#include <QtEndian>
#include "../UtilsLib/Crc.h"
#include "../UtilsLib/WUtils.h"
#include "TuningDataStorage.h"
#include "../TuningService/TuningSource.h"


namespace  Tuning
{

	// -------------------------------------------------------------------------------------
	//
	// TuningData class implementation
	//
	// -------------------------------------------------------------------------------------

	const char* TuningData::TUNING_DATA_ELEMENT = "TuningData";
	const char* TuningData::LM_ID = "LmID";
	const char* TuningData::UNIQUE_ID = "UniqueID";

	const char* TuningData::TUNING_FLASH = "TuningFlashMemory";
	const char* TuningData::TUNING_FLASH_FRAME_COUNT = "FrameCount";
	const char* TuningData::TUNING_FLASH_FRAME_PAYLOAD_B = "FramePayloadB";
	const char* TuningData::TUNING_FLASH_FRAME_SIZE_B = "FrameSizeB";

	const char* TuningData::TUNING_DATA = "TuningDataMemory";
	const char* TuningData::TUNING_DATA_OFFSET_W = "OffsetW";
	const char* TuningData::TUNING_DATA_SIZE_W = "SizeW";
	const char* TuningData::TUNING_DATA_FRAME_COUNT = "FrameCount";
	const char* TuningData::TUNING_DATA_FRAME_PAYLOAD_W = "FramePayloadW";
	const char* TuningData::TUNING_DATA_FRAME_SIZE_W = "FrameSizeW";
	const char* TuningData::TUNING_DATA_USED_FRAMES_COUNT = "UsedFramesCount";

	const char* TuningData::TUNING_ALL_SIGNALS_COUNT = "TuningSignalsCount";
	const char* TuningData::TUNING_ANALOG_FLOAT_SIGNALS = "AnalogFloatSignals";
	const char* TuningData::TUNING_ANALOG_INT32_SIGNALS = "AnalogInt32Signals";
	const char* TuningData::TUNING_DISCRETE_SIGNALS = "DiscreteSignals";
	const char* TuningData::TUNING_SIGNALS_COUNT = "Count";

	QStringList TuningData::m_metadataFields;

	TuningData::TuningData()
	{
	}

	TuningData::TuningData(const QString& lmID,
						   int tuningFlashFrameCount,
						   int tuningFlashFramePayloadB,
						   int tuningFlashFrameSizeB,
						   int tuningDataOffsetW,
						   int tuningDataSizeW,
						   int tuningDataFrameCount,
						   int tuningDataFramePayloadW,
						   int tuningDataFrameSizeW) :
		m_lmEquipmentID(lmID),
		m_tuningFlashFrameCount(tuningFlashFrameCount),
		m_tuningFlashFramePayloadB(tuningFlashFramePayloadB),
		m_tuningFlashFrameSizeB(tuningFlashFrameSizeB),
		m_tuningDataOffsetW(tuningDataOffsetW),
		m_tuningDataSizeW(tuningDataSizeW),
		m_tuningDataFrameCount(tuningDataFrameCount),
		m_tuningDataFramePayloadW(tuningDataFramePayloadW),
		m_tuningDataFrameSizeW(tuningDataFrameSizeW)
	{
		for(int& v : m_tuningSignalSizes)
		{
			v = 0;
		}
	}

	// constructor for IPEN tuning only
	//
	TuningData::TuningData(const QString& lmID) :
		m_lmEquipmentID(lmID),
		m_tuningFlashFrameCount(256),
		m_tuningFlashFramePayloadB(1016),
		m_tuningFlashFrameSizeB(1024),
		m_tuningDataOffsetW(46336),
		m_tuningDataSizeW(7620),
		m_tuningDataFrameCount(15),
		m_tuningDataFramePayloadW(508),
		m_tuningDataFrameSizeW(512)
	{
		for(int& v : m_tuningSignalSizes)
		{
			v = 0;
		}
	}

	TuningData::~TuningData()
	{
		if (m_tuningData != nullptr)
		{
			delete [] m_tuningData;
		}

		if (m_deleteSignals == false)
		{
			return;
		}

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			for(Signal* signal : m_tuningSignals[type])
			{
				delete signal;
			}

			m_tuningSignals[type].clear();
		}
	}

	bool TuningData::buildTuningSignalsLists(HashedVector<QString, Signal *> lmAssociatedSignals, Builder::IssueLogger* log)
	{
		bool result = true;

		for(QVector<Signal*>& signalList : m_tuningSignals)
		{
			signalList.clear();
		}

		for(Signal* signal : lmAssociatedSignals)
		{
			if (signal->enableTuning() == false)
			{
				continue;
			}

			if (signal->isAnalog() == true)
			{
				if (signal->dataSize() != SIZE_32BIT)
				{
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
						  QString(tr("Signal '%1' for tuning must have 32-bit dataSize")).
						  arg(signal->appSignalID()));
					result = false;
				}
				else
				{
					if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
					{
						m_tuningSignals[TYPE_ANALOG_FLOAT].append(signal);
					}
					else
					{
						if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
						{
							m_tuningSignals[TYPE_ANALOG_INT32].append(signal);
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

		// calculate tuning signals sizes
		//
		int t = TYPE_ANALOG_FLOAT;

		int totalSize = 0;

		for(QVector<Signal*>& signalList : m_tuningSignals)
		{
			sortSignalsByAcquiredProperty(signalList);

			int signalCount = signalList.size();

			switch(t)
			{
			case TYPE_ANALOG_FLOAT:
				m_tuningSignalSizes[t] = signalCount * sizeof(float);
				break;

			case TYPE_ANALOG_INT32:
				m_tuningSignalSizes[t] = signalCount * sizeof(qint32);
				break;

			case TYPE_DISCRETE:
				m_tuningSignalSizes[t] = signalCount / SIZE_8BIT + ((signalCount % SIZE_8BIT) == 0 ? 0 : 1) ;
				break;

			default:
				assert(false);
			}

			totalSize += m_tuningSignalSizes[t];

			t++;
		}

		// calculate used tuning frames count
		//
		int tuningFramePayloadBytes = m_tuningDataFramePayloadW * WORD_SIZE_IN_BYTES;

		if (tuningFramePayloadBytes == 0)
		{
			m_tuningDataUsedFramesCount = 0;
		}
		else
		{
			m_tuningDataUsedFramesCount = (totalSize / tuningFramePayloadBytes + ((totalSize % tuningFramePayloadBytes) == 0 ? 0 : 1)) * TRIPLE_FRAMES;
		}

		return result;
	}


	void TuningData::buildTuningData()
	{
		// allocate m_tuningData
		//
		m_metadata.clear();

		int tuningFramePayloadBytes = m_tuningDataFramePayloadW * WORD_SIZE_IN_BYTES;

		m_tuningDataSizeB = m_tuningDataUsedFramesCount * tuningFramePayloadBytes;

		m_tuningData = new quint8 [m_tuningDataSizeB];

		memset(m_tuningData, 0, m_tuningDataSizeB);

		// copy tuning signals default values and ranges
		//

		int sizeB = 0;

		for(int t = TYPE_ANALOG_FLOAT; t < TYPES_COUNT; t++)
		{
			int discreteCount = 0;

			for(Signal* signal : m_tuningSignals[t])
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				QVariantList metaData;

				metaData.append(QVariant(signal->appSignalID()));
				metaData.append(QVariant(signal->customAppSignalID()));
				metaData.append(QVariant(signal->caption()));

				// generate metadata
				//

				quint8* dataPtr = m_tuningData + sizeB;

				bool testSizeB = false;

				switch(t)
				{
				case TYPE_ANALOG_FLOAT:
					{
						float defaultValue = signal->tuningDefaultValue().floatValue();
						float lowBound = static_cast<float>(signal->tuningLowBound().floatValue());
						float highBound = static_cast<float>(signal->tuningHighBound().floatValue());

						// in first frame - default value
						//
						*reinterpret_cast<float*>(dataPtr) =
								reverseFloat(defaultValue);

						// in second frame - low bound
						//
						*reinterpret_cast<float*>(dataPtr + tuningFramePayloadBytes) =
								reverseFloat(lowBound);

						// in third frame - high bound
						//
						*reinterpret_cast<float*>(dataPtr + tuningFramePayloadBytes * 2) =
								reverseFloat(highBound);

						signal->setTuningAddr(Address16(sizeB / WORD_SIZE_IN_BYTES, 0));

						signal->setTuningAbsAddr(Address16(m_tuningDataOffsetW + sizeB / WORD_SIZE_IN_BYTES, 0));

						sizeB += sizeof(float);
						testSizeB = true;

						metaData.append(QVariant(QString("AnalogFloat")));
						metaData.append(QVariant(defaultValue));
						metaData.append(QVariant(lowBound));
						metaData.append(QVariant(highBound));
					}
					break;

				case TYPE_ANALOG_INT32:
					{
						qint32 defaultValue = signal->tuningDefaultValue().int32Value();
						qint32 lowBound = static_cast<qint32>(signal->tuningLowBound().int32Value());
						qint32 highBound = static_cast<qint32>(signal->tuningHighBound().int32Value());

						// in first frame - default value
						//
						*reinterpret_cast<qint32*>(dataPtr) =
								reverseInt32(defaultValue);

						// in second frame - low bound
						//
						*reinterpret_cast<qint32*>(dataPtr + tuningFramePayloadBytes) =
								reverseInt32(lowBound);

						// in third frame - high bound
						//
						*reinterpret_cast<qint32*>(dataPtr + tuningFramePayloadBytes * 2) =
								reverseInt32(highBound);

						signal->setTuningAddr(Address16(sizeB / WORD_SIZE_IN_BYTES, 0));

						signal->setTuningAbsAddr(Address16(m_tuningDataOffsetW + sizeB / WORD_SIZE_IN_BYTES, 0));

						sizeB += sizeof(qint32);
						testSizeB = true;

						metaData.append(QVariant(QString("AnalogInt")));
						metaData.append(QVariant(defaultValue));
						metaData.append(QVariant(lowBound));
						metaData.append(QVariant(highBound));
					}
					break;

				case TYPE_DISCRETE:
					{
						quint16 defaultValue = static_cast<quint16>(signal->tuningDefaultValue().discreteValue());

						int bitNo = discreteCount % SIZE_32BIT;

						writeBigEndianUint32Bit(dataPtr, bitNo, defaultValue);
						writeBigEndianUint32Bit(dataPtr + tuningFramePayloadBytes, bitNo, 0);
						writeBigEndianUint32Bit(dataPtr + tuningFramePayloadBytes * 2, bitNo, 1);

						signal->setTuningAddr(Address16(sizeB / WORD_SIZE_IN_BYTES, bitNo));

						// tunable discrete signals pack in 32-bit container that place in LM memory in BigEndian format
						// but access to this discretes is performeds as word-addressed
						// so,
						//   if discrete bitNo < 16 we add 1 to signal offset (in words)
						//   if discrete bitNo >= 16 we set bit no for discrete is bitNo % 16
						//
						int additionalOffsetToDiscreteW = 0;

						if (bitNo < 16)
						{
							additionalOffsetToDiscreteW = 1;
						}
						else
						{
							additionalOffsetToDiscreteW = 0;

							bitNo = bitNo % SIZE_16BIT;
						}

						signal->setTuningAbsAddr(Address16(m_tuningDataOffsetW + sizeB / WORD_SIZE_IN_BYTES + additionalOffsetToDiscreteW, bitNo));

						//

						discreteCount++;

						if ((discreteCount % SIZE_32BIT) == 0)
						{
							sizeB += sizeof(quint32);
							testSizeB = true;
						}

						metaData.append(QVariant(QString("Discrete")));
						metaData.append(QVariant(defaultValue));
						metaData.append(QVariant(0));
						metaData.append(QVariant(1));
					}
					break;

				default:
					assert(false);
				}

				if (testSizeB == true && (sizeB % tuningFramePayloadBytes) == 0)
				{
					// frame full
					// skip lowBound and highBound frames
					//
					sizeB += tuningFramePayloadBytes * 2;
				}

				metaData.append(QVariant(signal->tuningAbsAddr().offset()));
				metaData.append(QVariant(signal->tuningAbsAddr().bit()));

				m_metadata.push_back(metaData);
			}
		}

		assert(sizeB <= m_tuningDataSizeB);
	}

	void TuningData::getTuningData(QByteArray* tuningData) const
	{
		if (tuningData == nullptr)
		{
			assert(false);
			return;
		}

		tuningData->clear();

		if (m_tuningData == nullptr)
		{
			assert(false);
			return;
		}

		tuningData->append(reinterpret_cast<const char*>(m_tuningData), m_tuningDataSizeB);
	}


	quint64 TuningData::generateUniqueID(const QString& lmEquipmentID)
	{
		Crc64 crc;

		crc.add(lmEquipmentID);

		for(QVector<Signal*>& signalList : m_tuningSignals)
		{
			for(Signal* signal : signalList)
			{
				crc.add(signal->appSignalID());
				crc.add(signal->equipmentID());
				crc.add(signal->tuningDefaultValue().toDouble());		// real tuning value type does not matter
				crc.add(signal->tuningLowBound().toDouble());
				crc.add(signal->tuningHighBound().toDouble());
			}
		}

		m_uniqueID = crc.result();

		return m_uniqueID;
	}


	void TuningData::getSignals(QVector<Signal*>& signalList) const
	{
		signalList.clear();

		for(const QVector<Signal*>& list : m_tuningSignals)
		{
			signalList.append(list);
		}
	}


	const QVector<Signal *>& TuningData::getSignals(int type) const
	{
		if (type < TYPE_ANALOG_FLOAT || type > TYPE_DISCRETE)
		{
			assert(false);
			type = TYPE_ANALOG_FLOAT;
		}

		return m_tuningSignals[type];
	}

	void TuningData::getAcquiredAnalogSignals(QVector<Signal *>& analogSignals)
	{
		for(Signal* s : m_tuningSignals[TYPE_ANALOG_FLOAT])
		{
			if (s->isAcquired() == true)
			{
				analogSignals.append(s);
			}
		}

		for(Signal* s : m_tuningSignals[TYPE_ANALOG_INT32])
		{
			if (s->isAcquired() == true)
			{
				analogSignals.append(s);
			}
		}
	}

	void TuningData::getAcquiredDiscreteSignals(QVector<Signal*>& discreteSignals)
	{
		for(Signal* s : m_tuningSignals[TYPE_DISCRETE])
		{
			if (s->isAcquired() == true)
			{
				discreteSignals.append(s);
			}
		}
	}

	int TuningData::getSignalsCount() const
	{
		int count = 0;

		for(int i = TYPE_ANALOG_FLOAT; i < TYPES_COUNT; i++)
		{
			count += m_tuningSignals[i].count();
		}

		return count;
	}

	void TuningData::getMetadataFields(QStringList& metadataFields, int* metadataVersion) const
	{
		if (metadataVersion)
		{
			const int TUNING_METADATA_VERSION = 1;
			*metadataVersion = TUNING_METADATA_VERSION;
		}

		if (m_metadataFields.isEmpty() == true)
		{
			m_metadataFields.append("AppSignalID");
			m_metadataFields.append("CustomSignalID");
			m_metadataFields.append("Caption");
			m_metadataFields.append("Type");
			m_metadataFields.append("Default");
			m_metadataFields.append("Min");
			m_metadataFields.append("Max");
			m_metadataFields.append("Offset");
			m_metadataFields.append("BitNo");
		}

		metadataFields = m_metadataFields;
	}


	const std::vector<QVariantList>& TuningData::metadata() const
	{
		return m_metadata;
	}


	void TuningData::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement(TUNING_FLASH);			// <TuningFlashMemory>

		xml.writeIntAttribute(TUNING_FLASH_FRAME_COUNT, m_tuningFlashFrameCount);
		xml.writeIntAttribute(TUNING_FLASH_FRAME_PAYLOAD_B, m_tuningFlashFramePayloadB);
		xml.writeIntAttribute(TUNING_FLASH_FRAME_SIZE_B, m_tuningFlashFrameSizeB);

		xml.writeEndElement();							// </TuningFlashMemory>

		xml.writeStartElement(TUNING_DATA);				// <TuningDataMemory>

		xml.writeIntAttribute(TUNING_DATA_OFFSET_W, m_tuningDataOffsetW);
		xml.writeIntAttribute(TUNING_DATA_SIZE_W, m_tuningDataSizeW);
		xml.writeIntAttribute(TUNING_DATA_FRAME_COUNT, m_tuningDataFrameCount);
		xml.writeIntAttribute(TUNING_DATA_FRAME_PAYLOAD_W, m_tuningDataFramePayloadW);
		xml.writeIntAttribute(TUNING_DATA_FRAME_SIZE_W, m_tuningDataFrameSizeW);
		xml.writeIntAttribute(TUNING_DATA_USED_FRAMES_COUNT, m_tuningDataUsedFramesCount);

		xml.writeEndElement();							// </TuningDataMemory>

		xml.writeStartElement(TUNING_DATA_ELEMENT);		// <TuningData>

		xml.writeStringAttribute(LM_ID, m_lmEquipmentID);
		xml.writeUInt64Attribute(UNIQUE_ID, m_uniqueID, true);

		int signalCount = 0;

		for(QVector<Signal*>& signalList : m_tuningSignals)
		{
			signalCount += signalList.count();
		}

		xml.writeIntAttribute(TUNING_ALL_SIGNALS_COUNT, signalCount);

		const char* typeSection[TYPES_COUNT] =
		{
			TUNING_ANALOG_FLOAT_SIGNALS,
			TUNING_ANALOG_INT32_SIGNALS,
			TUNING_DISCRETE_SIGNALS
		};

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			QVector<Signal*>& tuningSignals = m_tuningSignals[type];

			xml.writeStartElement(typeSection[type]);	//	<typeSection[type]>

			xml.writeIntAttribute(TUNING_SIGNALS_COUNT, tuningSignals.count());

			for(Signal* signal : tuningSignals)
			{
				signal->writeToXml(xml);
			}

			xml.writeEndElement();						//	</typeSection[type]>
		}

		xml.writeEndElement();							//	</ <TuningData>
	}


	bool TuningData::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		if (xml.findElement(TUNING_FLASH) == false)
		{
			return false;
		}

		result &= xml.readIntAttribute(TUNING_FLASH_FRAME_COUNT, &m_tuningFlashFrameCount);
		result &= xml.readIntAttribute(TUNING_FLASH_FRAME_PAYLOAD_B, &m_tuningFlashFramePayloadB);
		result &= xml.readIntAttribute(TUNING_FLASH_FRAME_SIZE_B, &m_tuningFlashFrameSizeB);

		if (result == false)
		{
			return false;
		}

		if (xml.findElement(TUNING_DATA) == false)
		{
			return false;
		}

		result &= xml.readIntAttribute(TUNING_DATA_OFFSET_W, &m_tuningDataOffsetW);
		result &= xml.readIntAttribute(TUNING_DATA_SIZE_W, &m_tuningDataSizeW);
		result &= xml.readIntAttribute(TUNING_DATA_FRAME_COUNT, &m_tuningDataFrameCount);
		result &= xml.readIntAttribute(TUNING_DATA_FRAME_PAYLOAD_W, &m_tuningDataFramePayloadW);
		result &= xml.readIntAttribute(TUNING_DATA_FRAME_SIZE_W, &m_tuningDataFrameSizeW);
		result &= xml.readIntAttribute(TUNING_DATA_USED_FRAMES_COUNT, &m_tuningDataUsedFramesCount);

		if (result == false)
		{
			return false;
		}

		if (xml.findElement(TUNING_DATA_ELEMENT) == false)
		{
			return false;
		}

		result &= xml.readStringAttribute(LM_ID, &m_lmEquipmentID);
		result &= xml.readUInt64Attribute(UNIQUE_ID, &m_uniqueID);

		int totalSignalsCount = 0;

		result &= xml.readIntAttribute(TUNING_ALL_SIGNALS_COUNT, &totalSignalsCount);

		m_deleteSignals = true;		// !

		const char* typeSection[TYPES_COUNT] =
		{
			TUNING_ANALOG_FLOAT_SIGNALS,
			TUNING_ANALOG_INT32_SIGNALS,
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

				Signal* signal = new Signal();

				result &= signal->readFromXml(xml);

				m_tuningSignals[type].append(signal);
			}
		}

		return result;
	}


	void TuningData::writeBigEndianUint32Bit(quint8* dataPtr, int bitNo, quint32 bitValue)
	{
		if (dataPtr == nullptr)
		{
			assert(false);
			return;
		}

		assert(bitNo >= 0 && bitNo < 32);

		quint32* data32Ptr = reinterpret_cast<quint32*>(dataPtr);

		// read dword and convert from BigEndian to LittleEndian
		//
		quint32 value = reverseUint32(*data32Ptr);

		quint32 bitMask = 1 << bitNo;

		if (bitValue == 0)
		{
			value &= ~bitMask;
		}
		else
		{
			value |= bitMask;
		}

		// convert value from LittleEndian to BigEndian and write dword
		//
		*data32Ptr = reverseUint32(value);
	}

	void TuningData::sortSignalsByAcquiredProperty(QVector<Signal *>& tuningSignals)
	{
		int count = tuningSignals.count();

		QVector<Signal*> acquired;
		QVector<Signal*> nonAcquired;

		acquired.reserve(count);
		nonAcquired.reserve(count);

		for(Signal* s : tuningSignals)
		{
			TEST_PTR_CONTINUE(s);

			if (s->acquire() == true)
			{
				acquired.append(s);
			}
			else
			{
				nonAcquired.append(s);
			}
		}

		sortByAppSignalID(acquired);
		sortByAppSignalID(nonAcquired);

		tuningSignals.clear();

		tuningSignals.append(acquired);
		tuningSignals.append(nonAcquired);
	}

	void TuningData::sortByAppSignalID(QVector<Signal*>& signalList)
	{
		int count = signalList.size();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				Signal* s1 = signalList[i];
				Signal* s2 = signalList[k];

				TEST_PTR_CONTINUE(s1);
				TEST_PTR_CONTINUE(s2);

				if (s1->appSignalID() > s2->appSignalID())
				{
					signalList[i] = s2;
					signalList[k] = s1;
				}
			}
		}
	}

	int TuningData::signalValueSizeBits(int type)
	{
		switch(type)
		{
		case TYPE_ANALOG_FLOAT:
			return sizeof(float) * SIZE_8BIT;

		case TYPE_ANALOG_INT32:
			return sizeof(qint32) * SIZE_8BIT;

		case TYPE_DISCRETE:
			return 1;

		default:
			assert(false);
		}

		return 0;
	}


	int TuningData::getSignalType(const Signal* signal)
	{
		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
		{
			return TYPE_ANALOG_FLOAT;
		}

		if (signal->isAnalog() && signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
		{
			return TYPE_ANALOG_INT32;
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

}
