#pragma once

#include "../lib/ProtoSerialization.h"
#include "../lib/DeviceObject.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

class QJsVariantList : public QObject
{
    Q_OBJECT

    QVariantList l;

public:
    QJsVariantList(QObject* parent);
    ~QJsVariantList();

    void append(QVariant v);
    Q_INVOKABLE int jsSize();
    Q_INVOKABLE QVariant jsAt(int i);


};

namespace Hardware
{

	class ModuleFirmwareData
	{
	public:
		quint64 uniqueID = 0;
		QByteArray data;
	};

	class ModuleFirmware : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int UartID READ uartId)
		Q_PROPERTY(int SSKey READ ssKey)
		Q_PROPERTY(int FrameSize READ frameSize)
		Q_PROPERTY(int FrameCount READ frameCount)

	public:
		ModuleFirmware();
		virtual ~ModuleFirmware();

		// Methods
		//
	public:
		void init(QString caption, QString subsysId, int uartId, int ssKey, int frameSize, int frameCount, const QString &projectName, const QString &userName, int changesetId, const QStringList& descriptionFields);
		bool load(QString fileName, QString &errorCode);
		bool isEmpty() const;

		Q_INVOKABLE bool setData8(int frameIndex, int offset, quint8 data);
		Q_INVOKABLE bool setData16(int frameIndex, int offset, quint16 data);
		Q_INVOKABLE bool setData32(int frameIndex, int offset, quint32 data);
        bool setData64(int frameIndex, int offset, quint64 data);

		Q_INVOKABLE quint8 data8(int frameIndex, int offset);
		Q_INVOKABLE quint16 data16(int frameIndex, int offset);
		Q_INVOKABLE quint32 data32(int frameIndex, int offset);

        Q_INVOKABLE QJsVariantList* calcHash64(QString dataString);
        Q_INVOKABLE QString storeCrc64(int frameIndex, int start, int count, int offset);
        Q_INVOKABLE QString storeHash64(int frameIndex, int offset, QString dataString);

        Q_INVOKABLE void writeLog(QString logString);

		Q_INVOKABLE void jsSetDescriptionFields(QString fields);

		void setDescriptionFields(const QStringList& fields);

		Q_INVOKABLE void jsAddDescription(int channel, QString descriptionCSV);

        std::vector<quint8> frame(int frameIndex);



	private:

		bool load_version1(const QJsonObject& jConfig);
		bool load_version2(const QJsonObject& jConfig);

		// Properties
		//
	public:
		QString caption() const;
		QString subsysId() const;
		quint16 ssKey() const;
		int uartId() const;
		int frameSize() const;
		int frameCount() const;
		int changesetId() const;
		int fileVersion() const;
		int maxFileVersion() const;
        const QByteArray& log() const;

		// Data
		//
	protected:
		QString m_caption;
		QString m_subsysId;
		quint16 m_ssKey = 0;
		int m_uartId = 0;
		int m_frameSize = 0;
		int m_changesetId = 0;
		int m_fileVersion = 0;
		int m_maxFileVersion = 2;

		QString m_projectName;
		QString m_userName;

		// data description
		//
		QStringList m_descriptionFields;
		std::map<int, std::vector<QVariantList>> m_descriptonData;

		// channel data
		//
		std::map<int, ModuleFirmwareData> m_channelData;

		// binary data
		//
		std::vector<std::vector<quint8>> m_frames;


		QByteArray m_log;

    };


}

