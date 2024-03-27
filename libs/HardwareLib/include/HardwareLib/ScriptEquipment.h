#pragma once

namespace Hardware
{
	class DeviceObject;
	class ScriptDeviceObject;

	/// @brief The ScriptEquipment class represents equipment in the script.
	/// @details The equipment is structured as a tree of objects. Instances of this class allow
	/// access to the equipment hierarchy from within the script.\n
	/// The global variable 'equipment' provides access to an instance of this class.\n
	/// Example:\n
	/// @code
	/// (function(schemaItem) {
	///   let module = equipment.find("USB_RACK01_CH01_MD05");
	///   schemaItem.text = module.propertyValue("ActuatorID");
	/// })
	/// @endcode
	class ScriptEquipment : public QObject
	{
		Q_OBJECT

	public:
		ScriptEquipment(QJSEngine& jsEngine, QObject* parent);
		virtual ~ScriptEquipment() override;

	public slots:

		/// @brief Returns the root object of the equipment (type ScriptDeviceObject).
		/// @return The root ScriptDeviceObject of the equipment.
		QJSValue root() const;

		void setRoot(std::shared_ptr<DeviceObject> root);

		/// @brief Finds an object by its equipmentId (type ScriptDeviceObject).
		/// @param equipmentId The equipmentId of the device object to find.
		/// @return The ScriptDeviceObject with the specified equipmentId, or nullptr if no such object exists.
		/// @note The search by equipmentId is case-sensitive.
		/// 
		QJSValue find(QString equipmentId) const;

	private:
		void fillDeviceTable(const std::shared_ptr<DeviceObject>& parent, int recursionLevel = 0);

	private:
		QJSEngine& m_jsEngine;
		std::shared_ptr<DeviceObject> m_root; // It can be any object, not just DeviceRoot.

		std::unordered_map<QString, std::shared_ptr<DeviceObject>> m_deviceTable; // key is equipmentId
	};

} // namespace Hardware