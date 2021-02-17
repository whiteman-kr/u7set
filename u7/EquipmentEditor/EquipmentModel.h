#pragma once

class DbController;
class DbFileInfo;

namespace Hardware
{
	class DeviceObject;
}

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

	void sortDeviceObject(Hardware::DeviceObject* object, int column, Qt::SortOrder order);
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

	Hardware::DeviceObject* deviceObject(QModelIndex& index);
	const Hardware::DeviceObject* deviceObject(const QModelIndex& index) const;

	std::shared_ptr<Hardware::DeviceObject> deviceObjectSharedPtr(QModelIndex& index);

	QString usernameById(int userId) const;

	void reset();

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
	DbController* dbController();
	DbController* dbController() const;

	bool isPresetMode() const;
	bool isConfigurationMode() const;

public:
	enum Columns
	{
		ObjectNameColumn,
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
	QWidget* m_parentWidget;

	std::shared_ptr<Hardware::DeviceObject> m_root;
	std::shared_ptr<Hardware::DeviceObject> m_configuration;
	std::shared_ptr<Hardware::DeviceObject> m_preset;

	int m_sortColumn = ObjectPlaceColumn ;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	std::map<int, QString> m_users;
};
