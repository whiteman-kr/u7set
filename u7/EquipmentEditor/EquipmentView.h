#pragma once
#include "../../HardwareLib/DeviceObject.h"

class EquipmentModel;

//
//
// EquipmentView
//
//
class EquipmentView : public QTreeView
{
	Q_OBJECT

public:
	EquipmentView() = delete;
	explicit EquipmentView(DbController* dbcontroller);
	virtual ~EquipmentView();

	bool isPresetMode() const;
	bool isConfigurationMode() const;

signals:
	void updateState();

public:
	void saveSession() const;
	void restoreSession();

public slots:
	void projectOpened();
	void projectClosed();

	void addSystem();
	void addRack();
	void addChassis();
	void addModule();
	void addController();
	void addSignal();
	void addWorkstation();
	void addSoftware();

	void addPreset();
	void replaceObject();

	void addPresetRack();
	void addPresetChassis();
	void addPresetModule();
	void addPresetController();
	void addPresetWorkstation();
	void addPresetSoftware();

	void choosePreset(Hardware::DeviceType type);

	std::shared_ptr<Hardware::DeviceObject> addPresetToConfiguration(const DbFileInfo& fileInfo, bool addToEquipment);
	QModelIndex addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object,
								QModelIndex parentModelIndex,
								bool clearPrevSelection,
								bool newUuids);

	void createInOutsToSignals();
	void addInOutsToSignals(std::shared_ptr<Hardware::DeviceModule> module);
	void addInOutsToSignals(std::vector<std::shared_ptr<Hardware::DeviceAppSignal>> hardwareAppSignals);

	void showAppSignals(bool refreshSignalList, bool exactMatch);			// Show application signals for this object
	void createInternalAppSignal();

	void addLogicSchemaToLm();
	void showLogicSchemaForLm();

	void createConnection();
	void showObjectConnections();

	void copySelectedDevices();
	void pasteDevices();
	void pasteDevices(const ::Proto::EnvelopeSet& messageItems,
					  const Proto::EnvelopeSetShortDescription& messageDescr,
					  bool newUuids);
	bool canPaste() const;
	bool canPaste(const ::Proto::EnvelopeSetShortDescription& message) const;

	void findObject();
	bool findObject(QString equiepmentId);

	std::shared_ptr<Hardware::DeviceObject> deviceObject(QString equiepmentId) const;
	std::vector<std::shared_ptr<Hardware::DeviceObject>> deviceObjects(QString equipmentId) const;

	void deleteSelectedDevices();
	void checkInSelectedDevices();
	void checkOutSelectedDevices();
	void undoChangesSelectedDevices();
	void showHistory();
	void compare();
	void refreshSelectedDevices();

	void updateSelectedDevices();

	void updateFromPreset();
	bool updateDeviceFromPreset(std::shared_ptr<Hardware::DeviceObject> device,
								std::shared_ptr<Hardware::DeviceObject> preset,
								const QStringList& forceUpdateProperties,
								const QStringList& presetsToUpdate,
								std::vector<std::shared_ptr<Hardware::DeviceObject>>* updateDeviceList,
								std::vector<Hardware::DeviceObject*>* deleteDeviceList,
								std::vector<std::pair<int, int>>* addDeviceList,
								std::vector<const Hardware::DeviceAppSignal*>* deviceSignalsToUpdateAppSignals);

	// Events
	//
protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void focusInEvent(QFocusEvent* event) override;
	virtual void focusOutEvent(QFocusEvent* event) override;

	// Properties
	//
protected:
	EquipmentModel* equipmentModel();
	EquipmentModel* equipmentModel() const;
	DbController* db();
	const DbController* db() const;

	// Data
	//
private:
	DbController* m_dbController;

	// Postpone restore session to showEvent()
	//
	bool m_requireRestoreSession = false;

public:
	static const char* mimeType;					// = "application/x-deviceobjecs";
	static const char* mimeTypeShortDescription;	// = "application/x-deviceobjecs-sd";
};
