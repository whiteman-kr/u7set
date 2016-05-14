#pragma once

#include "../include/DataSource.h"
#include "../u7/Builder/TuningDataStorage.h"
#include "TuningSocket.h"


class TuningDataSourceInfo
{
public:
	int channel = 0;
	DataSource::DataType dataType = DataSource::DataType::App;
	QString lmEquipmentID;								// unique ID
	QString lmCaption;
	QString lmAdapterID;
	bool lmDataEnable;
	HostAddressPort lmAddressPort;
	ulong lmDataID = 0;

	QVector<Signal> tuningSignals;
};


class TuningDataSourceState
{
public:
	QString lmEquipmentID;

	bool hasConnection;
	int sentRequestCount;
	int receivedReplyCount;

	FotipHeaderFlags flags;
};


struct TuningSignalState
{
	double currentValue = 0;
	double lowLimit = 0;
	double highLimit = 0;

	bool valid = false;
};


class TuningDataSource : public DataSource
{
private:
	TuningData* m_tuningData = nullptr;

	bool m_deleteTuningData = false;

	quint16 m_numerator = 0;

	bool m_waiReply = false;

	int m_frameToRequest = 0;		//	0 .. m_tuningData->totalFtamesCount - 1

	// state fields
	//
	qint64 m_lastReplyTime = 0;
	bool m_hasConnection = false;
	int m_sentRequestCount = 0;
	int m_receivedRepyCount = 0;
	FotipHeaderFlags m_fotipFlags;

public:
	TuningDataSource();
	~TuningDataSource();

	void setTuningData(TuningData* tuningData);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) override;
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

	void getTuningDataSourceInfo(TuningDataSourceInfo& info);

	quint16 numerator() const { return m_numerator; }
	void incNumerator() { m_numerator++; }

	int frameToRequest() const { return m_frameToRequest; }
	int nextFrameToRequest();

	void setWaitReply() { m_waiReply = true; }
	void resetWaitReply() { m_waiReply = false; }

	void processReply(const Tuning::SocketReply& reply);

	bool getSignalState(const QString& appSignalID, TuningSignalState* tss);
	bool setSignalState(const QString& appSignalID, double value, Tuning::SocketRequest* sr);

	quint64 uniqueID();

	TuningDataSourceState getState();

	void incSentRequestCount() { m_sentRequestCount++; }

	void testConnection(qint64 nowTime);
};


class TuningDataSources : public QHash<QString, TuningDataSource*>
{
	QHash<quint32, TuningDataSource*> m_ip2DataSource;

public:
	~TuningDataSources();

	void clear();

	void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

	void buildIP2DataSourceMap();

	TuningDataSource* getDataSourceByIP(quint32 ip);
};
