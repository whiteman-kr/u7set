#ifndef DIALOGSIGNALSNAPSHOT_H
#define DIALOGSIGNALSNAPSHOT_H

#include "../lib/AppSignalManager.h"
#include "../lib/ExportPrint.h"
#include "../VFrame30/Schema.h"

class SignalSnapshotModel;

class SnapshotExportPrint : public ExportPrint
{
public:

	SnapshotExportPrint(QString projectName, QString softwareEquipmentId, QWidget* parent);

private:
	virtual void generateHeader(QTextCursor& cursor) override;

	QString m_projectName;
	QString m_softwareEquipmentId;
};

class SignalSnapshotSorter
{
public:
	  SignalSnapshotSorter(int column, SignalSnapshotModel* model);

	  bool operator()(int index1, int index2) const
	  {
		  return sortFunction(index1, index2);
	  }

	  bool sortFunction(int index1, int index2) const;

private:
	  int m_column = -1;

	  SignalSnapshotModel* m_model = nullptr;
};


enum class SnapshotColumns
{
	SignalID = 0,		// Signal Param Columns
	EquipmentID,
	AppSignalID,
	Caption,
	Type,
	Tags,

	SystemTime,			// Signal State Columns
	LocalTime,
	PlantTime,
	Value,
	Units,
	Valid,
	StateAvailable,
	Simulated,
	Blocked,
	Mismatch,
	OutOfLimits,

	ColumnCount
};

Q_DECLARE_METATYPE(SnapshotColumns);


class SignalSnapshotModel : public QAbstractItemModel
{
	Q_OBJECT

	friend class SignalSnapshotSorter;

public:

	enum class SignalType
	{
		All = 0,
		AnalogInput,
		AnalogOutput,
		DiscreteInput,
		DiscreteOutput
	};

	enum class MaskType
	{
        All = 0,
		AppSignalId,
		CustomAppSignalId,
		EquipmentId
	};

public:
	SignalSnapshotModel(IAppSignalManager* appSignalManager, QObject *parent);

	void setSignals(std::vector<AppSignalParam>& signalList);

public:
	// Properties

	QStringList columnsNames() const;

	// Overrides

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	// Operations

	void setSignalType(SignalType type);

	void setMasks(const QStringList& masks);

	void setTags(const QStringList& tags);

	void setSchemaAppSignals(std::set<QString> schemaAppSignals);

	void fillSignals();

	void updateStates(int from, int to);

	void sort(int column, Qt::SortOrder order) override;

	AppSignalParam signalParam(int rowIndex, bool* found);

	AppSignalState signalState(int rowIndex, bool* found);

protected:
	QModelIndex parent(const QModelIndex &index) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	IAppSignalManager* m_appSignalManager = nullptr;

	QStringList m_columnsNames;

	// Model data

	std::vector<AppSignalParam> m_allSignals;

	std::vector<AppSignalState> m_allStates;

	std::vector<int> m_filteredSignals;

	// Filtering parameters

	SignalType m_signalType = SignalType::All;

	QStringList m_masks;

	QStringList m_tags;

	std::set<QString> m_schemaAppSignals;
};

struct DialogSignalSnapshotSettings
{
	QPoint m_signalSnapshotPos;
	QByteArray m_signalSnapshotGeometry;
	QByteArray m_snapshotHorzHeader;
	int m_snapshotHorzHeaderCount = 0;	// Stores SnapshotColumns::ColumnCount constant to restore default settings if columns set changes
	SignalSnapshotModel::SignalType m_signalSnapshotSignalType = SignalSnapshotModel::SignalType::All;
	QStringList m_signalSnapshotMaskList;
	QStringList m_signalSnapshotTagsList;
	SignalSnapshotModel::MaskType m_signalSnapshotMaskType = SignalSnapshotModel::MaskType::AppSignalId;
	int m_signalSnapshotSortColumn = 0;
	Qt::SortOrder m_signalSnapshotSortOrder = Qt::AscendingOrder;

	void restore(QSettings& s);
	void store(QSettings& s);
};

class DialogSignalSnapshot : public QDialog
{
	Q_OBJECT

protected:
	explicit DialogSignalSnapshot(IAppSignalManager* appSignalManager,
								  QString projectName,
								  QString softwareEquipmentId,
								  QWidget *parent);
	virtual ~DialogSignalSnapshot();


public slots:
	void on_schemasUpdate();
	void on_signalsUpdate();		// Should be called when new signals arrived from AppDataService

protected:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() = 0;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) = 0;

	virtual void showEvent(QShowEvent* e) override;

signals:
	void signalContextMenu(const QStringList signalList);
	void signalInfo(QString appSignalId);

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

private slots:
	void on_DialogSignalSnapshot_finished(int result);
	void on_contextMenuRequested(const QPoint& pos);
	void on_tableView_doubleClicked(const QModelIndex &index);
	void on_sortIndicatorChanged(int column, Qt::SortOrder order);
	void on_typeCombo_currentIndexChanged(int index);
	void on_editMask_returnPressed();
	void on_editTags_returnPressed();
	void on_schemaCombo_currentIndexChanged(int index);
	void on_comboMaskType_currentIndexChanged(int index);
	void on_buttonExport_clicked();
	void on_buttonPrint_clicked();

private:
	void setupUi();

	void fillSchemas();

	void fillSignals();

	virtual void timerEvent(QTimerEvent* event) override;

	void maskChanged();

	void tagsChanged();

private:

	QComboBox* m_typeCombo = nullptr;
	QComboBox* m_schemaCombo = nullptr;
	QComboBox* m_comboMaskType = nullptr;

	QLineEdit* m_editMask = nullptr;
	QLineEdit* m_editTags = nullptr;

	QPushButton* m_buttonFixate = nullptr;

	QTableView* m_tableView = nullptr;

	//

	QString m_projectName;
	QString m_softwareEquipmentId;

	IAppSignalManager* m_appSignalManager = nullptr;

	SignalSnapshotModel *m_model = nullptr;

	int m_updateStateTimerId = -1;

	bool m_firstShow = true;

	QCompleter* m_maskCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	static const QString m_maskHelp;
	static const QString m_tagsHelp;
};

extern DialogSignalSnapshotSettings theDialogSignalSnapshotSettings;

#endif // DIALOGSIGNALSNAPSHOT_H
