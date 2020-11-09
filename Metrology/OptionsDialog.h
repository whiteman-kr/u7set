#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

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

const char* const				OptionGroupTitle[] =
{
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Connect to server"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Module"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Linearity"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Comparators"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "List of measurements"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Panels information"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Database"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Language"),
};

const int						OPTION_GROUP_COUNT			= sizeof(OptionGroupTitle)/sizeof(OptionGroupTitle[0]);

const int						OPTION_GROUP_UNDEFINED		= -1,
								OPTION_GROUP_SERVER			= 0,
								OPTION_GROUP_MODULE			= 1,
								OPTION_GROUP_LINEARITY		= 2,
								OPTION_GROUP_COMPARATOR		= 3,
								OPTION_GROUP_MEASURE_VIEW	= 4,
								OPTION_GROUP_PANEL_INFO		= 5,
								OPTION_GROUP_DATABASE		= 6,
								OPTION_GROUP_LANGUAGE		= 7;

// ==============================================================================================

const char* const				OptionPageTitle[] =
{
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Config Server - TCP/IP"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Application Data Server - TCP/IP"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Tuning Server - TCP/IP"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measuring of module"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements of linearity"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Point of linearity"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measure comparators"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying data in the list of measurements"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying columns in the list of measurements"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying information of signals"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying information of сomparators"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Database location"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Database backup"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Language of application"),
};

const int						OPTION_PAGE_COUNT				= sizeof(OptionPageTitle)/sizeof(OptionPageTitle[0]);

const int						OPTION_PAGE_UNDEFINED			= -1,
								OPTION_PAGE_CONFIG_SOCKET		= 0,
								OPTION_PAGE_SIGNAL_SOCKET		= 1,
								OPTION_PAGE_TUNING_SOCKET		= 2,
								OPTION_PAGE_MODULE_MEASURE		= 3,
								OPTION_PAGE_LINEARITY_MEASURE	= 4,
								OPTION_PAGE_LINEARITY_POINT		= 5,
								OPTION_PAGE_COMPARATOR_MEASURE	= 6,
								OPTION_PAGE_MEASURE_VIEW_TEXT	= 7,
								OPTION_PAGE_MEASURE_VIEW_COLUMN	= 8,
								OPTION_PAGE_SIGNAL_INFO			= 9,
								OPTION_PAGE_COMPARATOR_INFO		= 10,
								OPTION_PAGE_DATABASE_LOCATION	= 11,
								OPTION_PAGE_DATABASE_BACKUP		= 12,
								OPTION_PAGE_LANGUAGE			= 13;

// ----------------------------------------------------------------------------------------------

const char* const				OptionPageShortTitle[OPTION_PAGE_COUNT] =
{
								QT_TRANSLATE_NOOP("OptionsDialog.h", "ConfigurationService"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "AppDataService"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "TuningService"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measuring"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Points"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Columns"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Signal information"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Comparator information"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Location"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Backup"),
								QT_TRANSLATE_NOOP("OptionsDialog.h", "Language"),
};

// ----------------------------------------------------------------------------------------------

const int						OptionGroupPage[OPTION_PAGE_COUNT] =
{
								OPTION_GROUP_SERVER,		// Group: Connect to server --			Page : ConfigService"),
								OPTION_GROUP_SERVER,		// Group: Connect to server --			Page : AppDataService"),
								OPTION_GROUP_SERVER,		// Group: Connect to server --			Page : TuningService"),
								OPTION_GROUP_MODULE,		// Group: Module --						Page : Measuring"),
								OPTION_GROUP_LINEARITY,		// Group: Linearity --					Page : Measurements"),
								OPTION_GROUP_LINEARITY,		// Group: Linearity --					Page : Points"),
								OPTION_GROUP_COMPARATOR,	// Group: Comparators --				Page : Measurements"),
								OPTION_GROUP_MEASURE_VIEW,	// Group: List of measurements --		Page : Display"),
								OPTION_GROUP_MEASURE_VIEW,	// Group: List of measurements --		Page : Columns"),
								OPTION_GROUP_PANEL_INFO,	// Group: Information of signal --		Page : Displaying"),
								OPTION_GROUP_PANEL_INFO,	// Group: Information of comparator --	Page : Displaying"),
								OPTION_GROUP_DATABASE,		// Group: Database --					Page : Location"),
								OPTION_GROUP_DATABASE,		// Group: Database --					Page : Backup"),
								OPTION_GROUP_LANGUAGE,		// Group: Language of application --	Page : Language"),
};

// ==============================================================================================

const int						PROPERTY_PAGE_TYPE_UNDEFINED	= -1,
								PROPERTY_PAGE_TYPE_LIST			= 0,
								PROPERTY_PAGE_TYPE_DIALOG		= 1;

const int						PROPERTY_PAGE_TYPE_COUNT		= 2;

// ----------------------------------------------------------------------------------------------

class PropertyPage : public QObject
{
	Q_OBJECT

public:

	PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor);
	explicit PropertyPage(QDialog* dialog);
	virtual ~PropertyPage();

private:

	int							m_type = PROPERTY_PAGE_TYPE_UNDEFINED;

	QWidget*					m_pWidget = nullptr;

	// PROPERTY_PAGE_TYPE_LIST
	//
	QtVariantPropertyManager*	m_pManager = nullptr;
	QtVariantEditorFactory*		m_pFactory = nullptr;
	QtTreePropertyBrowser*		m_pEditor = nullptr;

	// PROPERTY_PAGE_TYPE_DIALOG
	//
	QDialog*					m_pDialog = nullptr;

public:

	QWidget*					getWidget() { return m_pWidget; }
	int							type() const { return m_type; }

	int							m_page = OPTION_PAGE_UNDEFINED;
	QTreeWidgetItem*			m_pTreeWidgetItem = nullptr;

	QtTreePropertyBrowser*		treeEditor() { return m_pEditor; }
};

// ==============================================================================================

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionsDialog(const Options& options, QWidget *parent = nullptr);
	virtual ~OptionsDialog();

private:

	Options						m_options;

	static int					m_activePage;
	bool						setActivePage(int page);

	void						createInterface();

	QTreeWidget*				m_pPageTree = nullptr;
	QHBoxLayout*				m_pagesLayout = nullptr;
	QHBoxLayout*				m_buttonsLayout = nullptr;

	QHBoxLayout*				createPages();
	void						removePages();

	QHBoxLayout*				createButtons();

	QVector<PropertyPage*>		m_pageList;

	PropertyPage*				createPage(int page);
	PropertyPage*				createPropertyList(int page);
	PropertyPage*				createPropertyDialog(int page);


	QMap<QtProperty*,int>		m_propertyItemList;
	QMap<QtProperty*,QVariant>	m_propertyValueList;

	void						appendProperty(QtProperty* property, int page, int param);
	void						expandProperty(QtTreePropertyBrowser* pEditor, int page, int param, bool expanded);
	void						clearProperty();

	QtProperty*					m_currentPropertyItem = nullptr;
	QVariant					m_currentPropertyValue = 0;

	void						restoreProperty();
	void						applyProperty();

	void						loadSettings();
	void						saveSettings();

public:

	Options&					options() { return m_options; }

protected:

	bool						event(QEvent * e);

private slots:

	void						onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void						onPropertyValueChanged(QtProperty *property, const QVariant &value);

	void						onBrowserItem(QtBrowserItem*pItem);

	void						updateServerPage();
	void						updateLinearityPage(bool isDialog);
	void						updateMeasureViewPage(bool isDialog);

	void						onOk();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
