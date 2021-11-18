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

#include "Options.h"

// ==============================================================================================

enum OptionGroup
{
	NoOptionGroup = -1,
	Server = 0,
	Module = 1,
	Linearity = 2,
	Comparator = 3,
	MeasureView = 4,
	PanelInfo = 5,
	Database = 6,
	Language = 7,
};

const int OptionGroupCount = 8;

#define ERR_OPTION_GROUP(group) (static_cast<int>(group) < 0 || static_cast<int>(group) >= OptionGroupCount)

QString groupCaption(int group);

// ==============================================================================================


enum OptionPage
{
	NoOptionPage = -1,
	Socket_Cfg = 0,
	Socket_AppDataSrv = 1,
	Socket_Tuning = 2,
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

const int OptionPageCount = 14;

#define ERR_OPTION_PAGE(page) (static_cast<int>(page) < 0 || static_cast<int>(page) >= OptionPageCount)

QString pageCaption(OptionPage page);
QString pageShortCaption(OptionPage page);
OptionGroup groupByPage(OptionPage page);

// ==============================================================================================

enum PropertyPageType
{
	NoPageType = -1,
	List = 0,
	Dialog = 1,
};

const int PropertyPageTypeCount = 2;

#define ERR_PROPERTY_PAGE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= PropertyPageTypeCount)

// ----------------------------------------------------------------------------------------------

class PropertyPage : public QObject
{
	Q_OBJECT

public:

	PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor);
	explicit PropertyPage(QDialog* dialog);
	virtual ~PropertyPage() override;

public:

	QWidget*					getWidget() { return m_pWidget; }
	PropertyPageType			type() const { return m_type; }

	OptionPage					m_page = OptionPage::NoOptionPage;
	QTreeWidgetItem*			m_pTreeWidgetItem = nullptr;

	QtTreePropertyBrowser*		treeEditor() { return m_pEditor; }

private:

	PropertyPageType			m_type = PropertyPageType::NoPageType;

	QWidget*					m_pWidget = nullptr;

	// PropertyPageType::List
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	// PropertyPageType::Dialog
	//
	QDialog*					m_pDialog = nullptr;
};

// ==============================================================================================

class DialogOptions : public QDialog
{
	Q_OBJECT

public:

	explicit DialogOptions(const Options& options, QWidget* parent = nullptr);
	virtual ~DialogOptions() override;

public:

	Options&					options() { return m_options; }

private:

	Options						m_options;

	static OptionPage			m_activePage;
	bool						setActivePage(OptionPage page);

	void						createInterface();

	QTreeWidget*				m_pPageTree = nullptr;
	QHBoxLayout*				m_pagesLayout = nullptr;
	QHBoxLayout*				m_buttonsLayout = nullptr;

	QHBoxLayout*				createPages();
	void						removePages();

	QHBoxLayout*				createButtons();

	std::vector<PropertyPage*>	m_pageList;

	PropertyPage*				createPage(OptionPage page);
	PropertyPage*				createPropertyList(OptionPage page);
	PropertyPage*				createPropertyDialog(OptionPage page);


	QMap<QtProperty*,int>		m_propertyItemList;
	QMap<QtProperty*,QVariant>	m_propertyValueList;

	void						appendProperty(QtProperty* property, OptionPage page, int param);
	void						expandProperty(QtTreePropertyBrowser* pEditor, OptionPage page, int param, bool expanded);
	void						clearProperty();

	QtProperty*					m_currentPropertyItem = nullptr;
	QVariant					m_currentPropertyValue = 0;

	void						restoreProperty();
	void						applyProperty();

	void						loadSettings();
	void						saveSettings();

protected:

	bool						event(QEvent* e) override;

private slots:

	void						onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void						onPropertyValueChanged(QtProperty* property, const QVariant &value);

	void						onBrowserItem(QtBrowserItem* pItem);

	void						updateServerPage();
	void						updateLinearityPage(bool isDialog);
	void						updateMeasureViewPage(bool isDialog);

	void						onOk();
};

// ==============================================================================================

#endif // DIALOGOPTIONS_H
