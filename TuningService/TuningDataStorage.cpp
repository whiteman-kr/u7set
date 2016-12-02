#include <QtEndian>
#include "../lib/Crc.h"
#include "../lib/WUtils.h"
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
	const char* TuningData::TUNING_FRAME_SIZE_BYTES = "FrameSizeBytes";
	const char* TuningData::TUNING_FRAMES_COUNT = "FramesCount";
	const char* TuningData::TUNING_USED_FRAMES_COUNT = "UsedFramesCount";
	const char* TuningData::TUNING_ALL_SIGNALS_COUNT = "TuningSignalsCount";
	const char* TuningData::TUNING_ANALOG_FLOAT_SIGNALS = "AnalogFloatSignals";
	const char* TuningData::TUNING_ANALOG_INT_SIGNALS = "AnalogIntSignals";
	const char* TuningData::TUNING_DISCRETE_SIGNALS = "DiscreteSignals";
	const char* TuningData::TUNING_SIGNALS_COUNT = "Count";

	QStringList TuningData::m_metadataFields;

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

		// calculate tuning signals sizes
		//
		int t = TYPE_ANALOG_FLOAT;

		int totalSize = 0;

		for(QList<Signal*>& signalList : m_tuningSignals)
		{
			int signalCount = signalList.size();

			switch(t)
			{
			case TYPE_ANALOG_FLOAT:
				m_tuningSignalSizes[t] = signalCount * sizeof(float);
				break;

			case TYPE_ANALOG_INT:
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
		m_usedFramesCount = (totalSize / m_tuningFrameSizeBytes + ((totalSize % m_tuningFrameSizeBytes) == 0 ? 0 : 1)) * TRIPLE_FRAMES;

		return result;
	}


	bool TuningData::buildTuningData()
	{
		// allocate m_tuningData
		//

		m_tuningDataSize = m_usedFramesCount * m_tuningFrameSizeBytes;

		m_tuningData = new quint8 [m_tuningDataSize];

		memset(m_tuningData, 0, m_tuningDataSize);

		// copy tuning signals default values and ranges
		//

		int sizeB = 0;

		for(int t = TYPE_ANALOG_FLOAT; t < TYPES_COUNT; t++)
		{
			for(Signal* signal : m_tuningSignals[t])
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				quint8* dataPtr = m_tuningData + sizeB;

				switch(t)
				{
				case TYPE_ANALOG_FLOAT:
					{
						// in first frame - default value
						//
						*reinterpret_cast<float*>(dataPtr) =
							reverseFloat(static_cast<float>(signal->tuningDefaultValue()));

						// in second frame - low bound
						//
						*reinterpret_cast<float*>(dataPtr + m_tuningFrameSizeBytes) =
							reverseFloat(static_cast<float>(signal->lowEngeneeringUnits()));

						// in third frame - high bound
						//
						*reinterpret_cast<float*>(dataPtr + m_tuningFrameSizeBytes * 2) =
							reverseFloat(static_cast<float>(signal->highEngeneeringUnits()));

						sizeB += sizeof(float);
					}
					break;

				case TYPE_ANALOG_INT:
					{
						// in first frame - default value
						//
						*reinterpret_cast<qint32*>(dataPtr) =
							reverseFloat(static_cast<qint32>(signal->tuningDefaultValue()));

						// in second frame - low bound
						//
						*reinterpret_cast<qint32*>(dataPtr + m_tuningFrameSizeBytes) =
							reverseFloat(static_cast<qint32>(signal->lowEngeneeringUnits()));

						// in third frame - high bound
						//
						*reinterpret_cast<qint32*>(dataPtr + m_tuningFrameSizeBytes * 2) =
							reverseFloat(static_cast<qint32>(signal->highEngeneeringUnits()));

						sizeB += sizeof(qint32);
					}
					break;

				case TYPE_DISCRETE:
					break;

				default:
					assert(false);
				}

				if ((sizeB % m_tuningFrameSizeBytes) == 0)
				{
					// frame full
					// skip lowBound and highBound frames
					//
					sizeB += m_tuningFrameSizeBytes * 2;
				}
			}
		}


/*		m_usedFramesCount = 0;

		m_metadata.clear();

		for(int type = TYPE_ANALOG_FLOAT; type < TYPES_COUNT; type++)
		{
			TuningFramesData& framesData = m_tuningFramesData[type];
			QList<Signal*>& tuningSignals = m_tuningSignals[type];

			framesData.init(m_usedFramesCount, m_tuningFrameSizeBytes, signalValueSizeBits(type), tuningSignals.count());
			framesData.copySignalsData(tuningSignals, m_metadata);


			m_usedFramesCount += framesData.usedFramesCount();
		}*/

		return true;
	}


	bool TuningData::initTuningData()
	{
/*		int firstFrame = 0;

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
		}*/

		return true;
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

		tuningData->append(reinterpret_cast<const char*>(m_tuningData), m_tuningDataSize);
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
				crc.add(signal->lowEngeneeringUnits());
				crc.add(signal->highEngeneeringUnits());
			}
		}

		m_uniqueID = crc.result();

		return m_uniqueID;
	}


	void TuningData::getSignals(QList<Signal*>& signalList)
	{
		signalList.clear();

		for(QList<Signal*>& list : m_tuningSignals)
		{
			signalList.append(list);
		}
	}


	const QStringList& TuningData::metadataFields()
	{
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

		return m_metadataFields;
	}


	const std::vector<QVariantList>& TuningData::metadata() const
	{
		return m_metadata;
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

				Signal* signal = new Signal();

				result &= signal->readFromXml(xml);

				m_tuningSignals[type].append(signal);
			}
		}

		return result;
	}


	int TuningData::signalValueSizeBits(int type)
	{
		switch(type)
		{
		case TYPE_ANALOG_FLOAT:
			return sizeof(float) * SIZE_8BIT;

		case TYPE_ANALOG_INT:
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

}
