#include "TuningIPENSource.h"


namespace TuningIPEN
{

	// -------------------------------------------------------------------------------
	//
	// TuningDataSource class implementation
	//
	// -------------------------------------------------------------------------------

	TuningSource::TuningSource()
	{
		setLmDataType(DataSource::DataType::Tuning);

		m_fotipFlags.all = 0;
	}


	TuningSource::~TuningSource()
	{
		if (m_deleteTuningData == true)
		{
			delete m_tuningData;
		}
	}


	void TuningSource::setTuningData(TuningData* tuningData)
	{
		if (tuningData == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningData = tuningData;
	}


	void TuningSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml) const
	{
		if (m_tuningData == nullptr)
		{
			TuningData td(lmEquipmentID());
			td.writeToXml(xml);
			return;
		}

		m_tuningData->writeToXml(xml);
	}


	bool TuningSource::readAdditionalSectionsFromXml(XmlReadHelper& xml)
	{
		assert(m_tuningData == nullptr);

		m_tuningData = new TuningData();

		m_deleteTuningData = true;

		m_tuningData->readFromXml(xml);
		m_tuningData->initTuningData();

		return true;
	}


	void TuningSource::getTuningDataSourceInfo(TuningSourceInfo& info)
	{
		info.channel = 0;	//lmChannel();
		info.dataType = lmDataType();
		info.lmEquipmentID = lmEquipmentID();
		info.lmCaption = lmCaption();
		info.lmAdapterID = lmAdapterID();
		info.lmDataEnable = lmDataEnable();
		info.lmAddressPort = lmAddressPort();
		info.lmDataID = lmDataID();

		info.tuningSignals.clear();

		if (m_tuningData == nullptr)
		{
			return;
		}

		QVector<Signal*> signalList;

		m_tuningData->getSignals(signalList);

		info.tuningSignals.resize(signalList.count());

		int index = 0;

		for(Signal* signal : signalList)
		{
			if (index >= info.tuningSignals.count())
			{
				assert(false);
				break;
			}

			info.tuningSignals[index] = *signal;

			index++;
		}
	}


	int TuningSource::nextFrameToRequest()
	{
		if (m_tuningData != nullptr)
		{
			m_frameToRequest++;

			if (m_frameToRequest >= m_tuningData->usedFramesCount())
			{
				m_frameToRequest = 0;
			}
		}

		return m_frameToRequest;
	}


	// -------------------------------------------------------------------------------
	//
	// TuningDataSources class implementation
	//
	// -------------------------------------------------------------------------------

	TuningSources::~TuningSources()
	{
		clear();
	}


	void TuningSources::clear()
	{
		for(TuningSource* ds : *this)
		{
			delete ds;
		}

		QHash<QString, TuningSource*>::clear();
		m_ip2Source.clear();
	}


	void TuningSources::getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info)
	{
		info.clear();

		info.resize(count());

		int index = 0;

		for(TuningSource* source : (*this))
		{
			if (source == nullptr)
			{
				assert(false);
				continue;
			}

			if (index >= info.count())
			{
				assert(false);
				break;
			}

			source->getTuningDataSourceInfo(info[index]);

			index++;
		}
	}


	void TuningSources::buildIP2DataSourceMap()
	{
		for(TuningSource* source : *this)
		{
			m_ip2Source.insert(source->lmAddress32(), source);
		}
	}


	TuningSource* TuningSources::getDataSourceByIP(quint32 ip)
	{
		if (m_ip2Source.contains(ip))
		{
			return m_ip2Source[ip];
		}

		return nullptr;
	}


	void TuningSource::processReply(const SocketReply& reply)
	{
		m_receivedRepyCount++;

		m_hasConnection = true;

		m_fotipFlags = reply.fotipHeader.flags;

		m_lastReplyTime = QDateTime::currentMSecsSinceEpoch();

		if (m_tuningData == nullptr)
		{
			assert(false);
			return;
		}

		m_fotipFlags = reply.fotipHeader.flags;

		if (reply.fotipHeader.flags.all != 0)
		{
			return;
		}

		m_tuningData->setFrameData(reply.frameNo, reply.fotipData);
	}


	void TuningSource::testConnection(qint64 nowTime)
	{
		if (nowTime - m_lastReplyTime > 2000)		// connection timeout == 2 seconds
		{
			m_hasConnection = false;
		}
	}


	bool TuningSource::getSignalState(const QString& appSignalID, TuningSignalState* tss)
	{
		if (tss == nullptr)
		{
			assert(false);
			return false;
		}

		if (m_hasConnection == false)
		{
			tss->valid = false;
			return true;
		}


		if (m_tuningData == nullptr)
		{
			return false;
		}

		return m_tuningData->getSignalState(appSignalID, tss);
	}


	bool TuningSource::setSignalState(const QString& appSignalID, double value, SocketRequest* sr)
	{
		if (m_tuningData == nullptr)
		{
			return false;
		}

		return m_tuningData->setSignalState(appSignalID, value, sr);
	}


	quint64 TuningSource::uniqueID()
	{
		if (m_tuningData == nullptr)
		{
			assert(false);
			return 0;
		}

		return m_tuningData->uniqueID();
	}


	TuningSourceState TuningSource::getState()
	{
		TuningSourceState state;

		state.lmEquipmentID = lmEquipmentID();
		state.hasConnection = m_hasConnection;
		state.receivedReplyCount = m_receivedRepyCount;
		state.sentRequestCount = m_sentRequestCount;
		state.flags = m_fotipFlags;

		return state;
	}

}
