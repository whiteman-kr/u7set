#include <QtEndian>
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/Crc.h"
#include "../UtilsLib/WUtils.h"
#include "AppSignal.h"
#include "TuningDataStorage.h"


namespace  Tuning
{
	// -------------------------------------------------------------------------------------
	//
	// TuningData class implementation
	//
	// -------------------------------------------------------------------------------------

	QStringList TuningData::m_metadataFields;

	TuningData::TuningData() :
		TYPES_COUNT(E::metaEnum<E::TuningSignalType>().keyCount())
	{
		m_tuningSignals.resize(TYPES_COUNT);
		m_tuningSignalSizesB.fill(0, TYPES_COUNT);
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
		TYPES_COUNT(E::metaEnum<E::TuningSignalType>().keyCount()),
		m_tuningFlashFrameCount(tuningFlashFrameCount),
		m_tuningFlashFramePayloadB(tuningFlashFramePayloadB),
		m_tuningFlashFrameSizeB(tuningFlashFrameSizeB),
		m_tuningDataOffsetW(tuningDataOffsetW),
		m_tuningDataSizeW(tuningDataSizeW),
		m_tuningDataFrameCount(tuningDataFrameCount),
		m_tuningDataFramePayloadW(tuningDataFramePayloadW),
		m_tuningDataFrameSizeW(tuningDataFrameSizeW)
	{
		m_tuningSignals.resize(TYPES_COUNT);
		m_tuningSignalSizesB.fill(0, TYPES_COUNT);
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
		for(int& v : m_tuningSignalSizesB)
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
			for(AppSignal* signal : m_tuningSignals[type])
			{
				delete signal;
			}

			m_tuningSignals[type].clear();
		}
	}

	void TuningData::clearSignalLists()
	{
		for(QVector<AppSignal*>& signalList : m_tuningSignals)
		{
			signalList.clear();
		}
	}

	void TuningData::appendTuningSignal(E::TuningSignalType tunSignalType, AppSignal* appSignal)
	{
		TEST_PTR_RETURN(appSignal);

		m_tuningSignals[static_cast<int>(tunSignalType)].append(appSignal);
	}

	void TuningData::buildTuningData()
	{
		// calculate tuning signals sizes
		//
		int totalSizeB = 0;

		for(int t = TYPE_ANALOG_FLOAT; t < TYPES_COUNT; t++)
		{
			QVector<AppSignal*>& signalList = m_tuningSignals[t];

			sortSignalsByAcquiredProperty(signalList);

			int signalCount = static_cast<int>(signalList.size());

			switch(t)
			{
			case TYPE_ANALOG_FLOAT:
				m_tuningSignalSizesB[t] = signalCount * sizeof(float);
				break;

			case TYPE_ANALOG_INT32:
				m_tuningSignalSizesB[t] = signalCount * sizeof(qint32);
				break;

			case TYPE_DISCRETE:
				m_tuningSignalSizesB[t] = signalCount / SIZE_8BIT + ((signalCount % SIZE_8BIT) == 0 ? 0 : 1) ;
				break;

			default:
				assert(false);
			}

			totalSizeB += m_tuningSignalSizesB[t];
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
			m_tuningDataUsedFramesCount = (totalSizeB / tuningFramePayloadBytes + ((totalSizeB % tuningFramePayloadBytes) == 0 ? 0 : 1)) * TRIPLE_FRAMES;
		}

		// allocate m_tuningData
		//
		m_metadata.clear();

		m_tuningDataSizeB = m_tuningDataUsedFramesCount * tuningFramePayloadBytes;

		m_tuningData = new quint8 [m_tuningDataSizeB];

		memset(m_tuningData, 0, m_tuningDataSizeB);

		// copy tuning signals default values and ranges
		//

		int sizeB = 0;

		for(int t = TYPE_ANALOG_FLOAT; t < TYPES_COUNT; t++)
		{
			int discreteCount = 0;

			for(AppSignal* signal : m_tuningSignals[t])
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

	void TuningData::calcTuningDataUID(const QString& lmEquipmentID)
	{
		Crc64 crc;

		crc.add(lmEquipmentID);

		for(QVector<AppSignal*>& signalList : m_tuningSignals)
		{
			for(AppSignal* signal : signalList)
			{
				crc.add(signal->appSignalID());
				crc.add(signal->equipmentID());
				crc.add(signal->tuningDefaultValue().toDouble());		// real tuning value type does not matter
				crc.add(signal->tuningLowBound().toDouble());
				crc.add(signal->tuningHighBound().toDouble());
			}
		}

		m_rupTuningDataUID = crc.result32();
		m_fotipTuningDataUID = crc.result();
	}

	int TuningData::usedTuningDataSizeW() const
	{
		int sizeW = 0;

		if (m_tuningSignalSizesB.size() != TYPES_COUNT)
		{
			Q_ASSERT(false);
			return 0;
		}

		for(int t = TYPE_ANALOG_FLOAT; t < TYPES_COUNT; t++)
		{
			switch(t)
			{
			case TYPE_ANALOG_FLOAT:
				Q_ASSERT((m_tuningSignalSizesB[t] % sizeof(float)) == 0);
				sizeW += m_tuningSignalSizesB[t] / sizeof(quint16);	// it's Ok! size returned in words!
				break;

			case TYPE_ANALOG_INT32:
				Q_ASSERT((m_tuningSignalSizesB[t] % sizeof(qint32)) == 0);
				sizeW += m_tuningSignalSizesB[t] / sizeof(quint16);	// it's Ok! size returned in words!
				break;

			case TYPE_DISCRETE:
				{
					int sizeB = m_tuningSignalSizesB[t];

					sizeW += sizeB / sizeof(quint16);

					if ((sizeB % sizeof(quint32)) != 0)		// it's Ok! size for discretes reserved by
															// 32-bit
					{
						sizeW += 2;		// sizeof(quint32) / sizeof(quint16)!
					}
				}
				break;

			default:
				Q_ASSERT(false);
			}
		}

		return sizeW;
	}

	void TuningData::getSignals(QVector<AppSignal*>* signalList) const
	{
		TEST_PTR_RETURN(signalList);

		signalList->clear();

		for(const QVector<AppSignal*>& list : m_tuningSignals)
		{
			signalList->append(list);
		}
	}

	const QVector<AppSignal *>& TuningData::getSignals(int type) const
	{
		if (type < TYPE_ANALOG_FLOAT || type > TYPE_DISCRETE)
		{
			assert(false);
			type = TYPE_ANALOG_FLOAT;
		}

		return m_tuningSignals[type];
	}

	void TuningData::getAcquiredAnalogSignals(QVector<AppSignal *>& analogSignals)
	{
		for(AppSignal* s : m_tuningSignals[TYPE_ANALOG_FLOAT])
		{
			if (s->isAcquired() == true)
			{
				analogSignals.append(s);
			}
		}

		for(AppSignal* s : m_tuningSignals[TYPE_ANALOG_INT32])
		{
			if (s->isAcquired() == true)
			{
				analogSignals.append(s);
			}
		}
	}

	void TuningData::getAcquiredDiscreteSignals(QVector<AppSignal*>& discreteSignals)
	{
		for(AppSignal* s : m_tuningSignals[TYPE_DISCRETE])
		{
			if (s->isAcquired() == true)
			{
				discreteSignals.append(s);
			}
		}
	}

	int TuningData::getSignalsCount() const
	{
		qsizetype count = 0;

		for(int i = TYPE_ANALOG_FLOAT; i < TYPES_COUNT; i++)
		{
			count += m_tuningSignals[i].count();
		}

		return static_cast<int>(count);
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
		xml.writeStartElement(XmlElement::TUNING_FLASH_MEMORY);			// <TuningFlashMemory>

		xml.writeInt32Attribute(XmlAttribute::FRAME_COUNT, m_tuningFlashFrameCount);
		xml.writeInt32Attribute(XmlAttribute::FRAME_PAYLOAD_B, m_tuningFlashFramePayloadB);
		xml.writeInt32Attribute(XmlAttribute::FRAME_SIZE_B, m_tuningFlashFrameSizeB);

		xml.writeEndElement();							// </TuningFlashMemory>

		xml.writeStartElement(XmlElement::TUNING_DATA_MEMORY);				// <TuningDataMemory>

		xml.writeInt32Attribute(XmlAttribute::DATA_OFFSET_W, m_tuningDataOffsetW);
		xml.writeInt32Attribute(XmlAttribute::DATA_SIZE_W, m_tuningDataSizeW);
		xml.writeInt32Attribute(XmlAttribute::FRAME_COUNT, m_tuningDataFrameCount);
		xml.writeInt32Attribute(XmlAttribute::FRAME_PAYLOAD_W, m_tuningDataFramePayloadW);
		xml.writeInt32Attribute(XmlAttribute::FRAME_SIZE_W, m_tuningDataFrameSizeW);
		xml.writeInt32Attribute(XmlAttribute::USED_FRAMES_COUNT, m_tuningDataUsedFramesCount);

		xml.writeEndElement();							// </TuningDataMemory>

		xml.writeStartElement(XmlElement::TUNING_DATA);		// <TuningData>

		xml.writeStringAttribute(XmlAttribute::LM_EQUIPMENT_ID, m_lmEquipmentID);
		xml.writeUInt64Attribute(XmlAttribute::FOTIP_TUNING_DATA_UID, m_fotipTuningDataUID, true);

		int signalCount = 0;

		for(QVector<AppSignal*>& signalList : m_tuningSignals)
		{
			signalCount += static_cast<int>(signalList.count());
		}

		xml.writeInt32Attribute(XmlAttribute::SIGNALS_COUNT, signalCount);

		Q_ASSERT(m_signalTypeXmlElement.size() == TYPES_COUNT);

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			QVector<AppSignal*>& tuningSignals = m_tuningSignals[type];

			xml.writeStartElement(m_signalTypeXmlElement[type]);	//	<typeSection[type]>

			xml.writeInt32Attribute(XmlAttribute::COUNT, static_cast<int>(tuningSignals.count()));

			for(AppSignal* signal : tuningSignals)
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

		if (xml.findElement(XmlElement::TUNING_FLASH_MEMORY) == false)
		{
			return false;
		}

		result &= xml.readIntAttribute(XmlAttribute::FRAME_COUNT, &m_tuningFlashFrameCount);
		result &= xml.readIntAttribute(XmlAttribute::FRAME_PAYLOAD_B, &m_tuningFlashFramePayloadB);
		result &= xml.readIntAttribute(XmlAttribute::FRAME_SIZE_B, &m_tuningFlashFrameSizeB);

		if (result == false)
		{
			return false;
		}

		if (xml.findElement(XmlElement::TUNING_DATA_MEMORY) == false)
		{
			return false;
		}

		result &= xml.readIntAttribute(XmlAttribute::DATA_OFFSET_W, &m_tuningDataOffsetW);
		result &= xml.readIntAttribute(XmlAttribute::DATA_SIZE_W, &m_tuningDataSizeW);
		result &= xml.readIntAttribute(XmlAttribute::FRAME_COUNT, &m_tuningDataFrameCount);
		result &= xml.readIntAttribute(XmlAttribute::FRAME_PAYLOAD_W, &m_tuningDataFramePayloadW);
		result &= xml.readIntAttribute(XmlAttribute::FRAME_SIZE_W, &m_tuningDataFrameSizeW);
		result &= xml.readIntAttribute(XmlAttribute::USED_FRAMES_COUNT, &m_tuningDataUsedFramesCount);

		if (result == false)
		{
			return false;
		}

		if (xml.findElement(XmlElement::TUNING_DATA) == false)
		{
			return false;
		}

		result &= xml.readStringAttribute(XmlAttribute::LM_EQUIPMENT_ID, &m_lmEquipmentID);
		result &= xml.readUInt64Attribute(XmlAttribute::FOTIP_TUNING_DATA_UID, &m_fotipTuningDataUID);

		int totalSignalsCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::SIGNALS_COUNT, &totalSignalsCount);

		m_deleteSignals = true;		// !

		Q_ASSERT(m_signalTypeXmlElement.size() == TYPES_COUNT);

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			if (xml.findElement(m_signalTypeXmlElement[type]) == false)
			{
				return false;
			}

			int count = 0;

			result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

			for(int i = 0; i < count; i++)
			{
				if (xml.findElement("Signal") == false)
				{
					result = false;
					break;
				}

				AppSignal* signal = new AppSignal();

				result &= signal->readFromXml(xml);

				m_tuningSignals[type].append(signal);
			}
		}

		return result;
	}

	void TuningData::getTuningSignalsFramesInfo(std::vector<std::pair<quint32, quint32>>* framesInfo) const
	{
		// pair - <frameAbsStartAddrW, frameUsedSizeW>
		//
		TEST_PTR_RETURN(framesInfo);

		framesInfo->clear();

		int totalSizeB = 0;

		Q_ASSERT((m_tuningSignalSizesB[TYPE_ANALOG_FLOAT] % sizeof(float)) == 0);

		totalSizeB += m_tuningSignalSizesB[TYPE_ANALOG_FLOAT];

		Q_ASSERT((m_tuningSignalSizesB[TYPE_ANALOG_INT32] % sizeof(qint32)) == 0);

		totalSizeB += m_tuningSignalSizesB[TYPE_ANALOG_INT32];

		// discrete tuning signals allocated by 32 bits
		//
		totalSizeB += m_tuningSignalSizesB[TYPE_DISCRETE] +
						(sizeof(quint32) - (m_tuningSignalSizesB[TYPE_DISCRETE] % sizeof(quint32)));

		Q_ASSERT(m_tuningFlashFramePayloadB == m_tuningDataFramePayloadW * sizeof(quint16));

		quint32 frameStartAddrW = m_tuningDataOffsetW;

		while(totalSizeB > 0)
		{
			Q_ASSERT((totalSizeB % sizeof(quint32)) == 0);

			int frameUsedSizeW = 0;

			if (totalSizeB >= m_tuningFlashFramePayloadB)
			{
				frameUsedSizeW = m_tuningFlashFramePayloadB / sizeof(quint16);		// bytes => words
				totalSizeB -= m_tuningFlashFramePayloadB;
			}
			else
			{
				frameUsedSizeW = totalSizeB / sizeof(quint16);		// bytes => words
				totalSizeB = 0;
			}

			framesInfo->push_back({frameStartAddrW, frameUsedSizeW});

			frameStartAddrW += m_tuningDataFramePayloadW * 3;	// skip 3 frames:
																//		1) tuning signal values
																//		2) tuning signal low bound
																//		3) tuning signal high bound
		}
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

	void TuningData::sortSignalsByAcquiredProperty(QVector<AppSignal*>& tuningSignals)
	{
		qsizetype count = tuningSignals.count();

		QVector<AppSignal*> acquired;
		QVector<AppSignal*> nonAcquired;

		acquired.reserve(count);
		nonAcquired.reserve(count);

		for(AppSignal* s : tuningSignals)
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

	void TuningData::sortByAppSignalID(QVector<AppSignal*>& signalList)
	{
		qsizetype count = signalList.size();

		for(qsizetype i = 0; i < count - 1; i++)
		{
			for(qsizetype k = i + 1; k < count; k++)
			{
				AppSignal* s1 = signalList[i];
				AppSignal* s2 = signalList[k];

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


	int TuningData::getSignalType(const AppSignal* signal)
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

	bool TuningDataStorage::appendTuningData(const QString& lmEquipmentID, TuningDataShared tuningData)
	{
		TEST_PTR_RETURN_FALSE(tuningData);

		if (m_tuningDataMap.contains(lmEquipmentID) == true)
		{
			Q_ASSERT(false);
			return false;
		}

		m_tuningDataMap.insert({lmEquipmentID, tuningData});

		return true;
	}

	TuningDataShared TuningDataStorage::getTuningData(const QString& lmEquipmentID)
	{
		auto it = m_tuningDataMap.find(lmEquipmentID);

		if (it == m_tuningDataMap.end())
		{
			return nullptr;
		}

		return it->second;
	}

}
