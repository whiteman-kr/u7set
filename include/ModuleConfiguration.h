#pragma once

#include "../include/ProtoSerialization.h"

// ----------------------------------------------------------------------------
//
//						Module Configuration
//
// ----------------------------------------------------------------------------

namespace Hardware
{
	class ModuleConfFirmware : public QObject
	{
		Q_OBJECT

		Q_PROPERTY(int UartID READ uartId)
		Q_PROPERTY(int FrameSize READ frameSize)
		Q_PROPERTY(int FrameCount READ frameCount)

	public:
		ModuleConfFirmware();
		virtual ~ModuleConfFirmware();

		// Methods
		//
	public:
		void init(QString type, QString subsysId, int uartId, int frameSize, int frameCount, const QString &projectName, const QString &userName, int changesetId);
		bool save(QByteArray &dest) const;
        bool load(QString fileName);
        bool isEmpty() const;

		Q_INVOKABLE bool setData8(int frameIndex, int offset, quint8 data);
		Q_INVOKABLE bool setData16(int frameIndex, int offset, quint16 data);
		Q_INVOKABLE bool setData32(int frameIndex, int offset, quint32 data);
        bool setData64(int frameIndex, int offset, quint64 data);

		Q_INVOKABLE quint8 data8(int frameIndex, int offset);
		Q_INVOKABLE quint16 data16(int frameIndex, int offset);
		Q_INVOKABLE quint32 data32(int frameIndex, int offset);

		Q_INVOKABLE bool storeCrc64(int frameIndex, int start, int count, int offset);
		Q_INVOKABLE bool storeHash64(int frameIndex, int offset, quint16 data);

        std::vector<quint8> frame(int frameIndex);


		// Properties
		//
	public:
        QString type() const;
		QString subsysId() const;
		int uartId() const;
		int frameSize() const;
		int frameCount() const;
		int changesetId() const;

		// Data
		//
    private:
        QString m_type;
		QString m_subsysId;
		int m_uartId = 0;
		int m_frameSize = 0;
		int m_changesetId = 0;

		QString m_projectName;
		QString m_userName;

        std::vector<std::vector<quint8>> m_frames;
    };

    class ModuleConfCollection : public QObject
	{
		Q_OBJECT

	public:
		ModuleConfCollection(const QString& projectName, const QString& userName, int changesetId);
		virtual ~ModuleConfCollection();

		// Methods
		//
	public:
		Q_INVOKABLE QObject* jsGet(QString type, QString subsysId, int uartId, int frameSize, int frameCount);

		// Properties
		//
	public:

		// Data
		//
	public:
		const std::map<QString, ModuleConfFirmware>& firmwares() const;

	private:
		std::map<QString, ModuleConfFirmware> m_firmwares;

		QString m_projectName;
		QString m_userName;
		int m_changesetId;
	};
}

