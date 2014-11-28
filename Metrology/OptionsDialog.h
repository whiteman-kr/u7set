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

const char* const               OptionPage[] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connection to server TCP/IP"),
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

const int                       OPTION_PAGE_COUNT				= sizeof(OptionPage)/sizeof(char*);

const int                       OPTION_PAGE_UNKNOWN				= -1,
                                OPTION_PAGE_TCP_IP              = 0,
                                OPTION_PAGE_LINEARETY_MEASURE	= 1,
                                OPTION_PAGE_LINEARETY_POINT		= 2,
                                OPTION_PAGE_SETTING_MEASURE		= 3,
                                OPTION_PAGE_MEASURE_VIEW_TEXT	= 4,
                                OPTION_PAGE_MEASURE_VIEW_COLUMN	= 5,
                                OPTION_PAGE_SIGNAL_INFO			= 6,
                                OPTION_PAGE_REPORT_HEADER		= 7,
                                OPTION_PAGE_DATABASE			= 8,
                                OPTION_PAGE_BACKUP				= 9;

const char* const               OptionPageShort[OPTION_PAGE_COUNT] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "TCP/IP"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Points"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Columns"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Displaying"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Reports"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Settings"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Copy measurements"),
};

// ==============================================================================================

const char* const               OptionGroup[] =
{
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Connect to server"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Linearity"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Comparators"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "List of measurements"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Information of signal"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Reports"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Database"),
                                QT_TRANSLATE_NOOP("OptionsDialog.h", "Backup measurements"),

};

const int                       OPTION_GROUP_COUNT              = sizeof(OptionGroup)/sizeof(char*);

const int                       OPTION_GROUP_UNKNOWN        = -1,
                                OPTION_GROUP_TCP_IP         = 0,
                                OPTION_GROUP_LINEARETY      = 1,
                                OPTION_GROUP_SETTING        = 2,
                                OPTION_GROUP_MEASURE_VIEW   = 3,
                                OPTION_GROUP_SIGNAL_INFO    = 4,
                                OPTION_GROUP_REPORT_HEADER  = 5,
                                OPTION_GROUP_DATABASE       = 6,
                                OPTION_GROUP_BACKUP         = 7;


// ----------------------------------------------------------------------------------------------

const int                       OptionGroupPage[OPTION_PAGE_COUNT] =
{
                                OPTION_GROUP_TCP_IP,        //("Linearity -- Measurements"),
                                OPTION_GROUP_LINEARETY,		//("Linearity -- Measurements"),
                                OPTION_GROUP_LINEARETY,		//("Linearity -- Points"),
                                OPTION_GROUP_SETTING,		//("Comparators -- Measurements"),
                                OPTION_GROUP_MEASURE_VIEW,	//("List of measurements -- Display"),
                                OPTION_GROUP_MEASURE_VIEW,	//("List of measurements -- Columns"),
                                OPTION_GROUP_SIGNAL_INFO,	//("Information of signal -- Displaying"),
                                OPTION_GROUP_REPORT_HEADER,	//("Reports"),
                                OPTION_GROUP_DATABASE,		//("Database"),
                                OPTION_GROUP_BACKUP,		//("Backup measurements"),
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

    explicit                    PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor);
    explicit                    PropertyPage(QDialog* dialog);
                                ~PropertyPage();

    QWidget*                    getWidget() { return m_pWidget; }
    int                         getType()   { return m_type; }

    int                         m_page = OPTION_PAGE_UNKNOWN;
    QTreeWidgetItem*            m_pTreeWidgetItem = nullptr;

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

    void                        onBrowserItem(QtBrowserItem*);

    void                        updateLinearityPage(bool isDialog);
    void                        updateMeasureViewPage(bool isDialog);

    void                        onOk();
    void                        onApply();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
