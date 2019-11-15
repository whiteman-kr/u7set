#ifndef OBJECTPROPERTYDIALOG_H
#define OBJECTPROPERTYDIALOG_H

#include <QDebug>
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

#include "Options.h"
#include "SignalBase.h"

// Project property
//
// ==============================================================================================

const char* const				ProjectPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Project"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Host"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "File version"),
};

const int						PROJECT_PROPERTY_GROUP_COUNT			= sizeof(ProjectPropertyGroup)/sizeof(ProjectPropertyGroup[0]);

const int						PROJECT_PROPERTY_GROUP_INFO				= 0,
								PROJECT_PROPERTY_GROUP_HOST				= 1,
								PROJECT_PROPERTY_GROUP_VERSION			= 2;


// ----------------------------------------------------------------------------------------------

class ProjectPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit ProjectPropertyDialog(const ProjectInfo& info, QWidget *parent = nullptr);
	virtual ~ProjectPropertyDialog();

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

class RackPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	RackPropertyDialog(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget *parent = nullptr);
	virtual ~RackPropertyDialog();

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

public:

	Metrology::RackParam		rack() const { return m_rack; }

signals:

private slots:

	void						onPropertyValueChanged(QtProperty *property, const QVariant &value);

	void						onOk();
};


// Rack group property
//
// ==============================================================================================

const int						RACK_GROUP_COLUMN_CAPTION = 0;

// ----------------------------------------------------------------------------------------------

class RackGroupPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit RackGroupPropertyDialog(const RackBase& rackBase, QWidget *parent = nullptr);
	virtual ~RackGroupPropertyDialog();

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

	void						updateGroupList();

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

public:

	RackGroupBase&				rackGroups() { return m_groupBase; }

protected:

	bool						event(QEvent* e);

signals:

private slots:

	// slots of menu
	//
	void						appendGroup();
	void						removeGroup();

	// slots of property list
	//
	void						onPropertyValueChanged(QtProperty *property, const QVariant &value);

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
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Signal ID"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Position"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Electric range: "),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Engeneering range: "),
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
								SIGNAL_PROPERTY_ITEM_EL_RANGE_R0		= 6,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_PRECISION	= 7,

								SIGNAL_PROPERTY_ITEM_EN_RANGE_LOW		= 8,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_HIGH		= 9,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_UNIT		= 10,
								SIGNAL_PROPERTY_ITEM_EN_RANGE_PRECISION	= 11;

const int						SIGNAL_PROPERTY_ITEM_COUNT				= 12;

// ----------------------------------------------------------------------------------------------

class SignalPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalPropertyDialog(const Metrology::SignalParam& param, QWidget *parent = nullptr);
	virtual ~SignalPropertyDialog();

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

public:

	Metrology::SignalParam		param() const { return m_param; }

signals:

private slots:

	void						onPropertyValueChanged(QtProperty *property, const QVariant &value);
	void						onPropertyExpanded(QtBrowserItem *item);

	void						onOk();
};

// Comparator property
//
// ==============================================================================================

const char* const				ComparatorPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Signal ID"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Position"),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Electric range: "),
								QT_TRANSLATE_NOOP("ObjectPropertyDialog.h", "Engeneering range: "),
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

								COMPARATOR_PROPERTY_ITEM_HYST_EL_VALUE		= 3,
								COMPARATOR_PROPERTY_ITEM_HYST_EN_VALUE		= 4;

const int						COMPARATOR_PROPERTY_ITEM_COUNT				= 5;

// ----------------------------------------------------------------------------------------------

class ComparatorPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit ComparatorPropertyDialog(const Builder::Comparator& comparator, QWidget *parent = nullptr);
	virtual ~ComparatorPropertyDialog();

private:

	Builder::Comparator			m_comparator;

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

public:

	Builder::Comparator			comparator() const { return m_comparator; }

signals:

private slots:

	void						onPropertyValueChanged(QtProperty *property, const QVariant &value);
	void						onPropertyExpanded(QtBrowserItem *item);

	void						onOk();
};

// ==============================================================================================

#endif // OBJECTPROPERTYDIALOG_H
