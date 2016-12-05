#include "TuningSource.h"


namespace Tuning
{

	// -------------------------------------------------------------------------------
	//
	// TuningSource class implementation
	//
	// -------------------------------------------------------------------------------

	TuningSource::TuningSource()
	{
		m_lmDataType = DataSource::DataType::Tuning;
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


	void TuningSource::writeAdditionalSectionsToXml(XmlWriteHelper& xml)
	{
		if (m_tuningData == nullptr)
		{
			TuningData td(lmEquipmentID(), 0, 0);
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

		return true;
	}


	// -------------------------------------------------------------------------------
	//
	// TuningSources class implementation
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


	/*void TuningSources::getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info)
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
	}*/


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


	/*void TuningSource::processReply(const Tuning::SocketReply& reply)
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
	}*/

}
