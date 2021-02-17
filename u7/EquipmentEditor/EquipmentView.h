#pragma once
#include "../../lib/DeviceObject.h"

namespace Hardware
{
	class DeviceObject;
}

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

public slots:
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
	QModelIndex addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentModelIndex, bool clearPrevSelection);

	void addInOutsToSignals();
	void showAppSignals(bool refreshSignalList = false);			// Show application signals for this object
	void addAppSignal();

	void addLogicSchemaToLm();
	void showLogicSchemaForLm();

	void addOptoConnection();
	void showObjectConnections();

	void copySelectedDevices();
	void pasteDevices();
	void pasteDevices(const ::Proto::EnvelopeSet& messageItems, const Proto::EnvelopeSetShortDescription& messageDescr);
	bool canPaste() const;
	bool canPaste(const ::Proto::EnvelopeSetShortDescription& message) const;

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
								QVector<Hardware::DeviceSignal*>* deviceSignalsToUpdateAppSignals);

	// Events
	//
protected:
	virtual void focusInEvent(QFocusEvent* event) override;
	virtual void focusOutEvent(QFocusEvent* event) override;

	// Properties
	//
protected:
	EquipmentModel* equipmentModel();
	EquipmentModel* equipmentModel() const;
	DbController* db();

	// Data
	//
private:
	DbController* m_dbController;

public:
	static const char* mimeType;					// = "application/x-deviceobjecs";
	static const char* mimeTypeShortDescription;	// = "application/x-deviceobjecs-sd";
};
