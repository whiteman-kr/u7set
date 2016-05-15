#include "TuningDataSource.h"


namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TuningDataSource class implementation
	//
	// -------------------------------------------------------------------------------

	TuningDataSource::TuningDataSource()
	{
		m_lmDataType = DataSource::DataType::Tuning;

		m_fotipFlags.all = 0;
	}


	TuningDataSource::~TuningDataSource()
	{
		if (m_deleteTuningData == true)
		{
			delete m_tuningData;
		}
	}


	void TuningDataSource::setTuningData(TuningData* tuningData)
	{
		if (tuningData == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningData = tuningData;
	}


	void TuningDataSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml)
	{
		if (m_tuningData == nullptr)
		{
			TuningData td(lmEquipmentID(), 0, 0);
			td.writeToXml(xml);
			return;
		}

		m_tuningData->writeToXml(xml);
	}


	bool TuningDataSource::readAdditionalSectionsFromXml(XmlReadHelper& xml)
	{
		assert(m_tuningData == nullptr);

		m_tuningData = new TuningData();

		m_deleteTuningData = true;

		m_tuningData->readFromXml(xml);
		m_tuningData->initTuningData();

		return true;
	}


	void TuningDataSource::getTuningDataSourceInfo(TuningDataSourceInfo& info)
	{
		info.channel = m_lmChannel;
		info.dataType = m_lmDataType;
		info.lmEquipmentID = m_lmEquipmentID;
		info.lmCaption = m_lmCaption;
		info.lmAdapterID = m_lmAdapterID;
		info.lmDataEnable = m_lmDataEnable;
		info.lmAddressPort = m_lmAddressPort;
		info.lmDataID = m_lmDataID;

		info.tuningSignals.clear();

		if (m_tuningData == nullptr)
		{
			return;
		}

		QList<Signal*> signalList;

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


	int TuningDataSource::nextFrameToRequest()
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

	TuningDataSources::~TuningDataSources()
	{
		clear();
	}


	void TuningDataSources::clear()
	{
		for(TuningDataSource* ds : *this)
		{
			delete ds;
		}

		QHash<QString, TuningDataSource*>::clear();
		m_ip2DataSource.clear();
	}


	void TuningDataSources::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
	{
		info.clear();

		info.resize(count());

		int index = 0;

		for(TuningDataSource* source : (*this))
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


	void TuningDataSources::buildIP2DataSourceMap()
	{
		for(TuningDataSource* source : *this)
		{
			m_ip2DataSource.insert(source->lmAddress32(), source);
		}
	}


	TuningDataSource* TuningDataSources::getDataSourceByIP(quint32 ip)
	{
		if (m_ip2DataSource.contains(ip))
		{
			return m_ip2DataSource[ip];
		}

		return nullptr;
	}


	void TuningDataSource::processReply(const Tuning::SocketReply& reply)
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


	void TuningDataSource::testConnection(qint64 nowTime)
	{
		if (nowTime - m_lastReplyTime > 2000)		// connection timeout == 2 seconds
		{
			m_hasConnection = false;
		}
	}


	bool TuningDataSource::getSignalState(const QString& appSignalID, TuningSignalState* tss)
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


	bool TuningDataSource::setSignalState(const QString& appSignalID, double value, Tuning::SocketRequest* sr)
	{
		if (m_tuningData == nullptr)
		{
			return false;
		}

		return m_tuningData->setSignalState(appSignalID, value, sr);
	}


	quint64 TuningDataSource::uniqueID()
	{
		if (m_tuningData == nullptr)
		{
			assert(false);
			return 0;
		}

		return m_tuningData->uniqueID();
	}


	TuningDataSourceState TuningDataSource::getState()
	{
		TuningDataSourceState state;

		state.lmEquipmentID = m_lmEquipmentID;
		state.hasConnection = m_hasConnection;
		state.receivedReplyCount = m_receivedRepyCount;
		state.sentRequestCount = m_sentRequestCount;

		return state;
	}

}
