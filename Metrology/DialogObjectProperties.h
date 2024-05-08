#ifndef DIALOGOBJECTPROPERTY_H
#define DIALOGOBJECTPROPERTY_H

#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QMenuBar>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHeaderView>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QMessageBox>


#include <UiLib/PropertyEditor.h>

#include "MetrologySignal.h"
#include "DialogList.h"
#include "Options.h"

// ==============================================================================================
//
// Project property
//
// ==============================================================================================

enum ProjectPropertyCategory
{
	Info = 0,
	Host = 1,
	Version = 2,
};

const int ProjectPropertyCategoryCount = 3;

#define ERR_PROJECT_PROPERTY_CATEGORY(category) (static_cast<int>(category) < 0 || static_cast<int>(category) >= ProjectPropertyCategoryCount)

QString ProjectPropertyCategoryCaption(ProjectPropertyCategory category);

// ----------------------------------------------------------------------------------------------

class DialogProjectProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogProjectProperty(const ProjectInfo& info, QWidget* parent = nullptr);
	virtual ~DialogProjectProperty() override;

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(ProjectInfo* pObject);

	private:

		ProjectInfo* m_pObject = nullptr;
	};

	ProjectInfo m_info;
	ExtWidgets::PropertyEditor*	m_pPropertyEditor = nullptr;

	void createPropertyList();
};

// ==============================================================================================
//
// Rack property
//
// ==============================================================================================

class DialogRackProperty : public QDialog
{
	Q_OBJECT

public:

	DialogRackProperty(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget* parent = nullptr);
	virtual ~DialogRackProperty() override;

public:

	Metrology::RackParam rack() const { return m_rack; }

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(Metrology::RackParam* pObject, RackBase* pRackBase);

	private:

		Metrology::RackParam* m_pObject = nullptr;
	};

	Metrology::RackParam m_rack;
	RackBase m_rackBase;

	//
	//
	ExtWidgets::PropertyEditor* m_pPropertyEditor = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	void createPropertyList();

	bool foundDuplicateGroups();

private slots:

	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void onOk();
};

// ==============================================================================================
//
// Rack group property
//
// ==============================================================================================

const int RACK_GROUP_COLUMN_CAPTION = 0;

// ----------------------------------------------------------------------------------------------

class DialogRackGroupProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogRackGroupProperty(const RackBase& rackBase, QWidget* parent = nullptr);
	virtual ~DialogRackGroupProperty() override;

public:

	RackBase& racks() { return m_rackBase; }
	RackGroupBase& rackGroups() { return m_groupBase; }

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(RackBase* pObject);

	private:

		RackBase* m_pObject = nullptr;
	};

	RackBase m_rackBase;
	RackGroupBase m_groupBase;

	//
	//
	QMenuBar* m_pMenuBar = nullptr;
	QMenu* m_pGroupMenu = nullptr;
	QMenu* m_pContextMenu = nullptr;

	QAction* m_pAppendGroupAction = nullptr;
	QAction* m_pRemoveGroupAction = nullptr;


	//
	//
	QTableWidget* m_pGroupView = nullptr;
	ExtWidgets::PropertyEditor* m_pPropertyEditor = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	//
	//
	void createPropertyList();

	void updateGroupList(const Hash& hash = UNDEFINED_HASH);
	void updateRackList();

	bool foundDuplicateRacks();

protected:

	bool event(QEvent* e) override;

private slots:

	// slots of menu
	//
	void appendGroup();
	void removeGroup();

	// slots of property list
	//
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);

	// slot of view
	//
	void onContextMenu(QPoint);
	void captionGroupChanged(int row, int column);
	void groupSelected();

	// slots of buttons
	//
	void onOk();
};

// ==============================================================================================
//
// Signal property
//
// ==============================================================================================

const char* const				PrComparatorListColumn[] =
{
								QT_TRANSLATE_NOOP("DialogSignalProperty", "Compare to"),
								QT_TRANSLATE_NOOP("DialogSignalProperty", "Set point"),
								QT_TRANSLATE_NOOP("DialogSignalProperty", "SignalID (Discrete)"),
};

const int						PR_COMPARATOR_LIST_COLUMN_COUNT			= sizeof(PrComparatorListColumn)/sizeof(PrComparatorListColumn[0]);

const int						PR_COMPARATOR_LIST_COLUMN_CMP_TO		= 0,
								PR_COMPARATOR_LIST_COLUMN_SETPOINT		= 1,
								PR_COMPARATOR_LIST_COLUMN_OUTPUT		= 2;

const int						PrComparatorListColumnWidth[PR_COMPARATOR_LIST_COLUMN_COUNT] =
{
								250,	// PR_COMPARATOR_LIST_COLUMN_CMP_TO
								150,	// PR_COMPARATOR_LIST_COLUMN_SETPOINT
								250,	// PR_COMPARATOR_LIST_COLUMN_OUTPUT
};

// ==============================================================================================

class PrComparatorListTable : public ListTable<std::shared_ptr<Metrology::ComparatorEx>>
{
	Q_OBJECT

public:

	explicit PrComparatorListTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~PrComparatorListTable() override {}

public:

	QString text(int row, int column, std::shared_ptr<Metrology::ComparatorEx> comparatorEx) const;

private:

	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

enum SignalPropertyCategory
{
	SignalID = 0,
	SignalPosition = 1,
	ElectricLimit = 2,
	EngineeringLimit = 3,
};

const int SignalPropertyCategoryCount = 4;

#define ERR_SIGNAL_PROPERTY_CATEGORY(category) (static_cast<int>(category) < 0 || static_cast<int>(category) >= SignalPropertyCategoryCount)

QString SignalPropertyCategoryCaption(SignalPropertyCategory category);

// ----------------------------------------------------------------------------------------------

class DialogSignalProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogSignalProperty(const Metrology::SignalParam& param, QWidget* parent = nullptr);
	virtual ~DialogSignalProperty() override;

public:

	Metrology::SignalParam param() const { return m_param; }

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(Metrology::SignalParam* pObject);

	private:

		Metrology::SignalParam* m_pObject = nullptr;
	};

	Metrology::SignalParam m_param;

	//
	//
	QTabWidget* m_pTab = nullptr;

	//
	//
	ExtWidgets::PropertyEditor* m_pPropertyEditor = nullptr;

	//
	//
	QMenu* m_pContextMenu = nullptr;
	QAction* m_pCopyAction = nullptr;
	QAction* m_pCopyCellAction = nullptr;
	QAction* m_pComparatorPropertyAction = nullptr;

	QTableView* m_pComparatorView = nullptr;
	PrComparatorListTable m_comparatorTable;
	std::set<Hash> m_requestStateList;

	// buttons
	//
	QDialogButtonBox* m_buttonBox = nullptr;

	// timer
	//
	QTimer* m_updateComparatorStateTimer = nullptr;

	void createContextMenu();
	void createPropertyList();

protected:

	void closeEvent(QCloseEvent* e) override;

private slots:

	// slots of property editor
	//
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);

	// slots of menu
	//
	void onContextMenu(QPoint);
	void onCopy();
	void onCopyCell();
	void onComparatorProperty();

	// slots of timer
	//
	void updateComparatorState();

	// slots of dialog
	//
	void onOk();
	void onCancel();
};

// ==============================================================================================
//
// Comparator property
//
// ==============================================================================================

enum ComparatorPropertyCategory
{
	Schema = 0,
	Input = 1,
	Comapre = 2,
	Hysteresis	= 3,
	Output = 4,
};

const int ComparatorPropertyCategoryCount = 5;

#define ERR_COMPARATOR_PROPERTY_CATEGORY(category) (static_cast<int>(category) < 0 || static_cast<int>(category) >= ComparatorPropertyCategoryCount)

QString ComparatorPropertyCategoryCaption(ComparatorPropertyCategory category);

// ----------------------------------------------------------------------------------------------

class DialogComparatorProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogComparatorProperty(const Metrology::ComparatorEx& comparator, QWidget* parent = nullptr);
	virtual ~DialogComparatorProperty() override;

public:

	Metrology::ComparatorEx comparator() const { return m_comparatorEx; }

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(Metrology::ComparatorEx* pObject);

		QString comapreTo();
		double electricConstValue();

	private:

		Metrology::ComparatorEx* m_pObject = nullptr;
	};

	Metrology::ComparatorEx m_comparatorEx;

	//
	//
	ExtWidgets::PropertyEditor* m_pPropertyEditor = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	void createPropertyList();

private slots:

	// slots of property editor
	//
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);

	void onOk();
};

// ==============================================================================================
//
// Measurement property
//
// ==============================================================================================

enum MeasurePropertyCategory
{
	MeasureID = 0,
	MeasurePosition = 1,
	Limits = 2,
	Errors = 3,
};

const int MeasurePropertyCategoryCount = 4;

#define ERR_MEASURE_PROPERTY_CATEGORY(category) (static_cast<int>(category) < 0 || static_cast<int>(category) >= MeasurePropertyCategoryCount)

QString MeasurePropertyCategoryCaption(MeasurePropertyCategory category);

// ----------------------------------------------------------------------------------------------

class DialogMeasureProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogMeasureProperty(Measure::Item* pMeasurement, QWidget* parent = nullptr);
	virtual ~DialogMeasureProperty() override;

private:

	class PropertyPattern: public PropertyObject
	{
	public:

		explicit PropertyPattern(Measure::Item* pObject);

	private:

		Measure::Item* m_pObject = nullptr;

		QString engineeringLimitStr();
		QString electricLimitStr();

		double errorLimit();
		void setErrorLimit(double value);
	};

	Measure::Item* m_pMeasurement = nullptr;

	//
	//
	ExtWidgets::PropertyEditor*	m_pPropertyEditor = nullptr;
	QDialogButtonBox* m_buttonBox = nullptr;

	//
	//
	void createPropertyList();

private slots:

	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);
};

// ==============================================================================================

#endif // DIALOGOBJECTPROPERTY_H
