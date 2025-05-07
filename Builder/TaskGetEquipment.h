#pragma once

namespace Builder
{
	class Context;
	class IssueLogger;


	class TaskGetEquipment : public QObject
	{
	public:
		explicit TaskGetEquipment(::Builder::Context& context);

		bool doIt();

	private:
		// Get equipment from the database.
		//
		bool getEquipment(Hardware::DeviceObject* parent);

		// Remove excluded devices from the equipment.
		//
		bool removeExcludedDevices(Hardware::DeviceObject* parent);

		// Expand Devices StrId
		//
		bool expandDeviceStrId(Hardware::DeviceObject* device);

		// Check same UUIDs and same StrIds
		//
		bool checkUuidAndStrId(Hardware::DeviceObject* root);
		bool checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
									 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
									 std::map<QString, Hardware::DeviceObject*>& strIdMap);

		bool execPreBuildScript(std::shared_ptr<Hardware::DeviceObject> root);
		bool execPreBuildScriptWorker(Hardware::DeviceObject* device, QJSEngine& engine, QJSValue& jsEquipment);

		bool checkChildRestrictions(std::shared_ptr<Hardware::DeviceObject> root);
		bool checkChildRestrictionsWorker(std::shared_ptr<Hardware::DeviceObject> device);

	private:
		::Builder::Context& m_context;
		::Builder::IssueLogger& m_log;
	};
} // namespace Builder