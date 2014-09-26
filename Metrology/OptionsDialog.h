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

#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include "Options.h"

// ==============================================================================================

const char* const OptionPage[] =
{
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

const int	OPTION_PAGE_COUNT				= sizeof(OptionPage)/sizeof(char*);

const int	OPTION_PAGE_UNKNOWN				= -1,
            OPTION_PAGE_LINEARETY_MEASURE	= 0,
            OPTION_PAGE_LINEARETY_POINT		= 1,
            OPTION_PAGE_SETTING_MEASURE		= 2,
            OPTION_PAGE_MEASURE_VIEW_TEXT	= 3,
            OPTION_PAGE_MEASURE_VIEW_COLUMN	= 4,
            OPTION_PAGE_SIGNAL_INFO			= 5,
            OPTION_PAGE_REPORT_HEADER		= 6,
            OPTION_PAGE_DATABASE			= 7,
            OPTION_PAGE_BACKUP				= 8;

const char* const OptionPageShort[OPTION_PAGE_COUNT] =
{
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

const char* const OptionGroup[] =
{
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Linearity"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Comparators"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "List of measurements"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Information of signal"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Reports"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Database"),
            QT_TRANSLATE_NOOP("OptionsDialog.h", "Backup measurements"),

};

const int	OPTION_GROUP_COUNT              = sizeof(OptionGroup)/sizeof(char*);

const int	OPTION_GROUP_UNKNOWN        = -1,
            OPTION_GROUP_LINEARETY      = 0,
            OPTION_GROUP_SETTING        = 1,
            OPTION_GROUP_MEASURE_VIEW   = 2,
            OPTION_GROUP_SIGNAL_INFO    = 3,
            OPTION_GROUP_REPORT_HEADER  = 4,
            OPTION_GROUP_DATABASE       = 5,
            OPTION_GROUP_BACKUP         = 6;


// ----------------------------------------------------------------------------------------------

const int OptionGroupPage[OPTION_PAGE_COUNT] =
{
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

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit OptionsDialog(QWidget *parent = 0);
    explicit OptionsDialog(const Options& options, QWidget *parent = 0);
            ~OptionsDialog();

    void                    setLinearity(const Options& options)    { m_options = options; }
    Options&                getLinearity()                          { return m_options; }

private:

    Options                 m_options;


    int                     m_activePage = OPTION_PAGE_UNKNOWN;
    bool                    setActivePage(int page);


    void                    createInterface();

    QHBoxLayout*            m_pagesLayout = nullptr;
    QHBoxLayout*            m_buttonsLayout = nullptr;

    QHBoxLayout*            createPages();
    QHBoxLayout*            createButtons();


    QList<QWidget*>         m_pageList;

    QWidget*                createPage(int page);
    QWidget*                createPropertyList(int page);
    QWidget*                createPropertyDialog(int page);


    QMap<QtProperty*,int>   m_propertyList;

    void                    appendProperty(QtProperty* property, int page, int param);

private slots:

    void                    onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void                    onPropertyChanged(QtProperty *property, const QVariant &value);

    void                    updateLinearityPage(bool isDialog);

    void                    onOk();
    void                    onApply();
};

// ==============================================================================================

#endif // OPTIONSDIALOG_H
