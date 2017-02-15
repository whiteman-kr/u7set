#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QMap>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "../qtpropertybrowser/src/qtpropertymanager.h"
#include "../qtpropertybrowser/src/qtvariantproperty.h"
#include "../qtpropertybrowser/src/qttreepropertybrowser.h"

#include "Options.h"

// ==============================================================================================

const char* const               OptionGroupTitle[] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connect to server"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Linearity"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Comparators"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "List of measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Panel signal information"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Reports"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Database"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Backup measurements"),
};

const int                       OPTION_GROUP_COUNT          = sizeof(OptionGroupTitle)/sizeof(OptionGroupTitle[0]);

const int                       OPTION_GROUP_UNKNOWN        = -1,
                                OPTION_GROUP_SERVER         = 0,
                                OPTION_GROUP_LINEARITY      = 1,
                                OPTION_GROUP_SETTING        = 2,
                                OPTION_GROUP_MEASURE_VIEW   = 3,
                                OPTION_GROUP_SIGNAL_INFO    = 4,
                                OPTION_GROUP_REPORT_HEADER  = 5,
                                OPTION_GROUP_DATABASE       = 6,
                                OPTION_GROUP_BACKUP         = 7;

// ==============================================================================================

const char* const               OptionPageTitle[] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Config Server - TCP/IP"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Application Data Server - TCP/IP"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to Tuning Server - TCP/IP"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements of linearity"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Point of linearity"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measure comparators"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying data in the list of measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying columns in the list of measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying information of signal"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Reports of metrological inspection"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Database settings"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Backup measurements"),

};

const int                       OPTION_PAGE_COUNT				= sizeof(OptionPageTitle)/sizeof(OptionPageTitle[0]);

const int                       OPTION_PAGE_UNKNOWN				= -1,
                                OPTION_PAGE_CONFIG_SOCKET       = 0,
                                OPTION_PAGE_SIGNAL_SOCKET       = 1,
                                OPTION_PAGE_TUNING_SOCKET       = 2,
                                OPTION_PAGE_LINEARITY_MEASURE	= 3,
                                OPTION_PAGE_LINEARITY_POINT		= 4,
                                OPTION_PAGE_COMPARATOR_MEASURE	= 5,
                                OPTION_PAGE_MEASURE_VIEW_TEXT	= 6,
                                OPTION_PAGE_MEASURE_VIEW_COLUMN	= 7,
                                OPTION_PAGE_SIGNAL_INFO			= 8,
                                OPTION_PAGE_REPORT              = 9,
                                OPTION_PAGE_DATABASE			= 10,
                                OPTION_PAGE_BACKUP				= 11;

// ----------------------------------------------------------------------------------------------

const char* const               OptionPageShortTitle[OPTION_PAGE_COUNT] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "ConfigService"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "AppDataService"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "TuningService"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Points"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Columns"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Settings"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Settings"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Settings"),
};

// ----------------------------------------------------------------------------------------------

const int                       OptionGroupPage[OPTION_PAGE_COUNT] =
{
                                OPTION_GROUP_SERVER,        // Group: Connect to server --      Page : ConfigService"),
                                OPTION_GROUP_SERVER,        // Group: Connect to server --      Page : AppDataService"),
                                OPTION_GROUP_SERVER,        // Group: Connect to server --      Page : TuningService"),
                                OPTION_GROUP_LINEARITY,		// Group: Linearity --              Page : Measurements"),
                                OPTION_GROUP_LINEARITY,		// Group: Linearity --              Page : Points"),
                                OPTION_GROUP_SETTING,		// Group: Comparators --            Page : Measurements"),
                                OPTION_GROUP_MEASURE_VIEW,	// Group: List of measurements --   Page : Display"),
                                OPTION_GROUP_MEASURE_VIEW,	// Group: List of measurements --   Page : Columns"),
                                OPTION_GROUP_SIGNAL_INFO,	// Group: Information of signal --  Page : Displaying"),
                                OPTION_GROUP_REPORT_HEADER,	// Group: Reports --                Page : Settings"),
                                OPTION_GROUP_DATABASE,		// Group: Database --               Page : Settings"),
                                OPTION_GROUP_BACKUP,		// Group: Backup measurements --    Page : Settings"),
};

// ==============================================================================================

const int                       PROPERTY_PAGE_TYPE_UNKNOWN  = -1,
                                PROPERTY_PAGE_TYPE_LIST     = 0,
                                PROPERTY_PAGE_TYPE_DIALOG   = 1;

const int                       PROPERTY_PAGE_TYPE_COUNT    = 2;


// ----------------------------------------------------------------------------------------------

class PropertyPage : public QObject
{
    Q_OBJECT

public:

                                PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor);
    explicit                    PropertyPage(QDialog* dialog);
                                ~PropertyPage();

    QWidget*                    getWidget() { return m_pWidget; }
    int                         type() const { return m_type; }

    int                         m_page = OPTION_PAGE_UNKNOWN;
    QTreeWidgetItem*            m_pTreeWidgetItem = nullptr;

    QtTreePropertyBrowser*      treeEditor() { return m_pEditor; }

private:

    int                         m_type = PROPERTY_PAGE_TYPE_UNKNOWN;

    QWidget*                    m_pWidget = nullptr;

    // PROPERTY_PAGE_TYPE_LIST
    //
    QtVariantPropertyManager*   m_pManager = nullptr;
    QtVariantEditorFactory*     m_pFactory = nullptr;
    QtTreePropertyBrowser*      m_pEditor = nullptr;

    // PROPERTY_PAGE_TYPE_DIALOG
    //
    QDialog*                    m_pDialog = nullptr;
};

// ==============================================================================================

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit                    OptionsDialog(QWidget *parent = 0);
                                ~OptionsDialog();

private:

    Options                     m_options;

    static int                  m_activePage;
    bool                        setActivePage(int page);

    void                        createInterface();

    QTreeWidget*                m_pPageTree = nullptr;
    QHBoxLayout*                m_pagesLayout = nullptr;
    QHBoxLayout*                m_buttonsLayout = nullptr;

    QHBoxLayout*                createPages();
    void                        removePages();

    QHBoxLayout*                createButtons();

    QList<PropertyPage*>        m_pageList;

    PropertyPage*               createPage(int page);
    PropertyPage*               createPropertyList(int page);
    PropertyPage*               createPropertyDialog(int page);


    QMap<QtProperty*,int>       m_propertyItemList;
    QMap<QtProperty*,QVariant>  m_propertyValueList;

    void                        appendProperty(QtProperty* property, int page, int param);
    void                        expandProperty(QtTreePropertyBrowser* pEditor, int page, int param, bool expanded);
    void                        clearProperty();

    QtProperty*                 m_currentPropertyItem = nullptr;
    QVariant                    m_currentPropertyValue = 0;

    void                        restoreProperty();
    void                        applyProperty();

    void                        loadSettings();
    void                        saveSettings();

protected:

    bool                        event(QEvent * e);

private slots:

    void                        onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void                        onPropertyValueChanged(QtProperty *property, const QVariant &value);

    void                        onBrowserItem(QtBrowserItem*pItem);

    void                        updateLinearityPage(bool isDialog);
    void                        updateMeasureViewPage(bool isDialog);
    void                        updateReportHeaderPage();

    void                        onOk();
    void                        onApply();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
