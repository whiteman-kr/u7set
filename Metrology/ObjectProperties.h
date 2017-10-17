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
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "../lib/Hash.h"

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

#include "SignalBase.h"

// ==============================================================================================

const char* const				SignalPropertyGroup[] =
{
								QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Signal ID"),
								QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Position"),
								QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Physical range: "),
								QT_TRANSLATE_NOOP("SignalPropertyDialog.h", "Electric range: "),
};

const int						SIGNAL_PROPERTY_GROUP_COUNT				= sizeof(SignalPropertyGroup)/sizeof(SignalPropertyGroup[0]);

const int						SIGNAL_PROPERTY_GROUP_ID				= 0,
								SIGNAL_PROPERTY_GROUP_POSITION			= 1,
								SIGNAL_PROPERTY_GROUP_PH_RANGE			= 2,
								SIGNAL_PROPERTY_GROUP_EL_RANGE			= 3;

// ----------------------------------------------------------------------------------------------

const int						SIGNAL_PROPERTY_ITEM_CUSTOM_ID			= 0,
								SIGNAL_PROPERTY_ITEM_CAPTION			= 1,
								SIGNAL_PROPERTY_ITEM_PH_RANGE_LOW		= 2,
								SIGNAL_PROPERTY_ITEM_PH_RANGE_HIGH		= 3,
								SIGNAL_PROPERTY_ITEM_PH_RANGE_UNIT		= 4,
								SIGNAL_PROPERTY_ITEM_PH_RANGE_PRECISION	= 5,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_LOW		= 6,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_HIGH		= 7,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_UNIT		= 8,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_SENSOR	= 9,
								SIGNAL_PROPERTY_ITEM_EL_RANGE_PRECISION	= 10;

const int						SIGNAL_PROPERTY_ITEM_COUNT				= 11;

// ----------------------------------------------------------------------------------------------

class SignalPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SignalPropertyDialog(const Metrology::SignalParam& param, QWidget *parent = 0);
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

	RackPropertyDialog(const Metrology::RackParam& rack, const RackBase& rackBase, QWidget *parent = 0);
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

// ==============================================================================================

const int						RACK_GROUP_COLUMN_CAPTION = 0;

// ----------------------------------------------------------------------------------------------

class RackGroupPropertyDialog : public QDialog
{
	Q_OBJECT

public:

	explicit RackGroupPropertyDialog(const RackBase& rackBase, QWidget *parent = 0);
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

// ==============================================================================================

#endif // OBJECTPROPERTYDIALOG_H
