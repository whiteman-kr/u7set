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

#include "../lib/Hash.h"

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

#include "SignalBase.h"
#include "Options.h"

// Project property
//
// ==============================================================================================

const char* const				ProjectPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Project"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Host"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "File version"),
};

const int						PROJECT_PROPERTY_GROUP_COUNT			= sizeof(ProjectPropertyGroup)/sizeof(ProjectPropertyGroup[0]);

const int						PROJECT_PROPERTY_GROUP_INFO				= 0,
								PROJECT_PROPERTY_GROUP_HOST				= 1,
								PROJECT_PROPERTY_GROUP_VERSION			= 2;

// ----------------------------------------------------------------------------------------------

class DialogProjectProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogProjectProperty(const ProjectInfo& info, QWidget* parent = nullptr);
	virtual ~DialogProjectProperty();

private:

	ProjectInfo					m_info;

	// Property list
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	static bool					m_showGroupHeader[PROJECT_PROPERTY_GROUP_COUNT];
	QtBrowserItem*				m_browserItemList[PROJECT_PROPERTY_GROUP_COUNT];

	QMap<QtProperty*,int>		m_propertyMap;

	QtProperty*					m_propertyGroupList[PROJECT_PROPERTY_GROUP_COUNT];

	void						createPropertyList();
};

// Rack property
//
// ==============================================================================================

const int						RACK_PROPERTY_ITEM_ID		= 0,
								RACK_PROPERTY_ITEM_CAPTION	= 1,
								RACK_PROPERTY_ITEM_GROUP	= 2,
								RACK_PROPERTY_ITEM_CHANNEL	= 3;

const int						RACK_PROPERTY_ITEM_COUNT	= 4;

// ----------------------------------------------------------------------------------------------

class DialogRackProperty : public QDialog
{
	Q_OBJECT

public:

	DialogRackProperty(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget* parent = nullptr);
	virtual ~DialogRackProperty();

public:

	Metrology::RackParam		rack() const { return m_rack; }

private:

	Metrology::RackParam		m_rack;
	RackBase					m_rackBase;

	// Property list
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	// buttons
	//
	QDialogButtonBox*			m_buttonBox = nullptr;

	QMap<QtProperty*,int>		m_propertyMap;

	void						createPropertyList();

	bool						foundDuplicateGroups();

private slots:

	void						onPropertyValueChanged(QtProperty* property, const QVariant &value);

	void						onOk();
};

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
	virtual ~DialogRackGroupProperty();

public:

	RackBase&					racks() { return m_rackBase; }
	RackGroupBase&				rackGroups() { return m_groupBase; }

private:

	RackBase					m_rackBase;
	RackGroupBase				m_groupBase;

	//
	//
	QMenuBar*					m_pMenuBar = nullptr;
	QMenu*						m_pGroupMenu = nullptr;
	QMenu*						m_pContextMenu = nullptr;

	QAction*					m_pAppendGroupAction = nullptr;
	QAction*					m_pRemoveGroupAction = nullptr;

	// Group list
	//
	QTableWidget*				m_pGroupView = nullptr;

	void						updateGroupList(const Hash& hash = UNDEFINED_HASH);

	// Property list
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	void						createPropertyList();

	void						updateRackList();

	// buttons
	//
	QDialogButtonBox*			m_buttonBox = nullptr;

	QMap<QtProperty*,int>		m_propertyMap;

	bool						foundDuplicateRacks();

protected:

	bool						event(QEvent* e);

private slots:

	// slots of menu
	//
	void						appendGroup();
	void						removeGroup();

	// slots of property list
	//
	void						onPropertyValueChanged(QtProperty* property, const QVariant &value);

	// slot of view
	//
	void						onContextMenu(QPoint);
	void						captionGroupChanged(int row, int column);
	void						groupSelected();

	// slots of buttons
	//
	void						onOk();
};

// Signal property
//
// ==============================================================================================

const char* const				SignalPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Signal ID"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Position"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Electric range: "),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Engineering range: "),
};

const int						SIGNAL_PROPERTY_GROUP_COUNT				= sizeof(SignalPropertyGroup)/sizeof(SignalPropertyGroup[0]);

const int						SIGNAL_PROPERTY_GROUP_ID				= 0,
								SIGNAL_PROPERTY_GROUP_POSITION			= 1,
								SIGNAL_PROPERTY_GROUP_EL_RANGE			= 2,
								SIGNAL_PROPERTY_GROUP_EN_RANGE			= 3;

// ----------------------------------------------------------------------------------------------

const int						SIGNAL_PROPERTY_ITEM_CUSTOM_ID			= 0,
								SIGNAL_PROPERTY_ITEM_CAPTION			= 1,

								SIGNAL_PROPERTY_ITEM_EL_RANGE_LOW		= 2,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_HIGH		= 3,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_UNIT		= 4,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_SENSOR	= 5,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_RLOAD		= 6,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_R0		= 7,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_PRECISION	= 8,

								SIGNAL_PROPERTY_ITEM_EN_RANGE_LOW		= 9,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_HIGH		= 10,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_UNIT		= 11,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_PRECISION	= 12;

const int						SIGNAL_PROPERTY_ITEM_COUNT				= 13;

// ----------------------------------------------------------------------------------------------

class DialogSignalProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogSignalProperty(const Metrology::SignalParam& param, QWidget* parent = nullptr);
	virtual ~DialogSignalProperty();

public:

	Metrology::SignalParam		param() const { return m_param; }

private:

	Metrology::SignalParam		m_param;

	// Property list
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	// buttons
	//
	QDialogButtonBox*			m_buttonBox = nullptr;

	static bool					m_showGroupHeader[SIGNAL_PROPERTY_GROUP_COUNT];
	QtBrowserItem*				m_browserItemList[SIGNAL_PROPERTY_GROUP_COUNT];

	QMap<QtProperty*,int>		m_propertyMap;

	QtProperty*					m_propertyGroupList[SIGNAL_PROPERTY_GROUP_COUNT];

	void						createPropertyList();

	void						updateGroupHeader(int index);

private slots:

	void						onPropertyValueChanged(QtProperty* property, const QVariant &value);
	void						onPropertyExpanded(QtBrowserItem* item);

	void						onOk();
};

// Comparator property
//
// ==============================================================================================

const char* const				ComparatorPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Signal ID"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Position"),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Electric range: "),
								QT_TRANSLATE_NOOP("DialogObjectProperty", "Engineering range: "),
};

const int						COMPARATOR_PROPERTY_GROUP_COUNT				= sizeof(ComparatorPropertyGroup)/sizeof(ComparatorPropertyGroup[0]);

const int						COMPARATOR_PROPERTY_GROUP_INPUT				= 0,
								COMPARATOR_PROPERTY_GROUP_COMPARE			= 1,
								COMPARATOR_PROPERTY_GROUP_HYSTERESIS		= 2,
								COMPARATOR_PROPERTY_GROUP_OUTPUT			= 3;

// ----------------------------------------------------------------------------------------------

const int						COMPARATOR_PROPERTY_ITEM_CMP_TYPE			= 0,

								COMPARATOR_PROPERTY_ITEM_CMP_EL_VALUE		= 1,
								COMPARATOR_PROPERTY_ITEM_CMP_EN_VALUE		= 2,
								COMPARATOR_PROPERTY_ITEM_CMP_PRECESION		= 3,

								COMPARATOR_PROPERTY_ITEM_HYST_EL_VALUE		= 4,
								COMPARATOR_PROPERTY_ITEM_HYST_EN_VALUE		= 5;

const int						COMPARATOR_PROPERTY_ITEM_COUNT				= 6;

// ----------------------------------------------------------------------------------------------

class DialogComparatorProperty : public QDialog
{
	Q_OBJECT

public:

	explicit DialogComparatorProperty(const Metrology::ComparatorEx& comparator, QWidget* parent = nullptr);
	virtual ~DialogComparatorProperty();

public:

	Metrology::ComparatorEx		comparator() const { return m_comparatorEx; }

private:

	Metrology::ComparatorEx		m_comparatorEx;

	// Property list
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	// buttons
	//
	QDialogButtonBox*			m_buttonBox = nullptr;

	static bool					m_showGroupHeader[SIGNAL_PROPERTY_GROUP_COUNT];
	QtBrowserItem*				m_browserItemList[SIGNAL_PROPERTY_GROUP_COUNT];

	QMap<QtProperty*,int>		m_propertyMap;

	QtProperty*					m_propertyGroupList[SIGNAL_PROPERTY_GROUP_COUNT];

	void						createPropertyList();

private slots:

	void						onPropertyValueChanged(QtProperty* property, const QVariant &value);
	void						onPropertyExpanded(QtBrowserItem* item);

	void						onOk();
};

// ==============================================================================================

#endif // DIALOGOBJECTPROPERTY_H
