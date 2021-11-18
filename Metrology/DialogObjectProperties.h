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


#include "../lib/PropertyEditor.h"
#include "../CommonLib/Hash.h"

#include "MetrologySignal.h"
#include "DialogList.h"
#include "Options.h"

// ==============================================================================================
//
// Project property
//
// ==============================================================================================

const char* const				ProjectPropertyCategory[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "1 Project"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "2 Host"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "3 File version"),
};

const int						PROJECT_PROPERTY_CATEGORY_COUNT			= sizeof(ProjectPropertyCategory)/sizeof(ProjectPropertyCategory[0]);

const int						PROJECT_PROPERTY_CATEGORY_INFO			= 0,
								PROJECT_PROPERTY_CATEGORY_HOST			= 1,
								PROJECT_PROPERTY_CATEGORY_VERSION		= 2;

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

const int						RACK_GROUP_COLUMN_CAPTION = 0;

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

const char* const				SignalPropertyCategory[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "1 Signal ID"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "2 Position"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "3 Electric range"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "4 Engineering range"),
};

const int						SIGNAL_PROPERTY_CATEGORY_COUNT				= sizeof(SignalPropertyCategory)/sizeof(SignalPropertyCategory[0]);

const int						SIGNAL_PROPERTY_CATEGORY_SIGNAL_ID			= 0,
								SIGNAL_PROPERTY_CATEGORY_POSITION			= 1,
								SIGNAL_PROPERTY_CATEGORY_EL_RANGE			= 2,
								SIGNAL_PROPERTY_CATEGORY_EN_RANGE			= 3;

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

	//
	//
	static bool m_showGroupHeader[SIGNAL_PROPERTY_CATEGORY_COUNT];
	//QtBrowserItem* m_browserItemList[SIGNAL_PROPERTY_CATEGORY_COUNT];
	//QtProperty* m_propertyGroupList[SIGNAL_PROPERTY_CATEGORY_COUNT];

	void createContextMenu();
	void createPropertyList();

protected:

	void closeEvent(QCloseEvent* e) override;

private slots:

	// slots of property editor
	//
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);
	//void onPropertyExpanded(QtBrowserItem* item);

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

const char* const				ComparatorPropertyCategory[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "1 Schema"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "2 Input"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "3 Compare"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "4 Hysteresis"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "5 Output"),
};

const int						COMPARATOR_PROPERTY_CATEGORY_COUNT			= sizeof(ComparatorPropertyCategory)/sizeof(ComparatorPropertyCategory[0]);

const int						COMPARATOR_PROPERTY_CATEGORY_SCHEMA			= 0,
								COMPARATOR_PROPERTY_CATEGORY_INPUT			= 1,
								COMPARATOR_PROPERTY_CATEGORY_COMPARE		= 2,
								COMPARATOR_PROPERTY_CATEGORY_HYSTERESIS		= 3,
								COMPARATOR_PROPERTY_CATEGORY_OUTPUT			= 4;

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

	static bool m_showGroupHeader[COMPARATOR_PROPERTY_CATEGORY_COUNT];
	//QtBrowserItem* m_browserItemList[COMPARATOR_PROPERTY_CATEGORY_COUNT];
	//QtProperty* m_propertyGroupList[COMPARATOR_PROPERTY_CATEGORY_COUNT];

	void createPropertyList();

private slots:

	// slots of property editor
	//
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);
	//void onPropertyExpanded(QtBrowserItem* item);

	void onOk();
};

// ==============================================================================================
//
// Measurement property
//
// ==============================================================================================


const char* const				MeasurePropertyCategory[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "1 Signal ID"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "2 Position"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "3 Limits"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "4 Errors"),
};

const int						MEASURE_PROPERTY_CATEGORY_COUNT			= sizeof(MeasurePropertyCategory)/sizeof(MeasurePropertyCategory[0]);

const int						MEASURE_PROPERTY_CATEGORY_SIGNAL_ID		= 0,
								MEASURE_PROPERTY_CATEGORY_POSITION		= 1,
								MEASURE_PROPERTY_CATEGORY_LIMITS		= 2,
								MEASURE_PROPERTY_CATEGORY_ERRORS		= 3;

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
