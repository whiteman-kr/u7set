#ifndef DIALOGOPTIONS_H
#define DIALOGOPTIONS_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMap>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "../lib/PropertyEditor.h"

#include "Options.h"

// ==============================================================================================

enum PropertyGroupType
{
	NoGroupType = -1,
	Service = 0,
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
	Service_Connection = 0,
	Module_Measure = 1,
	Linearity_Measure = 2,
	Linearity_Point = 3,
	Comparator_Measure = 4,
	MeasureView_Text = 5,
	MeasureView_Column = 6,
	Panel_SignalInfo = 7,
	Panel_ComparatorInfo = 8,
	Database_Location = 9,
	Database_Backup = 10,
	Language_App = 11,
};
Q_DECLARE_METATYPE(PropertyPageType)

const int PropertyPageTypeCount = 12;

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
Q_DECLARE_METATYPE(PropertyPageWidgetType)

const int PropertyPageWidgetTypeCount = 2;

#define ERR_PROPERTY_PAGE_WIDGET_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= PropertyPageWidgetTypeCount)

// ----------------------------------------------------------------------------------------------

class PropertyPage : public PropertyObject
{
	Q_OBJECT

public:

	PropertyPage(Options* options, PropertyPageType pageType, ExtWidgets::PropertyEditor* pPropertyEditor);
	explicit PropertyPage(Options* options, PropertyPageType pageType);
	virtual ~PropertyPage() override;

public:

	QWidget* baseWidget() const { return m_baseWidget; }
	PropertyPageWidgetType widgetType() const { return m_widgetType; }

	PropertyPageType pageType() const { return m_pageType; }
	void setPageType(PropertyPageType pageType) { m_pageType = pageType; }

	QTreeWidgetItem* pageTreeItem() const { return m_pageTreeItem; }
	void setPageTreeItem(QTreeWidgetItem* pageTreeItem) { m_pageTreeItem = pageTreeItem; }

private:

	void clear();

	Options* m_options = nullptr;

	QWidget* m_baseWidget = nullptr;
	PropertyPageWidgetType m_widgetType = PropertyPageWidgetType::NoWidgetType;

	PropertyPageType m_pageType = PropertyPageType::NoPageType;
	QTreeWidgetItem* m_pageTreeItem = nullptr;


signals:

	void dataUpdated(PropertyPageType pageType);

private slots:

	void dialogDataUpdated();
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
	QTreeWidget* m_pagesTree = nullptr;
	QHBoxLayout* m_pagesLayout = nullptr;
	ExtWidgets::PropertyEditor*	m_pPropertyEditor = nullptr;

	void createInterface();

	//
	//
	std::vector<std::shared_ptr<PropertyPage>> m_pagesList;

	void createPropertyPages();
	void removePropertyPages();

	std::shared_ptr<PropertyPage> createPropertyPage(PropertyPageType pageType);

	//
	//
	static PropertyPageType m_currentPage;

	bool pageTypeIsValid(PropertyPageType pageType);
	bool setCurrentPage(PropertyPageType pageType);
	bool hidePage(PropertyPageType pageType);
	bool showPage(PropertyPageType pageType);

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
	void onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects);

	//
	//
	void dialogDataUpdated(PropertyPageType pageType);	// load data from dialog to options

	//
	//
	void onOk();
};

// ==============================================================================================

#endif // DIALOGOPTIONS_H
