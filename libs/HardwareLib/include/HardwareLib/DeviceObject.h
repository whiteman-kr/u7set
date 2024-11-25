#pragma once

#include <CommonLib/DebugInstCounter.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <array>

class DbFileInfo;

namespace Hardware
{
	extern const std::array<QString, 10> DeviceObjectExtensions;
	extern const std::array<QString, 10> DeviceTypeNames;

	class DeviceObject;
	class DeviceRoot;
	class DeviceSystem;
	class DeviceRack;
	class DeviceChassis;
	class DeviceModule;
	class DeviceController;
	class Workstation;
	class Software;
	class DeviceAppSignal;
	class DiagSignal;
	class DiagSignalTypeObject;


	// Device type, for defining hierarchy, don't save these data to file, can be changed (new level) later
	// If you add or change order in this enum, DO NOT FORGET TO CHANGE !!!!DeviceObjectExtensions!!!!
	//
	enum class DeviceType
	{
		Root,
		System,
		Rack,
		Chassis,
		Module,

		Workstation,
		Software,

		Controller,

		AppSignal,
		DiagSignal,

		DeviceTypeCount
	};


	//
	//
	// DeviceObject
	//
	//
	class DeviceObject :
		public PropertyObject,
		public Proto::ObjectSerialization<DeviceObject>,
		public DebugInstCounter<DeviceObject>,
		public std::enable_shared_from_this<DeviceObject>
	{
		Q_OBJECT

	protected:
		explicit DeviceObject(DeviceType deviceType, bool preset = false, QObject* parent = nullptr);

	public:
		DeviceObject() = delete;
		virtual ~DeviceObject() = default;

		DeviceObject(const DeviceObject&) = delete;

	public:
		void dump(bool dumpProps, bool dumpTree, QString* out = nullptr, int nesting = 0) const;

		[[nodiscard]] std::shared_ptr<const DeviceObject> sharedPtr() const;
		[[nodiscard]] std::shared_ptr<DeviceObject> sharedPtr();

		// Serialization
		//
		friend Proto::ObjectSerialization<DeviceObject>;	// for call CreateObject from Proto::ObjectSerialization
	protected:
		// Implementing Proto::ObjectSerialization<DeviceObject>::SaveData, LoadData
		//
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this function only while serialization, as when object is created is not fully initialized
		// and must be read before use
		//
		[[nodiscard]] static std::shared_ptr<DeviceObject> CreateObject(const Proto::Envelope& message);

		// Public methods
		//
	public:
		// Save object with ALL children
		//
		bool SaveObjectTree(Proto::Envelope* message) const;
		bool SaveObjectTreeIf(Proto::Envelope* message, std::function<bool(const DeviceObject&)> predicate) const;

		// Expand EquipmentIDTemplate for this and for all children
		//
		virtual void expandEquipmentId();

		// Get all signals, including signals from child items
		//
		[[nodiscard]] std::vector<std::shared_ptr<DeviceAppSignal>> getAllAppSignals() const;

		virtual bool event(QEvent* e) override;

		// Protected methods
		//
	protected:

		// Get all signals, including signals from child items
		//
		void getAllAppSignalsRecursive(std::vector<std::shared_ptr<DeviceAppSignal>>* deviceSignals) const;

		// Properties, etc
		//
	public:
		[[nodiscard]] bool hasParent() const;

		[[nodiscard]] std::shared_ptr<DeviceObject> parent();
		[[nodiscard]] const std::shared_ptr<DeviceObject> parent() const;

		[[nodiscard]] DeviceType deviceType() const;
		[[nodiscard]] QString deviceTypeName() const;
		[[nodiscard]] static QString deviceTypeName(DeviceType type);

		[[nodiscard]] bool isRoot() const;
		[[nodiscard]] bool isSystem() const;
		[[nodiscard]] bool isRack() const;
		[[nodiscard]] bool isChassis() const;
		[[nodiscard]] bool isModule() const;
		[[nodiscard]] bool isController() const;
		[[nodiscard]] bool isWorkstation() const;
		[[nodiscard]] bool isSoftware() const;
		[[nodiscard]] bool isAppSignal() const;
		[[nodiscard]] bool isDiagSignal() const;

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceRoot> toRoot() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceRoot> toRoot();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceSystem> toSystem() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceSystem> toSystem();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceRack> toRack() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceRack> toRack();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceChassis> toChassis() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceChassis> toChassis();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceModule> toModule() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceModule> toModule();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceController> toController() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceController> toController();

		[[nodiscard]] std::shared_ptr<const Hardware::DeviceAppSignal> toAppSignal() const;
		[[nodiscard]] std::shared_ptr<Hardware::DeviceAppSignal> toAppSignal();

		[[nodiscard]] std::shared_ptr<const Hardware::DiagSignal> toDiagSignal() const;
		[[nodiscard]] std::shared_ptr<Hardware::DiagSignal> toDiagSignal();

		[[nodiscard]] std::shared_ptr<const Hardware::Workstation> toWorkstation() const;
		[[nodiscard]] std::shared_ptr<Hardware::Workstation> toWorkstation();

		[[nodiscard]] std::shared_ptr<const Hardware::Software> toSoftware() const;
		[[nodiscard]] std::shared_ptr<Hardware::Software> toSoftware();

	private:
		template <typename DT>
		[[nodiscard]] std::shared_ptr<const DT> toType() const
		{
			std::shared_ptr<const DT> result = std::dynamic_pointer_cast<const DT>(shared_from_this());
			return result;
		}

		template <typename DT>
		[[nodiscard]] std::shared_ptr<DT> toType()
		{
			std::shared_ptr<DT> result = std::dynamic_pointer_cast<DT>(shared_from_this());
			return result;
		}

	public:
		[[nodiscard]] const Hardware::DeviceController* getParentController() const;
		[[nodiscard]] const Hardware::DeviceModule* getParentModule() const;
		[[nodiscard]] const Hardware::Software* getParentSoftware() const;
		[[nodiscard]] const Hardware::Workstation* getParentWorkstation() const;
		[[nodiscard]] const Hardware::DeviceChassis* getParentChassis() const;
		[[nodiscard]] std::shared_ptr<const Hardware::DeviceChassis> getParentChassisShared() const;
		[[nodiscard]] const Hardware::DeviceRack* getParentRack() const;
		[[nodiscard]] const Hardware::DeviceSystem* getParentSystem() const;
		[[nodiscard]] const Hardware::DeviceRoot* getParentRoot() const;

	public:
		[[nodiscard]] QString fileExtension() const;
		[[nodiscard]] static QString fileExtension(DeviceType device);

		void setExpertToProperty(const QString& property, bool expert);

		// Children care
		//
		[[nodiscard]] int childrenCount() const;
		[[nodiscard]] int childIndex(const std::shared_ptr<const DeviceObject>& child) const;

		[[nodiscard]] const std::vector<std::shared_ptr<DeviceObject>>& children() const;

		[[nodiscard]] const std::shared_ptr<DeviceObject>& child(int index) const;
		[[nodiscard]] std::shared_ptr<DeviceObject> child(const QUuid& uuid) const;
		[[nodiscard]] std::shared_ptr<DeviceObject> childByPresetUuid(const QUuid& presetObjectUuid) const;
		
		[[nodiscard]] std::shared_ptr<DeviceObject> childByEquipmentId(const QString& id);
		[[nodiscard]] std::shared_ptr<const DeviceObject> childByEquipmentId(const QString& id) const;

		[[nodiscard]] bool canAddChild(const DeviceType childType) const;

		void addChild(const std::shared_ptr<DeviceObject>& child);
		void deleteChild(std::shared_ptr<DeviceObject> child);
		void deleteAllChildren();

		[[nodiscard]] bool checkChild(std::shared_ptr<DeviceObject> child, QString* errorMessage);

		static QString replaceEngeneeringToEngineering(const QString& data);

		// Props
		//
	public:
		/// @brief Check if this object or its parent is excluded from build.
		[[nodiscard]] bool isExcludedFromBuild() const;

		/// @brief Check if this object is excluded from build (parents are not checked).
		[[nodiscard]] bool excludeFromBuild() const;
		void setExcludeFromBuild(bool exclude);

		[[nodiscard]] QUuid uuid() const;
		void setUuid(QUuid value);

		[[nodiscard]] QString equipmentIdTemplate() const;
		void setEquipmentIdTemplate(const QString& value);

		[[nodiscard]] QString equipmentId() const;			// This unwinds equipmentIdTemplate

		[[nodiscard]] QString caption() const;
		void setCaption(QString value);

		[[nodiscard]] QString childRestriction() const;
		void setChildRestriction(QString value);

		[[nodiscard]] QString specificPropertiesStruct() const;
		void setSpecificPropertiesStruct(QString value);

		[[nodiscard]] int place() const;
		void setPlace(int value);

		[[nodiscard]] bool hasTag(const QString& tag) const;
		[[nodiscard]] bool hasTag(Hash tagHash) const;

		[[nodiscard]] QStringList tags() const;
		[[nodiscard]] QString tagsAsString() const;

		void setTags(const QStringList& tags);
		void setTags(const QString& tags);

		[[nodiscard]] QString details() const;		// JSON short description, uuid, equipmentId, caption, place, etc

		// Preset
		//
		[[nodiscard]] bool isPreset() const;
private:
		void setPreset(bool isPreset);
public:
		[[nodiscard]] bool presetRoot() const;
		void setPresetRoot(bool value);

		[[nodiscard]] int presetVersion() const;
		void setPresetVersion(int value);

		[[nodiscard]] QString presetName() const;
		void setPresetName(QString value);

		[[nodiscard]] QUuid presetObjectUuid() const;
		void setPresetObjectUuid(QUuid value);

		[[nodiscard]] QString presetProtectedPropertiesStr() const;
		void setPresetProtectedPropertiesStr(const QString& value);

		[[nodiscard]] const QStringList& presetProtectedProperties() const;
		void setPresetProtectedProperties(const QStringList& value);

		[[nodiscard]] DbFileInfo* data();
		[[nodiscard]] const DbFileInfo* data() const;
		void setData(std::shared_ptr<DbFileInfo> data);

		// Data
		//
	protected:
		const DeviceType m_deviceType = DeviceType::Root;
		bool m_excludeFromBuild = false;

		std::weak_ptr<DeviceObject> m_parent;
		std::vector<std::shared_ptr<DeviceObject>> m_children;

		QUuid m_uuid;
		QString m_equipmentId;
		QString m_caption;

		QString m_childRestriction;			// Restriction script for child items
		QString m_specificPropertiesStruct;	// Description of the Object's specific properties

		int m_place = -1;

		std::unordered_map<Hash, QString> m_tags;	// Tags for this object, key is a tag hash, value is a tag value.

	private:
		// Preset Data
		//
		bool m_preset = false;				// It is preset or part of it
		bool m_presetRoot = false;			// This object is preset root
		int m_presetVersion = 0;			// If this object is presetRoot, then this field contains preset version
		QString m_presetName;				// PresetName, if it is preset
		QUuid m_presetObjectUuid;			// In configuration this field has uuid of the PRESET object from which it was constructed
											// In preset edit mode this field has the same value with m_uuid
		QStringList m_presetProtectedProperties; // Properties that cannot be update from preset

		std::shared_ptr<DbFileInfo> m_data;	// Application-specific value associated with the specified item (DbFileInfo)
	};


	// Walk through equipment tree
	//
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren, std::function<void(Hardware::DeviceObject* device)> processAfterChildren);
	void equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren);

	void SerializeEquipmentFromXml(const QString &filePath, std::shared_ptr<DeviceRoot>& deviceRoot);

	QString expandDeviceSignalTemplate(const Hardware::DeviceObject& startDeviceObject,
									   const QString& templateStr,
									   QString* errMsg);

	QString expandDeviceObjectMacro(const Hardware::DeviceObject& startDeviceObject,
									const QString& macroStr,
									QString* errMsg);

	const Hardware::DeviceObject* getParentDeviceObjectOfType(const Hardware::DeviceObject& startObject,
															  const QString& parentObjectType,
															  QString* errMsg);
}

