#pragma once

#include "../lib/ExportPrint.h"
#include "../lib/ISignalDataServer.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../VFrame30/SchemaDetails.h"
#include "DragDropHelper.h"
#include "../AppSignalLib/IAppSignalManager.h"

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
	LmEquipmentID,
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
		Any = 0,
		Analog,
		Discrete,
		Count
	};

	enum class SignalRole
	{
		Any = 0,
		Input,
		Output,
		Internal,
		Tunable,
		Count
	};

	enum class MaskType
	{
        All = 0,
		AppSignalId,
		CustomAppSignalId,
		EquipmentId,
		LmEquipmentId,
		Count
	};

public:
	SignalSnapshotModel(IAppSignalManager* appSignalManager, ISignalDataServer* signalDataServer, QObject *parent);

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

	void setSignalRole(SignalRole role);

	void setMaskType(SignalSnapshotModel::MaskType type);

	void setMasks(const QStringList& masks);

	void setTags(const QStringList& tags);

	void setDataServiceId(const QString& dataServiceId);

	void setSchemaAppSignals(std::set<QString> schemaAppSignals);

	void fillSignals();

	void updateStates(int from, int to);

	void sort(int column, Qt::SortOrder order) override;

	AppSignalParam signalParam(int rowIndex, bool* found);

	AppSignalState signalState(int rowIndex, bool* found);

	E::AnalogFormat analogFormat() const;
	void setAnalogFormat(E::AnalogFormat format);

	int analogPrecision() const;
	void setAnalogPrecision(int precision);

protected:
	QModelIndex parent(const QModelIndex &index) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	IAppSignalManager* m_appSignalManager = nullptr;
	ISignalDataServer* m_signalDataServer = nullptr;

	QStringList m_columnsNames;

	// Model data

	std::vector<AppSignalParam> m_allSignals;
	std::vector<AppSignalState> m_allStates;
	std::vector<int> m_filteredSignals;

	// Filtering parameters

	SignalType m_signalType = SignalType::Any;
	SignalRole m_signalRole = SignalRole::Any;
	MaskType m_maskType = MaskType::CustomAppSignalId;
	QStringList m_masks;
	QStringList m_tags;
	QString m_dataServiceId;
	std::set<QString> m_schemaAppSignals;

	// View params

	E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;
	int m_analogPrecision = -1;
};

struct DialogSignalSnapshotSettings
{
	QPoint pos;
	QByteArray geometry;

	QByteArray horzHeader;
	int horzHeaderCount = 0;	// Stores SnapshotColumns::ColumnCount constant to restore default settings if columns set changes

	//bool typeSetAutomatically = false;
	//SignalSnapshotModel::SignalType signalType = SignalSnapshotModel::SignalType::All;

	//bool maskSetAutomatically = false;
	QStringList maskList;
	//SignalSnapshotModel::MaskType maskType = SignalSnapshotModel::MaskType::AppSignalId;

	//bool tagsSetAutomatically = false;
	//QStringList tagsList;

	int sortColumn = 0;
	Qt::SortOrder sortOrder = Qt::AscendingOrder;

	void restore();
	void store();
};

class SnapshotTableView : public QTableView
{
public:
	SnapshotTableView();

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	AppSignalParam m_appSignalParam;
	QPoint m_dragStartPosition;

	DragDropHelper m_dragDropHelper;
};

class DialogSignalSnapshot : public QDialog
{
	Q_OBJECT

protected:
	DialogSignalSnapshot(IAppSignalManager* appSignalManager,
						 ISignalDataServer* signalDataServer,	// Can be nullptr, e.g. in Simulator
						 const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,	// Can be empty, e.g. in Simulator
						 const QString& projectName,
						 const QString& equipmentId,
						 QWidget *parent);

	DialogSignalSnapshot(IAppSignalManager* appSignalManager,
						 const QString& projectName,
						 const QString& equipmentId,
						 QWidget *parent);

	virtual ~DialogSignalSnapshot();

	QString projectName() const;
	void setProjectName(const QString& projectName);

public:
	const std::vector<AppSignalParam>& specificSignals() const;
	void setSpecificSignals(const std::vector<AppSignalParam>& specificSignals);

	void setLmEquipmentId(const QString& lmEquipmentId);
	void setSignalsMask(const QStringList& masks);
	void setSignalsTags(const QStringList& tags);
	void resetSignalsType();

public slots:
	void schemasUpdated();
	void signalsUpdated();		// Should be called when new signals arrived from AppDataService

protected:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() = 0;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) = 0;

	virtual void showEvent(QShowEvent* e) override;
	virtual void keyPressEvent(QKeyEvent *event) override;

signals:
	void signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
	void signalInfo(QString appSignalId);

protected slots:
	void headerColumnContextMenuRequested(const QPoint& pos);
	void headerColumnToggled(bool checked);

private slots:
	void dialogFinished(int result);
	void contextMenuRequested(const QPoint& pos);
	void tableViewdoubleClicked(const QModelIndex &index);
	void sortIndicatorChanged(int column, Qt::SortOrder order);
	void typeComboCurrentIndexChanged(int index);
	void roleComboCurrentIndexChanged(int index);
	void editMaskReturnPressed();
	void editTagsReturnPressed();
	void schemaComboCurrentIndexChanged(int index);
	void maskTypeComboCurrentIndexChanged(int index);
	void serverComboIndexChanged(int index);
	void buttonExportClicked();
	void buttonPrintClicked();
    void buttonChooseTagsClicked();
	void buttonClearFilterClicked();

private:
	void createControls();
	void createMenus();
	void initFiltersView();
	void initSignalsView();

	void fillSchemas();
	void fillSignals();

	void timerEvent(QTimerEvent* event) override;
	void updateTableItems();

	void maskChanged(bool addToCompleter);
	void tagsChanged();

private:
	IAppSignalManager* m_appSignalManager = nullptr;
	ISignalDataServer* m_signalDataServer = nullptr;

	// Ui
	QComboBox* m_typeCombo = nullptr;
	QComboBox* m_roleCombo = nullptr;
	QComboBox* m_schemaCombo = nullptr;
	QComboBox* m_maskTypeCombo = nullptr;
	QComboBox* m_serverCombo = nullptr;

	QLineEdit* m_editMask = nullptr;
	QLineEdit* m_editTags = nullptr;
    QToolButton* m_buttonChooseTags = nullptr;

	QPushButton* m_buttonFixate = nullptr;

	SnapshotTableView* m_tableView = nullptr;
	SignalSnapshotModel* m_model = nullptr;

	QAction* m_formatAutoSelect = nullptr;
	QAction* m_formatDecimal = nullptr;
	QAction* m_formatExponential = nullptr;

	QAction* m_precisionDefault = nullptr;
	QList<QAction*> m_precisionActions;

	QCompleter* m_maskCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	QPushButton* m_clearFilterButton = nullptr;

	QMenu m_formatMenu;

	// Project Data
	QString m_projectName;
	QString m_equipmentId;
	std::vector<SoftwareEndpoint::AppDataService> m_appDataServices;

	std::vector<AppSignalParam> m_specificSignals;

	int m_updateStateTimerId = -1;

	bool m_firstShow = true;

	QString m_maskHelp;
	QString m_tagsHelp;

	DialogSignalSnapshotSettings m_settings;

	bool m_storeType = true;
	bool m_storeRole = true;
	bool m_storeMaskData = true;
	bool m_storeTags = true;
	static inline SignalSnapshotModel::SignalType m_storedType{SignalSnapshotModel::SignalType::Any};
	static inline SignalSnapshotModel::SignalRole m_storedRole{SignalSnapshotModel::SignalRole::Any};
	static inline SignalSnapshotModel::MaskType m_storedMaskType{SignalSnapshotModel::MaskType::All};
	static inline QStringList m_storedTags;
};

