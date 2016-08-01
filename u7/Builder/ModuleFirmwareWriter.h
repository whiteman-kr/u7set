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
		ModuleFirmwareCollection(const QString& projectName, const QString& userName, int changesetId);
		virtual ~ModuleFirmwareCollection();

		// Methods
		//
	public:
		Q_INVOKABLE QObject* jsGet(QString caption, QString subsysId, int ssKey, int uartId, int frameSize, int frameCount);

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
		int m_changesetId;

	};}

#endif // MODULEFIRMWAREWRITER_H
