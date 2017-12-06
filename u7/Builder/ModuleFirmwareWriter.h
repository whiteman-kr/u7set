#ifndef MODULEFIRMWAREWRITER_H
#define MODULEFIRMWAREWRITER_H

#include "../lib/ModuleFirmware.h"
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
	struct UartChannelData
	{
		// Module binary data, key is LMNumber
		//
		std::map<int, QByteArray> binaryDataMap;

		// Module Unique ID, key is LMNumber
		//
		std::map<int, quint64> uniqueIdMap;

		// Channel Data description, key is LMNumber
		//
		QStringList descriptionFields;		// Header line
		int descriptionFieldsVersion = 0;

		std::map<int, std::vector<QVariantList>> descriptonDataMap;
	};

	typedef std::map<int, UartChannelData> ModuleChannelData;

	//
	// ModuleFirmwareWriter
	//

	class ModuleFirmwareWriter : public ModuleFirmwareStorage
    {
		Q_OBJECT
    public:
		ModuleFirmwareWriter();

		UartChannelData& uartChannelData(const QString& subsysId, int uartId);

		bool save(QByteArray &dest, Builder::IssueLogger* log);

		// Functions called and used by Application Logic and Tuning Builder
		//
		void setDescriptionFields(const QString& subsysId, int uartId, int descriptionVersion, const QStringList& fields);
		bool setChannelData(const QString& subsysId, int uartId, QString equipmentID, int channel, int eepromFramePayloadSize, int eepromFrameCount, quint64 uniqueID, const QByteArray& binaryData, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log);

		// Functions called and used by Configuration Script
		//
		void setScriptFirmware(QString subsysId, int uartID);

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
		const QByteArray& scriptLog(const QString& subsysId) const;

		// Functions that are used to calculate Unique ID
		//
		quint64 uniqueID(const QString& subsysId, int uartId, int lmNumber);
		void setGenericUniqueId(const QString& subsysId, int lmNumber, quint64 genericUniqueId);

	private:

		bool storeChannelData(Builder::IssueLogger* log);

	private:

		// Channel data map, key is Subsystem ID
		//
		std::map<QString, ModuleChannelData> m_moduleChannelData;

		// Script execution log, key is Subsystem ID
		//
		std::map<QString, QByteArray> m_scriptLog;

		// Pointers to firmware and data currently processed by script, used by script functions
		//
		ModuleFirmware* scriptFirmware = nullptr;
		ModuleFirmwareData* scriptFirmwareData = nullptr;
		UartChannelData* scriptUartChannelData = nullptr;

	};

}

#endif // MODULEFIRMWAREWRITER_H
