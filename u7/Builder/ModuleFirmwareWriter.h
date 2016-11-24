#ifndef MODULEFIRMWAREWRITER_H
#define MODULEFIRMWAREWRITER_H

#include "../lib/ModuleConfiguration.h"
#include "IssueLogger.h"

namespace Hardware
{
    class ModuleFirmwareWriter : public ModuleFirmware
    {
    public:
        ModuleFirmwareWriter();
		bool save(QByteArray &dest, Builder::IssueLogger *log);
		bool setChannelData(QString equipmentID, int channel, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& data, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger *log);

	private:
        bool storeChannelData(Builder::IssueLogger *log);


    };

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
		Q_INVOKABLE QObject* jsGet(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount);

		quint64 getFirmwareUniqueId(const QString &subsystemID, int lmNumber);

		void setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId);

		// Properties
		//
	public:

		// Data
		//
	public:
		std::map<QString, ModuleFirmwareWriter> &firmwares();

	private:
		std::map<QString, ModuleFirmwareWriter> m_firmwares;

		QString m_projectName;
		QString m_userName;
		int m_buildNo = 0;
		bool m_debug = false;
        int m_changesetId = 0;

    };}

#endif // MODULEFIRMWAREWRITER_H
