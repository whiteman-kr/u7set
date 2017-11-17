#ifndef MODULEFIRMWAREWRITER_H
#define MODULEFIRMWAREWRITER_H

#include "../lib/ModuleConfiguration.h"
#include "IssueLogger.h"

//
// JsVariantList
//

class JsVariantList : public QObject
{
	Q_OBJECT
public:
	JsVariantList(QObject* parent);

	void append(QVariant v);
	Q_INVOKABLE int jsSize();
	Q_INVOKABLE QVariant jsAt(int i);

private:
	QVariantList m_list;
};


namespace Hardware
{
	//
	// ModuleFirmwareWriter
	//

	class ModuleFirmwareWriter : public ModuleFirmware
    {
		Q_OBJECT
    public:
        ModuleFirmwareWriter();

		bool save(QByteArray &dest, Builder::IssueLogger* log);

		// Functions called and used by Application Logic and Tuning Builder
		//
		void setDescriptionFields(int descriptionVersion, const QStringList& fields);
		bool setChannelData(QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& data, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log);

		// Functions called and used by Configuration Script
		//
		Q_INVOKABLE bool setData8(int frameIndex, int offset, quint8 data);
		Q_INVOKABLE bool setData16(int frameIndex, int offset, quint16 data);
		Q_INVOKABLE bool setData32(int frameIndex, int offset, quint32 data);
		bool setData64(int frameIndex, int offset, quint64 data);

		Q_INVOKABLE quint8 data8(int frameIndex, int offset);
		Q_INVOKABLE quint16 data16(int frameIndex, int offset);
		Q_INVOKABLE quint32 data32(int frameIndex, int offset);

		Q_INVOKABLE JsVariantList* calcHash64(QString dataString);
		Q_INVOKABLE QString storeCrc64(int frameIndex, int start, int count, int offset);
		Q_INVOKABLE QString storeHash64(int frameIndex, int offset, QString dataString);

		Q_INVOKABLE quint32 calcCrc32(int frameIndex, int start, int count);

		Q_INVOKABLE void jsSetDescriptionFields(int descriptionVersion, QString fields);

		Q_INVOKABLE void jsAddDescription(int channel, QString descriptionCSV);

		Q_INVOKABLE void jsSetUniqueID(int lmNumber, quint64 uniqueID);

		// Script execution log
		//
		Q_INVOKABLE void writeLog(QString logString);
		const QByteArray& scriptLog() const;

		// Functions that are used to calculate Unique ID
		//
		quint64 uniqueID(int lmNumber);
		void setGenericUniqueId(int lmNumber, quint64 genericUniqueId);

	private:

		bool storeChannelData(Builder::IssueLogger* log);

	private:
		// Channel data
		//
		std::map<int, QByteArray> m_channelData;

		// Channel Unique ID
		//
		std::map<int, quint64> m_channelUniqueId;

		// Channel Data description
		//
		QStringList m_descriptionFields;		// Header line
		int m_descriptionFieldsVersion = 0;
		std::map<int, std::vector<QVariantList>> m_descriptonData;

		// Script execution log
		//
		QByteArray m_scriptLog;
	};

	//
	// ModuleFirmwareCollection
	//

	class ModuleFirmwareCollection : public QObject
	{
		Q_OBJECT

	public:
		ModuleFirmwareCollection();
		virtual ~ModuleFirmwareCollection();

		void init(const QString& projectName, const QString& userName, int buildNo, bool debug, int changesetId);

		// Methods
		//
	public:
		ModuleFirmwareWriter* get(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount);
		Q_INVOKABLE QObject* jsGet(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount);

		quint64 getFirmwareUniqueId(const QString &subsystemID, int lmNumber);

		void setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId);

		// Properties
		//
	public:

		// Data
		//
	public:
		std::map<QString, ModuleFirmwareWriter>& firmwares();

	private:
		std::map<QString, ModuleFirmwareWriter> m_firmwares;

		QString m_projectName;
		QString m_userName;
		int m_buildNo = 0;
		bool m_debug = false;
        int m_changesetId = 0;

    };}

#endif // MODULEFIRMWAREWRITER_H
