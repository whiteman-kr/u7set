#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMap>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

#include "../lib/PropertyEditor.h"
#include "../CommonLib/PropertyObject.h"

#include "Options.h"

// ==============================================================================================

enum PropertyGroupType
{
	NoGroupType = -1,
	Server = 0,
	Module = 1,
	Linearity = 2,
	Comparator = 3,
	MeasureView = 4,
	PanelInfo = 5,
	Database = 6,
	Language = 7,
};

const int PropertyGroupTypeCount = 8;

#define ERR_PROPERTY_GROUP_TYPE(groupType) (static_cast<int>(groupType) < 0 || static_cast<int>(groupType) >= PropertyGroupTypeCount)

QString groupCaption(PropertyGroupType groupType);

// ==============================================================================================


enum PropertyPageType
{
	NoPageType = -1,
	Socket_CfgSrv = 0,
	Socket_AppDataSrv = 1,
	Socket_TuningSrv = 2,
	Module_Measure = 3,
	Linearity_Measure = 4,
	Linearity_Point = 5,
	Comparator_Measure = 6,
	MeasureView_Text = 7,
	MeasureView_Column = 8,
	Panel_SignalInfo = 9,
	Panel_ComparatorInfo = 10,
	Database_Location = 11,
	Database_Backup = 12,
	Language_App = 13,
};

const int PropertyPageTypeCount = 14;

#define ERR_PROPERTY_PAGE_TYPE(pageType) (static_cast<int>(pageType) < 0 || static_cast<int>(pageType) >= PropertyPageTypeCount)

QString pageCaption(PropertyPageType pageType);
QString pageShortCaption(PropertyPageType pageType);
PropertyGroupType groupByPage(PropertyPageType pageType);

// ==============================================================================================

enum PropertyPageWidgetType
{
	NoWidgetType = -1,
	List = 0,
	Dialog = 1,
};

const int PropertyPageWidgetTypeCount = 2;

#define ERR_PROPERTY_PAGE_WIDGET_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= PropertyPageWidgetTypeCount)

// ----------------------------------------------------------------------------------------------

class PropertyPage : public PropertyObject
{
	Q_OBJECT

public:

	PropertyPage(PropertyPageType pageType, QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor);
	explicit PropertyPage(PropertyPageType pageType, QDialog* dialog);
	virtual ~PropertyPage() override;

public:

	QWidget* baseWidget() const { return m_baseWidget; }
	PropertyPageWidgetType widgetType() const { return m_widgetType; }

	PropertyPageType pageType() const { return m_pageType; }
	void setPageType(PropertyPageType pageType) { m_pageType = pageType; }

	QTreeWidgetItem* pageTreeItem() const { return m_pageTreeItem; }
	void setPageTreeItem(QTreeWidgetItem* pageTreeItem) { m_pageTreeItem = pageTreeItem; }

	QtTreePropertyBrowser* treeEditor() { return m_pEditor; }

private:

	QWidget* m_baseWidget = nullptr;
	PropertyPageWidgetType m_widgetType = PropertyPageWidgetType::NoWidgetType;

	PropertyPageType m_pageType = PropertyPageType::NoPageType;
	QTreeWidgetItem* m_pageTreeItem = nullptr;


	// PropertyPageWidgetType::List
	//
	QtVariantPropertyManager* m_pManager = nullptr;
	QtVariantEditorFactory* m_pFactory = nullptr;
	QtTreePropertyBrowser* m_pEditor = nullptr;

	// PropertyPageWidgetType::Dialog
	//
	QDialog* m_pDialog = nullptr;
};

// ==============================================================================================

class DialogOptions : public QDialog
{
	Q_OBJECT

public:

	explicit DialogOptions(const Options& options, QWidget* parent = nullptr);
	virtual ~DialogOptions() override;

public:

	Options& options() { return m_options; }

private:

	//
	//
	Options m_options;

	//
	//
	static PropertyPageType m_activePage;
	bool setActivePage(PropertyPageType pageType);

	//
	//
	void createInterface();

	QTreeWidget* m_pagesTree = nullptr;
	QHBoxLayout* m_pagesLayout = nullptr;

	ExtWidgets::PropertyEditor*	m_pPropertyEditor = nullptr;

	//
	//
	std::vector<std::shared_ptr<PropertyPage>> m_pagesList;

	void createPropertyPages();
	void removePropertyPages();

	//
	//
	std::shared_ptr<PropertyPage> createPropertyPage(PropertyPageType pageType);
	std::shared_ptr<PropertyPage> createPropertyPageList(PropertyPageType pageType);
	std::shared_ptr<PropertyPage> createPropertyPageDialog(PropertyPageType pageType);

	//
	//
	QMap<QtProperty*,int> m_propertyItemList;
	QMap<QtProperty*,QVariant> m_propertyValueList;

	void appendProperty(QtProperty* property, PropertyPageType pageType, int param);
	void expandProperty(QtTreePropertyBrowser* pEditor, PropertyPageType pageType, int param, bool expanded);
	void clearProperty();

	QtProperty* m_currentPropertyItem = nullptr;
	QVariant m_currentPropertyValue = 0;

	void restoreProperty();
	void applyProperty();

	//
	//
	void loadSettings();
	void saveSettings();

protected:

	bool event(QEvent* e) override;

private slots:

	//
	//
	void onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void onPropertyValueChanged(QtProperty* property, const QVariant &value);
	void onPropertyValueChanged_1(QList<std::shared_ptr<PropertyObject>> objects);

	//
	//
	void onBrowserItem(QtBrowserItem* pItem);

	//
	//
	void updateServerPage();
	void updateLinearityPage(bool isDialog);
	void updateMeasureViewPage(bool isDialog);

	//
	//
	void onOk();
};

// ==============================================================================================

#endif // DIALOGOPTIONS_H
