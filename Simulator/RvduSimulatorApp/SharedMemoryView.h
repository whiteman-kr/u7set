#pragma once

class QTableView;

enum class MemoryType
{
	Read,
	Write
};

class SharedMemoryModel : public QAbstractTableModel
{
public:
	SharedMemoryModel(int dataSize, MemoryType memoryType);
	~SharedMemoryModel();

	void updateData();

private:
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	MemoryType m_memoryType = MemoryType::Read;
	unsigned char* m_data = nullptr;
	unsigned int m_dataSize = 0;

	QStringList m_tableHeaders;
};

class SharedMemoryView : public QWidget
{
	Q_OBJECT
public:
	static SharedMemoryView* create(QWidget* parent = nullptr);

private:
	explicit SharedMemoryView(QWidget* parent = nullptr);
	~SharedMemoryView();

private:
	void setupUi();

	/* void fillSignalsList();
	void updateSignalValues();*/

private slots:
	void onTimer();
	//void onSignalClicked(unsigned int index);

private:
	SharedMemoryModel m_readModel;
	SharedMemoryModel m_writeModel;

	QTableView* m_readTableView = nullptr;
	QTableView* m_writeTableView = nullptr;

	static inline SharedMemoryView* m_instance = nullptr;
};