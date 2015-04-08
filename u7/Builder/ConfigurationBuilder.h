#ifndef CONFIGURATIONBUILDER_H
#define CONFIGURATIONBUILDER_H

// Forware delcarations
//
class QThread;
class OutputLog;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class McFirmwareOld;
}


namespace Builder
{

	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
		ConfigurationBuilder(DbController* db, OutputLog* log, int changesetId, bool debug, QString projectName, QString userName);
		virtual ~ConfigurationBuilder();

		bool build();

	protected:
		// Get Equipment from the prokect database
		//
		bool getEquipment(DbController* db, Hardware::DeviceObject* parent);

		// Expand Devices StrId
		//
		bool expandDeviceStrId(Hardware::DeviceObject* device);


	private:
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:
		DbController* m_db = nullptr;
		mutable OutputLog* m_log = nullptr;
		int m_changesetId = 0;
		int m_debug = false;
		QString m_projectName;
		QString m_userName;

	};

}

#endif // CONFIGURATIONBUILDER_H
