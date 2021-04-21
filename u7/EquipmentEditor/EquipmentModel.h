#pragma once
#include "../../HardwareLib/DeviceObject.h"
#include "../../DbLib/DbStruct.h"

class DbController;

//
//
// EquipmentModel
//
//
class EquipmentModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	EquipmentModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent);
	virtual ~EquipmentModel();

	QModelIndex index(int row, const QModelIndex& parentIndex) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parentIndex) const override;

	virtual QModelIndex parent(const QModelIndex& childIndex) const override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	virtual bool hasChildren(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual bool canFetchMore(const QModelIndex& parent) const override;
	virtual void fetchMore(const QModelIndex& parent) override;

	void sortDeviceObject(std::shared_ptr<Hardware::DeviceObject>& object, int column, Qt::SortOrder order);
	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

	// --
	//
public:
	bool insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex);
	void deleteDeviceObject(const QModelIndexList& rowList);

private:
	void updateRowFuncOnCheckIn(QModelIndex modelIndex, const std::map<int, DbFileInfo>& updateFiles, std::set<void*>& updatedModelIndexes);
public:
	void checkInDeviceObject(QModelIndexList& rowList);

	void checkOutDeviceObject(QModelIndexList& rowList);
	void undoChangesDeviceObject(QModelIndexList& undowRowList);

	void refreshDeviceObject(QModelIndexList& rowList);
	void updateDeviceObject(QModelIndexList& rowList);

	std::shared_ptr<Hardware::DeviceObject> deviceObject(QModelIndex& index);
	std::shared_ptr<const Hardware::DeviceObject> deviceObject(const QModelIndex& index) const;

	QString usernameById(int userId) const;

	void reset();

private:
	void sortChildrenByCaption(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);
	void sortChildrenByType(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);
	void sortChildrenByEquipmentId(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);
	void sortChildrenByPlace(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);
	void sortChildrenByState(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);
	void sortChildrenByUser(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order);

public slots:
	void projectOpened();
	void projectClosed();

	void switchMode();

	void updateUserList();

signals:
	void objectVcsStateChanged();

	// Properties
	//
public:
	static const DbFileInfo* fileInfo(const Hardware::DeviceObject* deviceObject);
	static const DbFileInfo* fileInfo(const std::shared_ptr<Hardware::DeviceObject>& deviceObject);

	static void setFileInfo(Hardware::DeviceObject* deviceObject, const DbFileInfo& fileInfo);
	static void setFileInfo(std::shared_ptr<Hardware::DeviceObject> deviceObject, const DbFileInfo& fileInfo);

	DbController* dbController();
	DbController* dbController() const;

	bool isPresetMode() const;
	bool isConfigurationMode() const;

public:
	enum Columns
	{
		ObjectNameColumn,
		ObjectTypeColumn,
		ObjectEquipmentIdColumn,
		ObjectPlaceColumn,
		ObjectStateColumn,
		ObjectUserColumn,

		// Add other column befor this line
		//
		ColumnCount
	};

	// Data
	//
private:
	DbController* m_dbController;
	QWidget* m_parentWidget;			// Access to DbController requires widget for showing progress bar

	std::shared_ptr<Hardware::DeviceObject> m_root;
	std::shared_ptr<Hardware::DeviceObject> m_configuration;
	std::shared_ptr<Hardware::DeviceObject> m_preset;

	int m_sortColumn = ObjectPlaceColumn;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	std::map<int, QString> m_users;
};
